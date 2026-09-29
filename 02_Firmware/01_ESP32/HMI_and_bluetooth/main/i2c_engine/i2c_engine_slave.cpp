#include "i2c_engine_slave.h"
#include "assert.h"

static i2c_slave_dev_handle_t handler_i2c_dev_slave;
static i2c_slave_config_t i2c_config_slave;

#include "soc/i2c_struct.h"
extern i2c_dev_t I2C0;

#include "driver/i2c.h"
#include "esp_rom_sys.h"
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
} i2cCallbackData;

// element kolejki s_receive_queue: jedno zdarzenie I2C z kopią danych z chwili callbacku
typedef struct
{
	i2cCallbackData cbData;							// klasyfikacja i DIAGNOSTYKA tego zdarzenia
	uint8_t data[ESP32_SLAVE_RECEIVE_BUFFER_LEN];	// kopia bufora odbiorczego (edata->buffer) - task nie czyta bufora sterownika
} i2cSlaveRxEvent;
// bieżące zdarzenie budowane w ISR i kopiowane do kolejki; cbData.*_Last - pamięć ISR między callbackami
static i2cSlaveRxEvent rxEvent = {{recpeptionNotToMe, 0, 0, 0, 0, 0, 0, 0, 0}, {0}};

//----------------------------------------------
// Callback dla zdarzeń I2C Slave
//----------------------------------------------
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
		if (rxEvent.cbData.tx_fifo_start_addrLast != I2C0.fifo_st.tx_fifo_start_addr)
		{
			rxEvent.cbData.rxToEsp32 = transmitionFromMe;	// sprzęt wysłał bajty z TX FIFO
			if (sInstance != nullptr){
				sInstance->i2cTxStateNextFromISR();
			}
		}
		else
		{
			rxEvent.cbData.rxToEsp32 = transmitionNotFromMe;	// wskaźnik odczytu TX FIFO bez zmian - transakcja nie z ESP32
		}
		rxEvent.cbData.tx_fifo_start_addrLast = I2C0.fifo_st.tx_fifo_start_addr;
		break;
	}

	// 2. Wyślij zdarzenie do kolejki (z zabezpieczeniem przed NULL)
	if (receive_queue != NULL && edata != NULL)
	{
		memcpy(rxEvent.data, edata->buffer, ESP32_SLAVE_RECEIVE_BUFFER_LEN);	// kopia ramki - ramka kompletna, sterownik nie pisze teraz do bufora
		BaseType_t queueSendResult = xQueueSendFromISR(receive_queue, &rxEvent, &high_task_wakeup);
		assert(queueSendResult == pdTRUE);	// pełna kolejka = utrata zdarzenia i brak ponownego uzbrojenia odbiornika - błąd krytyczny
	}

	return high_task_wakeup == pdTRUE;
}

/*---------------------------------------------------------------
 * Metoda wywoływana z ISR i2c_slave_rx_done_callback, gdy master
 * odczytał dane z ESP32 (transmitionFromMe). Przesuwa maszynę
 * stanów i2cSlaveTransmit do kolejnego stanu.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
IRAM_ATTR void i2cEngin_slave::i2cTxStateNextFromISR(void)
{
	switch (this->i2cTxSlaveState)
	{
	case i2cTransmitionState::lenDataInTransmition:
		this->i2cTxSlaveState = i2cTransmitionState::lenDataTransmited;
		break;
	case i2cTransmitionState::packageDataInTransmition:
		this->i2cTxSlaveState = i2cTransmitionState::packageDataTransmited;
		break;
	default:
		break;
	}
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
	ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN));
	this->esp32i2cBusInitialised(); // informuje i2c master poprzez pierwsze interrupt request, że szyna i2c jest zainicjowana

	i2cFrame_commonHeader *fakeCommHeader = (i2cFrame_commonHeader *)rx_data.data; // potrzebny, aby przeczytać ilośc otrzymanych z i2c master byte'ów
	i2cFrame_transmitQueue tempFrameToParserQueue;
	while (1)
	{
		if (xQueueReceive(this->s_receive_queue, &rx_data, portMAX_DELAY) == pdTRUE)
		{
			ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN));
			if (rx_data.cbData.rxToEsp32 == recpeptionToMe)
			{
				assert(fakeCommHeader->dataSize <= ESP32_SLAVE_RECEIVE_BUFFER_LEN);	// długość spoza bufora = uszkodzona ramka; new/memcpy wyszłyby poza rx_data.data
				void *tempData = static_cast<void *>(new char[fakeCommHeader->dataSize]);
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
			printf("%s DIAG cb:%d slave_rw:%u slave_addressed:%u rx_fifo_end:%u tx_fifo_start:%u->%u tx_fifo_end:%u\n",
				   this->TAG, rx_data.cbData.rxToEsp32, rx_data.cbData.diag_slave_rw, rx_data.cbData.diag_slave_addressed, rx_data.cbData.diag_rx_fifo_end_addr,
				   rx_data.cbData.diag_tx_fifo_start_addrPrev, rx_data.cbData.diag_tx_fifo_start_addr, rx_data.cbData.diag_tx_fifo_end_addr);
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

/*---------------------------------------------------------------
 * Metoda poprzez pierwszy sygnał interrupt request (pusty) informuje
 * master i2c, że szyna i2c jest zainicjowana.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void i2cEngin_slave::esp32i2cBusInitialised(void)
{
	this->interruptRequestSet();
	vTaskDelay(pdMS_TO_TICKS(100));
	this->interruptRequestReset();
}

BaseType_t i2cEngin_slave::i2cSendDataToTransisionQueue(i2cFrame_transmitQueue *tempFrameToParserQueue)
{
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
						this->i2cTxSlaveState=i2cTransmitionState::lenDataInTransmition;
						this->interruptRequestSet();	// GPIO LOW: sygnał do STM32 "mam dane, czytaj"
						esp_rom_delay_us(20);			// minimalna szerokość impulsu dla EXTI STM32
						this->interruptRequestReset();
						txStateEntryTick = xTaskGetTickCount();
					}
					else
					{
						this->i2cTxSlaveState=i2cTransmitionState::errorInTransmition;
					}
				}
				break;
			case i2cTransmitionState::lenDataInTransmition:
				if (isTimeToPrintTxState(i2cTransmitionState::lenDataInTransmition)){
					printf("%s i2c slave transmit LEN_DATA_IN_TRANSMITION\n", this->TAG);
				}
				if ((xTaskGetTickCount() - txStateEntryTick) >= pdMS_TO_TICKS(this->tx_timeout_ms)){
					this->i2cTxSlaveState = i2cTransmitionState::timeoutInTransmition;	// STM32 nie odczytał len w czasie tx_timeout_ms
				}
				vTaskDelay(1);
				break;
			case i2cTransmitionState::lenDataTransmited:
				printf("%s i2c slave transmit LEN_DATA_TRANSMITED\n", this->TAG);
				retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t *)dataToTransmit.pData, dataToTransmit.dataSize, this->tx_timeout_ms);	// dane → bufor TX
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
					this->i2cTxSlaveState=i2cTransmitionState::errorInTransmition;
				}
				break;
			case i2cTransmitionState::packageDataInTransmition:
				if (isTimeToPrintTxState(i2cTransmitionState::packageDataInTransmition)){
					printf("%s i2c slave transmit PACKAGE_DATA_IN_TRANSMITION\n", this->TAG);
				}
				if ((xTaskGetTickCount() - txStateEntryTick) >= pdMS_TO_TICKS(this->tx_timeout_ms)){
					this->i2cTxSlaveState = i2cTransmitionState::timeoutInTransmition;	// STM32 nie odczytał danych w czasie tx_timeout_ms
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