/*
 * menu_radioMenu_periphery_tipCalibration.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_periphery_tipCalibration.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do peryferium (wejście do menu peryferiów lub przejście z poprzedniego peryferium)
void radioMenuPeripheryTipCalibration_Init(void *context) {
	(void) context;
	pPrintf->feedPrintf("Tip calibration Init");
	//inicjuj urządzenie

	//bindowanie klawiatury

	//ustawienie stepper motora

	//wyczyszczenie LED i ustawienie LED
}

//wyjście z peryferium (następne peryferium, VOL, długie EQ, timeout)
void radioMenuPeripheryTipCalibration_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Tip calibration DestroyInit");

	//destroj urządzenia
}
