/*
 * menu_radioMenu_periphery_leftRight.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_periphery_leftRight.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do peryferium (wejście do menu peryferiów lub przejście z poprzedniego peryferium)
void radioMenuPeripheryLeftRight_Init(void *context) {
	(void) context;
	pPrintf->feedPrintf("Left<->Right Init");
	//inicjuj urządzenie

	//bindowanie klawiatury

	//ustawienie stepper motora

	//wyczyszczenie LED i ustawienie LED
}

//wyjście z peryferium (następne peryferium, VOL, długie EQ, timeout)
void radioMenuPeripheryLeftRight_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Left<->Right DestroyInit");

	//destroj urządzenia
}
