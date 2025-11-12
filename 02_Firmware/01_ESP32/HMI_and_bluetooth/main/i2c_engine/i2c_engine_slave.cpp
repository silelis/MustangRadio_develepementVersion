#include "i2c_engine_slave.h"

static i2c_slave_dev_handle_t handler_i2c_dev_slave;
static i2c_slave_config_t i2c_config_slave;

#include "soc/i2c_struct.h"
extern i2c_dev_t I2C0;

#include "driver/i2c.h"
//----------------------------------------------
// Enumy i zmienne globalne (z volatile dla ISR)
//----------------------------------------------
typedef enum
{
	recpeptionNotToMe, // 0 - dane nie były do ESP32
	recpeptionToMe,	   // 1 - ESP32 odebrało dane
	transmition		   // 2 - ESP32 wysłało dane
} i2cCallbackState;

volatile static i2cCallbackState rxToEsp32 = recpeptionNotToMe;
volatile static uint32_t rx_fifo_end_addrLast = 0;

//----------------------------------------------
// Callback dla zdarzeń I2C Slave
//----------------------------------------------
IRAM_ATTR bool i2cEngin_slave::i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel,
														  const i2c_slave_rx_done_event_data_t *edata,
														  void *user_data)
{
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;

	// 1. Sprawdź kierunek transmisji
	if (I2C0.status_reg.slave_rw == 0)
	{
		// Master -> Slave (ESP32 odbiera)
		if (rx_fifo_end_addrLast != I2C0.fifo_st.rx_fifo_end_addr)
		{
			rxToEsp32 = recpeptionToMe; // Nowe dane dla ESP32
		}
		else
		{
			rxToEsp32 = recpeptionNotToMe; // Dane były dla innego slave'a
		}
		rx_fifo_end_addrLast = I2C0.fifo_st.rx_fifo_end_addr;
	}
	else
	{
		// Slave -> Master (ESP32 wysyła)
		rxToEsp32 = transmition;
	}

	// 2. Wyślij zdarzenie do kolejki (z zabezpieczeniem przed NULL)
	if (receive_queue != NULL && edata != NULL)
	{
		xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
	}

	return high_task_wakeup == pdTRUE;
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
	configASSERT(this->s_receive_queue = xQueueCreate(10, sizeof(i2c_slave_rx_done_event_data_t)));
	i2c_slave_event_callbacks_t cbs = {
		.on_recv_done = i2c_slave_rx_done_callback,
	};

	rx_fifo_end_addrLast = I2C0.fifo_st.rx_fifo_end_addr;

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
	uint8_t *data_rd = new uint8_t[ESP32_SLAVE_RECEIVE_BUFFER_LEN];

	// uint32_t size_rd = 0;
	i2c_slave_rx_done_event_data_t rx_data;
	ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN));
	this->esp32i2cBusInitialised(); // informuje i2c master poprzez pierwsze interrupt request, że szyna i2c jest zainicjowana

	i2cFrame_commonHeader *fakeCommHeader = (i2cFrame_commonHeader *)data_rd; // potrzebny, aby przeczytać ilośc otrzymanych z i2c master byte'ów
	i2cFrame_transmitQueue tempFrameToParserQueue;
	while (1)
	{
		memset(data_rd, 0, ESP32_SLAVE_RECEIVE_BUFFER_LEN);

		if (xQueueReceive(this->s_receive_queue, &rx_data, portMAX_DELAY) == pdTRUE)
		{
			ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN));
			if (rxToEsp32 == recpeptionToMe)
			{
				void *tempData = static_cast<void *>(new char[fakeCommHeader->dataSize]);
				if (tempData != nullptr)
				{

					// i2cFrame_hmiLeds tempToDelete;

					tempFrameToParserQueue.dataSize = fakeCommHeader->dataSize;
					memcpy(tempData, data_rd, tempFrameToParserQueue.dataSize);
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
			}
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

esp_err_t i2cEngin_slave::i2cSlaveTransmit(void)
{

	i2cFrame_transmitQueue dataToTransmit;
	esp_err_t retVal = ESP_FAIL;
	if (this->i2cSlaveTransmitDataQueue->QueueReceive(&dataToTransmit, portMAX_DELAY) == pdTRUE)
	{

#ifdef STM32_2_ESP32_I2C_IN_SEQUENCE

		retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t *)dataToTransmit.pData, dataToTransmit.dataSize, this->tx_timeout_ms);

#else

		retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t *)&dataToTransmit.dataSize, sizeof(dataToTransmit.dataSize), this->tx_timeout_ms);
		if (ESP_OK == retVal)
		{
			retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t *)dataToTransmit.pData, dataToTransmit.dataSize, this->tx_timeout_ms);
		}
#endif
		this->interruptRequestSet();
		this->interruptRequestReset();
		delete[] static_cast<char *>(dataToTransmit.pData);
		// return retVal;
	}
	return retVal;
}