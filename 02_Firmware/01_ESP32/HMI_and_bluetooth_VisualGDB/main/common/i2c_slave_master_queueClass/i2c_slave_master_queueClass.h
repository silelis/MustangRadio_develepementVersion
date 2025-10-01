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
	#include "D:\!!!__GitHUB_repositories\MustangRadio_develepementVersion\02_Firmware\01_ESP32\HMI_and_bluetooth\main\common\comunicationProtocol\comunicationProtocol.cpp"
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

class i2cQueue4DynamicData
{
public:
	i2cQueue4DynamicData(UBaseType_t uxQueueLength);
	~i2cQueue4DynamicData(void);
	
	BaseType_t QueueSend(/*const*/ /*void*/i2cFrame_transmitQueue * pvItemToQueue);
	BaseType_t QueueSendFromISR(/*const*/ /*void*/i2cFrame_transmitQueue * pvItemToQueue);
	BaseType_t  QueueReceive(/*void*/i2cFrame_transmitQueue * /*const*/ pvBuffer, TickType_t xTicksToWait);
	void QueueDeleteDataFromPointer(i2cFrame_transmitQueue structWithPointer);	
	UBaseType_t QueueMessagesWaiting(void);

protected:


private:
	QueueHandle_t handler_Queue;	//uchwyt do kolejki
};
