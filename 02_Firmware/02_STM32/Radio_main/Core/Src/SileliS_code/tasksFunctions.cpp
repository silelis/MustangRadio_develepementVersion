/*
 * tasksFunctions.c
 *
 *  Created on: Jul 13, 2024
 *      Author: dbank
 */

//#include "tasksFunctions.h"
#include <SileliS_code/device_ledsController.h>
#include <SileliS_code/device_StepperOptoPowerOFF.h>
#include <SileliS_code/device_btAudio.h>
#include <SileliS_code/menu_radioMegaStruct.h>
#include <SileliS_code/menu_radioMenu.h>
#include "SileliS_code/tasksFunctions.h"
//#include <SileliS_code/myList.h>
#include <new>


radioMegaStruct radioStruct;
static volatile bool esp32I2cInitialised = false; //true: esp32 potwierdził start (ACK na ping i2c w startUpTask_initTaskFunctions) - od tej chwili EXTI zlicza interrupt request; volatile - czytana w ISR
static TaskHandle_t taskHandle_i2cMasterReceiveFromSlaveDataTask = nullptr;	//task obsługujący czytanie danych z urządzeń slave i2c
static TaskHandle_t taskHandle_i2cMasterTransmitToSlaveDataTask = nullptr;//task wysyła dane do i2c slave
static TaskHandle_t taskHandle_esp32I2cIntrrruptRequest = nullptr;//uchwyt taska obsługującego komunikację (odczytywanie danych) z esp32, po pojawieniu się sygnału esp32 interrupt request
static TaskHandle_t taskHandle_i2cMasterParseReceivedData = nullptr;//uchwyt taska obsługującego parsowanie kolejki odbiorczej pi2cMaster->pReceiveQueueObject
static TaskHandle_t taskHandle_manageTheRadioManue = nullptr;//uchwyt do taska przetwarzajacego dane z klawiatury i przekazującego go go radioMenu
static TaskHandle_t taskHandle_PrintfTask = nullptr;//uchwyt do taska kontrolujący wyświetlaniekomunikatów na uart

i2cMaster *pi2cMaster = nullptr; //wsyaźnik do obiektu służącego do komunikacji stm32 po i2c jako master
static esp32_i2cComunicationDriver *pESP32 = nullptr; //wsyaźnik do obiektu obsługującego komunikację z ESP32
radioMenu *pRadioMenu = nullptr;
myPrintfTask *pPrintf = nullptr; //pointer do taska obsługuącego pisanie komunikatow na konsolę
ledsController *pHmiLeds = nullptr;
StepperOptoPowerOFF *pStepperMotor = nullptr;
btAudioDevice *pBtAudio = nullptr;	//obiekt BT audio (stan BT z ESP32) - używany w parserFunction::btAudioToBtAudioDevice() (extern)


//task parsujący otrzymane z i2c slave dane (przełącza parsowanie po adresie slave z jakiego dane zostały otrzymane
static void i2cMasterParseReceivedData(void *pNothing) {
	i2cFrame_transmitQueue tempI2CReceiveFrame;
	while (1) {
		if (pi2cMaster->takeReceivedI2cDataFromParsingQueue(
				&tempI2CReceiveFrame) == pdPASS) {
			switch (tempI2CReceiveFrame.slaveDevice7bitAddress) {
			case I2C_SLAVE_ADDRESS_ESP32:
				pESP32->parseReceivedData(tempI2CReceiveFrame);
				break;
			default:
				pPrintf->feedPrintf(
						"i2cMaster_pReceiveQueueObjectParser: Unknown i2c slave address: 0x%x (7bit).",
						tempI2CReceiveFrame.slaveDevice7bitAddress);

				pi2cMaster->ping(tempI2CReceiveFrame.slaveDevice7bitAddress);
				assert(0);
			}
			pi2cMaster->deleteAlocatedDataAfterParsing(tempI2CReceiveFrame); //BARDZO WAŻNA FUNKCJA, po parsowaniu otrzymanego z i2c pakiedy danych, który jest przetrzymywany pod zmienną alokowaną dynamicznie niszczy tą zmienną. Ta funkcja, w tym miejscu zapobiega wyciekom pamięci!!!!!
		};
	}
}

//task sprawdzajacy czy w kolejce nadawania danych i2c znajdują się jakieś dane do wysłania i jeśli tak to wysyła te dane do slave
static void i2cMasterTransmitToSlaveDataTask(void *pNothing) {
	i2cFrame_transmitQueue transmitFrame;
	while (1) {
		if (pi2cMaster->takeTransmitI2cDataToI2C_buffer(
				&transmitFrame)==pdTRUE) {
			pi2cMaster->i2cMasterMutexTake();
			switch (transmitFrame.slaveDevice7bitAddress) {
			case pESP32->esp32i2cSlaveAdress_7bit:
				pESP32->masterTransmitData(&transmitFrame);	//wraca po zakończeniu wysyłki + 7 ms dla ESP32 (błąd -> printf + assert w środku)
				break;
			default:
				pPrintf->feedPrintf("I2C slave address not recognized.");
				assert(0);
			}
			pi2cMaster->i2cMasterMutexGive();

			if (transmitFrame.pData != nullptr) {
				pi2cMaster->deleteAlocatedDataAfterParsing(transmitFrame);
			} else if (transmitFrame.pData == nullptr) {
				pPrintf->feedPrintf("error with memory allocation.");
				assert(0);
			}
		}

	}
}

//task sprawdza czy w kolejce do odczytu z i2c slave znajdują sie dane do odczytu i jesli takie sa to rozpoczyna komunikację w celu odczytu tych danych
static void i2cMasterReceiveFromSlaveDataTask(void *pNothing) {
	i2cFrame_transmitQueue I2CFrameToReadFromSlave;

	while (1) {
		if (pi2cMaster->getI2cAdressFromAdressQueue(
				&I2CFrameToReadFromSlave)==pdPASS) {
			pi2cMaster->i2cMasterMutexTake();
			switch (I2CFrameToReadFromSlave.slaveDevice7bitAddress) {
			case pESP32->esp32i2cSlaveAdress_7bit:		//czyta dane z ESP32
				pESP32->masterReceiveData(&I2CFrameToReadFromSlave);
				break;
			default:
				pPrintf->feedPrintf("I2C slave address not recognized.");
				assert(0);
			}
			pi2cMaster->i2cMasterMutexGive();
			if (I2CFrameToReadFromSlave.pData != nullptr) {	//po fazie len (esp32) pData == nullptr - nic do parsera; kontrola alokacji w esp32_i2cComunicationDriver::masterReceiveData()
				pi2cMaster->passReceivedI2cDataToParsingQueue(
						&I2CFrameToReadFromSlave);
			}
		}
	}
}

//jeśli hmi (ESP32) ma jakieś dane do wyslania to najpierw ustawia linie INTIREQ na poziom niski. To 2 STM32 uruchamia to przerwanie, które callback
// funkcja HAL_GPIO_EXTI_Callback podnosi wartość semafora zliczajacego dane do doczytania z ESP32, a ta funkcja odczytuje te dane i zmniejsza wartośc semafora
static void esp32I2cIntrrruptRequest(void *pNothing) {
	esp32_i2cComunicationDriver::i2cReceiveStates pESP32_I2cRxMasaterState;
	bool stayInLoop;
	while (1) {
		pESP32->isCountingSemaphoreOverflowed();
		if (pESP32->semaphoreTake__CountingSemaphore()) {//czeka dopuki nie pojawi się esp32 interrupt request

			stayInLoop = true;
			do{
				pESP32_I2cRxMasaterState=pESP32->getI2cRxMasaterState();

				if((pESP32_I2cRxMasaterState==esp32_i2cComunicationDriver::i2cReceiveStates::lenDataInReception)||
						(pESP32_I2cRxMasaterState==esp32_i2cComunicationDriver::i2cReceiveStates::packageDataInReception)||
						(pESP32_I2cRxMasaterState==esp32_i2cComunicationDriver::i2cReceiveStates::packageDataReceived)){
					vTaskDelay(1);
				}
				else{
					stayInLoop = false;
				}
			}while(stayInLoop);

			switch(pESP32_I2cRxMasaterState){
			case esp32_i2cComunicationDriver::i2cReceiveStates::idle:
			case esp32_i2cComunicationDriver::i2cReceiveStates::lenDataReceived:
				pi2cMaster->setI2cAdressToAdressQueue(pESP32->esp32i2cSlaveAdress_7bit);
				break;
			default:	//error/timeout zgłasza (printf + assert) task odbioru w miejscu wykrycia - tu tylko zabezpieczenie
				pPrintf->feedPrintf("esp32 interrupt request in unexpected rx state: %d",
						(int) pESP32_I2cRxMasaterState);
				//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
				assert(0);
				break;
			}
		}
		vTaskDelay(1);
	};
}

// funkcja której zadaniem jest odmierzenie czasu timeout. Czas ten liczony jest jako czas bezczynny w menu equalizera. Po domierzeniu czasu timeout (bezczynności) radio samo wychodzi z menu equalizaera do menu urządzenia odtwarzającego.
void peripheryMenuTimeoutFunction(void *thing) {
	radioMenu *ptrRadioMenu = (radioMenu*) thing;
	assert(ptrRadioMenu);
	bool timeoutElapsed;
	vTaskSuspend(NULL);
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(PERIPHERY_MENU_TIMEOUT_TASK_DELAY));

		xSemaphoreTake(ptrRadioMenu->peripheryMenu_TaskSuspendAllowedMutex,
				portMAX_DELAY);
		timeoutElapsed = (ptrRadioMenu->peripheryMenu_TimeoutCounterIncrement() >= EQUALIZER_TIMEOUT_PERIOD);//6 = 12 SECUNDS
		xSemaphoreGive(ptrRadioMenu->peripheryMenu_TaskSuspendAllowedMutex);	//mutex oddany ZAWSZE - task nie może zostać zawieszony trzymając mutex (groziło deadlockiem z menuFunction_returnForEqualizerMenu)
		if (timeoutElapsed) {
			keyboardUnion returnKey = { RETURN_FROM_EQUALIZER_MENU };
			ptrRadioMenu->queueRadioMenuKbrdSend(&returnKey);	//powrót jak po długim EQ: wykona go task RadioMenu (menuFunction_returnForEqualizerMenu), który też zawiesza ten task
		}
	}
}


//glówna funkcja radio inicjalizująca menu i podmenu i obslugę klawiszy
static void manageRadioButtonsAndMenu(void *thing) {
	radioMenu *ptrRadioMenu = (radioMenu*) thing;
	assert(ptrRadioMenu);
	keyboardUnion receivedKeyboard;

	while (1) {
		if (ptrRadioMenu->queueRadioMenuKbrdReceive(&receivedKeyboard)) {
			ptrRadioMenu->executeButtonFunction(receivedKeyboard);
		}
	}
}

static void initTaskFunctions(void) {

	//pi2cMaster tworzony wcześniej - w startUpTask_initTaskFunctions() (potrzebny do pingu ESP32 przed startem)
	static esp32_i2cComunicationDriver esp32DriverObject(pi2cMaster);	//static KONIECZNE: wywoływane z taska startUpTask_initTaskFunctions, który usuwa się sam (vTaskDelete)
	pESP32 = &esp32DriverObject;
	//obiekty urządzeń fizycznie w ESP32 - WSZYSTKIE przed taskami, które z nich korzystają (parser, menu); konstruktory pasywne - nic nie wysyłają, potrzebują tylko kolejki TX pi2cMaster
	static ledsController hmiLedsObject(pi2cMaster->getTransmitQueue());	//static KONIECZNE: jak wyżej (task usuwa się sam); PRZED pBtAudio - btAudioDevice dostanie pHmiLeds (dioda statusu)
	pHmiLeds = &hmiLedsObject;
	static StepperOptoPowerOFF stepperMotorObject(pi2cMaster->getTransmitQueue());	//jak wyżej
	pStepperMotor = &stepperMotorObject;
	static btAudioDevice btAudioObject(&radioStruct.audioDevices.bluetooth);	//jak wyżej; PRZED taskiem parsera (i2cMasterParseReceivedData) - ramki BT przetwarzane w locie, parser nie może trafić na nullptr
	pBtAudio = &btAudioObject;

	BaseType_t taskCreated;	//wynik xTaskCreate - operacja zawsze poza makrem asercji, makro tylko sprawdza wynik
	//tworzenie taska czytającego dane po I2C ze slave
	taskCreated = xTaskCreate(i2cMasterReceiveFromSlaveDataTask, "i2cReceive", 5*128, NULL, tskIDLE_PRIORITY + 2, &taskHandle_i2cMasterReceiveFromSlaveDataTask);	//+2: odczyt z ESP32 w czasie ESP32_I2C_TX_TIMEOUT_MS niezależnie od obciążenia (menu, printf); większość czasu czeka na kolejkę/semafor
	configASSERT(taskCreated == pdPASS);
	taskCreated = xTaskCreate(i2cMasterTransmitToSlaveDataTask, "i2cTransmit", 5*128, NULL, tskIDLE_PRIORITY + 2, &taskHandle_i2cMasterTransmitToSlaveDataTask);	//+2: jak odbiór (wspólny semafor szyny)
	configASSERT(taskCreated == pdPASS);

	//tworzy task callback na przerwanie od ESP32 informującę, że ESP32 ma jakieś dane do wysłania
	taskCreated = xTaskCreate(esp32I2cIntrrruptRequest, "esp32IntReq", 3*128, NULL, tskIDLE_PRIORITY+1, &taskHandle_esp32I2cIntrrruptRequest);
	configASSERT(taskCreated == pdPASS);
	//tworzy task przetwarzający dane (parsujący) z kolejki odbiorczej i2c Mastera
	taskCreated = xTaskCreate(i2cMasterParseReceivedData, "i2cMastRecQue, Pars", 3*128, NULL, tskIDLE_PRIORITY + 1, &taskHandle_i2cMasterParseReceivedData);	//+1: parsowanie odebranych ramek przed menu/printf
	configASSERT(taskCreated == pdPASS);

	//tutaj inicjoać wszytkie menu.

	static radioMenu radioMenuObject;	//jak wyżej; bez () - "radioMenu radioMenuObject();" byłoby deklaracją funkcji, nie obiektu
	pRadioMenu = &radioMenuObject;
	//tworzy task obsługujący pobieranie z kolejki klawiszy
	taskCreated = xTaskCreate(manageRadioButtonsAndMenu, "RadioMenu", 5*128, pRadioMenu, tskIDLE_PRIORITY, &taskHandle_manageTheRadioManue);
	configASSERT(taskCreated == pdPASS);
	//tworzy task timeoutu kontrolującego moment wyjścia z menu periphery (gdy radio jest w tym menu, a klawisze nie są używane)
	taskCreated = xTaskCreate(peripheryMenuTimeoutFunction, "periTimeout", 2*128, pRadioMenu, tskIDLE_PRIORITY, &pRadioMenu->peripheryMenu_taskHandle);
	configASSERT(taskCreated == pdPASS);
}

static TaskHandle_t taskHandle_initTaskFunctions = nullptr;	//uchwyt do taska (jodnorazowo wywolanego), który uruchamia całą konfigurację, trzeba tak to zrobić, aby printf działąło w tasku (task printf musi sie uruchomić  przed wszystkimi)
static void startUpTask_initTaskFunctions(void *noThing) {

	static i2cMaster i2cMasterObject(&hi2c1, I2C1_SCL_GPIO_Port,
			I2C1_SCL_Pin, I2C1_SDA_GPIO_Port, I2C1_SDA_Pin);	//piny SCL/SDA z main.h (etykiety CubeMX) - diagnostyka zawieszonej transmisji; static KONIECZNE: task usuwa się sam (vTaskDelete) - jego stos jest zwalniany
	pi2cMaster = &i2cMasterObject;

	//czeka, aż esp32 odpowie ACK na ping i2c - wtedy wiadomo, że jego i2c slave działa (ESP32 nie odpowiada ACK w bootloaderze /
	//resecie / flashowaniu). Dawniej sygnałem było pierwsze zbocze interrupt request, ale każde dodatkowe zbocze na GPIO0 ESP32
	//(reset brown-out, flashowanie, zakłócenia) psuło synchronizację.
	while (pi2cMaster->isDeviceReady(I2C_SLAVE_ADDRESS_ESP32) == false) {
		pPrintf->feedPrintf("waiting for esp32 i2c bus to be initialized.");
		vTaskDelay(pdMS_TO_TICKS(ESP32_READY_POLL_PERIOD_MS));
	}
	vTaskDelay(pdMS_TO_TICKS(ESP32_READY_SETTLE_MS));	//ESP32 kończy start (taski, uzbrojenie odbioru)
	esp32I2cInitialised = true;	//od teraz EXTI zlicza interrupt request (HAL_GPIO_EXTI_Callback)

	//vTaskDelay(pdMS_TO_TICKS(2500));
	initTaskFunctions();
	vTaskDelete(taskHandle_initTaskFunctions);
}

static void printfTask(void *noThing) {
	i2cFrame_transmitQueue itemToPrint;
	pPrintf->feedPrintf("Radio firmware version: %.2f", FW_VERSION);
	while (1) {
		if (pPrintf->QueueReceive(&itemToPrint, portMAX_DELAY) == pdTRUE) {
			pPrintf->myPrintf(itemToPrint);	// myPrintf() sam zwalnia itemToPrint.pData - nie wolno usuwać go tu ponownie (double free)
		}
	}
}

void startUpTask(void *noThing) {
	static myPrintfTask printfTaskObject(&huart1, 15);	//static KONIECZNE: startUpTask() wołana z main() przed osKernelStart() - obiekt lokalny bez static zniknąłby po powrocie (a MSP jest resetowany przez scheduler); .bss - brak RAM = błąd linkera
	pPrintf = &printfTaskObject;

	BaseType_t taskCreated;	//jak w initTaskFunctions()
	taskCreated = xTaskCreate(printfTask, "Printf", 3*128, NULL, tskIDLE_PRIORITY, &taskHandle_PrintfTask);
	assert(taskCreated == pdPASS);

	//tworzy task (JEDNORAZOWY), który wywołuje funkcję inicjalizacji calego sprzętu (inaczej nie dalo się zaimplementować printf działającego w tasku
	taskCreated = xTaskCreate(startUpTask_initTaskFunctions, "initTasks", 3*128, NULL, tskIDLE_PRIORITY, &taskHandle_initTaskFunctions);
	assert(taskCreated == pdPASS);
}

//callback od GPIO
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	switch (GPIO_Pin) {
	case esp32i2cInterruptReqest_Pin:		//esp32 interrupt request
		if ((esp32I2cInitialised == true) && (pESP32 != nullptr)) {	//zbocza przed potwierdzeniem startu ESP32 pingiem ignorowane (reset brown-out, flashowanie, zakłócenia GPIO0); pESP32 powstaje chwilę po esp32I2cInitialised (bez printf - feedPrintf nie jest ISR-safe)
			pESP32->incrementInterruptRequestCountingSemaphore();//inkrementacja semafora daje sygnał ,dla metody esp32_i2sComunicationDriver::intrrruptRequestCallback
		}
		break;
	default:	//przerwania od innych pinów EXTI - obecnie nieużywane
		break;
	}
}

//callbacki i2c
//HAL_I2C_ErrorCallback 			//każdy błąd (NACK, BERR, ARLO, OVR, DMA). Nie mówi, czy dotyczył RX czy TX
//HAL_I2C_AbortCpltCallback			//po HAL_I2C_Master_Abort_IT() (dziś nieużywane - I2C_Master_Abort() zostawione na przyszłą obsługę błędów)
//HAL_I2C_MasterRxCpltCallback		//wywołuje	HAL_I2C_Master_Receive_DMA()
//HAL_I2C_MemRxCpltCallback 		//wywołuje	HAL_I2C_Mem_Read_DMA()
//HAL_I2C_MasterTxCpltCallback		//wywołuje	HAL_I2C_Master_Transmit_DMA()
//HAL_I2C_MemTxCpltCallback			//wywołuje	HAL_I2C_Mem_Write_DMA()
