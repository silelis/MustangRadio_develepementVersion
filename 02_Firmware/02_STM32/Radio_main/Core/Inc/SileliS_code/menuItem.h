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
#include <iostream>
#include <functional>
#include "SileliS_code/myPrintfTask.h"

struct execute_t{
	keyboardUnion buttonSequence;		//kod klawisza wywołujący funkcję
	std::function<void()> functionPointer;		//wskaźnik do funkcji jaka ma być wywołana
};


class menuItem{
public:
	//http://www.embeddeddev.pl/menu-na-lcd-wprowadzenie/
	menuItem(const char* tag, uint8_t execFunctionArraySize);
	~menuItem();

	//void	mI_appendInit(void (*newFunc)());
	void	mI_appendInit(std::function<void()> newFunc);
	void	mI_executeInit(void);
	//void	mI_appendDeInit(void (*newFunc)());
	void	mI_appendDeInit(std::function<void()> newFunc);
	void	mI_executeDeInit(void);
	bool	mI_executeExecutableButtons(keyboardUnion buttonSequence);
	bool	mI_appendExecFunctionArry(keyboardUnion buttonSequence, std::function<void()> newFunc);

protected:
	const char*	mI_TAG;

private:
	execute_t*	pExecutableButtons;
	uint8_t		execFunctionArrySize;			//rozmiar tablicy execute_t
	uint8_t		execFunctionArryAppended;		//aktualna ilość par klawisze + funkcja, jakiew zostały appendowane do execute_t
	void	delete_pExecutableButtonsArray(void);
	bool	create_pExecutableButtonsArray(uint8_t arraySize);

	//void	(*Init)();
	//void	(*deInit)();


	std::function<void()> Init;
	std::function<void()> deInit;

	//void	appendFunctionPointer(void (**funcPtr)(), void (*newFunc)());
	void	appendFunctionPointer(std::function<void()>* funcPtr, std::function<void()> newFunc);
	void	executeFunctionPointer(std::function<void()>* funcPtr);

	uint8_t	searchExecFunctionForButtonSequence(keyboardUnion buttonSequence);
	bool	isExecFunctionInButtonSequence(keyboardUnion buttonSequence);
};

#endif /* SRC_SILELIS_CODE_MENUITEM_H_ */
