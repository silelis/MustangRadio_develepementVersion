#include "i2c_engine_slave.h"
#include "assert.h"

static i2c_slave_dev_handle_t handler_i2c_dev_slave;
static i2c_slave_config_t i2c_config_slave;

#include "soc/i2c_struct.h"
extern i2c_dev_t I2C0;

#include "driver/i2c.h"
#include "esp_rom_sys.h"
#include <new>	// std::nothrow

// Callback i maszyna stanów TX czytają rejestry I2C0 bezpośrednio - działa tylko, gdy slave jest na porcie 0
static_assert(I2C_SLAVE_PORT == 0, "i2c_engine_slave.cpp czyta rejestry I2C0 - przy zmianie I2C_SLAVE_PORT trzeba zmienić I2C0 na I2C1");

// Spinlock chroniący i2cTxSlaveState przy sprawdzaniu i zmianie stanu z ISR (callback) i z tasku i2cSlaveTransmit.
// ESP32 jest dwurdzeniowy - samo volatile nie wystarcza, potrzebna sekcja krytyczna (sprawdź-i-zmień musi być atomowe).
static portMUX_TYPE s_i2cTxStateMux = portMUX_INITIALIZER_UNLOCKED;

// Maksymalny czas czekania na przeniesienie danych z ring buffera do TX FIFO przez ISR sterownika (zwykle kilka us)
#define I2C_TX_FIFO_LOAD_TIMEOUT_US		1000
#define I2C_TX_FIFO_LOAD_POLL_US		10

// Długość, na jaką uzbrajany jest odbiór w i2c_slave_receive() - NIE WIĘCEJ niż sprzętowe FIFO (32 B).
// Błąd sterownika ESP-IDF 5.2 (i2c_slave.c, s_i2c_handle_complete): po STOP czyta t->rcv_fifo_cnt bajtów
// (reszta z receive_size, a nie liczba bajtów w FIFO) do data_buf[SOC_I2C_FIFO_LEN] - przy receive_size 52
// przepełnia data_buf i nadpisuje trans_data_length / callbacki sterownika -> Interrupt WDT / Double exception
// przy następnym i2c_slave_transmit(). Przy receive_size <= 32 przepełnienie jest niemożliwe.
// I2C_SLAVE_DRIVER_RECEIVE_LEN i ESP32_SLAVE_RECEIVE_BUFFER_LEN - w comunicationProtocol.h (wspólny ze STM32, tam liczba 32)
static_assert(ESP32_SLAVE_RECEIVE_BUFFER_LEN == SOC_I2C_FIFO_LEN, "ESP32_SLAVE_RECEIVE_BUFFER_LEN (comunicationProtocol.h) musi być równe SOC_I2C_FIFO_LEN - obejście błędu sterownika IDF 5.2");
static_assert(I2C_SLAVE_DRIVER_RECEIVE_LEN <= SOC_I2C_FIFO_LEN, "odbiór I2C slave dłuższy niż FIFO - przepełnienie data_buf w sterowniku IDF 5.2");
// ramki STM32 -> ESP32 muszą zmieścić się w jednym odbiorze
static_assert(sizeof(i2cFrame_hmiLeds) <= I2C_SLAVE_DRIVER_RECEIVE_LEN, "ramka LED dłuższa niż odbiór I2C slave");
static_assert(sizeof(i2cFrame_stepper) <= I2C_SLAVE_DRIVER_RECEIVE_LEN, "ramka silnika dłuższa niż odbiór I2C slave");
static_assert(sizeof(i2cFrame_btAudio) <= I2C_SLAVE_DRIVER_RECEIVE_LEN, "ramka BT dłuższa niż odbiór I2C slave");
//----------------------------------------------
// Enumy i zmienne globalne (z volatile dla ISR)
//----------------------------------------------
typedef enum
{
	recpeptionNotToMe,		// 0 - dane nie były do ESP32
	recpeptionToMe,			// 1 - ESP32 odebrało dane
	transmitionNotFromMe,	// 2 - STOP po obcej transakcji (slave_rw==1 zostało z poprzedniego odczytu ESP32)
	transmitionFromMe		// 3 - ESP32 wysłało dane (przesunął się tx_fifo_start_addr)
} i2cCallbackState;

// dane współdzielone przez ISR i2c_slave_rx_done_callback i task i2cSlaveReceive
typedef struct
{
	i2cCallbackState rxToEsp32;			// klasyfikacja ostatniego zdarzenia I2C
	uint32_t rx_fifo_end_addrLast;		// ostatni wskaźnik zapisu RX FIFO - wykrywanie odbioru do ESP32
	uint32_t tx_fifo_start_addrLast;	// ostatni wskaźnik odczytu TX FIFO - wykrywanie wysyłki z ESP32
	// DIAGNOSTYKA: migawka rejestrów I2C0 z chwili callbacku, drukowana w tasku i2cSlaveReceive
	uint8_t diag_slave_rw;
	uint8_t diag_slave_addressed;
	uint8_t diag_rx_fifo_end_addr;
	uint8_t diag_tx_fifo_start_addrPrev;
	uint8_t diag_tx_fifo_start_addr;
	uint8_t diag_tx_fifo_end_addr;
	uint8_t diag_tx_fifo_cnt;			// liczba bajtów w TX FIFO w chwili callbacku (0 przy odczycie z ESP32)
} i2cCallbackData;

// element kolejki s_receive_queue: jedno zdarzenie I2C z kopią danych z chwili callbacku
typedef struct
{
	i2cCallbackData cbData;							// klasyfikacja i DIAGNOSTYKA tego zdarzenia
	uint8_t data[ESP32_SLAVE_RECEIVE_BUFFER_LEN];	// kopia bufora odbiorczego (edata->buffer) - task nie czyta bufora sterownika
} i2cSlaveRxEvent;
// bieżące zdarzenie budowane w ISR i kopiowane do kolejki; cbData.*_Last - pamięć ISR między callbackami
static i2cSlaveRxEvent rxEvent = {{recpeptionNotToMe, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0}};

//----------------------------------------------
// Callback dla zdarzeń I2C Slave
//----------------------------------------------
/*---------------------------------------------------------------
 * KIEDY WYWOŁYWANY:
 * Sterownik IDF 5.2 (i2c_slave.c, s_i2c_handle_complete) wywołuje
 * callback z przerwania trans_complete, czyli przy KAŻDYM warunku
 * STOP na magistrali - także w transakcjach STM32 z innymi układami
 * (ESP32 jako slave "słyszy" całą magistralę). Po callbacku
 * sterownik WYŁĄCZA przerwanie trans_complete; ponownie włącza je
 * dopiero i2c_slave_receive() w tasku i2cSlaveReceive. Zdarzenie,
 * które wypadnie w tej przerwie, nie wywoła callbacku (patrz
 * zabezpieczenie w tasku niżej).
 *
 * ZAPIS STM32 -> ESP32 (slave_rw == 0):
 * Odbiór do ESP32 rozpoznawany po przesunięciu wskaźnika zapisu
 * RX FIFO (rx_fifo_end_addr). Obcy zapis nie zmienia RX FIFO ESP32.
 *
 * ODCZYT ESP32 PRZEZ STM32 (slave_rw == 1) - wykrywanie odczytu:
 * Rejestr slave_rw NIE zmienia się przy obcej transakcji (zostaje
 * wartość z ostatniej transakcji z ESP32), więc slave_rw == 1 nie
 * dowodzi, że ESP32 był czytany. Rejestr tx_fifo_start_addr na
 * ESP32 NIE przesuwa się przy odczycie przez mastera (sprawdzone
 * pomiarem: tx_fifo_start 0->0 mimo poprawnego odczytu 4 B przez
 * STM32), więc nie nadaje się do wykrywania odczytu.
 * Odczyt z ESP32 = spełnione JEDNOCZEŚNIE (i2cTxStateNextFromISR):
 *  1. slave_rw == 1                   - ostatnia transakcja to odczyt,
 *  2. i2cTxSlaveState == lenDataInTransmition lub
 *     packageDataInTransmition         - ESP32 czeka na odczyt,
 *  3. status_reg.tx_fifo_cnt == 0     - master opróżnił TX FIFO ESP32.
 * Warunek 3 jest właściwym dowodem: TX FIFO ESP32 opróżnia się TYLKO,
 * gdy master zaadresuje ESP32 i odczyta bajty. Aby był wiarygodny,
 * task i2cSlaveTransmit ustawia stan ...InTransmition dopiero PO
 * załadowaniu FIFO (i2cTxWaitForFifoLoaded: tx_fifo_cnt > 0) -
 * wcześniej FIFO jest puste z powodu "jeszcze nie załadowane", nie
 * "odczytane", a stan to nie ...InTransmition, więc STOP jest ignorowany.
 *
 * OBCE TRANSAKCJE STM32 (z innym układem na tej samej magistrali):
 *  - ESP32 w idle / lenDataTransmited / packageDataTransmited:
 *    warunek 2 niespełniony -> ignorowane,
 *  - ESP32 w ...InTransmition (np. STM32 odczytał len, potem
 *    rozmawia z innym układem, a dopiero potem czyta ramkę):
 *    FIFO ESP32 pełne (tx_fifo_cnt > 0) - warunek 3 niespełniony
 *    -> ignorowane, dane czekają w FIFO na odczyt przez STM32.
 * Jedyne ograniczenie: STM32 musi odczytać każdą fazę w czasie
 * tx_timeout_ms (ESP32_I2C_TX_TIMEOUT_MS), inaczej timeout.
 *
 * ZABEZPIECZENIE (callback przepadł):
 * Task i2cSlaveTransmit w stanach ...InTransmition co tick sam
 * sprawdza warunki 2 i 3 oraz status_reg.bus_busy == 0
 * (i2cTxStateNextIfReadDone). Stan przesuwa ten, kto pierwszy
 * stwierdzi odczyt; sprawdzenie i zmiana stanu są w sekcji
 * krytycznej s_i2cTxStateMux (ISR i task na różnych rdzeniach).
 *
 * Porty: callback czyta rejestry I2C0 - slave musi być na porcie 0
 * (static_assert na początku pliku). ESP32 jako master pracuje na
 * innym kontrolerze (I2C_MASTER_PORT = 1, rejestry I2C1) i nie wpływa
 * na rejestry I2C0.
 *---------------------------------------------------------------*/
IRAM_ATTR bool i2cEngin_slave::i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel,
														  const i2c_slave_rx_done_event_data_t *edata,
														  void *user_data)
{
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;

	// DIAGNOSTYKA: zapis rejestrów zanim się zmienią
	rxEvent.cbData.diag_slave_rw = I2C0.status_reg.slave_rw;
	rxEvent.cbData.diag_slave_addressed = I2C0.status_reg.slave_addressed;
	rxEvent.cbData.diag_rx_fifo_end_addr = I2C0.fifo_st.rx_fifo_end_addr;
	rxEvent.cbData.diag_tx_fifo_start_addrPrev = rxEvent.cbData.tx_fifo_start_addrLast;
	rxEvent.cbData.diag_tx_fifo_start_addr = I2C0.fifo_st.tx_fifo_start_addr;
	rxEvent.cbData.diag_tx_fifo_end_addr = I2C0.fifo_st.tx_fifo_end_addr;
	rxEvent.cbData.diag_tx_fifo_cnt = I2C0.status_reg.tx_fifo_cnt;

	// 1. Sprawdź kierunek transmisji
	switch ((i2c_slave_read_write_status_t)I2C0.status_reg.slave_rw)
	{
	case I2C_SLAVE_WRITE_BY_MASTER:
		// Master -> Slave (ESP32 odbiera)
		if (rxEvent.cbData.rx_fifo_end_addrLast != I2C0.fifo_st.rx_fifo_end_addr)
		{
			rxEvent.cbData.rxToEsp32 = recpeptionToMe; // Nowe dane dla ESP32
		}
		else
		{
			rxEvent.cbData.rxToEsp32 = recpeptionNotToMe; // Dane były dla innego slave'a
		}
		rxEvent.cbData.rx_fifo_end_addrLast = I2C0.fifo_st.rx_fifo_end_addr;
		break;
	case I2C_SLAVE_READ_BY_MASTER:
		// Slave -> Master (ESP32 wysyła) albo STOP po obcej transakcji (slave_rw nie zmienia się bez adresowania ESP32)
		// [BUG] tx_fifo_start_addr na ESP32 nie przesuwa się przy odczycie przez mastera (pomiar: 0->0) - porównanie nigdy nie wykrywało odczytu
		// if (rxEvent.cbData.tx_fifo_start_addrLast != I2C0.fifo_st.tx_fifo_start_addr)
		// [FIX] odczyt wykrywany po tx_fifo_cnt == 0 przy stanie ...InTransmition (opis mechanizmu nad callbackiem)
		if ((sInstance != nullptr) && sInstance->i2cTxStateNextFromISR())
		{
			rxEvent.cbData.rxToEsp32 = transmitionFromMe;	// master opróżnił TX FIFO ESP32, na które ESP32 czekało
		}
		else
		{
			rxEvent.cbData.rxToEsp32 = transmitionNotFromMe;	// obca transakcja albo ESP32 nie czekało na odczyt
		}
		rxEvent.cbData.tx_fifo_start_addrLast = I2C0.fifo_st.tx_fifo_start_addr;	// DIAGNOSTYKA (wartość tylko informacyjna)
		break;
	}

	// 2. Wyślij zdarzenie do kolejki (z zabezpieczeniem przed NULL)
	if (receive_queue != NULL && edata != NULL)
	{
		memcpy(rxEvent.data, edata->buffer, ESP32_SLAVE_RECEIVE_BUFFER_LEN);	// kopia ramki - ramka kompletna, sterownik nie pisze teraz do bufora
		BaseType_t queueSendResult = xQueueSendFromISR(receive_queue, &rxEvent, &high_task_wakeup);
		assert(queueSendResult == pdTRUE);	// pełna kolejka = utrata zdarzenia i brak ponownego uzbrojenia odbiornika - błąd krytyczny
	}

	// Sterownik IDF 5.2 (s_i2c_handle_complete) IGNORUJE wartość zwracaną przez callback - bez tego task "I2C slave rx"
	// budzi się dopiero przy następnym ticku (do 10 ms), a do tego czasu odbiór jest nieuzbrojony (KONTEKST 3.5).
	// Na ESP32 portYIELD_FROM_ISR() ustawia tylko flagę - przełączenie zadania nastąpi przy wyjściu z przerwania.
	if (high_task_wakeup == pdTRUE)
	{
		portYIELD_FROM_ISR();
	}
	return high_task_wakeup == pdTRUE;
}

/*---------------------------------------------------------------
 * Metoda wywoływana z ISR i2c_slave_rx_done_callback przy STOP po
 * odczycie (slave_rw == 1). Sprawdza, czy to był odczyt z ESP32
 * (stan ...InTransmition i tx_fifo_cnt == 0 - opis mechanizmu nad
 * callbackiem) i jeśli tak, przesuwa maszynę stanów i2cSlaveTransmit
 * do kolejnego stanu.
 * Parameters:
 * NONE
 * Returns:
 * bool						- true: odczyt z ESP32, stan przesunięty
 *							  false: obca transakcja / ESP32 nie czekało
 *---------------------------------------------------------------*/
IRAM_ATTR bool i2cEngin_slave::i2cTxStateNextFromISR(void)
{
	bool readDone = false;
	portENTER_CRITICAL_ISR(&s_i2cTxStateMux);
	if (I2C0.status_reg.tx_fifo_cnt == 0)	// master opróżnił TX FIFO ESP32
	{
		switch (this->i2cTxSlaveState)
		{
		case i2cTransmitionState::lenDataInTransmition:
			this->i2cTxSlaveState = i2cTransmitionState::lenDataTransmited;
			readDone = true;
			break;
		case i2cTransmitionState::packageDataInTransmition:
			this->i2cTxSlaveState = i2cTransmitionState::packageDataTransmited;
			readDone = true;
			break;
		default:	// ESP32 nie czeka na odczyt - STOP z obcej transakcji
			break;
		}
	}
	portEXIT_CRITICAL_ISR(&s_i2cTxStateMux);
	return readDone;
}

/*---------------------------------------------------------------
 * Zabezpieczenie na wypadek, gdy callback od odczytu przepadł
 * (przerwanie trans_complete wyłączone przez sterownik do czasu
 * i2c_slave_receive() w tasku i2cSlaveReceive). Wywoływana co tick
 * z tasku i2cSlaveTransmit w stanach ...InTransmition. Warunki jak
 * w i2cTxStateNextFromISR, dodatkowo bus_busy == 0 (master skończył
 * transakcję - przy ramkach > 32 B FIFO może chwilowo być puste
 * w trakcie odczytu, zanim ISR sterownika je uzupełni).
 * Parameters:
 * NONE
 * Returns:
 * bool						- true: odczyt z ESP32, stan przesunięty
 *---------------------------------------------------------------*/
bool i2cEngin_slave::i2cTxStateNextIfReadDone(void)
{
	bool readDone = false;
	portENTER_CRITICAL(&s_i2cTxStateMux);
	if ((I2C0.status_reg.tx_fifo_cnt == 0) && (I2C0.status_reg.bus_busy == 0))
	{
		switch (this->i2cTxSlaveState)
		{
		case i2cTransmitionState::lenDataInTransmition:
			this->i2cTxSlaveState = i2cTransmitionState::lenDataTransmited;
			readDone = true;
			break;
		case i2cTransmitionState::packageDataInTransmition:
			this->i2cTxSlaveState = i2cTransmitionState::packageDataTransmited;
			readDone = true;
			break;
		default:
			break;
		}
	}
	portEXIT_CRITICAL(&s_i2cTxStateMux);
	return readDone;
}

/*---------------------------------------------------------------
 * Czeka, aż ISR sterownika przeniesie dane z ring buffera do
 * TX FIFO (tx_fifo_cnt > 0). Wywoływać po i2c_slave_transmit(),
 * PRZED ustawieniem stanu ...InTransmition i impulsem INT - dzięki
 * temu tx_fifo_cnt == 0 w stanie ...InTransmition oznacza
 * "odczytane przez mastera", a nie "jeszcze nie załadowane".
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t				- ESP_OK: FIFO załadowane
 *							  ESP_ERR_TIMEOUT: FIFO puste po
 *							  I2C_TX_FIFO_LOAD_TIMEOUT_US
 *---------------------------------------------------------------*/
esp_err_t i2cEngin_slave::i2cTxWaitForFifoLoaded(void)
{
	for (uint32_t waited_us = 0; waited_us < I2C_TX_FIFO_LOAD_TIMEOUT_US; waited_us += I2C_TX_FIFO_LOAD_POLL_US)
	{
		if (I2C0.status_reg.tx_fifo_cnt > 0)
		{
			return ESP_OK;
		}
		esp_rom_delay_us(I2C_TX_FIFO_LOAD_POLL_US);
	}
	return (I2C0.status_reg.tx_fifo_cnt > 0) ? ESP_OK : ESP_ERR_TIMEOUT;
}

/*---------------------------------------------------------------
 * Konstruktor klasy odpwiadającej za komunikację ESP32 po i2c z
 * urządzeniami i2c master.
 * Parameters:
 * i2c_port_num_t i2c_port	- numer portu i2c w kontrolerze ESP32
 * gpio_num_t sda_io_num	- numer pinu kontrolera ESP32 do którego
 *							  przypisano sygnał SDA szyny i2c
 * gpio_num_t scl_io_num	- numer pinu kontrolera ESP32 do którego
 *							  przypisano sygnał SCL szyny i2c
 uint32_t slave_addr		- adres esp32 jako slave na magistrali
							  i2c
 i2c_addr_bit_len_t slave_addr_bit_len - długość (w bitach) adresu
							  i2c slave
 gpio_num_t intRequestPin   - numer GPIO w esp32, który odpowiada
								za zgłoszenie do slave konieczności
								komunikacji po i2c
 * Returns:
 * NONE
*---------------------------------------------------------------*/
i2cEngin_slave::i2cEngin_slave(i2c_port_num_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num, uint32_t slave_addr, i2c_addr_bit_len_t slave_addr_bit_len, gpio_num_t intRequestPin)
{
	sInstance = this;	// musi być przed i2c_slave_register_event_callbacks, żeby ISR nie widział nullptr
	i2c_config_slave.addr_bit_len = slave_addr_bit_len;
	i2c_config_slave.clk_source = I2C_CLK_SRC_DEFAULT;
	i2c_config_slave.i2c_port = i2c_port;
	i2c_config_slave.send_buf_depth = 2 * ESP32_SLAVE_RECEIVE_BUFFER_LEN; // 1024;

	i2c_config_slave.scl_io_num = scl_io_num;
	i2c_config_slave.sda_io_num = sda_io_num;
	i2c_config_slave.slave_addr = slave_addr;

	this->i2cSlave_intRequestPin = intRequestPin;
	gpio_config_t I2C_slave_IntRequestPinConfig;
	I2C_slave_IntRequestPinConfig.intr_type = GPIO_INTR_DISABLE;
	I2C_slave_IntRequestPinConfig.mode = GPIO_MODE_OUTPUT;
	I2C_slave_IntRequestPinConfig.pin_bit_mask = 0x1 << this->i2cSlave_intRequestPin;
	I2C_slave_IntRequestPinConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	I2C_slave_IntRequestPinConfig.pull_up_en = GPIO_PULLUP_ENABLE;
	this->interruptRequestReset(); // ustawiam wyjście na wysokie przed inicjalizacją GPIO, aby nie wywołać niepotrzebnie interrupt request
	ESP_ERROR_CHECK(gpio_config(&I2C_slave_IntRequestPinConfig));
	// this->interruptRequestReset();
	printf("%s bus interrupt request GPIO has been initialised on GPIO_num_%d.\n", this->TAG, this->i2cSlave_intRequestPin);

	ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_config_slave, &handler_i2c_dev_slave));

	configASSERT(this->i2cSlaveReceiveDataToDataParserQueue = new i2cQueue4DynamicData(20));
	configASSERT(this->i2cSlaveTransmitDataQueue = new i2cQueue4DynamicData(20));
	printf("%s bus has been initialised on port %d with address %lx.\n", this->TAG, i2c_port, slave_addr);

	// Tworzenie kolejki nadawczej
	// this->pTransmitQueueObject = NULL;
	// configASSERT(this->pTransmitQueueObject = new i2cQueue4DynamicData(DEFAULT_TRANSMIT_QUEUE_SIZE));

	// Tworzenie kolejki odbiorczej
	configASSERT(this->s_receive_queue = xQueueCreate(10, sizeof(i2cSlaveRxEvent)));	// element: zdarzenie I2C + kopia danych (ok. 72 B)
	i2c_slave_event_callbacks_t cbs = {
		.on_recv_done = i2c_slave_rx_done_callback,
	};

	rxEvent.cbData.rx_fifo_end_addrLast = I2C0.fifo_st.rx_fifo_end_addr;
	rxEvent.cbData.tx_fifo_start_addrLast = I2C0.fifo_st.tx_fifo_start_addr;

	ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(handler_i2c_dev_slave, &cbs, this->s_receive_queue));
	printf("%s bus has been initialised on port %d with address %lx.\n", this->TAG, i2c_port, slave_addr);

	this->i2cMasterCrcSumCounterErrorReset();
}

BaseType_t i2cEngin_slave::i2cMasterCrcSumCounterErrorIncrement(void)
{
	this->i2cMasterCrcSumCounterError++;
	if (this->i2cMasterCrcSumCounterError > 7)
	{
		return pdFALSE;
	}
	return pdTRUE;
}
void i2cEngin_slave::i2cMasterCrcSumCounterErrorReset(void)
{
	this->i2cMasterCrcSumCounterError = 0;
}

/*---------------------------------------------------------------
 * Metoda działa wewnątrz taska "i2cSlaveReceive" i jej zadanie
 * jest przesyłanie (przez kolejkę) otrzymanych z i2c master
 * danych do taska zajmującego się parsowaniem otrzymanych danych.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void i2cEngin_slave::i2cSlaveReceive(void)
{
	// Bufor roboczy sterownika I2C slave (ESP-IDF 5.2.8): i2c_slave_receive() przekazuje jego adres sterownikowi
	// (t->buffer), a ISR sterownika zapisuje do niego odebrane bajty. Musi istnieć, ale task NIE czyta z niego danych:
	// callback i2c_slave_rx_done_callback kopiuje zawartość (edata->buffer) do i2cSlaveRxEvent.data, a task parsuje
	// wyłącznie rx_data.data. Dzięki temu ponowne uzbrojenie (i2c_slave_receive) nie koliduje z parsowaniem.
	uint8_t *data_rd = new uint8_t[ESP32_SLAVE_RECEIVE_BUFFER_LEN];

	// uint32_t size_rd = 0;
	i2cSlaveRxEvent rx_data;	// kopia zdarzenia z kolejki - task pracuje wyłącznie na niej (nie na globalnym rxEvent ISR)
	ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, I2C_SLAVE_DRIVER_RECEIVE_LEN));	// max 32 B - błąd sterownika IDF 5.2 (opis przy I2C_SLAVE_DRIVER_RECEIVE_LEN)
	// Ten task nie może spać (np. w impulsie INT): po każdej ramce odbiór trzeba ponownie uzbroić (i2c_slave_receive niżej),
	// inaczej kolejne ramki STM32 zapychają RX FIFO -> NACK na STM32 (KONTEKST 3.8).

	i2cFrame_commonHeader *fakeCommHeader = (i2cFrame_commonHeader *)rx_data.data; // potrzebny, aby przeczytać ilośc otrzymanych z i2c master byte'ów
	i2cFrame_transmitQueue tempFrameToParserQueue;
	while (1)
	{
		if (xQueueReceive(this->s_receive_queue, &rx_data, portMAX_DELAY) == pdTRUE)
		{
			ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, I2C_SLAVE_DRIVER_RECEIVE_LEN));	// max 32 B - błąd sterownika IDF 5.2 (opis przy I2C_SLAVE_DRIVER_RECEIVE_LEN)
			if (rx_data.cbData.rxToEsp32 == recpeptionToMe)
			{
				assert(fakeCommHeader->dataSize <= ESP32_SLAVE_RECEIVE_BUFFER_LEN);	// długość spoza bufora = uszkodzona ramka; new/memcpy wyszłyby poza rx_data.data
				void *tempData = static_cast<void *>(new (std::nothrow) char[fakeCommHeader->dataSize]);	// nothrow: przy braku pamięci nullptr -> assert niżej (zwykłe new w IDF bez wyjątków C++ robi abort())
				if (tempData != nullptr)
				{

					// i2cFrame_hmiLeds tempToDelete;

					tempFrameToParserQueue.dataSize = fakeCommHeader->dataSize;
					memcpy(tempData, rx_data.data, tempFrameToParserQueue.dataSize);
					tempFrameToParserQueue.pData = tempData;

					this->i2cSlaveReceiveDataToDataParserQueue->QueueSendFromISR(&tempFrameToParserQueue); // funkcja ma od razu sprawdzanie czy pdTure, jeśli nie to usuwa zmienną zadeklarowaną dynamicznie
				}
				else
				{
					assert(0);
				}
				// printf("Data len is%s\n", data_rd);
				// printf("I2C rec. len:%d\n", fakeCommHeader->dataSize);
				// printf("I2C rec\n");
				printf("%s DIAG Reception to me\n", this->TAG);
			}
			// DIAGNOSTYKA rozpoznawania transakcji do/od ESP32 (jak w commit 8d705b12)
			else if (rx_data.cbData.rxToEsp32 == recpeptionNotToMe)
			{
				printf("%s DIAG Reception NOT to me\n", this->TAG);
			}
			else if (rx_data.cbData.rxToEsp32 == transmitionFromMe)
			{
				printf("%s DIAG Transmition from me\n", this->TAG);
			}
			else if (rx_data.cbData.rxToEsp32 == transmitionNotFromMe)
			{
				printf("%s DIAG Transmition NOT from me\n", this->TAG);
			}
			printf("%s DIAG cb:%d slave_rw:%u slave_addressed:%u rx_fifo_end:%u tx_fifo_start:%u->%u tx_fifo_end:%u tx_fifo_cnt:%u\n",
				   this->TAG, rx_data.cbData.rxToEsp32, rx_data.cbData.diag_slave_rw, rx_data.cbData.diag_slave_addressed, rx_data.cbData.diag_rx_fifo_end_addr,
				   rx_data.cbData.diag_tx_fifo_start_addrPrev, rx_data.cbData.diag_tx_fifo_start_addr, rx_data.cbData.diag_tx_fifo_end_addr, rx_data.cbData.diag_tx_fifo_cnt);
		}
	}
}
/*---------------------------------------------------------------
 * Metoda informuje i2c master o tym, że esp32 (i2c slave) ma dane
 * do wysłania. Ten sygmnał to zbocze opadające GPIO.
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t 				- ESP_OK lub ESP_FAIL
 *---------------------------------------------------------------*/
esp_err_t i2cEngin_slave::interruptRequestSet(void)
{
	return gpio_set_level(this->i2cSlave_intRequestPin, 0); // interrupt request is SET when pin goes low
}

/*---------------------------------------------------------------
 * Metoda resetująca informuję do i2c master o tym, że esp32 (i2c
 * slave) ma dane do wysłania.
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t 				- ESP_OK lub ESP_FAIL
 *---------------------------------------------------------------*/
esp_err_t i2cEngin_slave::interruptRequestReset(void)
{
	return gpio_set_level(this->i2cSlave_intRequestPin, 1); // interrupt request is RESET when pin goes high
}

BaseType_t i2cEngin_slave::i2cSendDataToTransisionQueue(i2cFrame_transmitQueue *tempFrameToParserQueue)
{
	if (tempFrameToParserQueue->dataSize > ESP32_SLAVE_RECEIVE_BUFFER_LEN)	// limit długości ramki (comunicationProtocol.h) - jak w STM32 masterTransmitData()
	{
		printf("%s TX frame too long: %u > %u\n", this->TAG, (unsigned int)tempFrameToParserQueue->dataSize, (unsigned int)ESP32_SLAVE_RECEIVE_BUFFER_LEN);
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
		assert(0);
	}
	return this->i2cSlaveTransmitDataQueue->QueueSendFromISR(tempFrameToParserQueue);
}

/*---------------------------------------------------------------
 * estruktor klasy.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
i2cEngin_slave::~i2cEngin_slave()
{
	ESP_ERROR_CHECK(this->interruptRequestReset());

	ESP_ERROR_CHECK(i2c_del_slave_device(handler_i2c_dev_slave));
	printf("%s bus has been destructed.\r\n", this->TAG);

	// usuwanie kolejki nadawczej oraz danych, które są poinicjowane (danych, do których wskazują wskaźniki ze struktury i2cFrame_transmitQueue kolejki
	// delete this->pTransmitQueueObject;
}

void i2cEngin_slave::i2cSlaveTransmit(void)
{
	i2cFrame_transmitQueue dataToTransmit;
	esp_err_t retVal = ESP_FAIL;

	// zmienne do cyklicznego (co txStatePrintPeriod_ms) wyświetlania stanu transmisji
	const uint32_t txStatePrintPeriod_ms = 1000;
	i2cTransmitionState txLastPrintedState = i2cTransmitionState::idle;
	TickType_t txLastPrintTick = 0;		// tick ostatniego wydruku stanu, do odmierzania kolejnego printf co txStatePrintPeriod_ms
	TickType_t txStateEntryTick = 0;	// tick wejścia w stan ...InTransmition, do wykrycia timeoutu (tx_timeout_ms)
	i2cTransmitionState txStateBeforeError = i2cTransmitionState::idle;	// DIAGNOSTYKA: stan, w którym wystąpił timeout/błąd (nadpisywany przez timeoutInTransmition/errorInTransmition)

	// zwraca true przy wejściu w nowy stan lub gdy od ostatniego wydruku minęło txStatePrintPeriod_ms
	auto isTimeToPrintTxState = [&](i2cTransmitionState currentState) -> bool {
		TickType_t now = xTaskGetTickCount();
		if ((currentState != txLastPrintedState) || ((now - txLastPrintTick) >= pdMS_TO_TICKS(txStatePrintPeriod_ms)))
		{
			txLastPrintedState = currentState;
			txLastPrintTick = now;
			return true;
		}
		return false;
	};

	// Brak impulsu startowego na INT: gotowość ESP32 STM32 sprawdza pingiem i2c (ACK na adres, i2cMaster::isDeviceReady).
	// INT jest na GPIO0 (pin strapping) - impuls startowy mylił się ze zboczami z resetu brown-out / flashowania (KONTEKST 7.5).
	while (1)
	{

		switch(this->i2cTxSlaveState){
			case i2cTransmitionState::idle:		
				if (this->i2cSlaveTransmitDataQueue->QueueReceive(&dataToTransmit, portMAX_DELAY) == pdTRUE)
				{
					printf("%s i2c slave transmit IDLE\n", this->TAG);
					retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t *)&dataToTransmit.dataSize, sizeof(dataToTransmit.dataSize), this->tx_timeout_ms);	// rozmiar danych → bufor TX
					if (ESP_OK == retVal)
					{
						retVal = this->i2cTxWaitForFifoLoaded();	// FIFO załadowane PRZED stanem ...InTransmition (opis nad callbackiem)
					}
					if (ESP_OK == retVal)
					{
						this->i2cTxSlaveState=i2cTransmitionState::lenDataInTransmition;
						this->interruptRequestSet();	// GPIO LOW: sygnał do STM32 "mam dane, czytaj"
						esp_rom_delay_us(20);			// minimalna szerokość impulsu dla EXTI STM32
						this->interruptRequestReset();
						txStateEntryTick = xTaskGetTickCount();
					}
					else
					{
						txStateBeforeError = i2cTransmitionState::idle;	// DIAGNOSTYKA
						this->i2cTxSlaveState=i2cTransmitionState::errorInTransmition;
					}
				}
				break;
			case i2cTransmitionState::lenDataInTransmition:
				if (isTimeToPrintTxState(i2cTransmitionState::lenDataInTransmition)){
					printf("%s i2c slave transmit LEN_DATA_IN_TRANSMITION\n", this->TAG);
				}
				// zabezpieczenie: wykrycie odczytu z tasku, gdy callback przepadł (opis nad callbackiem)
				if ((this->i2cTxStateNextIfReadDone() == false) && ((xTaskGetTickCount() - txStateEntryTick) >= pdMS_TO_TICKS(this->tx_timeout_ms))){
					portENTER_CRITICAL(&s_i2cTxStateMux);	// ISR mógł właśnie przesunąć stan - nie nadpisywać go timeoutem
					if (this->i2cTxSlaveState == i2cTransmitionState::lenDataInTransmition){
						txStateBeforeError = i2cTransmitionState::lenDataInTransmition;	// DIAGNOSTYKA
						this->i2cTxSlaveState = i2cTransmitionState::timeoutInTransmition;	// STM32 nie odczytał len w czasie tx_timeout_ms
					}
					portEXIT_CRITICAL(&s_i2cTxStateMux);
				}
				vTaskDelay(1);
				break;
			case i2cTransmitionState::lenDataTransmited:
				printf("%s i2c slave transmit LEN_DATA_TRANSMITED\n", this->TAG);
				retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t *)dataToTransmit.pData, dataToTransmit.dataSize, this->tx_timeout_ms);	// dane → bufor TX
				if (ESP_OK == retVal)
				{
					retVal = this->i2cTxWaitForFifoLoaded();	// FIFO załadowane PRZED stanem ...InTransmition (opis nad callbackiem)
				}
				if (ESP_OK == retVal)
				{
					this->i2cTxSlaveState=i2cTransmitionState::packageDataInTransmition;
					this->interruptRequestSet();	// GPIO LOW: sygnał do STM32 "mam dane, czytaj"
					esp_rom_delay_us(20);			// minimalna szerokość impulsu dla EXTI STM32
					this->interruptRequestReset();
					txStateEntryTick = xTaskGetTickCount();
				}
				else
				{
					txStateBeforeError = i2cTransmitionState::lenDataTransmited;	// DIAGNOSTYKA
					this->i2cTxSlaveState=i2cTransmitionState::errorInTransmition;
				}
				break;
			case i2cTransmitionState::packageDataInTransmition:
				if (isTimeToPrintTxState(i2cTransmitionState::packageDataInTransmition)){
					printf("%s i2c slave transmit PACKAGE_DATA_IN_TRANSMITION\n", this->TAG);
				}
				// zabezpieczenie: wykrycie odczytu z tasku, gdy callback przepadł (opis nad callbackiem)
				if ((this->i2cTxStateNextIfReadDone() == false) && ((xTaskGetTickCount() - txStateEntryTick) >= pdMS_TO_TICKS(this->tx_timeout_ms))){
					portENTER_CRITICAL(&s_i2cTxStateMux);	// ISR mógł właśnie przesunąć stan - nie nadpisywać go timeoutem
					if (this->i2cTxSlaveState == i2cTransmitionState::packageDataInTransmition){
						txStateBeforeError = i2cTransmitionState::packageDataInTransmition;	// DIAGNOSTYKA
						this->i2cTxSlaveState = i2cTransmitionState::timeoutInTransmition;	// STM32 nie odczytał danych w czasie tx_timeout_ms
					}
					portEXIT_CRITICAL(&s_i2cTxStateMux);
				}
				vTaskDelay(1);
				break;
			case i2cTransmitionState::packageDataTransmited:
				printf("%s i2c slave transmit PACKAGE_DATA_TRANSMITED\n", this->TAG);
				delete[] static_cast<char *>(dataToTransmit.pData);
				dataToTransmit.pData = nullptr;
				this->i2cTxSlaveState = i2cTransmitionState::idle;
				break;
			case i2cTransmitionState::timeoutInTransmition:
			case i2cTransmitionState::errorInTransmition:
				if (isTimeToPrintTxState(i2cTransmitionState::errorInTransmition)){
					switch(this->i2cTxSlaveState){
					case i2cTransmitionState::errorInTransmition:
						printf("%s i2c slave transmit ERROR_IN_TRANSMITION\n", this->TAG);
						break;
					case i2cTransmitionState::timeoutInTransmition:
					printf("%s i2c slave transmit TIMEOUT_IN_TRANSMITION\n", this->TAG);
						break;
					default:
						break;
					};
					// DIAGNOSTYKA: stan przed błędem + rejestry I2C0 (odczyt pełnych słów .val; NIE czytać fifo_data - odczyt zdejmuje bajt z RX FIFO)
					printf("%s DIAG TX FAIL: stateBefore:%d retVal:0x%x status_reg:0x%08lx fifo_st:0x%08lx int_raw:0x%08lx int_ena:0x%08lx | tx_fifo_start:%u tx_fifo_end:%u rx_fifo_end:%u slave_rw:%u\n",
						   this->TAG, (int)txStateBeforeError, (unsigned int)retVal,
						   (unsigned long)I2C0.status_reg.val, (unsigned long)I2C0.fifo_st.val, (unsigned long)I2C0.int_raw.val, (unsigned long)I2C0.int_ena.val,
						   (unsigned int)I2C0.fifo_st.tx_fifo_start_addr, (unsigned int)I2C0.fifo_st.tx_fifo_end_addr, (unsigned int)I2C0.fifo_st.rx_fifo_end_addr, (unsigned int)I2C0.status_reg.slave_rw);
					//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
					assert(0);
				}
				if (dataToTransmit.pData != nullptr){
					delete[] static_cast<char *>(dataToTransmit.pData);
					dataToTransmit.pData = nullptr;
				}
				vTaskDelay(1);
				break;
		}
	}
}