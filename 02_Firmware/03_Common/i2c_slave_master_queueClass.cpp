#include "i2c_slave_master_queueClass.h"



/********************************************************************
 * @brief  Konstruktor obiektu
 *
 * @param  [uxQueueLength] [UBaseType_t] Długośc kolejki jaka będzie
 * 			przechowywać dane typu "i2cFrame_transmitQueue"
 *
 * @return NONE
 * @retval NONE
 * @retval NONE
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
i2cQueue4DynamicData::i2cQueue4DynamicData(UBaseType_t uxQueueLength)
{
	this->handler_Queue = NULL;
	configASSERT(this->handler_Queue = xQueueCreate(uxQueueLength, sizeof(i2cFrame_transmitQueue)));
}


/********************************************************************
 * @brief  Destruktor obiektu
 *
 *
 * @param  [uxQueueLength] [UBaseType_t] Długośc kolejki jaka będzie
 * 			przechowywać dane typu "i2cFrame_transmitQueue"

 *
 * @return NONE
 * @retval NONE
 * @retval NONE
 *
 * @note   Przed destrukcją czyszczone są dane (tworzone dynamicznie.
 * 		   Indormacja o adresach tych danych zawarta jest w poli
 * 		   *pDataposzczególnych elementów trzymanych w
 * 		   i2cFrame_transmitQueue
 * @warning NONE
 *******************************************************************/
i2cQueue4DynamicData::~i2cQueue4DynamicData(void)
{
	i2cFrame_transmitQueue tempItemToDestrouQueue;
	BaseType_t tempQueueRetVal;
	do
	{
		tempQueueRetVal = xQueueReceive(this->handler_Queue, &tempItemToDestrouQueue, pdMS_TO_TICKS(1));
		if (tempQueueRetVal == pdPASS)
		{
			this->QueueDeleteDataFromPointer(tempItemToDestrouQueue);
		}			
	} while (tempQueueRetVal == pdPASS);
	vQueueDelete(this->handler_Queue);
}


/********************************************************************
 * @brief  Usuwa dane wskazane w *pData
 *
 * [Dłuższy opis funkcji, w którym można wyjaśnić szczegóły działania,
 *  algorytmy, przypadki brzegowe itp.]
 *
 * @param  [structWithPointer] [i2cFrame_transmitQueue] struktura
 * 			2cFrame_transmitQueue (z bufora i2c), któa zawiera m.inm
 * 			adres *pData, pod którym znajdują się dynamicznie alokowane
 * 			dane, które trzeba usunąć.
 *
 * @return NONE
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
void i2cQueue4DynamicData::QueueDeleteDataFromPointer(i2cFrame_transmitQueue structWithPointer)
{
	#warning TODO: Tutaj zrobić sprawdzanie czy adres jest do zmiennej dynamicznej czy const bo dla si468x firmware może być z const
	delete[] static_cast<char*>(structWithPointer.pData);	
}


/********************************************************************
 * @brief  Pobiera dane z bufora kolejki i2c
 *
 * [Dłuższy opis funkcji, w którym można wyjaśnić szczegóły działania,
 *  algorytmy, przypadki brzegowe itp.]
 *
 * @param  [pvBuffer] [i2cFrame_transmitQueue*] Wskaźnik do zmiennej
 * 			pod którą pobrane z kolejki dane będą wpisane.
 * @param  [xTicksToWait] [TickType_t] Maksymalny czas oczekiwania na
 * 			wodczyt z kolejki danych np. gdyby kolejka była posta.
 *
 * @return [BaseType_t] Status odczytu danych z kolejki
 * @retval [np. pdPASS] Wartość zwracana w przypadku odczytania danych z kolejki
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
BaseType_t  i2cQueue4DynamicData::QueueReceive(/*void*/i2cFrame_transmitQueue * /*const*/ pvBuffer, TickType_t xTicksToWait)
{
	
	return xQueueReceive(this->handler_Queue, pvBuffer, xTicksToWait);
}


/********************************************************************
 * @brief  Wysyła odebrane z i2c slave dane do kolejki butora odbiorczego
 * i2c.
 *
 * @param  [pvItemToQueue] [i2cFrame_transmitQueue*] Wskaźnik do danych,
 * 			kóre mają być wysłane go kolejki
 *
 * @return [BaseType_t] wartośc operacji
 * @retval [pdTRUE] W przypadku gdy dane zostały zapisane w kolejce
 * 					bufora odbiorczego
 * @retval [pdFALSE] W przypadku, gdy nie udąło się zapisac danych w
 * 					kolejce.
 *
 * @note   NONE
 * @warning W przypadku, gdy nie udąło się zapisać i2cFrame_transmitQueue
 * 			w kolejce następuje automatyczne zniszczenie lokowanych
 * 			dynamicznie danych, kóre są pod adresem *pData. Dane te to dane
 * 			jakie sąw ysyłane przez slave po i2c bus.
 *******************************************************************/
BaseType_t i2cQueue4DynamicData::QueueSend(/*const*/ /*void*/i2cFrame_transmitQueue * pvItemToQueue){
	if (xQueueSend(this->handler_Queue, pvItemToQueue, pdMS_TO_TICKS(700)) == pdTRUE)
	{
		return pdTRUE;
	}
	else
	{
		this->QueueDeleteDataFromPointer(*pvItemToQueue);
		//delete[] static_cast<char*>(pointerToData);
		return pdFALSE;
	}
}

BaseType_t i2cQueue4DynamicData::QueueSendFromISR(/*const*/ /*void*/i2cFrame_transmitQueue * pvItemToQueue){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	//xQueueSendFromISR(pvItemToQueue, &xHigherPriorityTaskWoken);
	//xQueueSendFromISR(this->handler_Queue, pvItemToQueue,&xHigherPriorityTaskWoken);
	if (xQueueSendFromISR(this->handler_Queue, pvItemToQueue,&xHigherPriorityTaskWoken) == pdTRUE)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		return pdTRUE;
	}
	else
	{
		this->QueueDeleteDataFromPointer(*pvItemToQueue);
		//delete[] static_cast<char*>(pointerToData);
		return pdFALSE;
	}
}




QueueHandle_t i2cQueue4DynamicData::returnHandlerQueue(void)
{
	return this->handler_Queue;
}
