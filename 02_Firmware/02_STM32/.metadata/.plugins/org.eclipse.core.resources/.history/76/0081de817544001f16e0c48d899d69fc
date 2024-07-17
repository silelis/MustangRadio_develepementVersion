#pragma once

//#include "hwConfigFile.h"
#include <stdlib.h>
#include <string.h>
//#include "comunicationProtocol.h"
//#include <iostream>

#ifdef  /*TOOLCHAIN_ENVIRONMENT == __esp32__ */ ESP_PLATFORM
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "freertos/queue.h"
//	#include "./../../../03_Common/comunicationProtocol.h"
#include "../03_Common/comunicationProtocol.h"
#elif /*TOOLCHAIN_ENVIRONMENT == __stm32__ */ __ARM_ARCH


#else
#error "TOOLCHAIN_ENVIRONMENT which is unknown!!!!"
#endif

class i2cTraRecQueue4DynamicData
{
public:
	i2cTraRecQueue4DynamicData(UBaseType_t uxQueueLength/*,   UBaseType_t uxItemSize*/);
	~i2cTraRecQueue4DynamicData(void);
	esp_err_t /*transmit*/QueueSend(const void * pvItemToQueue, size_t itemSize);
	esp_err_t  QueueReceive(void * const pvBuffer, TickType_t xTicksToWait);
	void QueueDeleteDataFromPointer(i2cFrame_transmitQueue structWithPointer);	
	
protected:

private:
	QueueHandle_t handler_transmitQueue;
	
	
};