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
#include "comunicationStructures.h"
#include <string.h>
#include <SileliS_code/menu_bindKeyboardToFunction.h>
#include <iostream>
#include <functional>
#include "SileliS_code/myPrintfTask.h"

struct execute_t {
	keyboardUnion buttonSequence;		//kod klawisza wywołujący funkcję
	std::function<void()> functionPointer;//wskaźnik do funkcji jaka ma być wywołana
};

class menuItem {
public:
	//http://www.embeddeddev.pl/menu-na-lcd-wprowadzenie/
	menuItem(const char *tag);
	~menuItem();

//	void	mI_appendInit(std::function<void()> newFunc);
//	void	mI_executeInit(void);
//	void	mI_appendDestroyInit(std::function<void()> newFunc);
//	void	mI_executeDestroyInit(void);

protected:
	const char *mI_TAG;
	//protected: wywoływane tylko przez myList (bindCurrent / execCurrent*) na BIEŻĄCYM węźle;
	//z zewnątrz (radioMenu) wywołanie na xxxDevices trafiałoby w głowę listy zamiast w bieżące urządzenie
	void mi_bindInit(void (*new_func)(void*));
	void mi_bindDestroyInit(void (*new_func)(void*));
	void mi_bindReturnInit(void (*new_func)(void*));
	void mi_execInit(void*);
	void mi_execDestroyInit(void*);
	void mi_execReturnInit(void*);

private:
//	std::function<void()> Init;
//	std::function<void()> destroyInit;
	void (*cInit)(void*) = nullptr;
	void (*cReturnInit)(void*)= nullptr;
	void (*cDestroyInit)(void*)= nullptr;

	void mi_bindFunction(void (**target)(void*), void (*new_func)(void*));
	void mi_execFunction(void (*callback)(void*), void *context);
	/*_execute_t	*pExecutableButtonsArray;		//pointer to executable button arrayi in radioMeny class*/

//	void	appendFunctionPointer(std::function<void()>* funcPtr, std::function<void()> newFunc);
//	void	executeFunctionPointer(std::function<void()>* funcPtr);

};

#endif /* SRC_SILELIS_CODE_MENUITEM_H_ */
