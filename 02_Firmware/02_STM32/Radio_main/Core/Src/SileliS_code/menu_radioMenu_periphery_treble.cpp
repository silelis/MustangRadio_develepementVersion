/*
 * menu_radioMenu_periphery_treble.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_periphery_treble.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do peryferium (wejście do menu peryferiów lub przejście z poprzedniego peryferium)
void radioMenuPeripheryTreble_Init(void *context) {
	(void) context;
	pPrintf->feedPrintf("Treble Init");
}

//wyjście z peryferium (następne peryferium, VOL, długie EQ, timeout)
void radioMenuPeripheryTreble_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Treble DestroyInit");
}
