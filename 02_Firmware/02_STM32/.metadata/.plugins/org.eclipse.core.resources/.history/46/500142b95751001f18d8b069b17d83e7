/*
 * tasksFunctions.c
 *
 *  Created on: Jul 13, 2024
 *      Author: dbank
 */


#include "tasksFunctions.h"
#include "comunicationProtocol.h"
#include "comunicationStructures.h"
#include "comunication_calculate_checksum.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "i2c.h"
#include <cstring>
#include "semphr.h"
#include "i2cEngine.h"
#include "esp32i2sComunicationDriver.h"

static TaskHandle_t taskHandle_esp32IntrrruptRequest = NULL;		//uchwyt taska obsługującego komunikację (odczytywanie danych) z esp32, po pojawieniu się sygnału esp32 interrupt request
static i2cMaster* pi2cMaster=NULL;  					//wsyaźnik do obiektu służącego do komunikacji stm32 po i2c jako master
static esp32_i2sComunicationDriver* pESP32=NULL; 		//wsyaźnik do obiektu obsługującego komunikację z ESP32



static SemaphoreHandle_t esp32IntrrruptRequest_CountingSemaphore;	//////////////////// 	//uchwyt semafora zliczającego ilość wsytąpień esp32 interrupt request i ilość odczytów danych z esp32
static BaseType_t esp32InrerruptRequest_CountingSemaphoreOverflow=pdFALSE;	//zmienna informująca o tym, że nastąpiło przepełnienie "esp32IntrrruptRequest_CountingSemaphore", aka. zbyt wiele oczekujących komunikatów, co może wskazywać na błąd.


static void esp32IntrrruptRequestCallback(void *pNothing){
	i2cFrame_transmitQueue tempI2CFrameReceivedFromESP32;
	tempI2CFrameReceivedFromESP32.slaveDevice7bitAddress = pESP32->get_i2cSlaveAddress_7bit();		//I2C_SLAVE_ADDRESS_ESP32;
	char* pdymanicDataPointer;
	while(1){
		//pdymanicDataPointer = NULL
		if( uxSemaphoreGetCount(esp32IntrrruptRequest_CountingSemaphore)==ESP32_INTERRUPT_REQUEST_COUNTING_SEMAPHORE_MAX){		//sprawdza czy licznik esp32 interrupt request nie jest przepełniony
			esp32InrerruptRequest_CountingSemaphoreOverflow = pdTRUE;
			printf("!!! ESP32 interrupt request counter overflowed   !!!\r\n");
		}
		if (xSemaphoreTake(esp32IntrrruptRequest_CountingSemaphore, portMAX_DELAY) == pdTRUE){		//czeka dopuki nie pojawi się esp32 interrupt request
			printf("High prior task \r\n");


			pi2cMaster->i2cMasterSemaphoreTake();
			pi2cMaster->I2C_Master_Receive_DMA(tempI2CFrameReceivedFromESP32.slaveDevice7bitAddress, (uint8_t*) &tempI2CFrameReceivedFromESP32.dataSize, sizeof(size_t));
			//HAL_I2C_Master_Receive_DMA(&hi2c1, tempI2CFrameReceivedFromESP32.slaveDevice7bitAddress<<1, (uint8_t*) &tempI2CFrameReceivedFromESP32.dataSize, sizeof(size_t));
			//HAL_I2C_Master_Receive(&hi2c1, tempI2CFrameReceivedFromESP32.slaveDevice7bitAddress<<1, (uint8_t*) &tempI2CFrameReceivedFromESP32.dataSize, sizeof(size_t), 500);
			//while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){

			//};
			pi2cMaster->while_I2C_STATE_READY();
			/*char**/
			pdymanicDataPointer = new char[tempI2CFrameReceivedFromESP32.dataSize];
			printf("1 \r\n");
			if (pdymanicDataPointer!=nullptr){
				HAL_I2C_Master_Receive_DMA(&hi2c1, tempI2CFrameReceivedFromESP32.slaveDevice7bitAddress<<1, (uint8_t*) pdymanicDataPointer, tempI2CFrameReceivedFromESP32.dataSize);
				//HAL_I2C_Master_Receive(&hi2c1, tempI2CFrameReceivedFromESP32.slaveDevice7bitAddress<<1, (uint8_t*) pdymanicDataPointer, tempI2CFrameReceivedFromESP32.dataSize, 500);
				while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){};
				printf("2 \r\n");
				//tempI2CFrameReceivedFromESP32.pData = pdymanicDataPointer;

			}
			else{
				printf("error with memory allocation\r\n");
				#warning zrobić porządną obsługę błędów
				//TODO: zrobić porządną obsługę błędów
			}

			pi2cMaster->i2cMasterSemaphoreGive();
		}
	};
}


void initTaskFunctions(void){
	printf("Radio main firmware version: %.2f\r\n", FW_VERSION);




	//////////////////////////////////////////////
	// NIE POTRZEBNE
	configASSERT(esp32IntrrruptRequest_CountingSemaphore = xSemaphoreCreateCounting(ESP32_INTERRUPT_REQUEST_COUNTING_SEMAPHORE_MAX, 0));
////////////////////////////////////////////////////////////




	//pi2cMaster = NULL;
	assert(pi2cMaster = new i2cMaster(&hi2c1));
	assert(pESP32 = new esp32_i2sComunicationDriver(pi2cMaster));
	pESP32->ping();


	//tworzy task callback na przerwanie od ESP32 informującę, że ESP32 ma jakieś dane do wysłania
	configASSERT(xTaskCreate(esp32IntrrruptRequestCallback, "esp32IntReq", 3*128, NULL, tskIDLE_PRIORITY+1, &taskHandle_esp32IntrrruptRequest));

}












void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
		pESP32->incrementInterruptRequestCountingSemaphore();		//inkrementacja semafora daje sygnał ,dla metody esp32_i2sComunicationDriver::intrrruptRequestCallback
		//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		//xSemaphoreGiveFromISR(esp32IntrrruptRequest_CountingSemaphore, &xHigherPriorityTaskWoken);
		//portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
