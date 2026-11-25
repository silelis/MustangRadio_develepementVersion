#include <SileliS_code/menu_myList.h>

extern myPrintfTask *pPrintf;

// Konstruktor
myList::myList(ListHeader *pointerListHeader,
		const char *nodeName) :
		menuItem(nodeName) {
	assert(pointerListHeader);	//nagłówek listy używany od razu niżej (head, indexCounter) - nullptr = crash
	this->pListHeader = pointerListHeader;

	if (this->pListHeader->head == nullptr) {
		this->pListHeader->head = this;
	}
	nextListNode = nullptr;
	this->pListHeader->indexCounter++;
	this->index = this->pListHeader->indexCounter;
	this->pListHeader->currentListNode = this;
}

// Destruktor
myList::~myList() {
//	if(head==nullptr)
//		return;
}

// Dodanie elementu na początku listy
void myList::addAtBeginning(
		const char *nodeName) {
	if (!canAddItem())
		return;
	myList *newNode; // = new myList(nodeName, execFunctionArraySize);
	newNode = new (std::nothrow) myList(pListHeader,
			nodeName);
	assert(newNode);
	newNode->nextListNode = this->pListHeader->head;
	this->pListHeader->head = newNode;
	this->pListHeader->currentListNode = newNode; // Ustawienie current na nowo dodany element
	renumberNodes();
}

// Dodanie elementu na końcu listy
void myList::addAtEnd(
		const char *nodeName) {
	if (!canAddItem())
		return;

	myList *newNode; // = new myList(nodeName, execFunctionArraySize);
	newNode = new (std::nothrow) myList(pListHeader,	//nothrow: przy braku pamięci nullptr -> assert niżej (zwykłe new przy -fno-exceptions robi abort(), assert byłby martwy) - jak w addAtBeginning()
			nodeName);
	assert(newNode);
	if (!this->pListHeader->head) {
		this->pListHeader->head = newNode;
	} else {
		myList *temp = this->pListHeader->head;
		while (temp->nextListNode) {
			temp = temp->nextListNode;
		}
		temp->nextListNode = newNode;
	}

	this->pListHeader->currentListNode = newNode; // Ustawienie current na nowo dodany element
	renumberNodes();
}

// Resetowanie current do pierwszego elementu listy
void myList::resetToFirst() {
	this->pListHeader->currentListNode = this->pListHeader->head;
}

// Przesunięcie current na następny element listy
myList* myList::moveToNext() {
	if (this->pListHeader->currentListNode) {
		this->pListHeader->currentListNode =
				this->pListHeader->currentListNode->nextListNode;
	}
	return this->pListHeader->currentListNode;
}

void myList::moveToEnd(void) {
	while (!this->isAtEnd()) {
		this->moveToNext();
	}
}

/********************************************************************
 * @brief  Ustawia current na węzeł o wskazanym indeksie.
 *
 * Zaczyna od pierwszego węzła (resetToFirst()) i przesuwa się do
 * przodu, dopóki index < nodeIndex i nie jest na końcu listy
 * (lista jednokierunkowa - nie da się cofać).
 * Tylko przestawia wskaźnik - nie wywołuje callbacków.
 *
 * @param  [nodeIndex] [uint8_t] indeks węzła (od 1);
 * 			0 -> pierwszy, indeks > liczby węzłów -> ostatni
 *
 * @return NONE
 * @note   Pusta lista (indexCounter == 0) - nic nie robi.
 *******************************************************************/
void myList::moveToIndex(uint8_t nodeIndex) {
	if (this->pListHeader->indexCounter == 0) return;	//pusta lista - currentListNode == nullptr
	this->resetToFirst();
	while (this->pListHeader->currentListNode->index < nodeIndex && !this->isAtEnd()) {
		this->moveToNext();
	}
}
/********************************************************************
 * @brief  Przejście na następny węzeł listy (w pętli: po ostatnim
 * 			wraca na pierwszy) z wykonaniem callbacków urządzeń.
 *
 * Kolejność: DestroyInit bieżącego węzła -> przesunięcie current ->
 * Init nowego bieżącego węzła. Obsługuje tylko przejścia WEWNĄTRZ
 * jednej listy. Przejścia między listami (audio <-> peryferia, start
 * radia, timeout) obsługuje radioMenu przez execCurrent*().
 *
 * @param  [context] [void*] wartość przekazywana do callbacków
 * 			DestroyInit i Init (z radioMenu: wskaźnik na radioMenu).
 *
 * @return NONE
 *
 * @note   Callbacki z nullptr są pomijane (menuItem::mi_execFunction).
 *******************************************************************/
void myList::moveToNextInLoop(void *context) {
	pPrintf->feedPrintf("Exit from %d: %s",
			this->pListHeader->currentListNode->index,
			this->pListHeader->currentListNode->mI_TAG);
	//this->pListHeader->currentListNode->mI_executeDestroyInit();
	this->execCurrentDestroyInit(context);

	if (!moveToNext())
		resetToFirst();
	pPrintf->feedPrintf("Enter to %d: %s",
			this->pListHeader->currentListNode->index,
			this->pListHeader->currentListNode->mI_TAG);

	//this->pListHeader->currentListNode->mI_executeInit();
	this->execCurrentInit(context);
}

/********************************************************************
 * @brief  Binduje callbacki Init, DestroyInit i ReturnInit do
 * 			BIEŻĄCEGO węzła listy (pListHeader->currentListNode).
 *
 * Wywoływać zaraz po konstruktorze myList lub po addAtEnd() /
 * addAtBeginning() - one ustawiają current na nowo dodany węzeł.
 * Nie używać xxxDevices->mi_bindInit() po addAtEnd(): xxxDevices
 * zawsze wskazuje głowę listy, więc nadpisałoby to callbacki
 * pierwszego węzła zamiast dodanego.
 *
 * @param  [init] [void(*)(void*)] wejście do urządzenia (tworzy je)
 * @param  [destroyInit] [void(*)(void*)] wyjście z urządzenia
 * 			(niszczy je)
 * @param  [returnInit] [void(*)(void*)] powrót do urządzenia, które
 * 			nie było niszczone (przywraca klawisze, LED, stepper);
 * 			nullptr gdy niepotrzebny (np. peryferia)
 *
 * @return NONE
 *
 * @note   current jest odczytywany RAZ do zmiennej lokalnej: wszystkie
 * 			trzy callbacki na pewno trafiają do tego samego węzła,
 * 			a kod jest krótszy niż trzykrotne
 * 			this->pListHeader->currentListNode->...
 *******************************************************************/
void myList::bindCurrent(void (*init)(void*), void (*destroyInit)(void*),
		void (*returnInit)(void*)) {
	myList *current = this->pListHeader->currentListNode;
	current->mi_bindInit(init);
	current->mi_bindDestroyInit(destroyInit);
	current->mi_bindReturnInit(returnInit);
}

/********************************************************************
 * @brief  Wykonuje callback Init BIEŻĄCEGO węzła listy.
 *
 * Używane przez radioMenu przy przejściach między listami (start
 * radia, wejście do peryferiów). curretDevice / xxxDevices wskazują
 * głowę listy - wywołanie mi_execInit() wprost na nich wykonałoby
 * callback pierwszego węzła, a nie bieżącego urządzenia.
 *
 * @param  [context] [void*] wartość przekazywana do callbacka
 *
 * @return NONE
 * @note   nullptr jest pomijany (menuItem::mi_execFunction)
 *******************************************************************/
void myList::execCurrentInit(void *context) {
	this->pListHeader->currentListNode->mi_execInit(context);
}

/********************************************************************
 * @brief  Wykonuje callback DestroyInit BIEŻĄCEGO węzła listy
 * 			(np. wyjście z peryferiów). Opis jak execCurrentInit().
 *******************************************************************/
void myList::execCurrentDestroyInit(void *context) {
	this->pListHeader->currentListNode->mi_execDestroyInit(context);
}

/********************************************************************
 * @brief  Wykonuje callback ReturnInit BIEŻĄCEGO węzła listy
 * 			(powrót do audio po timeout / długim EQ).
 * 			Opis jak execCurrentInit().
 *******************************************************************/
void myList::execCurrentReturnInit(void *context) {
	this->pListHeader->currentListNode->mi_execReturnInit(context);
}

/********************************************************************
 * @brief  Zwraca indeks bieżącego węzła listy (currentListNode).
 *
 * @return [uint8_t] indeks węzła (od 1); 0 - lista pusta
 *******************************************************************/
uint8_t myList::getCurrentNodeIndex() const {
	if (this->pListHeader->indexCounter == 0) return 0;	//pusta lista - brak węzła (indeksy węzłów zaczynają się od 1)
	return this->pListHeader->currentListNode->index;
}

/********************************************************************
 * @brief  Zwraca liczbę węzłów listy (pListHeader->indexCounter).
 *
 * Indeksy węzłów są ciągłe 1..getNodeCount() (renumberNodes()), więc
 * wynik jest jednocześnie indeksem ostatniego węzła - pozwala
 * przyciąć indeks do zakresu bez przechodzenia po liście.
 *
 * @return [uint8_t] liczba węzłów; 0 - lista pusta
 *******************************************************************/
uint8_t myList::getNodeCount() const {
	return this->pListHeader->indexCounter;
}

const char* myList::getCurrentNodeTag()/* const*/{
	return (const char*) this->pListHeader->currentListNode->mI_TAG;
}

// Sprawdzanie, czy current znajduje się na końcu listy
bool myList::isAtEnd() const {
	return this->pListHeader->currentListNode->nextListNode == nullptr;
}

// Wydrukowanie wszystkich elementów listy
void myList::printList() const {
	myList *temp = this->pListHeader->head;
	while (temp) {
		pPrintf->feedPrintf("Node Index: %d, Name: %s.\r\n", temp->index,
				temp->mI_TAG);
		temp = temp->nextListNode;
	}
}

// Wydrukowanie aktualnego elementu listy
void myList::printCurrent() const {
	if (this->pListHeader->currentListNode) {
		pPrintf->feedPrintf("%s: is current device.",
				this->pListHeader->currentListNode->mI_TAG);
	} else {
		pPrintf->feedPrintf("No current node.");
	}
}

// Renumeracja węzłów listy
void myList::renumberNodes() {
	myList *temp = this->pListHeader->head;
	uint8_t tempIndex = 0;
	while (temp) {
		temp->index = ++tempIndex;
		temp = temp->nextListNode;
	}
}

// Sprawdzanie, czy można dodać element do listy
bool myList::canAddItem() const {
	return this->pListHeader->indexCounter < UINT8_MAX; // Maksymalna wartość uint8_t to 255, więc sprawdzamy, czy jest mniejsza niż 255
}

void myList::deleteList() {
	ListHeader *header = this->pListHeader;	//lokalna kopia - w pętli usuwany jest także this (głowa listy), po tym nie wolno czytać pól this
	while (header->head != nullptr) {
		myList *temp = header->head;   // Zapamiętaj aktualny head
		header->head = header->head->nextListNode; // Przejdź do następnego elementu
		delete temp;           // Usuń aktualny element	aka ~myList()
		header->indexCounter--;
	}
	header->currentListNode = nullptr; // Po usunięciu wszystkich elementów ustaw current na nullptr
}

