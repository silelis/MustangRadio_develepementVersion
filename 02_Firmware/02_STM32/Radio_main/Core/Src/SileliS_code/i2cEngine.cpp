/*
 * i2cEngine.cpp
 *
 *  Created on: Jul 21, 2024
 *      Author: dbank
 */

//#include <i2cEngine.h>
#include "SileliS_code/i2cEngine.h"

extern myPrintfTask* pPrintf;


i2cMaster::i2cMaster(I2C_HandleTypeDef *hi2c1) {
	// TODO Auto-generated constructor stub
	this->p_hi2c1 =hi2c1;
	this->pI2C_MasterReceiveFromSlave_DataQueue = nullptr;
	this->pI2C_MasterTransmitToSlave_DataQueue = nullptr;
	this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue = nullptr;



	configASSERT(this->pI2C_MasterReceiveFromSlave_DataQueue = new i2cQueue4DynamicData(DEFAULT_RECEIVE_QUEUE_SIZE));
	configASSERT(this->pI2C_MasterTransmitToSlave_DataQueue = new i2cQueue4DynamicData(DEFAULT_RECEIVE_QUEUE_SIZE));
	configASSERT(this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue = new i2cQueue4DynamicData(DEFAULT_RECEIVE_QUEUE_SIZE));

	assert(this->handle_i2cBinarySemaphore = xSemaphoreCreateBinary());
	this->i2cMasterSemaphoreGive();
	this->i2cMasterSemaphoreTake();

	pPrintf->feedPrintf("%s bus had been initialized.",this->TAG);
	this->i2cMasterSemaphoreGive();
}

/********************************************************************
 * @brief  Blokuje dostęp do szyny i2c np. podczas czytania lub pisania
 *  przez jakiś task.
 *
 * [Należy użyć w każdym tasku przed rozpoczęciem zadań związanych z
 * fizyczną komunikacją po szyni ei2c]
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
BaseType_t i2cMaster::i2cMasterSemaphoreTake(void){
	return xSemaphoreTake(this->handle_i2cBinarySemaphore, portMAX_DELAY);
}

/********************************************************************
 * @brief  Oddaje dostęp do szyny i2c np. po czas czytaniu lub pisaniu
 *  przez jakiś task.
 *
 * [Należy użyć w każdym tasku po zakończeniu zadań związanych z
 * fizyczną komunikacją po szyni ei2c]
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
BaseType_t i2cMaster::i2cMasterSemaphoreGive(void){
	return xSemaphoreGive(this->handle_i2cBinarySemaphore);
}

/********************************************************************
 * @brief  Odpowiednik pinga (z ethernet) dla i2c.
 *
 * [NONE]
 *
 * @param  uint16_t DevAddress_7bit - adres urządzenia i2c w notacji
 * 					7bit
 *
 * @return HAL_StatusTypeDef retVal - HAL_OK dla wykrytego urządzenia
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
HAL_StatusTypeDef i2cMaster::ping(uint16_t DevAddress_7bit){
	HAL_StatusTypeDef retVal;
	retVal = HAL_I2C_IsDeviceReady(this->p_hi2c1, DevAddress_7bit<<1, 100, 1000);
	if(retVal==HAL_OK){
			pPrintf->feedPrintf("%s i2c slave avaliable on address: 0x%x (7bit).", this->TAG, DevAddress_7bit/*<<1*/);
	}
	else{
		pPrintf->feedPrintf("%s i2c slave NOT avaliable on address: 0x%x (7bit).", this->TAG, DevAddress_7bit/*<<1*/);
		assert(0);
	}
	return retVal;
}


/********************************************************************
 * @brief  Ustawia addres i2c urządzenia slave z jakiego mają być
 * czytane dane. Adres wpisywany jest do klejki, ktora przekazuje
 * informacje do taska odpowiedzialnego za odczyt danych ze slave.
 *
 * [Taskiem czytającym dane ze slave jest ten uruchamiający funckę
 * static void i2cMasterReceiveFromSlaveDataTask(void *pNothing)]
 *
 * @param  uint16_t DevAddress_7bit - adres urządzenia i2c w notacji
 * 					7bit
 *
 * @return BaseType_t - pdTRUE jeśłi udało się wpisać dane do kolejni
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
BaseType_t  i2cMaster::setI2cAdressToAdressQueue(uint16_t DevAddress_7bit){
	i2cFrame_transmitQueue I2CFrameWithAdressOfSlaveToRead;
	I2CFrameWithAdressOfSlaveToRead.slaveDevice7bitAddress = DevAddress_7bit;
	return this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue->QueueSendFromISR(&I2CFrameWithAdressOfSlaveToRead);
}



/********************************************************************
 * @brief  W tasku odpowiedzialnym za odczyt danych z i2c slave
 * obsługującym funkcję:
 * static void i2cMasterReceiveFromSlaveDataTask(void *pNothing)pobiera
 * z kolejki adres i2c slave z jakiego należy odczytać dane.
 *
 * [NONE]
 *
 * @return BaseType_t - pdTRUE jeśłi udało się odczytać dane do kolejni
 *
 * @note   NONE
 * @warning i2cFrame_transmitQueue* I2CFrameWithAdressOfSlaveToRead -
 * 						jest to wskaźnik do zmiennej przechwującej adres
 * 						i2c salve
 *******************************************************************/
//w tasku odpowiedzialnym za odczyt danych z i2c slave pobiera adres z kolejki
BaseType_t  i2cMaster::getI2cAdressFromAdressQueue(i2cFrame_transmitQueue* I2CFrameWithAdressOfSlaveToRead){
	return this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue->QueueReceive(I2CFrameWithAdressOfSlaveToRead, portMAX_DELAY);
}


//metoda przesuwa otrzymane z i2c slave dane to kolejki taska parsera
BaseType_t  i2cMaster::passReceivedI2cDataToParsingQueue(i2cFrame_transmitQueue* I2CFrameToParsingTask){
	return this->pI2C_MasterReceiveFromSlave_DataQueue->QueueSend(I2CFrameToParsingTask);
}

//metoda pozwala na pobranie z kolejki parsera (przez taks parsera)  danych do parsowania
BaseType_t  i2cMaster::takeReceivedI2cDataFromParsingQueue(i2cFrame_transmitQueue* I2CFrameToParsing){
	return this->pI2C_MasterReceiveFromSlave_DataQueue->QueueReceive(I2CFrameToParsing, portMAX_DELAY);
}

//metoda pobiera z kolejki danych do transmisji so I2C slave dane jakie mają być wysłane
BaseType_t  i2cMaster::takeTransmitI2cDataToI2C_buffer(i2cFrame_transmitQueue* I2CBufferToTransmit){
	return this->pI2C_MasterTransmitToSlave_DataQueue->QueueReceive(I2CBufferToTransmit, portMAX_DELAY);
}



//po zakończeniu parsowania należy zwolnić dynamicznie alokowane dane
void		i2cMaster::deleteAlocatedDataAfterParsing(i2cFrame_transmitQueue I2CFrameWithPointerToDelete){
	this->pI2C_MasterReceiveFromSlave_DataQueue->QueueDeleteDataFromPointer(I2CFrameWithPointerToDelete);
}


i2cMaster::~i2cMaster() {
	// TODO Auto-generated destructor stub
	this->i2cMasterSemaphoreTake();
	this->p_hi2c1 = NULL;
	delete this->pI2C_MasterReceiveFromSlave_DataQueue;
	delete this->pI2C_MasterTransmitToSlave_DataQueue;
	delete this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue;
	vSemaphoreDelete(this->handle_i2cBinarySemaphore);
}

void i2cMaster::while_I2C_STATE_READY(void){
	while(HAL_I2C_GetState(this->p_hi2c1)!= HAL_I2C_STATE_READY){};
}

HAL_StatusTypeDef i2cMaster::I2C_Master_Receive_DMA(uint16_t DevAddress_7bit, uint8_t *pData, uint16_t Size){
	//HAL_StatusTypeDef retVal;
	this->while_I2C_STATE_READY();
	return HAL_I2C_Master_Receive_DMA(this->p_hi2c1, DevAddress_7bit<<1, pData, Size);

	//return retVal;
}

HAL_StatusTypeDef i2cMaster::I2C_Master_Transmite_DMA(uint16_t DevAddress_7bit, uint8_t *pData, uint16_t Size){
	//HAL_StatusTypeDef retVal;
	this->while_I2C_STATE_READY();
	return HAL_I2C_Master_Transmit_DMA(this->p_hi2c1, DevAddress_7bit<<1, pData, Size);
}



i2cQueue4DynamicData* i2cMaster::getTransmitQueue(){
	return pI2C_MasterTransmitToSlave_DataQueue;
}
