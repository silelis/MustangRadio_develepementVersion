/*
 * menu_radioMenu_audio_usb.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_audio_usb.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do urządzenia (power on / przejście z poprzedniego audio): utworzenie urządzenia + ReturnInit (klawisze, LED, stepper)
void radioMenuAudioUsb_Init(void *context) {
	pPrintf->feedPrintf("USB Init");
	radioMenuAudioUsb_ReturnInit(context);
}

//powrót do urządzenia, które nie było niszczone (po timeout / długim EQ z peryferiów)
void radioMenuAudioUsb_ReturnInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("USB ReturnInit");
}

//wyjście z urządzenia (przejście do następnego audio)
void radioMenuAudioUsb_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("USB DestroyInit");
}
