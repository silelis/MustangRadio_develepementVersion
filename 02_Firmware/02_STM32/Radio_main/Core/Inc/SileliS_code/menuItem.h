/*
 * menuItem.h
 *
 *  Created on: Aug 11, 2024
 *      Author: dbank
 */

#ifndef SRC_SILELIS_CODE_MENUITEM_H_
#define SRC_SILELIS_CODE_MENUITEM_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>

class menuItem{
public:
	//http://www.embeddeddev.pl/menu-na-lcd-wprowadzenie/
	menuItem(const char* tag);
	~menuItem();


protected:

private:
	const char* TAG;
	//uint8_t menuNumber;
};





#endif /* SRC_SILELIS_CODE_MENUITEM_H_ */
