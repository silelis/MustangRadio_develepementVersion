#include "i2c_master_hw_init.h"


esp_err_t i2c_master_hw_init(i2c_port_num_t portNumber, i2c_master_bus_config_t* i2c_master_config, i2c_master_bus_handle_t* bus_handle, gpio_num_t pinSDA, gpio_num_t pinSCL, uint32_t pullUpEnable)
{
	//esp_err_t retVal;
		
	i2c_master_config->clk_source = I2C_CLK_SRC_DEFAULT;
	i2c_master_config->i2c_port = portNumber;
	i2c_master_config->scl_io_num = pinSCL;
	i2c_master_config->sda_io_num = pinSDA;
	i2c_master_config->glitch_ignore_cnt = 7;
	i2c_master_config->flags.enable_internal_pullup = pullUpEnable;
	return i2c_new_master_bus(i2c_master_config, bus_handle);
	/*if (retVal == ESP_OK)
	{
		printf("I2C master bus had beed initialized on port %d.\r\n", portNumber);
	}
	else
	{
		assert(0);
	}
	return retVal; */
}