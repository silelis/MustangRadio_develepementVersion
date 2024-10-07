#include "i2c_engine.h"
#include "soc/i2c_reg.h"
//#include "soc/i2c_periph.h"
#include "soc/i2c_struct.h"
#include "hal/i2c_ll.h"
static i2c_master_bus_handle_t handler_i2c_bus_master;
static i2c_master_bus_config_t i2c_bus_config_master;
 
static i2c_slave_dev_handle_t handler_i2c_dev_slave;
static i2c_slave_config_t i2c_config_slave;

extern i2c_dev_t I2C0;
/*---------------------------------------------------------------
 * Funkcja statyczna callbacku po zkończeniu dobiuru danych z i2c
 * master.
 * Parameters:
 * i2c_slave_dev_handle_t channel	- uchwyt do i2c slave bus (do
 * 						interfejsu i2c)
 * const i2c_slave_rx_done_event_data_t *edata - Struktura zdarzeń
 *						używana w trybie slave I2C
 * void *user_data - Dane użytkownika, które zostaną bezpośrednio
 *						przekazane do funkcji (callback), w tych 
 *						danych zostanią zapisane informacje 
 *						otrzymane z i2c master 
 * Returns:
 * bool				- ESP_OK: Pomyślne ustawienie funkcji 
 *					zwrotnych (callback) transakcji I2C
 *					ESP_ERR_INVALID_ARG: Ustawienie funkcji
 *					 zwrotnych transakcji I2C nie powiodło się z
 * 					 powodu nieprawidłowego argumentu
 *					ESP_FAIL: Ustawienie funkcji zwrotnych 
 *					transakcji I2C nie powiodło się z powodu innego
 *					błędu
*---------------------------------------------------------------*/
static IRAM_ATTR bool i2cSlaveReceive_finishedCallback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *edata, void *user_data)
{
	i2cFrame_transmitQueue tempReceivedFrame;
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;
	tempReceivedFrame.dataSize = *(size_t*)edata->buffer;
	
	//I2C_LL_SLAVE_RX_EVENT_INTR  (I2C_TRANS_COMPLETE_INT_ENA_M|I2C_RX_REC_FULL_INT_ST_M)
	//I2C_LL_SLAVE_RX_INT(I2C_RXFIFO_FULL_INT_ENA_M | I2C_TRANS_COMPLETE_INT_ENA_M)
	

	//			 i2c_struct.h  i2c_reg.h
	//#define I2C_LL_GET_HW(i2c_num)        (((i2c_num) == 0) ? &I2C0 : &I2C1)
	i2c_dev_t* whichDev = I2C_LL_GET_HW(0);
	i2c_ll_get_interrupt_status_reg(whichDev);
	//									receive		TRANSM
	//I2C0.int_ena.trans_complete;		//1			1
	//I2C0.int_ena.rx_rec_full;			//1			1
	I2C0.int_status.trans_complete;		//0			1
	I2C0.int_status.rx_rec_full;		//
	//I2C0.int_status.rx_rec_full;		//0			0
	I2C0.int_raw.rx_fifo_full;			//1			0
	//I2C0.int_raw.tx_fifo_empty;			//1			1
	//I2C0.int_raw.tx_send_empty;			//0			0
	//I2C0.int_raw.slave_tran_comp;		//1			1
	I2C0.int_raw.trans_complete;		//0			1
	//I2C0.int_ena.rx_fifo_full;			//0			0
	//I2C0.fifo_data.data;
	//void* receivedData;
	char*  receivedData = new char[tempReceivedFrame.dataSize]; //(char*) malloc(receivedFrame.dataSize); 
	char* data_start = (char*)edata->buffer + sizeof(size_t);
	
	memcpy(receivedData, data_start, tempReceivedFrame.dataSize);
	tempReceivedFrame.pData =  receivedData;
	
	//xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
	if (xQueueSendFromISR((QueueHandle_t)user_data/*receive_queue*/, &tempReceivedFrame, &high_task_wakeup) == pdFAIL)
	{
		delete[]tempReceivedFrame.pData;
		assert(0);
	}
	if (high_task_wakeup == pdTRUE) {
		portYIELD_FROM_ISR();
		return pdTRUE;
	}
	//void* data = receivedFrame->pData;
	return pdFALSE;
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
	i2c_config_slave.addr_bit_len =slave_addr_bit_len;
	i2c_config_slave.clk_source = I2C_CLK_SRC_DEFAULT;
	i2c_config_slave.i2c_port = i2c_port;
	i2c_config_slave.send_buf_depth = 1024;
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
	this->interruptRequestReset();			//ustawiam wyjście na wysokie przed inicjalizacją GPIO, aby nie wywołać niepotrzebnie interrupt request
	ESP_ERROR_CHECK(gpio_config(&I2C_slave_IntRequestPinConfig));
	//this->interruptRequestReset();
	printf("%s bus interrupt request GPIO has been initialised on GPIO_num_%d.\n", this->TAG , this->i2cSlave_intRequestPin);
	
	ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_config_slave, &handler_i2c_dev_slave));
	printf("%s bus has been initialised on port %d with address %lx.\n", this->TAG, i2c_port, slave_addr);

	

	
	
	//Tworzenie kolejki nadawczej
	this->pTransmitQueueObject = NULL;
	configASSERT(this->pTransmitQueueObject = new i2cQueue4DynamicData(ESP32_DEFAULT_TRANSMIT_QUEUE_SIZE));
	
	//Tworzenie kolejki nodbiorczej
	this->pReceivedQueueObject = NULL;
	configASSERT(this->pReceivedQueueObject = new i2cQueue4DynamicData(ESP32_DEFAULT_RECEIVE_QUEUE_SIZE));
	
	//rejestracja callbacka i2c reveive finished
	i2c_slave_event_callbacks_t cbs; // = {
	cbs.on_recv_done = i2cSlaveReceive_finishedCallback; //,
	//};
	ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(handler_i2c_dev_slave, &cbs, this->pReceivedQueueObject->returnHandlerQueue()));
	printf("%s reveive callback has been initialised.\n", this->TAG);
	//I2C0.int_status.slave_tran_comp;
	//I2C0.int_status.slave_tran_comp;
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
	return gpio_set_level(this->i2cSlave_intRequestPin, 0); //interrupt request is SET when pin goes low
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
	return gpio_set_level(this->i2cSlave_intRequestPin, 1); //interrupt request is RESET when pin goes high
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
	
	//usuwanie kolejki nadawczej oraz danych, które są poinicjowane (danych, do których wskazują wskaźniki ze struktury i2cFrame_transmitQueue kolejki
	delete this->pTransmitQueueObject;
}

/*---------------------------------------------------------------
 * Metoda wpisuje dane do bufora nadawczego esp32 po i2c w trybie
 * slave. Dane pobierane są z kolejki QueueReceive obiektu
 * pTransmitQueueObject.
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t 				- ESP_OK lub ESP_FAIL
*---------------------------------------------------------------*/
esp_err_t i2cEngin_slave::slaveTransmit()
{
	esp_err_t retVal =ESP_FAIL;
	i2cFrame_transmitQueue ItemWithPointerToTransmit;
	
	if (pdPASS == this->pTransmitQueueObject->QueueReceive(&ItemWithPointerToTransmit, portMAX_DELAY)) //kolejka zawiera dane;
	{
		this->interruptRequestSet();
		retVal= i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t*) &ItemWithPointerToTransmit.dataSize, sizeof(ItemWithPointerToTransmit.dataSize), this->tx_timeout_ms);
		if (ESP_OK == retVal)
		{
			retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t*) ItemWithPointerToTransmit.pData, ItemWithPointerToTransmit.dataSize, this->tx_timeout_ms);
		}
		this->pTransmitQueueObject->QueueDeleteDataFromPointer(ItemWithPointerToTransmit);
		this->interruptRequestReset();
	}
	return retVal;	
}




esp_err_t i2cEngin_slave::i2cSlaveReceiveFromCallback(uint8_t *data)
{
	return i2c_slave_receive(handler_i2c_dev_slave, data, ESP32_DEFAULT_RECEIVE_QUEUE_SIZE);
}


/*---------------------------------------------------------------
 * Konstruktor klasy odpwiadającej za komunikację ESP32 po i2c z
 * urządzeniami i2c slave.
 * Parameters:
 * i2c_port_num_t i2c_port	- numer portu i2c w kontrolerze ESP32
 * gpio_num_t sda_io_num	- numer pinu kontrolera ESP32 do którego
 *							  przypisano sygnał SDA szyny i2c
 * gpio_num_t scl_io_num	- numer pinu kontrolera ESP32 do którego
 *							  przypisano sygnał SCL szyny i2c
 * Returns:
 * NONE
*---------------------------------------------------------------*/
i2cEngin_master::i2cEngin_master(i2c_port_num_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num)
{
	i2c_bus_config_master.clk_source = I2C_CLK_SRC_DEFAULT;
	i2c_bus_config_master.i2c_port = i2c_port;
	i2c_bus_config_master.scl_io_num = scl_io_num;
	i2c_bus_config_master.sda_io_num = sda_io_num;
	i2c_bus_config_master.glitch_ignore_cnt = 7;
	i2c_bus_config_master.flags.enable_internal_pullup = false;

	phandler_i2c_bus = &handler_i2c_bus_master;
	assert(!i2c_new_master_bus(&i2c_bus_config_master, phandler_i2c_bus));
	this->devicesOnBus = 0;
		
	vTaskDelay(pdMS_TO_TICKS(50));
	this->xI2CMasterMutex = NULL;
	this->xI2CMasterMutex = xSemaphoreCreateMutex();
	assert(this->xI2CMasterMutex);
	xSemaphoreGive(this->xI2CMasterMutex);
	printf("%s bus has been initialised on port %d.\r\n", this->TAG, i2c_port);

}

/*---------------------------------------------------------------
 * Metoda pozwalająca sprawdzić czy urządzenie slave o adresie 	
 * i2c_address jest dostępne na szynie i2c.
 * Parameters:
 * uint8_t i2c_address	- adres i2c urządzenia slave 8-bit 
 * Returns:
 * esp_err_t ret		-  ESP_OK, ESP_FAIL, etc
*---------------------------------------------------------------*/
esp_err_t i2cEngin_master::i2cPing(uint8_t i2c_address)
{
	xSemaphoreTake(this->xI2CMasterMutex, portMAX_DELAY);
	esp_err_t ret = i2c_master_probe(*phandler_i2c_bus, i2c_address, 150);
	xSemaphoreGive(this->xI2CMasterMutex);
	if (ret == ESP_OK) {
		printf("%s, master have detected I2C slave device with address 0x%x.\r\n", this->TAG, i2c_address);
	}
	else {
		printf("%s master HAVE NOT detected I2C slave device with address 0x%x.\r\n", this->TAG, i2c_address);
	}
	return ret;
}


/*---------------------------------------------------------------
* Metoda rejestruje dodane do szyny i2c urządzenie typu slave.
* Jest to metoda pomocnicza dostarczająca dane wykorzystywane
* przy destruktorze obiektu.
* Parameters:
* NONE
* Returns:
* uint16 devicesOnBus	-  aktualna ilość urządzeń na szynie i2c
*---------------------------------------------------------------*/
uint16_t i2cEngin_master::devicesOnBusIncrement()
{
	if (this->devicesOnBus == UINT16_MAX)
		assert(0);
		#warning napisać jakąś leprzą pbsługę błędów
		//TODO:	 napisać jakąś leprzą pbsługę błędów
	else
		this->devicesOnBus++;	
	return this->devicesOnBus;
}


/*---------------------------------------------------------------
* Metoda rejestruje usunięte z szyny i2c urządzenie typu slave.
* Jest to metoda pomocnicza dostarczająca dane wykorzystywane
* przy destruktorze obiektu.
* Parameters:
* NONE
* Returns:
* uint16 devicesOnBus	-  aktualna ilość urządzeń na szynie i2c
*---------------------------------------------------------------*/
uint16_t i2cEngin_master::devicesOnBusDecrement()
{
	if (this->devicesOnBus == 0)
		assert(0);
		#warning napisać jakąś leprzą pbsługę błędów
		//TODO:	 napisać jakąś leprzą pbsługę błędów
	else
		this->devicesOnBus--;
	return this->devicesOnBus;
}

/*---------------------------------------------------------------
* Destruktor klasy odpwiadającej za komunikację ESP32 po i2c z
* urządzeniami i2c slave.
* Parameters:
* NONE
* Returns:
* NONE
*---------------------------------------------------------------*/
i2cEngin_master::~i2cEngin_master()
{
	if (this->devicesOnBus == 0) {
		xSemaphoreTake(this->xI2CMasterMutex, portMAX_DELAY);
		i2c_del_master_bus(*phandler_i2c_bus);
		vSemaphoreDelete(this->xI2CMasterMutex);
		printf("%s bus has been destructed.\r\n", this->TAG);
	}
	else
	{
		printf("%s there is  %d devices on bus. You CAN NOT use object destructor \(~i2cEngin_master\).\r\n", this->TAG, this->devicesOnBus);
		assert(0);
	}	
}