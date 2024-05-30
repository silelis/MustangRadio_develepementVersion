#include "my_i2c_master.h"


static i2c_master_bus_handle_t handler_i2c_bus;
static i2c_master_bus_config_t i2c_bus_config;

my_i2c_master::my_i2c_master()
{

	i2c_bus_config.clk_source = I2C_CLK_SRC_DEFAULT; //,
	i2c_bus_config.i2c_port = I2C_MASTER_PORT; //,
	i2c_bus_config.scl_io_num = I2C_MASTER_PIN_SCL; //,
	i2c_bus_config.sda_io_num = I2C_MASTER_PIN_SDA; //,
	i2c_bus_config.glitch_ignore_cnt = 7; //,
	i2c_bus_config.flags.enable_internal_pullup = false; //,

	phandler_i2c_bus = &handler_i2c_bus;
	assert(!i2c_new_master_bus(&i2c_bus_config, phandler_i2c_bus/*&handler_i2c_bus*/));
	this->devicesOnBus = 0;
		
	vTaskDelay(pdMS_TO_TICKS(50));
	this->xI2CMasterMutex = NULL;
	this->xI2CMasterMutex = xSemaphoreCreateMutex();
	assert(this->xI2CMasterMutex);
	xSemaphoreGive(this->xI2CMasterMutex);
	printf("I2C master bus has been initialised on port %d.\r\n", I2C_MASTER_PORT);

}

esp_err_t my_i2c_master::i2cPing(uint8_t i2c_address)
{
	xSemaphoreTake(this->xI2CMasterMutex, portMAX_DELAY);
	esp_err_t ret = i2c_master_probe(*phandler_i2c_bus/*handler_i2c_bus*/, i2c_address/*address*/, 50);
	xSemaphoreGive(this->xI2CMasterMutex);
	if (ret == ESP_OK) {
		printf("I2C slave with address 0x%x has been detected.\r\n", i2c_address);
	}
	else {
		printf("I2C slave with address 0x%x has NOT been detected.\r\n", i2c_address);
	}
	return ret;
}


uint16_t my_i2c_master::devicesOnBusIncrement()
{
	if (this->devicesOnBus == UINT16_MAX)
		assert(0);
	else
		this->devicesOnBus++;	
	return this->devicesOnBus;
}
uint16_t my_i2c_master::devicesOnBusDecrement()
{
	if (this->devicesOnBus == 0)
		assert(0);
	else
		this->devicesOnBus--;
	
	return this->devicesOnBus;
}


my_i2c_master::~my_i2c_master()
{
	if (this->devicesOnBus == 0) {
		xSemaphoreTake(this->xI2CMasterMutex, portMAX_DELAY);
		i2c_del_master_bus(*phandler_i2c_bus);
		vSemaphoreDelete(this->xI2CMasterMutex);
		printf("I2C master bus has beed destructed.\r\n");
	}
	else
	{
		printf("There is  %d devices on i2c master bus. You CAN NOT use object destructor \(~my_i2c_master\).\r\n", this->devicesOnBus);
		assert(0);
	}
	
}