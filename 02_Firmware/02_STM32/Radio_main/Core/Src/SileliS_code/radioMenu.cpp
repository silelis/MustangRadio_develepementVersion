/*
 * radioMenu.cpp
 *
 *  Created on: Aug 25, 2024
 *      Author: dbank
 */

#include <SileliS_code/radioMenu.h>

radioMenu::radioMenu() {
	queueRadioMenuKbrd = nullptr;
	configASSERT(queueRadioMenuKbrd = xQueueCreate(20, sizeof(keyboardUnion)));

	this->peripheryDevices = nullptr;
	this->audioDevices=nullptr;
	this->radioMainMenu = nullptr;


	this->createDeviceMenuList_periphery();
	this->createDeviceMenuList_audio();
	this->createDeviceMenuList_mainMenu();

	this->setCurrentDeviceMenu_audio();
}

void radioMenu::createDeviceMenuList_audio(void){

	assert(this->audioDevices = new myList(&(this->ListHeader_audioDevices),"1st audio",5));
	this->audioDevices->addAtEnd("2nd audio",5);
	this->audioDevices->addAtEnd("3rd audio",5);

	//todo: tutaj zamiast ustawiania pierwszego urządzenia powinno by ć zapamiętywanie, któe urządzenie było ustawione, a jeśli inicjalizacja to pierwsze
	this->peripheryDevices->resetToFirst();
	//this->audioDevices->moveToEnd();
	//this->audioDevices->moveToNextInLoop();
	this->audioDevices->printList();

	//this->setCurrentDeviceMenu_audio();
	//this->curretDevice->printCurrent();
}




void radioMenu::createDeviceMenuList_periphery(void){

	assert(this->peripheryDevices = new myList(&(this->ListHeader_peripheryDevices),"1st periph",3));
	this->peripheryDevices->addAtEnd("2nd periph",3);
	this->peripheryDevices->addAtEnd("3rd periph",3);

	this->peripheryDevices->resetToFirst();
	this->peripheryDevices->printList();
}

void radioMenu::createDeviceMenuList_mainMenu(void){

	assert(this->radioMainMenu = new myList(&(this->ListHeader_mainMenu),"MainMenu",5));

	//this->radioMainMenu->mI_appendDeInit(newFunc);
	//this->radioMainMenu->mI_appendInit(newFunc);
	keyboardUnion buttonSequence;

	//this->radioMainMenu->mI_appendExecFunctionArry(buttonSequence, newFunc);


	this->radioMainMenu->printList();
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
	return xQueueReceive(this->queueRadioMenuKbrd, kbrdUnionReceived, portMAX_DELAY);
}

radioMenu::~radioMenu() {
		//vTaskDelete(this->taskHandle_manageTheRadioManue);
		vQueueDelete(queueRadioMenuKbrd);

		delete [] this->radioMainMenu;
		delete [] this->audioDevices;
		delete [] this->peripheryDevices;

		//memset(&(this->ListHeader_audioDevices),0,sizeof(ListHeader));
		//memset(&(this->ListHeader_peripheryDevices),0,sizeof(ListHeader));
		//memset(&(this->ListHeader_mainMenu),0,sizeof(ListHeader));
		this->curretDevice=nullptr;
}

