#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"
#include "driver/i2c_types.h"

//#include "chip_config_ESP32_WROOM32D_DEVBOARD.h"


class i2cEngin_slave
{
public:
	i2cEngin_slave(i2c_port_num_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num, gpio_num_t interrupt_request_io_num);
	~i2cEngin_slave();
protected:
	
	
private:
	
	
	
};
	

class i2cEngin_master {
public:
	i2cEngin_master(i2c_port_num_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num);
	~i2cEngin_master();
	esp_err_t i2cPing(uint8_t i2c_address);
	i2c_master_bus_handle_t* phandler_i2c_bus;
	uint16_t devicesOnBusIncrement();
	uint16_t devicesOnBusDecrement();
	
protected:
	

	
private:
	uint16_t devicesOnBus;
	
	SemaphoreHandle_t xI2CMasterMutex; //mutex which controlls access to I2C
};