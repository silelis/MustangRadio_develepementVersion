#ifndef myLIST_H
#define myLIST_H

#include <stdint.h>
//#include <iostream>
//#include <cstring>
#include <string.h>
#include <SileliS_code/menu_bindKeyboardToFunction.h>
#include <SileliS_code/menu_menuItem.h>

class myList;
// Forward declaration

struct ListHeader {
	myList *head; // Wskaźnik do pierwszego elementu listy (pierwszego elementu menu)
	myList *currentListNode; // Wskaźnik do aktualnego elementu listy (aktualnego elementu menu)
	uint8_t indexCounter;         // Licznik indeksów

	ListHeader() :
			head(nullptr), currentListNode(nullptr), indexCounter(0) {
	}
};

class myList: public menuItem {
public:
	// Konstruktor
	myList(ListHeader *pointerListHeader,
			const char *nodeName/*, _execute_t*ExecutableButtonsArray /*uint8_t execFunctionArraySize*/);

	// Destruktor
	void deleteList();
	~myList();

	// Dodawanie elementów do listy
	void addAtBeginning(
			const char *nodeName/*, _execute_t *ExecutableButtonsArray /*uint8_t execFunctionArraySize*/);
	void addAtEnd(
			const char *nodeName/*, _execute_t *ExecutableButtonsArray /*uint8_t execFunctionArraySize*/);

	// Poruszanie się po liście
	void resetToFirst();
	myList* moveToNext();
	void moveToNextInLoop(void *context);	//DestroyInit bieżącego, przejście na następny, Init następnego; context przekazywany do callbacków
	void moveToEnd(void);
	void moveToIndex(uint8_t nodeIndex);	//ustawia current na węzeł o indeksie (bez callbacków); 0 -> pierwszy, indeks > liczby węzłów -> ostatni
	uint8_t getCurrentNodeIndex() const;
	uint8_t getNodeCount() const;	//liczba węzłów listy (= indeks ostatniego węzła); 0 - lista pusta
	const char* getCurrentNodeTag();         // const;
	bool isAtEnd() const;

	// Bindowanie i wykonywanie callbacków BIEŻĄCEGO węzła (pListHeader->currentListNode), a nie węzła, na którym wywołano metodę
	void bindCurrent(void (*init)(void*), void (*destroyInit)(void*), void (*returnInit)(void*));
	void execCurrentInit(void *context);
	void execCurrentDestroyInit(void *context);
	void execCurrentReturnInit(void *context);

	// Usuwanie elementu z listy
	//void removeElement(uint8_t indexToDelete);

	// Wydrukowanie elementów listy
	void printList() const;
	void printCurrent() const;

private:
	//char* name;      // Nazwa węzła

	//static myList* currentListNode; // Wskaźnik do aktualnego elementu listy
	myList *nextListNode;      // Wskaźnik do następnego elementu w liście
	//static myList* head;   // Wskaźnik do pierwszego elementu listy
	uint8_t index;   // Indeks węzła
	//static uint8_t indexCounter; // Licznik indeksów

	ListHeader *pListHeader;
	// Prywatna funkcja do renumeracji węzłów
	void renumberNodes();

	// Prywatna funkcja do sprawdzania, czy można dodać element do listy
	bool canAddItem() const;

	// Prywatna funkcja do zwalniania pamięci
	// void freeList();
	myList* createNode(const char *nodeName);
};

#endif // LIST_H
