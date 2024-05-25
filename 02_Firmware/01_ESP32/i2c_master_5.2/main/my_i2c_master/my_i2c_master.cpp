#include "my_i2c_master.h"

//static i2c_master_bus_config_t i2c_master_config;
//static i2c_master_bus_handle_t master_bus_handle;


i2c_master_bus_config_t i2c_master::i2c_master_config;
i2c_master_bus_handle_t i2c_master::master_bus_handle;

i2c_master::i2c_master(i2c_port_num_t portNumber, gpio_num_t pinSDA, gpio_num_t pinSCL, uint32_t pullUpEnable)
{	//esp_err_t retVal;
	//retVal = i2c_master_hw_init(portNumber, &i2c_master_config, &master_bus_handle, pinSDA, pinSCL, pullUpEnable);
	
	if (i2c_master_hw_init(portNumber, &i2c_master_config, &master_bus_handle, pinSDA, pinSCL, pullUpEnable) == ESP_OK)
	{
		printf("I2C master bus had beed initialized on port %d.\r\n", portNumber);
	}
	else
	{
		assert(0);
	}
}

/*esp_err_t i2c_master::i2c_ping(uint16_t address, int timeout_ms)
{
	
}  */

i2c_master::~i2c_master()
{
	assert(!i2c_del_master_bus(master_bus_handle));
	printf("I2C master bus had beed deinitialized.\r\n");
}