#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"

#include "chip_config_ESP32_WROOM32D_DEVBOARD.h"


class my_i2c_master{
public:
	my_i2c_master();
	~my_i2c_master();
	esp_err_t i2cPing(uint8_t i2c_address);
	
protected:
	i2c_master_bus_handle_t* phandler_i2c_bus;
	uint16_t devicesOnBusIncrement();
	uint16_t devicesOnBusDecrement();
	
private:
	uint16_t devicesOnBus;
	
	SemaphoreHandle_t xI2CMasterMutex; //mutex which controlls access to I2C
};