/*
 * menu_radioMenu_audio_bluetooth.cpp
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu_audio_bluetooth.h>
#include "SileliS_code/myPrintfTask.h"
#include <SileliS_code/device_ledsController.h>

extern myPrintfTask *pPrintf;
extern ledsController *pHmiLeds;	//obiekt sterowania diodami HMI (ESP32 przez I2C) - tworzony w tasksFunctions.cpp:203

//wersja testowa: tylko log, context (radioMenu*) nieużywany

//wejście do urządzenia (power on / przejście z poprzedniego audio): utworzenie urządzenia + ReturnInit (klawisze, LED, stepper)
void radioMenuAudioBluetooth_Init(void *context) {
	pPrintf->feedPrintf("Bluetooth Init");
	//inicjuj urządzenie (w tym antena i active antena oraz i2s)

	//odczyt aktualnego numeru węzła RadioMenu

	//przełączenie wejścia korektora audio i wy  łaczneie audio mute

	//Wzmacniacz audio włączony

	//załaczenie grania

	radioMenuAudioBluetooth_ReturnInit(context);
}

//powrót do urządzenia, które nie było niszczone (po timeout / długim EQ z peryferiów)
void radioMenuAudioBluetooth_ReturnInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Bluetooth ReturnInit");
	//bindowanie klawiatury

	//ustawienie stepper motora

	//wyczyszczenie LED i ustawienie LED
	pHmiLeds->setLedAllCleaned();
	pHmiLeds->setLedSourceWithColor(COLOR_BLUE);
	//ustawienie diody BT connection
	if (pHmiLeds->sendDataToI2cTransmitQueueLeds() != pdTRUE) {	//kolejka TX do ESP32 pełna > 700 ms - diody bez zmiany, radio działa dalej
		pPrintf->feedPrintf("Bluetooth ReturnInit: LED send error (I2C TX queue full)");
	}
}

//wyjście z urządzenia (przejście do następnego audio)
void radioMenuAudioBluetooth_DestroyInit(void *context) {
	(void) context;
	pPrintf->feedPrintf("Bluetooth DestroyInit");
	//Włączenie audio mute na korektorze audio i wzmacniacz audio wyłaczyć

	//zatrzymaj odtwarzanie

	//i2s w stan wysokiej impedancji

	//destroj urządzenia
}
