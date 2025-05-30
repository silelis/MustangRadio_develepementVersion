/*
 * radioMenu.h
 *
 *  Created on: Aug 25, 2024
 *      Author: dbank
 */

#ifndef INC_SILELIS_CODE_RADIOMENU_H_
#define INC_SILELIS_CODE_RADIOMENU_H_
#include "SileliS_code/myList.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "comunicationStructures.h"
#include "SileliS_code/keyboardToFunction.h"
#include <iostream>
#include <functional>
#include "SileliS_code/radioMegaStruct.h"

class radioMenu: public keyboardToFunction {

public:
	radioMenu();
	virtual ~radioMenu();
	BaseType_t queueRadioMenuKbrdSend(const void * kbrdUnionSend);
	BaseType_t queueRadioMenuKbrdReceive(keyboardUnion* kbrdUnionReceived);

	bool executeButtonFrom_radioMainMenu(keyboardUnion buttonSequence);
	bool executeButtonFrom_curretDevice(keyboardUnion buttonSequence);
	const char* getCurrentNodeTag();// const;


protected:


	void setCurrentDeviceMenu_audio(void);
	void setCurrentDeviceMenu_periphery(void);
private:
	const char	*TAG = "Main menu log: ";
	QueueHandle_t queueRadioMenuKbrd;	//kolejka, którza pobiera klawisze otrzymane z ESP32 i przekazuje do menuRadio


	ListHeader  ListHeader_audioDevices;
	ListHeader  ListHeader_peripheryDevices;
	ListHeader  ListHeader_mainMenu;

	myList*		radioMainMenu;				//zerowe menu/lista radio obsługująca but_ON/OFF, long_but_ON/OFF,VOL_CW,VOL_CCW,but_EQU,long_but_EQU
	myList*		curretDevice;				//wskaźnik do obecnie obsługiwanej przesz menu listy przełącza się pomiędzy audioDevices i peripheryDevices
	myList*		audioDevices;				//lista wszystkich dostępnych menu urządzęń audio
	myList*		peripheryDevices;			//lista wszystkich dostępnych do konfiguracji peryferiów

	void createDeviceMenuList_audio(void);
	void createDeviceMenuList_periphery(void);
	void createDeviceMenuList_mainMenu(void);

	void 		menuFunction_equButShortPressed(void);
	void 		menuFunction_volButShortPressed(void);
	void 		menuFunction_swithPeripheryDeviceToAudioDevice(void);
	//friend		void manageRadioButtonsAndManue(void* thing);

	uint8_t				peripheryMenu_TimeoutCounter;
	SemaphoreHandle_t	peripheryMenu_TimeoutCounterSemaphore;
	SemaphoreHandle_t	peripheryMenu_TaskSuspendAllowedSemaphore;
public:
	TaskHandle_t		peripheryMenu_taskHandle;
private:
	uint8_t peripheryMenu_TimeoutCounterIncrement(void);
	void peripheryMenu_TimeoutCounterReset(void);
	void peripheryMenu_onTimeoutActions(void);
	friend void peripheryMenuTimeoutFunction(void* thing);
};

#endif /* INC_SILELIS_CODE_RADIOMENU_H_ */
