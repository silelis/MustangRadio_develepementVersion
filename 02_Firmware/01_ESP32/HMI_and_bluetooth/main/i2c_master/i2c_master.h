#pragma once

#include "esp_log.h"
#include "driver/i2c.h"


#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class i2cMaster
{
public:
	i2cMaster(int pinSDA, int pinSCL, uint32_t i2cSpeed, size_t rxBuffLen, size_t txBuffLen);
	~i2cMaster();
	esp_err_t i2cPing(uint8_t i2c_address);
	esp_err_t i2cWriteData(i2c_cmd_handle_t cmd_handle); //TODO: przenieœæ tê funkcjê do protected
	
	
protected:
	uint8_t geti2cInstanceCounterState();
	
private:
	const char *TAG = "I2C master log:";
	static uint8_t i2cInstanceCounter; //thisone should be initioalised to "0" at !!!very top!!!

	static int i2cMasterPort;
	static i2c_config_t i2cConfig;
	static SemaphoreHandle_t xI2CMasterMutex;		//mutex which controlls access to I2C interface
};