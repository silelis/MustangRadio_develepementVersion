#include "i2c_engine.h"


static i2c_master_bus_handle_t handler_i2c_bus_master;
static i2c_master_bus_config_t i2c_bus_config_master;

 
static i2c_slave_dev_handle_t handler_i2c_dev_slave;
static i2c_slave_config_t i2c_config_slave;


i2cEngin_slave::i2cEngin_slave(i2c_port_num_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num, uint32_t slave_addr, i2c_addr_bit_len_t slave_addr_bit_len)
{
	//i2c_slave_config_t i2c_slv_config = {
	i2c_config_slave.addr_bit_len =slave_addr_bit_len;	//I2C_ADDR_BIT_LEN_7;
	i2c_config_slave.clk_source = I2C_CLK_SRC_DEFAULT;
	i2c_config_slave.i2c_port = i2c_port;		//0
	i2c_config_slave.send_buf_depth = 256;
	i2c_config_slave.scl_io_num = scl_io_num;	//GPIO_NUM_22
	i2c_config_slave.sda_io_num = sda_io_num;	//GPIO_NUM_21
	i2c_config_slave.slave_addr = slave_addr;	//0x3C;
	//};
}

i2cEngin_master::i2cEngin_master(i2c_port_num_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num)
{

	i2c_bus_config_master.clk_source = I2C_CLK_SRC_DEFAULT; //,
	i2c_bus_config_master.i2c_port = i2c_port; //I2C_MASTER_PORT; //,
	i2c_bus_config_master.scl_io_num = scl_io_num;//I2C_MASTER_PIN_SCL; //,
	i2c_bus_config_master.sda_io_num = sda_io_num;//I2C_MASTER_PIN_SDA; //,
	i2c_bus_config_master.glitch_ignore_cnt = 7; //,
	i2c_bus_config_master.flags.enable_internal_pullup = false; //,

	phandler_i2c_bus = &handler_i2c_bus_master;
	assert(!i2c_new_master_bus(&i2c_bus_config_master, phandler_i2c_bus/*&handler_i2c_bus*/));
	this->devicesOnBus = 0;
		
	vTaskDelay(pdMS_TO_TICKS(50));
	this->xI2CMasterMutex = NULL;
	this->xI2CMasterMutex = xSemaphoreCreateMutex();
	assert(this->xI2CMasterMutex);
	xSemaphoreGive(this->xI2CMasterMutex);
	printf("I2C master bus has been initialised on port %d.\r\n", i2c_port);

}

esp_err_t i2cEngin_master::i2cPing(uint8_t i2c_address)
{
	xSemaphoreTake(this->xI2CMasterMutex, portMAX_DELAY);
	esp_err_t ret = i2c_master_probe(*phandler_i2c_bus/*handler_i2c_bus*/, i2c_address/*address*/, 150);
	xSemaphoreGive(this->xI2CMasterMutex);
	if (ret == ESP_OK) {
		printf("I2C slave with address 0x%x has been detected.\r\n", i2c_address);
	}
	else {
		printf("I2C slave with address 0x%x has NOT been detected.\r\n", i2c_address);
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
		printf("I2C master bus has beed destructed.\r\n");
	}
	else
	{
		printf("There is  %d devices on i2c master bus. You CAN NOT use object destructor \(~my_i2c_master\).\r\n", this->devicesOnBus);
		assert(0);
	}
	
}