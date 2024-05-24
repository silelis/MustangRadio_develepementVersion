#pragma once
#include "hwConfigFile.h"
#include "comunicationProtocol.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "i2c_slave_hw_init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


class i2c_slave
{
public:
	i2c_slave();
	~i2c_slave();
	void setInterruptRequest();
	void resetInterruptRequest();

protected:

private:
	int i2cSlavePort;	
	SemaphoreHandle_t xI2CSlaveMutex; //mutex which controlls access to I2C interface
	//gpio_config_t pinInterruptRequest;	

};

