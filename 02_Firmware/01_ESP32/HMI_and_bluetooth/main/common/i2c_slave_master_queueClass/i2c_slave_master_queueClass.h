#pragma once

//#include "hwConfigFile.h"
#include <stdlib.h>
#include <string.h>
//#include "comunicationProtocol.h"
//#include <iostream>

#define DEFAULT_TRANSMIT_QUEUE_SIZE	20
#define DEFAULT_RECEIVE_QUEUE_SIZE	20

#ifdef  /*TOOLCHAIN_ENVIRONMENT == __esp32__ */ ESP_PLATFORM
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "freertos/queue.h"
	#include "common/comunicationProtocol/comunicationProtocol.h"
	#include "../../hwConfigFile.h"
#elif /*TOOLCHAIN_ENVIRONMENT == __stm32__ */ __ARM_ARCH
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "comunicationProtocol.h"
#include "hwConfigFile.h"
#else
	#error "TOOLCHAIN_ENVIRONMENT which is unknown!!!!"
#endif

class i2cQueue4DynamicData {
public:
	i2cQueue4DynamicData(UBaseType_t uxQueueLength);
	~i2cQueue4DynamicData(void);

	BaseType_t QueueSend(
			i2cFrame_transmitQueue *pvItemToQueue,
			TickType_t xTicksToWait = pdMS_TO_TICKS(700));	//domyślnie 700 ms jak dotychczas; 0 = bez czekania
	BaseType_t QueueSendFromISR(
			i2cFrame_transmitQueue *pvItemToQueue);
	BaseType_t QueueReceive(i2cFrame_transmitQueue *pvBuffer,
			TickType_t xTicksToWait);
	void QueueDeleteDataFromPointer(i2cFrame_transmitQueue structWithPointer);
	UBaseType_t QueueMessagesWaiting(void);

protected:

private:
	QueueHandle_t handler_Queue;	//uchwyt do kolejki
};
