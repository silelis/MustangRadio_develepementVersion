/*
 * i2cEngine.h
 *
 *  Created on: Jul 21, 2024
 *      Author: dbank
 */

#ifndef INC_I2CENGINE_H_
#define INC_I2CENGINE_H_

#include "i2c.h"
#include "printfRedirect.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "i2c_slave_master_queueClass.h"
#include "SileliS_code/myPrintfTask.h"

class i2cMaster {
public:
	i2cMaster(I2C_HandleTypeDef *hi2c1, GPIO_TypeDef *sclGpioPort, uint16_t sclPin,
			GPIO_TypeDef *sdaGpioPort, uint16_t sdaPin);	//piny SCL/SDA - tylko do diagnostyki stanu linii i2c (waitForTransferEnd)
	virtual ~i2cMaster();
	HAL_StatusTypeDef ping(uint16_t DevAddress_7bit);
	BaseType_t setI2cAdressToAdressQueue(uint16_t DevAddress_7bit);
	BaseType_t getI2cAdressFromAdressQueue(
			i2cFrame_transmitQueue *I2CFrameWithAdressOfSlaveToRead);

	BaseType_t passReceivedI2cDataToParsingQueue(
			i2cFrame_transmitQueue *I2CFrameToParsingTask);
	BaseType_t takeReceivedI2cDataFromParsingQueue(
			i2cFrame_transmitQueue *I2CFrameToParsing);
	BaseType_t takeTransmitI2cDataToI2C_buffer(
			i2cFrame_transmitQueue *I2CBufferToTransmit);
	void deleteAlocatedDataAfterParsing(
			i2cFrame_transmitQueue I2CFrameWithPointerToDelete);

	i2cQueue4DynamicData* getTransmitQueue();

	//i2cQueue4DynamicData* pI2C_MasterTransmitToSlave_DataQueue;			//kolejka przechowująca dane jakie mają zostać wysłane do slave i2c

	HAL_StatusTypeDef I2C_Master_Receive_DMA(uint16_t DevAddress_7bit,
			uint8_t *pData, uint16_t Size);
	HAL_StatusTypeDef I2C_Master_Transmite_DMA(uint16_t DevAddress_7bit,
			uint8_t *pData, uint16_t Size);
	HAL_StatusTypeDef I2C_Master_Abort(uint16_t DevAddress_7bit);	//przerywa trwającą transmisję/odbiór master (np. po timeoucie)
	BaseType_t i2cMasterSemaphoreTake(void);
	BaseType_t i2cMasterSemaphoreGive(void);

protected:
	i2cQueue4DynamicData *pI2C_MasterInitialiseReadFromSlave_AdressessQueue; //kolejka przechowująca informacje o tym, z ktorego slave należy przeczytać dane (przechowuje adres i2c slave do odczytu)
	i2cQueue4DynamicData *pI2C_MasterReceiveFromSlave_DataQueue;//kolejka przechowująca otrzymane z i2c slave dane, ktore przekazywane są do parsowania
	i2cQueue4DynamicData *pI2C_MasterTransmitToSlave_DataQueue;	//kolejka przechowująca dane jakie mają zostać wysłane do slave i2c

private:
	I2C_HandleTypeDef *p_hi2c1;	//adres interfejsu i2c po którym odbywa się komunikacja
	GPIO_TypeDef *p_sclGpioPort;	//port pinu SCL interfejsu p_hi2c1 - odczyt stanu linii przy diagnostyce zawieszonej transmisji
	uint16_t sclPin;				//pin SCL (j.w.)
	GPIO_TypeDef *p_sdaGpioPort;	//port pinu SDA interfejsu p_hi2c1 - odczyt stanu linii przy diagnostyce zawieszonej transmisji
	uint16_t sdaPin;				//pin SDA (j.w.)
	const char *TAG = "I2C master log:";
	SemaphoreHandle_t handle_i2cBinarySemaphore;//semafor blokujący zasoby do interfejsu i2c SMT'a podczas transmisji/ odbioru danych przez inny wątek
	HAL_StatusTypeDef waitForTransferEnd(uint16_t DevAddress_7bit);	//czeka na koniec bieżącej transmisji DMA - wywoływana WYŁĄCZNIE wewnątrz metod transmisji, PO starcie DMA
	//void createI2C_DataObject(i2cQueue4DynamicData* queueToCreate);		//tworzy kolejkę przechowującą dane

};

#endif /* INC_I2CENGINE_H_ */
