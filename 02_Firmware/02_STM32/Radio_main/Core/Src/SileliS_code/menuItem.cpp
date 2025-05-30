/*
 * menuItem.cpp
 *
 *  Created on: Aug 11, 2024
 *      Author: dbank
 */

#include "SileliS_code/menuItem.h"

extern myPrintfTask* pPrintf;


menuItem::menuItem(const char* tag/*, _execute_t *ExecutableButtonsArray*/) {
//	if (execFunctionArraySize<= UINT8_MAX){
		this->mI_TAG = tag;

		//this->Init= nullptr;
		//this->deInit = nullptr;
		/*this->pExecutableButtonsArray = ExecutableButtonsArray;*/
		pPrintf->feedPrintf("%s: menuItem had been created.", this->mI_TAG);
//	}
};

/*
void menuItem::mi_bindInit(void (*new_init)(void*)){
	if (new_init!=nullptr)
		this->cInit = new_init;
}

void menuItem::	mi_bindDeInit(void (*new_init)(void*)){
	if (new_init!=nullptr)
		this->cDeInit= new_init;
}

void menuItem::	mi_bindBackInit(void (*new_init)(void*)){
	if (new_init!=nullptr)
		this->cBackInit= new_init;
}*/

void menuItem::mi_bindInit(void (*new_init)(void*)) {
	this->mi_bindFunction(&this->cInit, new_init);
}

void menuItem::mi_bindDeInit(void (*new_init)(void*)) {
	this->mi_bindFunction(&this->cDeInit, new_init);
}

void menuItem::mi_bindBackInit(void (*new_init)(void*)) {
	this->mi_bindFunction(&this->cBackInit, new_init);
}

void menuItem::mi_bindFunction(void (**target)(void*), void (*new_func)(void*)) {
    if (new_func != nullptr && target != nullptr) {
        *target = new_func;
    }
}




void menuItem::mi_execInit(void* context = nullptr) {
	this->mi_execFunction(this->cInit, context);
}

void menuItem::mi_execDeInit(void* context = nullptr) {
	this->mi_execFunction(this->cDeInit, context);
}

void menuItem::mi_execBackInit(void* context = nullptr) {
	this->mi_execFunction(this->cBackInit, context);
}


void menuItem::mi_execFunction(void (*callback)(void*), void* context = nullptr) {
    if (callback != nullptr) {
        callback(context);
    }
}


/*
void menuItem::appendFunctionPointer(std::function<void()>* funcPtr, std::function<void()> newFunc){
	*funcPtr = newFunc;
}*/


/*
void menuItem::executeFunctionPointer(std::function<void()>* funcPtr) {
    if (*funcPtr) {
        (*funcPtr)(); // Prawidłowe wywołanie std::function<void()>
    } else {
        pPrintf->feedPrintf("%s: Pointer to function is empty.", this->mI_TAG);
    }
}*/

/*
void menuItem::mI_appendInit(std::function<void()> newFunc) {
	appendFunctionPointer(&this->Init, newFunc);
}*/

/*
void menuItem::mI_executeInit(void){
	this->executeFunctionPointer(&this->Init);
}*/


/*
void menuItem::mI_appendDeInit(std::function<void()> newFunc){
	appendFunctionPointer(&this->deInit, newFunc);
}

void menuItem::mI_executeDeInit(void){
	this->executeFunctionPointer(&this->deInit);
}*/


menuItem::~menuItem(){
//	this->delete_pExecutableButtonsArray();
}
