/*
 * radioMenu.h
 *
 *  Created on: Aug 11, 2024
 *      Author: dbank
 */

#ifndef INC_RADIOMENU_H_
#define INC_RADIOMENU_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include <list>
#include "SileliS_code/list.h"
#include "SileliS_code/menuItem.h"


/*typedef struct {
	const char *menuTAG;
	std::list<menuItem> deviceList;					//lista wszystkich urządzeń w menu
	std::list<menuItem>::iterator currentDevice;	//iterator (wskaźnik do) aktualnego urządzenia używanego w menu
	std::list<menuItem>::iterator maxDevices;		//ilość wszystkich urządzeń w menu
}menuState;*/

class radioMenus {
public:
	radioMenus();
	virtual ~radioMenus();
	List* pAudioDeviceList;
	List* pPeripheryDeviceList;

	//listy specjalne jednowymiarowe dla rotora volume
	List* pVolLongList;		//długie przyciśniecie przycisku Vol
	List* pVolShortList;		//długie przyciśniecie przycisku Vol
	List* pVolCWList;		//długie przyciśniecie przycisku Vol
	List* pVolCCWList;		//długie przyciśniecie przycisku Vol

	//listy specjalne jednowymiarowe dla rotora equalisera
	//List* pEquLongList;		//długie przyciśniecie przycisku Vol
	List* pEquShortList;		//długie przyciśniecie przycisku Vol
	List* pEquCWList;		//długie przyciśniecie przycisku Vol
	List* pEquCCWList;		//długie przyciśniecie przycisku Vol


protected:


private:
	const char *TAG = "Menu log: ";
	//menuState audioDevices;

	//void

};

#endif /* INC_RADIOMENU_H_ */
