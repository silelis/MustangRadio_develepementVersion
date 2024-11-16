#include "i2c_engine.h"

static i2c_master_bus_handle_t handler_i2c_bus_master;
static i2c_master_bus_config_t i2c_bus_config_master;
 
static i2c_slave_dev_handle_t handler_i2c_dev_slave;
static i2c_slave_config_t i2c_config_slave;




#include "soc/i2c_struct.h"
extern i2c_dev_t I2C0;

uint32_t rx_fifo_end_addrLast;
enum i2cCallbackState
{
	recpeptionNotToMe,
	recpeptionToMe,
	transmition
};
enum i2cCallbackState rxToEsp32 = recpeptionNotToMe;

static uint8_t data_rd[ESP32_SLAVE_RECEIVE_BUFFER_LEN];

static IRAM_ATTR bool i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *edata, void *user_data)
{	
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;
	xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);

	//if (I2C0.int_status.trans_complete == 0)
	if (I2C0.status_reg.slave_rw == 0)
	{
		if (rx_fifo_end_addrLast != I2C0.fifo_st.rx_fifo_end_addr)
		{
			rxToEsp32 = recpeptionToMe;
		}
		else
		{
			rxToEsp32 = recpeptionNotToMe;
		}
		
	}
	else
	{
		rxToEsp32 = transmition;	
	}
	rx_fifo_end_addrLast = I2C0.fifo_st.rx_fifo_end_addr;
	
	/*if (high_task_wakeup == pdTRUE) {
		// Wybudź zadanie o wyższym priorytecie
		portYIELD_FROM_ISR(high_task_wakeup);
	}	  */
	return high_task_wakeup == pdTRUE;
}





/*
static IRAM_ATTR bool i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *edata, void *user_data)
{
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;
	xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
	return high_task_wakeup == pdTRUE;
}
  */


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
	i2c_config_slave.send_buf_depth = 3*ESP32_SLAVE_RECEIVE_BUFFER_LEN; //1024;
	
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
	//this->pTransmitQueueObject = NULL;
	//configASSERT(this->pTransmitQueueObject = new i2cQueue4DynamicData(DEFAULT_TRANSMIT_QUEUE_SIZE));
	
	//Tworzenie kolejki odbiorczej
	configASSERT(this->s_receive_queue = xQueueCreate(10, sizeof(i2c_slave_rx_done_event_data_t))) ;
	i2c_slave_event_callbacks_t cbs = {
		.on_recv_done = i2c_slave_rx_done_callback,
	};
	
	rx_fifo_end_addrLast = I2C0.fifo_st.rx_fifo_end_addr;
	
	ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(handler_i2c_dev_slave, &cbs, this->s_receive_queue));
	printf("%s bus has been initialised on port %d with address %lx.\n", this->TAG, i2c_port, slave_addr);

}


void i2cEngin_slave::i2cSlaveReceive(void)
{
	//uint8_t *data_rd = (uint8_t *) malloc(ESP32_SLAVE_RECEIVE_BUFFER_LEN);
	//uint8_t *data_rd =  new uint8_t[ESP32_SLAVE_RECEIVE_BUFFER_LEN];
	
	memset(data_rd, 0, ESP32_SLAVE_RECEIVE_BUFFER_LEN);
	uint32_t size_rd = 0;
	i2c_slave_rx_done_event_data_t rx_data;
	ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, 6));
	this->esp32i2cBusInitialised(); //informuje i2c master poprzez pierwsze interrupt request, że szyna i2c jest zainicjowana
	while (1)
	{
		memset(data_rd, 0, ESP32_SLAVE_RECEIVE_BUFFER_LEN);
		
		if (xQueueReceive(this->s_receive_queue, &rx_data, portMAX_DELAY)==pdTRUE)
		{
			ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN));
			if (rxToEsp32 == recpeptionToMe)
			{
				printf("%s\n", data_rd);
			}
			else if (rxToEsp32 == recpeptionNotToMe)
			{
				printf("Not to me\n");	
			}
			else if (rxToEsp32 ==transmition)
			{
				printf("T\n");	 
			}
			//ESP_ERROR_CHECK(i2c_slave_receive(handler_i2c_dev_slave, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN));
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
	//delete this->pTransmitQueueObject;
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
//esp_err_t i2cEngin_slave::slaveTransmit()
//{
//	esp_err_t retVal =ESP_FAIL;
//	i2cFrame_transmitQueue ItemWithPointerToTransmit;
//	
//	if (pdPASS == this->pTransmitQueueObject->QueueReceive(&ItemWithPointerToTransmit, portMAX_DELAY)) //kolejka zawiera dane;
//	{
//		
//		
//		retVal = this->slaveTransmit(ItemWithPointerToTransmit);
//						/*
//		retVal= i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t*) &ItemWithPointerToTransmit.dataSize, sizeof(ItemWithPointerToTransmit.dataSize), this->tx_timeout_ms);
//		if (ESP_OK == retVal)
//		{
//			retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t*) ItemWithPointerToTransmit.pData, ItemWithPointerToTransmit.dataSize, this->tx_timeout_ms);
//		}
//		
//		this->interruptRequestSet();
//		this->interruptRequestReset();
//		this->pTransmitQueueObject->QueueDeleteDataFromPointer(ItemWithPointerToTransmit);		*/
//			
//	}
//	return retVal;	
//}



esp_err_t i2cEngin_slave::slaveTransmit(i2cFrame_transmitQueue ItemWithPointer)
{
	esp_err_t retVal = ESP_FAIL;
	retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t*) &ItemWithPointer.dataSize, sizeof(ItemWithPointer.dataSize), this->tx_timeout_ms);
	if (ESP_OK == retVal)
	{
		retVal = i2c_slave_transmit(handler_i2c_dev_slave, (const uint8_t*) ItemWithPointer.pData, ItemWithPointer.dataSize, this->tx_timeout_ms);
	}
		
	this->interruptRequestSet();
	this->interruptRequestReset();
	delete[] static_cast<char*>(ItemWithPointer.pData);
	return retVal;
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

