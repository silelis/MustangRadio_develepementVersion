/*
 * menu_radioMenu_audio_sd.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_audio_sd.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do urządzenia (power on / przejście z poprzedniego audio): utworzenie urządzenia + ReturnInit (klawisze, LED, stepper)
void radioMenuAudioSd_Init(void *context) {
	pPrintf->feedPrintf("SD Init");
	radioMenuAudioSd_ReturnInit(context);
}

//powrót do urządzenia, które nie było niszczone (po timeout / długim EQ z peryferiów)
void radioMenuAudioSd_ReturnInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("SD ReturnInit");
}

//wyjście z urządzenia (przejście do następnego audio)
void radioMenuAudioSd_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("SD DestroyInit");
}
