#pragma once

//#include "hwConfigFile.h"
#include <stdlib.h>
#include <string.h>
//#include "comunicationProtocol.h"
//#include <iostream>

#define DEFAULT_TRANSMIT_QUEUE_SIZE	20
#define DEFAULT_RECEIVE_QUEUE_SIZE	25

#ifdef  /*TOOLCHAIN_ENVIRONMENT == __esp32__ */ ESP_PLATFORM
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "freertos/queue.h"
//	#include "./../../../03_Common/comunicationProtocol.h"
	#include "../03_Common/comunicationProtocol.h"
	#include "hwConfigFile.h"
#elif /*TOOLCHAIN_ENVIRONMENT == __stm32__ */ __ARM_ARCH
	#include "FreeRTOS.h"
	#include "task.h"
	#include "queue.h"
	#include "comunicationProtocol.h"
	#include "hwConfigFile.h"
#else
	#error "TOOLCHAIN_ENVIRONMENT which is unknown!!!!"
#endif

class i2cQueue4DynamicData
{
public:
	i2cQueue4DynamicData(UBaseType_t uxQueueLength);
	~i2cQueue4DynamicData(void);
	#ifdef I2C_STM32_TO_ESP32_ROLE_MASTER
		BaseType_t QueueSend(const void * pvItemToQueue);
	#else
		BaseType_t QueueSend(const void * pvItemToQueue, size_t itemSize);
	#endif
	BaseType_t  QueueReceive(void * const pvBuffer, TickType_t xTicksToWait);
	void QueueDeleteDataFromPointer(i2cFrame_transmitQueue structWithPointer);	

protected:


private:
	QueueHandle_t handler_Queue;	//uchwyt do kolejki

};