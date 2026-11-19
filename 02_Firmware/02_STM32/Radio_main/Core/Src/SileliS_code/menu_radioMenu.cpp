/*
 * radioMenu.cpp
 *
 *  Created on: Aug 25, 2024
 *      Author: dbank
 */

#include <SileliS_code/menu_radioMenu.h>
#include <SileliS_code/menu_radioMenu_audio_fm.h>
#include <SileliS_code/menu_radioMenu_audio_dab.h>
#include <SileliS_code/menu_radioMenu_audio_am.h>
#include <SileliS_code/menu_radioMenu_audio_bluetooth.h>
#include <SileliS_code/menu_radioMenu_audio_usb.h>
#include <SileliS_code/menu_radioMenu_audio_sd.h>
#include <SileliS_code/menu_radioMenu_audio_aux.h>
#include <SileliS_code/menu_radioMenu_periphery_antena.h>
#include <SileliS_code/menu_radioMenu_periphery_treble.h>
#include <SileliS_code/menu_radioMenu_periphery_middle.h>
#include <SileliS_code/menu_radioMenu_periphery_bass.h>
#include <SileliS_code/menu_radioMenu_periphery_subwoofer.h>
#include <SileliS_code/menu_radioMenu_periphery_lhBoost.h>
#include <SileliS_code/menu_radioMenu_periphery_leftRight.h>
#include <SileliS_code/menu_radioMenu_periphery_frontRear.h>
#include <SileliS_code/menu_radioMenu_periphery_backlight.h>
#include <SileliS_code/menu_radioMenu_periphery_tipCalibrationLeftRight.h>
//#include "SileliS_code/i2cEngine.h"

extern myPrintfTask *pPrintf;
//extern i2cMaster* pi2cMaster;

//radioMegaStruct radioStruct;

radioMenu::radioMenu():bindKeyboardToFunction(){
//	keyboardToFunction(radioStruct.control.ExecutableButtonsArray) {
	queueRadioMenuKbrd = nullptr;
	configASSERT(queueRadioMenuKbrd = xQueueCreate(20, sizeof(keyboardUnion)));

	this->peripheryDevices = nullptr;
	this->audioDevices = nullptr;
	this->radioMainMenu = nullptr;
	this->curretDevice = nullptr;	//do setCurrentDeviceMenu_audio() brak aktywnej listy - strażniki nullptr w get/set działają od początku

	this->peripheryMenu_TimeoutCounter = 0;
	this->peripheryMenu_TimeoutCounterSemaphore = NULL;
	configASSERT(
			this->peripheryMenu_TimeoutCounterSemaphore = xSemaphoreCreateBinary());
	xSemaphoreGive(this->peripheryMenu_TimeoutCounterSemaphore);

	configASSERT(
			this->peripheryMenu_TaskSuspendAllowedSemaphore = xSemaphoreCreateBinary());
	xSemaphoreGive(this->peripheryMenu_TaskSuspendAllowedSemaphore);

	this->peripheryMenu_taskHandle = NULL;

	this->createDeviceMenuList_periphery();
	this->createDeviceMenuList_audio();
	this->createDeviceMenuList_mainMenu();

	this->setCurrentDeviceMenu_audio();
	this->setCurrentAudioNodeIndex(1, true);	//power on: FM (indeks 1), bez DestroyInit - żadne urządzenie nie było jeszcze zainicjalizowane; docelowo zapamiętany indeks; peryferia NIE są inicjowane przy starcie
}

void radioMenu::createDeviceMenuList_audio(void) {

	assert(
			this->audioDevices = new (std::nothrow) myList(
					&(this->ListHeader_audioDevices),
					"FM"));
	this->audioDevices->bindCurrent(radioMenuAudioFm_Init, radioMenuAudioFm_DestroyInit, radioMenuAudioFm_ReturnInit);	//bindCurrent: zawsze do węzła dodanego przed chwilą (current), nie do głowy listy
	this->audioDevices->addAtEnd("DAB+");
	this->audioDevices->bindCurrent(radioMenuAudioDab_Init, radioMenuAudioDab_DestroyInit, radioMenuAudioDab_ReturnInit);
	this->audioDevices->addAtEnd("AM");
	this->audioDevices->bindCurrent(radioMenuAudioAm_Init, radioMenuAudioAm_DestroyInit, radioMenuAudioAm_ReturnInit);
	this->audioDevices->addAtEnd("Bluetooth");
	this->audioDevices->bindCurrent(radioMenuAudioBluetooth_Init, radioMenuAudioBluetooth_DestroyInit, radioMenuAudioBluetooth_ReturnInit);
	this->audioDevices->addAtEnd("USB");
	this->audioDevices->bindCurrent(radioMenuAudioUsb_Init, radioMenuAudioUsb_DestroyInit, radioMenuAudioUsb_ReturnInit);
	this->audioDevices->addAtEnd("SD");
	this->audioDevices->bindCurrent(radioMenuAudioSd_Init, radioMenuAudioSd_DestroyInit, radioMenuAudioSd_ReturnInit);
	this->audioDevices->addAtEnd("Aux");
	this->audioDevices->bindCurrent(radioMenuAudioAux_Init, radioMenuAudioAux_DestroyInit, radioMenuAudioAux_ReturnInit);
}

void radioMenu::createDeviceMenuList_periphery(void) {

	assert(
			this->peripheryDevices = new (std::nothrow) myList(
					&(this->ListHeader_peripheryDevices),
					"Antena"));
	this->peripheryDevices->bindCurrent(radioMenuPeripheryAntena_Init, radioMenuPeripheryAntena_DestroyInit, nullptr);	//peryferia: ReturnInit = nullptr (do peryferium nigdy się nie wraca)
	this->peripheryDevices->addAtEnd(
			"Treble");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryTreble_Init, radioMenuPeripheryTreble_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd("Midle");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryMiddle_Init, radioMenuPeripheryMiddle_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd("Bass");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryBass_Init, radioMenuPeripheryBass_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd(
			"Subwoofer");
	this->peripheryDevices->bindCurrent(radioMenuPeripherySubwoofer_Init, radioMenuPeripherySubwoofer_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd(
			"L_H_Boost");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryLhBoost_Init, radioMenuPeripheryLhBoost_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd(
			"Left<->Right");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryLeftRight_Init, radioMenuPeripheryLeftRight_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd(
			"Front<->Rear");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryFrontRear_Init, radioMenuPeripheryFrontRear_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd(
			"Backlight");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryBacklight_Init, radioMenuPeripheryBacklight_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd(
			"Tip calibration Left");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryTipCalibrationLeft_Init, radioMenuPeripheryTipCalibrationLeft_DestroyInit, nullptr);
	this->peripheryDevices->addAtEnd(
			"Tip calibration Right");
	this->peripheryDevices->bindCurrent(radioMenuPeripheryTipCalibrationRight_Init, radioMenuPeripheryTipCalibrationRight_DestroyInit, nullptr);

	this->peripheryDevices->resetToFirst();
	//this->peripheryDevices->printList();
}

void radioMenu::createDeviceMenuList_mainMenu(void) {

	assert(
			this->radioMainMenu = new (std::nothrow) myList(
					&(this->ListHeader_mainMenu),
					"MainMenu: "));

	this->appendButtonArrayWithFunctionPointer( { POWER_ON_OFF_SHORT_PRESS },
			std::bind(&radioMenu::menuFunction_volButShortPressed, this));

	this->appendButtonArrayWithFunctionPointer( { POWER_ON_OFF_LONG_RELEASED },
			std::bind(&radioMenu::menuFunction_volButLongRelease, this));

	this->appendButtonArrayWithFunctionPointer( { EQUALIZER_SHORT_PRESS },
			std::bind(&radioMenu::menuFunction_equButShortPressed, this));

	this->appendButtonArrayWithFunctionPointer( { VOL_INCREMENT },
			std::bind(&radioMenu::menuFunction_volIncrement, this));

	this->appendButtonArrayWithFunctionPointer( { VOL_DECREMENT },
			std::bind(&radioMenu::menuFunction_volDecrement, this));

	this->appendButtonArrayWithFunctionPointer( { RESET_TO_DEFAULT },
			std::bind(&radioMenu::menuFunction_resetToDefaultLongPress, this));


	this->appendButtonArrayWithFunctionPointer( { RETURN_FROM_EQUALIZER_MENU },
			std::bind(&radioMenu::menuFunction_returnForEqualizerMenu, this));

	this->radioMenuButtonsAssignedInMainMenu= true;
}

/********************************************************************
 * @brief  Wybiera listę urządzeń audio jako aktywną
 * 			(curretDevice = audioDevices).
 *
 * Zmienia tylko to, KTÓRA lista jest obsługiwana przez menu.
 * Nie zmienia bieżącego węzła w liście i nie wywołuje callbacków
 * (Init/DestroyInit/ReturnInit) - to robi wywołujący.
 * Musi być wywołana przed pierwszym użyciem curretDevice
 * (konstruktor) oraz przy każdym powrocie z peryferiów.
 *
 * @return NONE
 *******************************************************************/
void radioMenu::setCurrentDeviceMenu_audio(void) {
	this->curretDevice = this->audioDevices;
}

void radioMenu::setCurrentDeviceMenu_periphery(void) {
	this->curretDevice = this->peripheryDevices;
}

BaseType_t radioMenu::queueRadioMenuKbrdSend(const void *kbrdUnionSend) {
	return xQueueSend(this->queueRadioMenuKbrd, kbrdUnionSend,
			pdMS_TO_TICKS(700));
}

BaseType_t radioMenu::queueRadioMenuKbrdReceive(
		keyboardUnion *kbrdUnionReceived) {
	BaseType_t retVal = xQueueReceive(this->queueRadioMenuKbrd,
			kbrdUnionReceived, portMAX_DELAY);
	this->peripheryMenu_TimeoutCounterReset();
	return retVal;
}

radioMenu::~radioMenu() {
	//vTaskDelete(this->taskHandle_manageTheRadioManue);
	vQueueDelete(queueRadioMenuKbrd);
	xSemaphoreTake(this->peripheryMenu_TimeoutCounterSemaphore, portMAX_DELAY);
	vSemaphoreDelete(this->peripheryMenu_TimeoutCounterSemaphore);
	xSemaphoreTake(this->peripheryMenu_TaskSuspendAllowedSemaphore,
			portMAX_DELAY);
	vSemaphoreDelete(this->peripheryMenu_TaskSuspendAllowedSemaphore);

	vTaskDelete(this->peripheryMenu_taskHandle);

	this->radioMainMenu->deleteList();		//usuwa wszystkie węzły listy (obiekty z new, nie new[]) - samo delete usunęłoby tylko pierwszy węzeł
	this->audioDevices->deleteList();
	this->peripheryDevices->deleteList();
	this->radioMainMenu = nullptr;			//wskaźniki wskazywały na usunięte głowy list
	this->audioDevices = nullptr;
	this->peripheryDevices = nullptr;

	//memset(&(this->ListHeader_audioDevices),0,sizeof(ListHeader));
	//memset(&(this->ListHeader_peripheryDevices),0,sizeof(ListHeader));
	//memset(&(this->ListHeader_mainMenu),0,sizeof(ListHeader));
	this->curretDevice = nullptr;

	//this->curretDevice->getCurrentNodeTag()
}

const char* radioMenu::getCurrentNodeTag()/* const*/{
	return (const char*) this->curretDevice->getCurrentNodeTag();
}

/********************************************************************
 * @brief  Zwraca indeks bieżącego węzła AKTYWNEJ listy
 * 			(curretDevice: audio albo peryferia).
 *
 * @return [uint8_t] indeks węzła (od 1); 0 - brak aktywnej listy
 * 			lub lista pusta
 *******************************************************************/
uint8_t radioMenu::getCurrentNodeIndex()/* const*/{
	if (this->curretDevice == nullptr) return 0;	//brak aktywnej listy (np. po destruktorze)
	return this->curretDevice->getCurrentNodeIndex();
}

/********************************************************************
 * @brief  Przełącza aktywne urządzenie audio na węzeł o wskazanym
 * 			indeksie: [DestroyInit poprzedniego] -> moveToIndex() ->
 * 			printCurrent() -> Init nowego.
 *
 * Działa TYLKO gdy radio jest w trybie sterowania audio
 * (curretDevice == audioDevices). W innym trybie (peryferia), gdy
 * lista audio nie istnieje lub jest pusta - nic nie zmienia,
 * wypisuje komunikat i zwraca false.
 *
 * @param  [nodeIndex] [uint8_t] indeks urządzenia (od 1);
 * 			0 -> pierwsze, indeks > liczby urządzeń -> ostatnie
 * @param  [noNodeInitialised] [bool] true - żadne urządzenie audio
 * 			nie było jeszcze zainicjalizowane, pomija DestroyInit
 * 			poprzedniego (TYLKO raz, w konstruktorze);
 * 			false (domyślnie) - normalne przełączenie z DestroyInit
 * 			aktualnie używanego urządzenia.
 * 			Jeśli wskazane urządzenie już gra - nic nie robi (bez
 * 			DestroyInit/Init, bez przerwy w graniu) i zwraca true.
 * 			Dotyczy to też indeksu poprawionego do zakresu, np. gra
 * 			ostatnie urządzenie (Aux, 7) i podano 255 -> 255 zostaje
 * 			zamienione na 7 -> to samo urządzenie -> nic nie robi.
 *
 * @return [bool] true - przełączono lub to samo urządzenie;
 * 			false - nie tryb audio / brak urządzeń
 *
 * @note   private: wywoływać z tasku radioMenu (konstruktor, obsługa
 * 			klawiszy) - callbacki urządzeń nie są chronione przed
 * 			równoległym wywołaniem z innego tasku.
 *******************************************************************/
bool radioMenu::setCurrentAudioNodeIndex(uint8_t nodeIndex, bool noNodeInitialised) {
	if (this->audioDevices == nullptr || this->curretDevice != this->audioDevices) {	//nie jesteśmy w trybie audio - nic nie zmieniamy
		pPrintf->feedPrintf("%ssetCurrentAudioNodeIndex(%d): not in audio mode.", this->TAG, nodeIndex);
		return false;
	}
	uint8_t nodeCount = this->audioDevices->getNodeCount();
	if (nodeCount == 0) {	//pusta lista - execCurrentInit() odwołałby się do nullptr
		pPrintf->feedPrintf("%ssetCurrentAudioNodeIndex(%d): no audio devices.", this->TAG, nodeIndex);
		return false;
	}
	if (nodeIndex == 0) nodeIndex = 1;							//0 -> pierwsze urządzenie
	else if (nodeIndex > nodeCount) nodeIndex = nodeCount;		//za duży -> ostatnie urządzenie

	if (!noNodeInitialised) {
		if (nodeIndex == this->audioDevices->getCurrentNodeIndex()) return true;	//to samo urządzenie - nic nie robimy
		this->audioDevices->execCurrentDestroyInit(this);		//deinit poprzedniego urządzenia
	}
	this->audioDevices->moveToIndex(nodeIndex);
	this->audioDevices->printCurrent();
	this->audioDevices->execCurrentInit(this);					//init nowego urządzenia
	return true;
}

void radioMenu::menuFunction_equButShortPressed(void) {
	if (this->curretDevice != this->peripheryDevices) {
		//printf("%s switch to peripheryDevices.\r\n", this->radioMainMenu->getCurrentNodeTag());
		pPrintf->feedPrintf("%s switch to peripheryDevices.",
				this->radioMainMenu->getCurrentNodeTag());
		this->setCurrentDeviceMenu_periphery();
		this->curretDevice->printCurrent();
		this->peripheryDevices->execCurrentInit(this);	//wejście do peryferiów: Init bieżącego (pierwszego) peryferium; DestroyInit audio NIE jest wykonywany - urządzenie audio działa dalej w tle
		this->peripheryMenu_TimeoutCounterReset();
		vTaskResume(this->peripheryMenu_taskHandle);
	} else {
		this->curretDevice->moveToNextInLoop(this);	//this (radioMenu*) trafia jako context do callbacków DestroyInit/Init - to zwykłe funkcje, bez this nie mają dostępu do radioMenu (klawisze, LED); wcześniej przekazywano NULL
	}

}


void radioMenu::menuFunction_volButLongRelease(void){
	pPrintf->feedPrintf("%s Dorobic power off radio",
			this->radioMainMenu->getCurrentNodeTag());
	vTaskDelay(pdMS_TO_TICKS(2000));
	assert(0);
}

void radioMenu::menuFunction_volIncrement(void){
	pPrintf->feedPrintf("%s Dorobic zwiekszenie glosnosci.",
			this->radioMainMenu->getCurrentNodeTag());
	vTaskDelay(pdMS_TO_TICKS(2000));
	assert(0);
}

void radioMenu::menuFunction_volDecrement(void){
	pPrintf->feedPrintf("%s Dorobic zmniejszenie glosnosci.",
			this->radioMainMenu->getCurrentNodeTag());
	vTaskDelay(pdMS_TO_TICKS(2000));
	assert(0);
}

void radioMenu:: menuFunction_resetToDefaultLongPress(void){
	pPrintf->feedPrintf("%s Dorobic reset do ustawien fabrycznych",
			this->radioMainMenu->getCurrentNodeTag());
	vTaskDelay(pdMS_TO_TICKS(2000));
	assert(0);
}

//powrót z peryferiów do TEGO SAMEGO urządzenia audio: długie EQ (RETURN_FROM_EQUALIZER_MENU) oraz timeout (task timeoutu wysyła ten sam klawisz do kolejki)
void radioMenu::menuFunction_returnForEqualizerMenu(void) {
	if (eTaskGetState(this->peripheryMenu_taskHandle) == eSuspended) {	//semafor i suspend taska timeoutu - jak w menuFunction_volButShortPressed()
		xSemaphoreGive(this->peripheryMenu_TaskSuspendAllowedSemaphore);
	}
	xSemaphoreTake(this->peripheryMenu_TaskSuspendAllowedSemaphore,
			portMAX_DELAY);
	if (this->curretDevice == this->peripheryDevices) {	//strażnik: klawisz mógł czekać w kolejce, a VOL zdążył już wyjść z peryferiów
		pPrintf->feedPrintf("%s return from peripheryDevices to audioDevices.",
				this->radioMainMenu->getCurrentNodeTag());
		this->menuFunction_swithPeripheryDeviceToAudioDevice();	//DestroyInit peryferium + przełączenie na listę audio
		this->audioDevices->execCurrentReturnInit(this);		//ReturnInit TEGO SAMEGO audio (nie było niszczone) - inaczej niż VOL short, który przechodzi do następnego
		vTaskSuspend(this->peripheryMenu_taskHandle);
	}
	xSemaphoreGive(this->peripheryMenu_TaskSuspendAllowedSemaphore);
}

void radioMenu::menuFunction_volButShortPressed(void) {
	if (eTaskGetState(this->peripheryMenu_taskHandle) == eSuspended) {
		xSemaphoreGive(this->peripheryMenu_TaskSuspendAllowedSemaphore);
	}
	xSemaphoreTake(this->peripheryMenu_TaskSuspendAllowedSemaphore,
			portMAX_DELAY);
	if (this->curretDevice != this->audioDevices) {
		//this->peripheryDevices->mI_executeDestroyInit();
		this->menuFunction_swithPeripheryDeviceToAudioDevice();	//DestroyInit peryferium, BEZ ReturnInit audio - za chwilę moveToNextInLoop() (linia 256) zrobi DestroyInit bieżącego audio i Init następnego
		vTaskSuspend(this->peripheryMenu_taskHandle);
	}
	xSemaphoreGive(this->peripheryMenu_TaskSuspendAllowedSemaphore);
	//printf("%s switch to next audioDevices.\r\n", this->radioMainMenu->getCurrentNodeTag());
	pPrintf->feedPrintf("%s switch to next audioDevices.",
			this->radioMainMenu->getCurrentNodeTag());
	this->curretDevice->moveToNextInLoop(this);	//this (radioMenu*) jako context dla callbacków - jak w menuFunction_equButShortPressed()
}

void radioMenu::menuFunction_swithPeripheryDeviceToAudioDevice(void) {
	//printf("%s switch from peripheryDevices to audioDevices.\r\n", this->radioMainMenu->getCurrentNodeTag());
	pPrintf->feedPrintf("%s switch from peripheryDevices to audioDevices.",
			this->radioMainMenu->getCurrentNodeTag());
	this->peripheryDevices->execCurrentDestroyInit(this);	//wyjście z peryferiów: DestroyInit BIEŻĄCEGO peryferium - musi być PRZED resetToFirst(), potem current = pierwsze peryferium
	this->peripheryDevices->resetToFirst();
	this->setCurrentDeviceMenu_audio();
	this->curretDevice->printCurrent();
	//ReturnInit audio NIE tutaj: VOL short (menuFunction_volButShortPressed) przechodzi dalej do następnego audio (moveToNextInLoop),
	//ReturnInit wykonuje tylko powrót po timeout / długim EQ (menuFunction_returnForEqualizerMenu)
}

uint8_t radioMenu::peripheryMenu_TimeoutCounterIncrement(void) {
	if (xSemaphoreTake(this->peripheryMenu_TimeoutCounterSemaphore,
			portMAX_DELAY) == pdTRUE) {
		uint8_t retVal = this->peripheryMenu_TimeoutCounter++;
		xSemaphoreGive(this->peripheryMenu_TimeoutCounterSemaphore);
		return retVal;
	}
	return this->peripheryMenu_TimeoutCounter;
}

void radioMenu::peripheryMenu_TimeoutCounterReset(void) {
	if (xSemaphoreTake(this->peripheryMenu_TimeoutCounterSemaphore,
			portMAX_DELAY) == pdTRUE) {
		this->peripheryMenu_TimeoutCounter = 0;
		xSemaphoreGive(this->peripheryMenu_TimeoutCounterSemaphore);
	}
}
