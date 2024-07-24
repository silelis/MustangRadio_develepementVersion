#include "i2c_engine.h"


static i2c_master_bus_handle_t handler_i2c_bus_master;
static i2c_master_bus_config_t i2c_bus_config_master;

 
static i2c_slave_dev_handle_t handler_i2c_dev_slave;
static i2c_slave_config_t i2c_config_slave;


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
	configASSERT(this->pTransmitQueueObject = new i2cTransmitQueue4DynamicData(DEFAULT_TRANSMIT_QUEUE_SIZE));
}

esp_err_t i2cEngin_slave::interruptRequestSet(void)
{
	return gpio_set_level(this->i2cSlave_intRequestPin, 0); //interrupt request is SET when pin is low
}

esp_err_t i2cEngin_slave::interruptRequestReset(void)
{
	return gpio_set_level(this->i2cSlave_intRequestPin, 1); //interrupt request is RESET when pin is high
}

i2cEngin_slave::~i2cEngin_slave()
{
	ESP_ERROR_CHECK(this->interruptRequestReset());
	
	ESP_ERROR_CHECK(i2c_del_slave_device(handler_i2c_dev_slave));
	printf("%s bus has been destructed.\r\n", this->TAG);
	
	//usuwanie kolejki nadawczej oraz danych, które są poinicjowane (danych, do których wskazują wskaźniki ze struktury i2cFrame_transmitQueue kolejki
	delete this->pTransmitQueueObject;
}


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


uint16_t i2cEngin_master::devicesOnBusIncrement()
{
	if (this->devicesOnBus == UINT16_MAX)
		assert(0);
	else
		this->devicesOnBus++;	
	return this->devicesOnBus;
}
uint16_t i2cEngin_master::devicesOnBusDecrement()
{
	if (this->devicesOnBus == 0)
		assert(0);
	else
		this->devicesOnBus--;
	return this->devicesOnBus;
}


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