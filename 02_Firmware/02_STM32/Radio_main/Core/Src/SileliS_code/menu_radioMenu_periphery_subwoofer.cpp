/*
 * menu_radioMenu_periphery_subwoofer.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_periphery_subwoofer.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do peryferium (wejście do menu peryferiów lub przejście z poprzedniego peryferium)
void radioMenuPeripherySubwoofer_Init(void *context) {
	(void) context;
	pPrintf->feedPrintf("Subwoofer Init");
}

//wyjście z peryferium (następne peryferium, VOL, długie EQ, timeout)
void radioMenuPeripherySubwoofer_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Subwoofer DestroyInit");
}
