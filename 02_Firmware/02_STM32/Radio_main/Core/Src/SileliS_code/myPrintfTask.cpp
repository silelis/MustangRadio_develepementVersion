/*
 * myPrintfTask.cpp
 *
 *  Created on: Sep 21, 2024
 *      Author: dbank
 */

#include "SileliS_code/myPrintfTask.h"


myPrintfTask::myPrintfTask(UART_HandleTypeDef* huart, UBaseType_t printfTaskQueueLength):i2cQueue4DynamicData(printfTaskQueueLength) {
	// TODO Auto-generated constructor stub
	this->pHuart = huart;
}

myPrintfTask::~myPrintfTask() {
	// TODO Auto-generated destructor stub
}


// Funkcja myPrintf, która zapisuje sformatowany tekst pod wskaźnikiem
BaseType_t myPrintfTask::feedPrintf(const char *format, ...) {

	BaseType_t retVal = pdFALSE;
	i2cFrame_transmitQueue itemToPrint;

	itemToPrint.slaveDevice7bitAddress=NULL;
	va_list args;
    va_start(args, format);

    // Najpierw określamy długość wynikowego sformatowanego ciągu
    itemToPrint.dataSize=vsnprintf(NULL, 0, format, args)+1;
    // Resetujemy va_list, aby móc go użyć ponownie
    va_end(args);
    va_start(args, format);

    // Alokujemy odpowiednią ilość pamięci
   // char *buffer = (char*)malloc((length) * sizeof(char));
    char *buffer = new char[itemToPrint.dataSize];
    assert(buffer);

    if (buffer != NULL) {
        // Właściwe formatowanie do bufora
        vsnprintf(buffer, itemToPrint.dataSize, format, args);
        itemToPrint.pData = buffer;
        retVal =this->QueueSend/*FromISR*/(&itemToPrint);
    }

    // Zwalniamy zasoby
    va_end(args);

    return retVal;  // Zwracamy wskaźnik na sformatowany ciąg
}

HAL_StatusTypeDef myPrintfTask::myPrintf(i2cFrame_transmitQueue itemToPrint) {
	//HAL_UART_Transmit_DMA(this->pHuart, (const uint8_t*) itemToPrint.pData, itemToPrint.dataSize);
	HAL_UART_Transmit(this->pHuart, (const uint8_t*) itemToPrint.pData, itemToPrint.dataSize, 150);
	HAL_UART_Transmit(this->pHuart, (const uint8_t*) "\r\n", 2, 150);
	//HAL_UART_Transmit_DMA(this->pHuart, (const uint8_t*) "\r\n", 2);

	this->QueueDeleteDataFromPointer(itemToPrint);
}
