/*
 * myPrintfTask.h
 *
 *  Created on: Sep 21, 2024
 *      Author: dbank
 */

#ifndef INC_MYPRINTFTASK_H_
#define INC_MYPRINTFTASK_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "comunicationProtocol.h"
#include "i2c_slave_master_queueClass.h"
#include "usart.h"

class myPrintfTask: public i2cQueue4DynamicData {
public:
	myPrintfTask(UART_HandleTypeDef* huart, UBaseType_t printfTaskQueueLength);
	virtual ~myPrintfTask();
	BaseType_t feedPrintf(const char *format, ...);
	HAL_StatusTypeDef myPrintf(i2cFrame_transmitQueue itemToPrint);


private:
	UART_HandleTypeDef* pHuart;
};

#endif /* INC_MYPRINTFTASK_H_ */
