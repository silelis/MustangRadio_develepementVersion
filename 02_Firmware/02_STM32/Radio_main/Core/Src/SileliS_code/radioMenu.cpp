/*
 * radioMenu.cpp
 *
 *  Created on: Aug 25, 2024
 *      Author: dbank
 */

#include <SileliS_code/radioMenu.h>
//#include "SileliS_code/i2cEngine.h"


extern myPrintfTask* pPrintf;
//extern i2cMaster* pi2cMaster;

radioMegaStruct radioStruct;





radioMenu::radioMenu() :keyboardToFunction(radioStruct.control.ExecutableButtonsArray) {
	queueRadioMenuKbrd = nullptr;
	configASSERT(queueRadioMenuKbrd = xQueueCreate(20, sizeof(keyboardUnion)));

	this->peripheryDevices = nullptr;
	this->audioDevices=nullptr;
	this->radioMainMenu = nullptr;

	this->peripheryMenu_TimeoutCounter=0;
	this->peripheryMenu_TimeoutCounterSemaphore=NULL;
	configASSERT(this->peripheryMenu_TimeoutCounterSemaphore = xSemaphoreCreateBinary());
	xSemaphoreGive(this->peripheryMenu_TimeoutCounterSemaphore);

	configASSERT(this->peripheryMenu_TaskSuspendAllowedSemaphore = xSemaphoreCreateBinary());
	xSemaphoreGive(this->peripheryMenu_TaskSuspendAllowedSemaphore);


	this->peripheryMenu_taskHandle=NULL;


	this->createDeviceMenuList_periphery();
	this->createDeviceMenuList_audio();
	this->createDeviceMenuList_mainMenu();

	this->setCurrentDeviceMenu_audio();
	this->curretDevice->printCurrent();
}

void radioMenu::createDeviceMenuList_audio(void){

	assert(this->audioDevices = new myList(&(this->ListHeader_audioDevices),"FM"/*, this->pExecutableButtonsArray*/));
	this->audioDevices->addAtEnd("DAB+"/*,pExecutableButtonsArray*/);
	this->audioDevices->addAtEnd("AM"/*,pExecutableButtonsArray*/);
	this->audioDevices->addAtEnd("Bluetooth"/*,this->pExecutableButtonsArray*/);
	this->audioDevices->addAtEnd("USB"/*,this->pExecutableButtonsArray*/);
	this->audioDevices->addAtEnd("SD"/*,this->pExecutableButtonsArray*/);
	this->audioDevices->addAtEnd("Aux"/*,this->pExecutableButtonsArray*/);

	//todo: tutaj zamiast ustawiania pierwszego urządzenia powinno by ć zapamiętywanie, któe urządzenie było ustawione, a jeśli inicjalizacja to pierwsze
	this->peripheryDevices->resetToFirst();
	//this->audioDevices->moveToEnd();
	//this->audioDevices->moveToNextInLoop();
	//this->audioDevices->printList();

	//this->setCurrentDeviceMenu_audio();
	//this->curretDevice->printCurrent();
}




void radioMenu::createDeviceMenuList_periphery(void){

	assert(this->peripheryDevices = new myList(&(this->ListHeader_peripheryDevices),"Antena"/*,this->pExecutableButtonsArray*/));
	this->peripheryDevices->addAtEnd("Treble"/*,this->pExecutableButtonsArray*/);
	this->peripheryDevices->addAtEnd("Midle"/*,this->pExecutableButtonsArray*/);
	this->peripheryDevices->addAtEnd("Bass"/*,this->pExecutableButtonsArray*/);
	this->peripheryDevices->addAtEnd("Subwoofer"/*,this->pExecutableButtonsArray*/);
	this->peripheryDevices->addAtEnd("L_H_Boost"/*,this->pExecutableButtonsArray*/);
	this->peripheryDevices->addAtEnd("Left<->Right"/*,this->pExecutableButtonsArray*/);
	this->peripheryDevices->addAtEnd("Front<->Rear"/*,this->pExecutableButtonsArray*/);
	this->peripheryDevices->addAtEnd("Backlight"/*,this->pExecutableButtonsArray*/);
	this->peripheryDevices->addAtEnd("Tip calibration"/*,this->pExecutableButtonsArray*/);

	this->peripheryDevices->resetToFirst();
	//this->peripheryDevices->printList();
}

void radioMenu::createDeviceMenuList_mainMenu(void){

	assert(this->radioMainMenu = new myList(&(this->ListHeader_mainMenu),"MainMenu: "/*,this->pExecutableButtonsArray*/));

	this->appendButtonArrayWithFunctionPointer({{'b', 0x3f}}, std::bind(&radioMenu::menuFunction_equButShortPressed, this));

	this->appendButtonArrayWithFunctionPointer({{'b', 0x7e}}, std::bind(&radioMenu::menuFunction_volButShortPressed, this));
}

void radioMenu::setCurrentDeviceMenu_audio(void){
	this->curretDevice=this->audioDevices;
}

void radioMenu::setCurrentDeviceMenu_periphery(void){
	this->curretDevice=this->peripheryDevices;
}


BaseType_t radioMenu::queueRadioMenuKbrdSend(const void * kbrdUnionSend){
	return xQueueSend(this->queueRadioMenuKbrd, kbrdUnionSend, pdMS_TO_TICKS(700));
}

BaseType_t radioMenu::queueRadioMenuKbrdReceive(keyboardUnion* kbrdUnionReceived){
	BaseType_t retVal =  xQueueReceive(this->queueRadioMenuKbrd, kbrdUnionReceived, portMAX_DELAY);
	this->peripheryMenu_TimeoutCounterReset();
	return  retVal;
}

radioMenu::~radioMenu() {
		//vTaskDelete(this->taskHandle_manageTheRadioManue);
		vQueueDelete(queueRadioMenuKbrd);
		xSemaphoreTake(this->peripheryMenu_TimeoutCounterSemaphore, portMAX_DELAY);
		vSemaphoreDelete(this->peripheryMenu_TimeoutCounterSemaphore);
		xSemaphoreTake(this->peripheryMenu_TaskSuspendAllowedSemaphore, portMAX_DELAY);
		vSemaphoreDelete(this->peripheryMenu_TaskSuspendAllowedSemaphore);

		vTaskDelete(this->peripheryMenu_taskHandle);

		delete [] this->radioMainMenu;
		delete [] this->audioDevices;
		delete [] this->peripheryDevices;

		//memset(&(this->ListHeader_audioDevices),0,sizeof(ListHeader));
		//memset(&(this->ListHeader_peripheryDevices),0,sizeof(ListHeader));
		//memset(&(this->ListHeader_mainMenu),0,sizeof(ListHeader));
		this->curretDevice=nullptr;

		//this->curretDevice->getCurrentNodeTag()
}

const char* radioMenu::getCurrentNodeTag()/* const*/{
	return (const char*) this->curretDevice->getCurrentNodeTag();
}


void radioMenu::menuFunction_equButShortPressed(void){
	if (this->curretDevice != this->peripheryDevices){
		//printf("%s switch to peripheryDevices.\r\n", this->radioMainMenu->getCurrentNodeTag());
		pPrintf->feedPrintf("%s switch to peripheryDevices.", this->radioMainMenu->getCurrentNodeTag());
		this->setCurrentDeviceMenu_periphery();
		this->curretDevice->printCurrent();
		this->peripheryMenu_TimeoutCounterReset();
		vTaskResume(this->peripheryMenu_taskHandle);
	}
	else {
		this->curretDevice->moveToNextInLoop();
	}

}

void radioMenu::menuFunction_volButShortPressed(void){
	if (eTaskGetState(this->peripheryMenu_taskHandle)==eSuspended){
		xSemaphoreGive(this->peripheryMenu_TaskSuspendAllowedSemaphore);
	}
	xSemaphoreTake(this->peripheryMenu_TaskSuspendAllowedSemaphore, portMAX_DELAY);
	if (this->curretDevice != this->audioDevices){
		//this->peripheryDevices->mI_executeDeInit();
		this->menuFunction_swithPeripheryDeviceToAudioDevice();
		vTaskSuspend(this->peripheryMenu_taskHandle);
	}
	xSemaphoreGive(this->peripheryMenu_TaskSuspendAllowedSemaphore);
	//printf("%s switch to next audioDevices.\r\n", this->radioMainMenu->getCurrentNodeTag());
	pPrintf->feedPrintf("%s switch to next audioDevices.", this->radioMainMenu->getCurrentNodeTag());
	this->curretDevice->moveToNextInLoop();
}

void radioMenu::menuFunction_swithPeripheryDeviceToAudioDevice(void){
	//printf("%s switch from peripheryDevices to audioDevices.\r\n", this->radioMainMenu->getCurrentNodeTag());
	pPrintf->feedPrintf("%s switch from peripheryDevices to audioDevices.", this->radioMainMenu->getCurrentNodeTag());
	this->peripheryDevices->resetToFirst();
	this->setCurrentDeviceMenu_audio();
	this->curretDevice->printCurrent();

	this->curretDevice->mi_execBackInit(NULL);
}

void radioMenu::peripheryMenu_onTimeoutActions(void){
	this->menuFunction_swithPeripheryDeviceToAudioDevice();
}


uint8_t radioMenu::peripheryMenu_TimeoutCounterIncrement(void){
	if (xSemaphoreTake(this->peripheryMenu_TimeoutCounterSemaphore, portMAX_DELAY) == pdTRUE){
		uint8_t retVal = this->peripheryMenu_TimeoutCounter++;
		xSemaphoreGive(this->peripheryMenu_TimeoutCounterSemaphore);
		return retVal;
	}
	return this->peripheryMenu_TimeoutCounter;
}

void radioMenu::peripheryMenu_TimeoutCounterReset(void){
	if (xSemaphoreTake(this->peripheryMenu_TimeoutCounterSemaphore, portMAX_DELAY) == pdTRUE){
		this->peripheryMenu_TimeoutCounter=0;
		xSemaphoreGive(this->peripheryMenu_TimeoutCounterSemaphore);
	}
}
