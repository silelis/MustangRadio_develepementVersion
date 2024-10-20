/*
 * tasksFunctions.c
 *
 *  Created on: Jul 13, 2024
 *      Author: dbank
 */


//#include "tasksFunctions.h"
#include "SileliS_code/tasksFunctions.h"
//#include <SileliS_code/myList.h>
#include <SileliS_code/radioMenu.h>


static bool esp32I2cInitialised = false;	//zmienna sprawdza czy esp32 zainiclował interfejs i2c
static TaskHandle_t taskHandle_i2cFromSlaveReceiveDataTask= nullptr;	//task obsługujący czytanie danych z urządzeń slave i2c
static TaskHandle_t taskHandle_i2cMasterToSlaveTransmitDataTask= nullptr;	//task obsługujący wysyłanie danych do i2c slave
static TaskHandle_t taskHandle_esp32IntrrruptRequest = nullptr;					//uchwyt taska obsługującego komunikację (odczytywanie danych) z esp32, po pojawieniu się sygnału esp32 interrupt request
static TaskHandle_t taskHandle_i2cMaster_pReceiveQueueObjectParser = nullptr;	//uchwyt taska obsługującego parsowanie kolejki odbiorczej pi2cMaster->pReceiveQueueObject
static TaskHandle_t taskHandle_manageTheRadioManue=nullptr;						//uchwyt do taska przetwarzajacego dane z klawiatury i przekazującego go go radioMenu
static TaskHandle_t taskHandle_PrintfTask=nullptr;								//uchwyt do taska kontrolujący wyświetlaniekomunikatów na uart

static i2cMaster* pi2cMaster=nullptr;  											//wsyaźnik do obiektu służącego do komunikacji stm32 po i2c jako master
static esp32_i2cComunicationDriver* pESP32=nullptr; 							//wsyaźnik do obiektu obsługującego komunikację z ESP32
radioMenu* pRadioMenu=nullptr;
myPrintfTask* pPrintf=nullptr;											//pointer do taska obsługuącego pisanie komunikatow na konsolę





static void i2cMaster_pReceivedQueueObjectParser(void *pNothing){
	i2cFrame_transmitQueue tempI2CReceiveFrame;
	while(1){
		if(pi2cMaster->pI2C_fromSlaveReceiveDataQueue->QueueReceive(&tempI2CReceiveFrame, portMAX_DELAY)==pdPASS){
			switch(tempI2CReceiveFrame.slaveDevice7bitAddress)
			{
			case I2C_SLAVE_ADDRESS_ESP32:




				pESP32->parseReceivedData(tempI2CReceiveFrame);
				break;
			default:
				//printf("i2cMaster_pReceiveQueueObjectParser: Unknown i2c slave address: 0x%x (7bit).\r\n", tempI2CReceiveFrame.slaveDevice7bitAddress);
				pPrintf->feedPrintf("i2cMaster_pReceiveQueueObjectParser: Unknown i2c slave address: 0x%x (7bit).", tempI2CReceiveFrame.slaveDevice7bitAddress);


				pi2cMaster->ping(tempI2CReceiveFrame.slaveDevice7bitAddress);
				assert(0);
			}
			pi2cMaster->pI2C_fromSlaveReceiveDataQueue->QueueDeleteDataFromPointer(tempI2CReceiveFrame);			//BARDZO WAŻNA FUNKCJA, po parsowaniu otrzymanego z i2c pakiedy danych, który jest przetrzymywany pod zmienną alokowaną dynamicznie niszczy tą zmienną. Ta funkcja, w tym miejscu zapobiega wyciekom pamięci!!!!!
		};
	}
}


static void i2cMasterToSlaveTransmitDataTask(void *pNothing){
	i2cFrame_transmitQueue I2CFrameToSendTolave;







	I2CFrameToSendTolave.slaveDevice7bitAddress=I2C_SLAVE_ADDRESS_ESP32;

	i2cFrame_keyboardFrame klawiatura;

	//klawiatura.i2cframeCommandHeader.crcSum=0;
	klawiatura.i2cframeCommandHeader.dataSize=sizeof(keyboardUnion);


	klawiatura.i2cframeCommandHeader.commandGroup=I2C_COMMAND_GROUP_KEYBOARD;
	klawiatura.keyboardData.array[0]='A';
	klawiatura.keyboardData.array[1]='C';

	I2CFrameToSendTolave.dataSize=sizeof(i2cFrame_commonHeader)+sizeof(keyboardUnion);

	if (I2CFrameToSendTolave.dataSize>ESP32_DEFAULT_RECEIVE_QUEUE_SIZE){
		assert(0);
	}

	I2CFrameToSendTolave.dataSize=sizeof(i2cFrame_keyboardFrame);

	klawiatura.i2cframeCommandHeader.crcSum=calculate_checksum(&klawiatura,I2CFrameToSendTolave.dataSize);

	I2CFrameToSendTolave.pData=&klawiatura;

	pi2cMaster->pI2C_toSlaveTransmitDataQueue->QueueSend(&I2CFrameToSendTolave);







	while(1){
		if(pi2cMaster->pI2C_toSlaveTransmitDataQueue->QueueReceive(&I2CFrameToSendTolave, portMAX_DELAY)==pdPASS)
		{
			HAL_StatusTypeDef retVal;
			pi2cMaster->i2cMasterSemaphoreTake();

			if (I2CFrameToSendTolave.dataSize<=ESP32_I2C_RECEIVE_DATA_BUFFER_LENGTH){		//jeżeli wielkość danych do wysłania nie przekracza maksymalnych dopuszczalnych przez I2C slave

				retVal= pi2cMaster->I2C_Master_Transmit_DMA(I2CFrameToSendTolave.slaveDevice7bitAddress, (uint8_t*) I2CFrameToSendTolave.pData, I2CFrameToSendTolave.dataSize);
				#warning TODO:dodać nadawanie sekwencyjne dla si468x
				//HAL_I2C_Master_Seq_Transmit_DMA(hi2c, DevAddress, pData, Size, XferOptions)
				if (I2CFrameToSendTolave.slaveDevice7bitAddress == I2C_SLAVE_ADDRESS_ESP32){
					pESP32->i2cComunicationHoldTime();
				}
				if(retVal!=HAL_OK){
					pPrintf->feedPrintf("i2cMaster: Unable to send data to slave 0x%X. Error value 0x%x.", I2CFrameToSendTolave.slaveDevice7bitAddress, retVal );
					pi2cMaster->ping(I2CFrameToSendTolave.slaveDevice7bitAddress);
				}
			}

			#warning sprawdzić czy działa niszczenie danych
			pi2cMaster->pI2C_toSlaveTransmitDataQueue->QueueDeleteDataFromPointer(I2CFrameToSendTolave);		//usuwa dane po wyslaniu (niezależnie od rezultatu)
			pi2cMaster->i2cMasterSemaphoreGive();
		}
	}
}


static void i2cFromSlaveReceiveDataTask(void *pNothing){
	i2cFrame_transmitQueue I2CFrameToReadFromSlave;

	while(1){
		if(pi2cMaster->pI2C_whichSlaveToReadQueue->QueueReceive(&I2CFrameToReadFromSlave, portMAX_DELAY)==pdPASS){
			pi2cMaster->i2cMasterSemaphoreTake();
			//TUTAJ nie kasujemy buffora pData, bo tutaj pobieramy dane z i2c slave do kolejki
			switch (I2CFrameToReadFromSlave.slaveDevice7bitAddress){
				case pESP32->esp32i2cSlaveAdress_7bit:		//czyta dane z ESP32
					pESP32->masterReceiveData(&I2CFrameToReadFromSlave);
					break;
				default:
					pPrintf->feedPrintf("I2C slave address not recognized.");
					assert(0);
			}
			pi2cMaster->i2cMasterSemaphoreGive();
			if (I2CFrameToReadFromSlave.pData==nullptr){
				//TUTAJ nie kasujemy buffora pData, bo tutaj pobieramy dane z i2c slave do kolejki
				pPrintf->feedPrintf("error with memory allocation.");
				assert(0);
			}
		}
	}
}


static void esp32IntrrruptRequestCallback(void *pNothing){
	i2cFrame_transmitQueue I2CFrameToReadFromESP32;			//
	I2CFrameToReadFromESP32.slaveDevice7bitAddress = pESP32->esp32i2cSlaveAdress_7bit;		//I2C_SLAVE_ADDRESS_ESP32;
	I2CFrameToReadFromESP32.dataSize=0;
	I2CFrameToReadFromESP32.pData=nullptr;
	while(1){
		//TUTAJ nie kasujemy buffora pData, bo tutaj pobieramy dane z i2c slave do kolejki
		pESP32->isCountingSemaphoreOverflowed();
		if (pESP32->semaphoreTake__CountingSemaphore()){								//czeka dopuki nie pojawi się esp32 interrupt request
			pi2cMaster->pI2C_whichSlaveToReadQueue->QueueSendFromISR(&I2CFrameToReadFromESP32);
		}
	};
}


void peripheryMenuTimeoutFunction(void* thing){
	radioMenu* ptrRadioMenu = (radioMenu*) thing;
	assert(ptrRadioMenu);
	bool selfSuspended = false;
	vTaskSuspend(NULL);
	while(1){
		vTaskDelay(pdMS_TO_TICKS(PERIPHERY_MENU_TIMEOUT_TASK_DELAY));

		xSemaphoreTake(ptrRadioMenu->peripheryMenu_TaskSuspendAllowedSemaphore, portMAX_DELAY);
		if (ptrRadioMenu->peripheryMenu_TimeoutCounterIncrement()>=6) {		//6 = 12 SECUNDS
			//ptrRadioMenu->peripheryMenu_TimeoutCounterReset();
			ptrRadioMenu->peripheryMenu_onTimeoutActions();
			//xSemaphoreGive(ptrRadioMenu->peripheryMenu_TaskSuspendAllowedSemaphore);
			vTaskSuspend(NULL);
			selfSuspended = true;
		}
		if (selfSuspended == false){
			xSemaphoreGive(ptrRadioMenu->peripheryMenu_TaskSuspendAllowedSemaphore);	//if self Suspended it means semaphore had been taken by menuFunction_equButShortPressed(void);
		}
		else{
			selfSuspended = false;		//reset self Suspend notification
		}

	}
}

static void manageRadioButtonsAndManue(void* thing){
	radioMenu* ptrRadioMenu = (radioMenu*) thing;
	assert(ptrRadioMenu);
	keyboardUnion receivedKeyboard;
	while(1){
		if(ptrRadioMenu->queueRadioMenuKbrdReceive(&receivedKeyboard)){
			if(!ptrRadioMenu->executeButtonFrom_radioMainMenu(receivedKeyboard)){
				if(!ptrRadioMenu->executeButtonFrom_curretDevice(receivedKeyboard)){
					//printf("%c %x - there is binded button.\r\n", receivedKeyboard.array[0], receivedKeyboard.array[1]);
					pPrintf->feedPrintf("%c %x - there is binded button.", receivedKeyboard.array[0], receivedKeyboard.array[1]);
				}
			}
		}
	}
}


static void initTaskFunctions(void){

	assert(pi2cMaster = new i2cMaster(&hi2c1));
	assert(pESP32 = new esp32_i2cComunicationDriver(pi2cMaster));

	//pętla opóźniająca oczekująza aż zakończy się proces bootowania ESP32
	pi2cMaster->i2cMasterSemaphoreTake();


//	while(HAL_I2C_IsDeviceReady(&hi2c1, pESP32->esp32i2cSlaveAdress_7bit<<1, 10000, 10000) != HAL_OK){
//		//printf("ESP32 i2c bus not responding\r\n");
//		pPrintf->feedPrintf("ESP32 i2c bus not responding...");
//	}

	pi2cMaster->i2cMasterSemaphoreGive();
	//pętla opóźniająca oczekująza aż zakończy si ę proces bootowania ESP32


	//printf("Radio main firmware version: %.2f\r\n", FW_VERSION);

	pi2cMaster->while_I2C_STATE_READY();
	//pESP32->ping();


	//tworzenie taska czytającego dane po I2C ze slave
	configASSERT(xTaskCreate(i2cFromSlaveReceiveDataTask, "i2cReceive", 5*128, NULL, tskIDLE_PRIORITY+5, &taskHandle_i2cFromSlaveReceiveDataTask));

	//tworzenie taska wysyłającego dane do slave I2C z master
	configASSERT(xTaskCreate(i2cMasterToSlaveTransmitDataTask, "i2cTransmit", 5*128, NULL, tskIDLE_PRIORITY+2, &taskHandle_i2cMasterToSlaveTransmitDataTask));


	//tworzy task callback na przerwanie od ESP32 informującę, że ESP32 ma jakieś dane do wysłania
	configASSERT(xTaskCreate(esp32IntrrruptRequestCallback, "esp32IntReq", 3*128, NULL, tskIDLE_PRIORITY+1, &taskHandle_esp32IntrrruptRequest));
	//tworzy task przetwarzający dane (parsujący) z kolejki odbiorczej i2c Mastera
	configASSERT(xTaskCreate(i2cMaster_pReceivedQueueObjectParser, "i2cMastRecQue, Pars", 3*128, NULL, tskIDLE_PRIORITY, &taskHandle_i2cMaster_pReceiveQueueObjectParser));


	assert(pRadioMenu=new radioMenu());
	//tworzy task obsługujący pobieranie z kolejki klawiszy
	configASSERT(xTaskCreate(manageRadioButtonsAndManue, "RadioMenu", 3*128, pRadioMenu, tskIDLE_PRIORITY, &/*(pRadioMenu->*/taskHandle_manageTheRadioManue/*)*/));
	//tworzy task timeoutu kontrolującego moment wyjścia z menu periphery (gdy radio jest w tym menu, a klawisze nie są używane)
	configASSERT(xTaskCreate(peripheryMenuTimeoutFunction, "periTimeout", 2*128, pRadioMenu, tskIDLE_PRIORITY, &pRadioMenu->peripheryMenu_taskHandle));
}


static TaskHandle_t taskHandle_initTaskFunctions=nullptr;						//uchwyt do taska (jodnorazowo wywolanego), który uruchamia całą konfigurację, trzeba tak to zrobić, aby printf działąło w tasku (task printf musi sie uruchomić  przed wszystkimi)
static void startUpTask_initTaskFunctions(void* noThing){

	//czeka, aż esp32 wyśle pierwszy sygnał interrupt request (pusty bez danych), wtedy wiadomo, że komunikacja jest zainicjowana.
	while(esp32I2cInitialised==false){
		pPrintf->feedPrintf("waiting for esp32 i2c bus to be initialized.");		//sygnałem o inicjalizacji jest pierwsze Interrupt request HIGH to LOW to HIGH
		vTaskDelay(pdMS_TO_TICKS(150));
	}

	//vTaskDelay(pdMS_TO_TICKS(2500));
	initTaskFunctions();
	vTaskDelete(taskHandle_initTaskFunctions);
}

static void printfTask(void* noThing){
	i2cFrame_transmitQueue itemToPrint;
	pPrintf->feedPrintf("Radio firmware version: %.2f", FW_VERSION);
	while(1){
		if(pPrintf->QueueReceive(&itemToPrint, portMAX_DELAY)==pdTRUE){
			pPrintf->myPrintf(itemToPrint);
			pPrintf->QueueDeleteDataFromPointer(itemToPrint);
		}
	}
}

void startUpTask(void* noThing){
	//printf("Radio main firmware version: %.2f\r\n", FW_VERSION);
	assert(pPrintf= new myPrintfTask(&huart1, 15));

	assert(xTaskCreate(printfTask, "Printf", 3*128, NULL, tskIDLE_PRIORITY, &taskHandle_PrintfTask));

	//pPrintf->feedPrintf("Radio main firmware version: %.2f\r\n", FW_VERSION);
	//tworzy task (JEDNORAZOWY), który wywołuje funkcję inicjalizacji calego sprzętu (inaczej nie dalo się zaimplementować printf działającego w tasku
	assert(xTaskCreate(startUpTask_initTaskFunctions, "initTasks", 3*128, NULL, tskIDLE_PRIORITY, &taskHandle_initTaskFunctions));
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(esp32I2cInitialised==false){
		esp32I2cInitialised=true;
	}
	else{
		pESP32->incrementInterruptRequestCountingSemaphore();		//inkrementacja semafora daje sygnał ,dla metody esp32_i2sComunicationDriver::intrrruptRequestCallback
	}

}
