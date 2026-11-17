/*
 * menu_radioMenu_periphery_frontRear.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_periphery_frontRear.h>
#include "SileliS_code/myPrintfTask.h"
#include <SileliS_code/device_ledsController.h>

extern myPrintfTask *pPrintf;
extern ledsController *pHmiLeds;	//obiekt sterowania diodami HMI (ESP32 przez I2C) - tworzony w tasksFunctions.cpp:203

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do peryferium (wejście do menu peryferiów lub przejście z poprzedniego peryferium)
void radioMenuPeripheryFrontRear_Init(void *context) {
	(void) context;
	pPrintf->feedPrintf("Front<->Rear Init");
	//inicjuj urządzenie

	//bindowanie klawiatury

	//ustawienie stepper motora

	//wyczyszczenie LED i ustawienie LED
	pHmiLeds->setLedAllCleaned();
	pHmiLeds->setLedEqualiserBlinking(COLOR_GREEN_BLUE, COLOR_BLACK);
	if (pHmiLeds->sendDataToI2cTransmitQueueLeds() != pdTRUE) {	//kolejka TX do ESP32 pełna > 700 ms - diody bez zmiany, radio działa dalej
		pPrintf->feedPrintf("Front<->Rear Init: LED send error (I2C TX queue full)");
	}
}

//wyjście z peryferium (następne peryferium, VOL, długie EQ, timeout)
void radioMenuPeripheryFrontRear_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Front<->Rear DestroyInit");

	//destroj urządzenia
}
