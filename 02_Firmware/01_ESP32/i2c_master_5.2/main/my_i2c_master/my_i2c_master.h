#pragma once

#include "driver/i2c_master.h"
#include "i2c_master_hw_init.h"



class i2c_master
{
public:
	i2c_master(i2c_port_num_t portNumber, gpio_num_t pinSDA, gpio_num_t pinSCL, uint32_t pullUpEnable = 0);
	~i2c_master();
	//i2c_master_bus_handle_t bus_handle;
	esp_err_t i2c_ping(uint16_t address, int timeout_ms);
	static i2c_master_bus_config_t i2c_master_config;
	static i2c_master_bus_handle_t master_bus_handle;
	
		
protected:
		
private:
	
	
//		   #error semafor kontrolujący przepływy
};