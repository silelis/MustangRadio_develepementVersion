/*
 * menu_radioMenu_periphery_tipCalibrationLeftRight.cpp
 *
 *  Created on: 17 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_periphery_tipCalibrationLeftRight.h>
#include "SileliS_code/myPrintfTask.h"
#include <SileliS_code/device_ledsController.h>

extern myPrintfTask *pPrintf;
extern ledsController *pHmiLeds;	//obiekt sterowania diodami HMI (ESP32 przez I2C) - tworzony w tasksFunctions.cpp

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//UWAGA: nie wiadomo jeszcze, czy zostaną klawisze TUNE_LEFT_TIP_BOARDER / TUNE_RIGHT_TIP_BOARDER
//(długie BUT1+BUT3 / BUT3+BUT5, menu_bindKeyboardToFunction_buttonDefinitions.h:42,47; ExecutableButtonsArray[24],[25]) -
//możliwe, że granice wskazówki będą ustawiane tylko z submenu Tip calibration Left / Right, a te klawisze zostaną usunięte

//tu wspólna część kalibracji jako funkcje static

//wejście do peryferium: kalibracja LEWEJ granicy wskazówki
void radioMenuPeripheryTipCalibrationLeft_Init(void *context) {
	(void) context;
	pPrintf->feedPrintf("Tip calibration Left Init");
	//UWAGA: możliwa rezygnacja z TUNE_LEFT_TIP_BOARDER / TUNE_RIGHT_TIP_BOARDER - opis na początku pliku

	//inicjuj urządzenie

	//bindowanie klawiatury

	//ustawienie stepper motora (dojazd do LEWEJ strony)

	//wyczyszczenie LED i ustawienie LED
	pHmiLeds->setLedAllCleaned();
	pHmiLeds->setLedEqualiserBlinking(COLOR_WHITE, COLOR_RED);	//WHITE_R
	if (pHmiLeds->sendDataToI2cTransmitQueueLeds() != pdTRUE) {	//kolejka TX do ESP32 pełna > 700 ms - diody bez zmiany, radio działa dalej
		pPrintf->feedPrintf("Tip calibration Left Init: LED send error (I2C TX queue full)");
	}
}

//wyjście z peryferium (następne peryferium, VOL, długie EQ, timeout)
void radioMenuPeripheryTipCalibrationLeft_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Tip calibration Left DestroyInit");

	//destroj urządzenia
}

//wejście do peryferium: kalibracja PRAWEJ granicy wskazówki
void radioMenuPeripheryTipCalibrationRight_Init(void *context) {
	(void) context;
	pPrintf->feedPrintf("Tip calibration Right Init");
	//UWAGA: możliwa rezygnacja z TUNE_LEFT_TIP_BOARDER / TUNE_RIGHT_TIP_BOARDER - opis na początku pliku

	//inicjuj urządzenie

	//bindowanie klawiatury

	//ustawienie stepper motora (dojazd do PRAWEJ strony)

	//wyczyszczenie LED i ustawienie LED
	pHmiLeds->setLedAllCleaned();
	pHmiLeds->setLedEqualiserBlinking(COLOR_WHITE, COLOR_BLUE);	//WHITE_B
	if (pHmiLeds->sendDataToI2cTransmitQueueLeds() != pdTRUE) {	//kolejka TX do ESP32 pełna > 700 ms - diody bez zmiany, radio działa dalej
		pPrintf->feedPrintf("Tip calibration Right Init: LED send error (I2C TX queue full)");
	}
}

//wyjście z peryferium (następne peryferium, VOL, długie EQ, timeout)
void radioMenuPeripheryTipCalibrationRight_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Tip calibration Right DestroyInit");

	//destroj urządzenia
}
