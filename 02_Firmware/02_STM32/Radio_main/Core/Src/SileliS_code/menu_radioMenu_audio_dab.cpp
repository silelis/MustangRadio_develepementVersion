/*
 * menu_radioMenu_audio_dab.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_audio_dab.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do urządzenia (power on / przejście z poprzedniego audio): utworzenie urządzenia + ReturnInit (klawisze, LED, stepper)
void radioMenuAudioDab_Init(void *context) {
	pPrintf->feedPrintf("DAB+ Init");
	//inicjuj urządzenie (w tym antena i active antena oraz i2s)

	//przełączenie wejścia korektora audio i wy  łaczneie audio mute

	//Wzmacniacz audio włączony

	//załaczenie grania

	radioMenuAudioDab_ReturnInit(context);
}

//powrót do urządzenia, które nie było niszczone (po timeout / długim EQ z peryferiów)
void radioMenuAudioDab_ReturnInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("DAB+ ReturnInit");
	//bindowanie klawiatury

	//ustawienie stepper motora

	//wyczyszczenie LED i ustawienie LED
}

//wyjście z urządzenia (przejście do następnego audio)
void radioMenuAudioDab_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("DAB+ DestroyInit");
	//Włączenie audio mute na korektorze audio i wzmacniacz audio wyłaczyć

	//zatrzymaj odtwarzanie

	//i2s w stan wysokiej impedancji

	//destroj urządzenia
}
