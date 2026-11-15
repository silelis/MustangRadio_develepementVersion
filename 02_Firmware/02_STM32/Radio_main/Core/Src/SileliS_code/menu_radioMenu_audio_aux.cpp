/*
 * menu_radioMenu_audio_aux.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_audio_aux.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do urządzenia (power on / przejście z poprzedniego audio): utworzenie urządzenia + ReturnInit (klawisze, LED, stepper)
void radioMenuAudioAux_Init(void *context) {
	pPrintf->feedPrintf("Aux Init");
	radioMenuAudioAux_ReturnInit(context);
}

//powrót do urządzenia, które nie było niszczone (po timeout / długim EQ z peryferiów)
void radioMenuAudioAux_ReturnInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Aux ReturnInit");
}

//wyjście z urządzenia (przejście do następnego audio)
void radioMenuAudioAux_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Aux DestroyInit");
}
