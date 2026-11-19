/*
 * menuItem.cpp
 *
 *  Created on: Aug 11, 2024
 *      Author: dbank
 */

#include <SileliS_code/menu_menuItem.h>

extern myPrintfTask *pPrintf;

menuItem::menuItem(const char *tag) {
//	if (execFunctionArraySize<= UINT8_MAX){
	this->mI_TAG = tag;

	//this->Init= nullptr;
	//this->destroyInit = nullptr;
	/*this->pExecutableButtonsArray = ExecutableButtonsArray;*/
	pPrintf->feedPrintf("%s: menuItem had been created.", this->mI_TAG);
//	}
}
;

/*
 void menuItem::mi_bindInit(void (*new_func)(void*)){
 if (new_func!=nullptr)
 this->cInit = new_func;
 }

 void menuItem::	mi_bindDestroyInit(void (*new_func)(void*)){
 if (new_func!=nullptr)
 this->cDestroyInit= new_func;
 }

 void menuItem::	mi_bindReturnInit(void (*new_func)(void*)){
 if (new_func!=nullptr)
 this->cReturnInit= new_func;
 }*/

/********************************************************************
 * @brief  Zapisuje w TYM węźle wskaźnik na callback Init - wejście
 * 			do urządzenia (utworzenie / uruchomienie urządzenia).
 *
 * @param  [new_func] [void(*)(void*)] funkcja callback; nullptr jest
 * 			ignorowany (poprzedni callback zostaje)
 *
 * @return NONE
 *
 * @note   protected: wołać przez myList::bindCurrent(), które wybiera
 * 			BIEŻĄCY węzeł listy. Wywołanie na xxxDevices (głowa
 * 			listy) po addAtEnd() nadpisałoby callback pierwszego węzła.
 *******************************************************************/
void menuItem::mi_bindInit(void (*new_func)(void*)) {
	this->mi_bindFunction(&this->cInit, new_func);
}

/********************************************************************
 * @brief  Zapisuje w TYM węźle wskaźnik na callback DestroyInit -
 * 			wyjście z urządzenia (zniszczenie / zatrzymanie urządzenia).
 * 			Parametry i uwagi jak mi_bindInit().
 *******************************************************************/
void menuItem::mi_bindDestroyInit(void (*new_func)(void*)) {
	this->mi_bindFunction(&this->cDestroyInit, new_func);
}

/********************************************************************
 * @brief  Zapisuje w TYM węźle wskaźnik na callback ReturnInit -
 * 			powrót do urządzenia, które NIE było niszczone (np. audio
 * 			po wyjściu z peryferiów): przywrócenie klawiszy, LED,
 * 			steppera bez ponownej inicjalizacji urządzenia.
 * 			Parametry i uwagi jak mi_bindInit().
 *******************************************************************/
void menuItem::mi_bindReturnInit(void (*new_func)(void*)) {
	this->mi_bindFunction(&this->cReturnInit, new_func);
}

/********************************************************************
 * @brief  Wspólna implementacja mi_bind*(): zapisuje new_func pod
 * 			adres target (jedno z pól cInit / cDestroyInit /
 * 			cReturnInit).
 *
 * @param  [target] [void(**)(void*)] adres pola przechowującego callback
 * @param  [new_func] [void(*)(void*)] nowy callback
 *
 * @return NONE
 *
 * @note   Gdy new_func lub target == nullptr - nic nie robi (callback
 * 			nie jest kasowany). Dlatego nie da się "odbindować"
 * 			callbacka przez przekazanie nullptr.
 *******************************************************************/
void menuItem::mi_bindFunction(void (**target)(void*),
		void (*new_func)(void*)) {
	if (new_func != nullptr && target != nullptr) {
		*target = new_func;
	}
}

/********************************************************************
 * @brief  Wykonuje callback Init TEGO węzła.
 *
 * @param  [context] [void*] wartość przekazywana do callbacka
 * 			(z radioMenu: wskaźnik na radioMenu)
 *
 * @return NONE
 *
 * @note   protected: wołać przez myList::execCurrentInit() lub
 * 			myList::moveToNextInLoop(), które wybierają BIEŻĄCY węzeł.
 *******************************************************************/
void menuItem::mi_execInit(void *context) {
	this->mi_execFunction(this->cInit, context);
}

/********************************************************************
 * @brief  Wykonuje callback DestroyInit TEGO węzła.
 * 			Parametry i uwagi jak mi_execInit().
 *******************************************************************/
void menuItem::mi_execDestroyInit(void *context) {
	this->mi_execFunction(this->cDestroyInit, context);
}

/********************************************************************
 * @brief  Wykonuje callback ReturnInit TEGO węzła.
 * 			Parametry i uwagi jak mi_execInit().
 *******************************************************************/
void menuItem::mi_execReturnInit(void *context) {
	this->mi_execFunction(this->cReturnInit, context);
}

/********************************************************************
 * @brief  Wspólna implementacja mi_exec*(): wywołuje callback
 * 			z przekazanym context.
 *
 * @param  [callback] [void(*)(void*)] callback do wykonania
 * @param  [context] [void*] argument callbacka
 *
 * @return NONE
 *
 * @note   callback == nullptr (nie zbindowany) jest bezpiecznie
 * 			pomijany - np. peryferia bez ReturnInit.
 *******************************************************************/
void menuItem::mi_execFunction(void (*callback)(void*),
		void *context) {
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
 void menuItem::mI_appendDestroyInit(std::function<void()> newFunc){
 appendFunctionPointer(&this->destroyInit, newFunc);
 }

 void menuItem::mI_executeDestroyInit(void){
 this->executeFunctionPointer(&this->destroyInit);
 }*/

menuItem::~menuItem() {
//	this->delete_pExecutableButtonsArray();
}
