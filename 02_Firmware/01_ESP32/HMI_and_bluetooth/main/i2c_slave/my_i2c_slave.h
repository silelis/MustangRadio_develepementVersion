#pragma once
#include "hwConfigFile.h"
#include "comunicationProtocol.h"
#include "driver/i2c.h"
#include "my_i2c_slave_hw_init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class i2c_slave
{
public:
	i2c_slave();
	~i2c_slave();

private:
	int i2cSlavePort;	
	SemaphoreHandle_t xI2CSlaveMutex; //mutex which controlls access to I2C interface
};

