/*
 * menu_radioMenu_periphery_bass.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_periphery_bass.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do peryferium (wejście do menu peryferiów lub przejście z poprzedniego peryferium)
void radioMenuPeripheryBass_Init(void *context) {
	(void) context;
	pPrintf->feedPrintf("Bass Init");
}

//wyjście z peryferium (następne peryferium, VOL, długie EQ, timeout)
void radioMenuPeripheryBass_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Bass DestroyInit");
}
