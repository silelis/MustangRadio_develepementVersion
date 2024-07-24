#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"
#include "driver/i2c_types.h"
#include "driver/gpio.h"
#include "hwConfigFile.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "./../../../03_Common/comunicationProtocol.h"
#include "./../../../03_Common/i2c_slave_master_queueClass.h"


//#include "chip_config_ESP32_WROOM32D_DEVBOARD.h"


class i2cEngin_slave
{
public:
	i2cEngin_slave(i2c_port_num_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num, uint32_t slave_addr, i2c_addr_bit_len_t slave_addr_bit_len, gpio_num_t intRequestPin);
	~i2cEngin_slave();
	esp_err_t interruptRequestSet(void);
	esp_err_t interruptRequestReset(void);
	//esp_err_t transmitQueueSend(const void * pvItemToQueue, size_t itemSize);
	esp_err_t slaveTransmit();
	i2cTransmitQueue4DynamicData* pTransmitQueueObject;
protected:
	
	
private:
	const char *TAG = "I2C SLAVE log:";
	gpio_num_t i2cSlave_intRequestPin;
	//QueueSetHandle_t handler_i2cSlaveQueueSet;
	//QueueHandle_t handler_transmitQueue;
	//const UBaseType_t transmitQueueSize = 20;
	const int tx_timeout_ms = 500;
	//void transmitQueueDestruct(void);
	//void transmitQueueDeleteDataFromPointer(i2cFrame_transmitQueue structWithPointer);
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
	const char *TAG = "I2C MASTER log:";
	SemaphoreHandle_t xI2CMasterMutex; //mutex which controlls access to I2C
};