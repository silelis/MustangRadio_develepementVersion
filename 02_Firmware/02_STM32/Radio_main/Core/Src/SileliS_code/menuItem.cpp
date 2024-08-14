/*
 * menuItem.cpp
 *
 *  Created on: Aug 11, 2024
 *      Author: dbank
 */

#include "SileliS_code/menuItem.h"



menuItem::menuItem(const char* tag) : TAG(tag) {
	//printf("%s: Empty menu had been created. Please set it up.\r\n", this->TAG);
};


menuItem::~menuItem(){
	#warning zastanowić si ę nad destruktorem
}

