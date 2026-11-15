/*
 * menu_radioMenu_audio_bluetooth.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_audio_bluetooth.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do urządzenia (power on / przejście z poprzedniego audio): utworzenie urządzenia + ReturnInit (klawisze, LED, stepper)
void radioMenuAudioBluetooth_Init(void *context) {
	pPrintf->feedPrintf("Bluetooth Init");
	radioMenuAudioBluetooth_ReturnInit(context);
}

//powrót do urządzenia, które nie było niszczone (po timeout / długim EQ z peryferiów)
void radioMenuAudioBluetooth_ReturnInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Bluetooth ReturnInit");
}

//wyjście z urządzenia (przejście do następnego audio)
void radioMenuAudioBluetooth_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Bluetooth DestroyInit");
}
