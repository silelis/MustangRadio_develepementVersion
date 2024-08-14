/*
 * radioMenu.cpp
 *
 *  Created on: Aug 11, 2024
 *      Author: dbank
 */

#include "SileliS_code/radioMenu.h"



radioMenus::radioMenus() {

	printf("%sMain menu object had been created.\r\n", this->TAG);

	//listy wielowymiarowe dla urzadzęń audio i equalizera
	this->pAudioDeviceList=nullptr;
	this->pAudioDeviceList = new List();
	assert(this->pAudioDeviceLis);
}


radioMenus::~radioMenus() {
	// TODO Auto-generated destructor stub
}

