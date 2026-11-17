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
	//inicjuj urządzenie (w tym antena i active antena oraz i2s)

	//przełączenie wejścia korektora audio i wy  łaczneie audio mute

	//Wzmacniacz audio włączony

	//załaczenie grania

	radioMenuAudioUsb_ReturnInit(context);
}

//powrót do urządzenia, które nie było niszczone (po timeout / długim EQ z peryferiów)
void radioMenuAudioUsb_ReturnInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("USB ReturnInit");
	//bindowanie klawiatury

	//ustawienie stepper motora

	//wyczyszczenie LED i ustawienie LED
}

//wyjście z urządzenia (przejście do następnego audio)
void radioMenuAudioUsb_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("USB DestroyInit");
	//Włączenie audio mute na korektorze audio i wzmacniacz audio wyłaczyć

	//zatrzymaj odtwarzanie

	//i2s w stan wysokiej impedancji

	//destroj urządzenia
}
