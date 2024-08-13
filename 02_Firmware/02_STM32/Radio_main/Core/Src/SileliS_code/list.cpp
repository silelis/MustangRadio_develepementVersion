/*
 * list.cpp
 *
 *  Created on: Aug 12, 2024
 *      Author: dbank
 */

#include <SileliS_code/list.h>

// Konstruktor: inicjalizuje pustą listę
List::List() : head(nullptr), current(nullptr), indexCounter(0) {}

// Destruktor: zwalnia pamięć zajmowaną przez listę
List::~List() {
    freeList();
}

// Prywatna funkcja pomocnicza do tworzenia nowego węzła
Node* List::createNode(void) {
    Node* newNode = new Node;  // Alokacja pamięci dla nowego węzła
    if (newNode!=0){
    	this->indexCounter++;
        newNode->nodeIndex = this->indexCounter;      // Ręczne przypisanie danych
        newNode->next = nullptr;   // Ręczne ustawienie wskaźnika na nullptr
        return newNode;
    }

}

bool List::canAddItem(void){
	if (this->indexCounter<UINT8_MAX){
		return true;
	}
	else{
		printf("List had reached max number of items.\r\n");
		return false;
	}
}

// Dodawanie elementu na początku listy
void List::addAtBeginning(void) {
	if (this->canAddItem()){
	    Node* newNode = createNode();
	    newNode->next = head;
	    head = newNode;
	}
}

// Dodawanie elementu na końcu listy
void List::addAtEnd() {
	if(this->canAddItem()){
	    Node* newNode = createNode();
	    if (head == nullptr) {
	        head = newNode;
	    } else {
	        Node* temp = head;
	        while (temp->next != nullptr) {
	            temp = temp->next;
	        }
	        temp->next = newNode;
	    }
	}
}

// Ustawienie wskaźnika current na pierwszy element listy
void List::resetToFirst() {
    current = head;
}

// Przesunięcie wskaźnika current na następny element
void List::moveToNext() {
	if (head!=nullptr){
		current = current->next;
		if (current== nullptr) {	//osiągnięto ostatni element listy
			current = head;
		}
	}

/*    if (current != nullptr) {
        current = current->next;
    }*/
}

// Zwrócenie aktualnego elementuisAtEnd
uint8_t List::getCurrentElement() {
    if (current != nullptr) {
        return current->nodeIndex;
    } else {
        printf("Nie ma bieżącego elementu.\n");
        return 0;  // Można również użyć innego wskaźnika błędu
    }
}

// Sprawdzenie, czy wskaźnik current jest na końcu listy
bool List::isAtEnd() {
    return current == nullptr;
}

// Usuwanie pierwszego wystąpienia elementu z listy
void List::removeElement(uint8_t data) {
    if (head == nullptr) {
        printf("Lista jest pusta!\r\n");
        return;
    }

    Node* temp = head;
    Node* prev = nullptr;

    if (temp != nullptr && temp->nodeIndex == data) {
        head = temp->next;
        if (current == temp) {
            current = temp->next;
        }
        delete temp;
        this->indexCounter--;
        return;
    }

    while (temp != nullptr && temp->nodeIndex != data) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == nullptr) {
        printf("Element %d nie został znaleziony na liście.\n", data);
        return;
    }

    prev->next = temp->next;
    if (current == temp) {
        current = temp->next;
    }
    delete temp;
    this->indexCounter--;
#error tą funkcję bym poprawił o renumerację i tylko w jednym miejscu delete
}

// Wydrukowanie elementów listy
void List::printList() {
    Node* temp = head;
    while (temp != nullptr) {
        printf("%d -> ", temp->nodeIndex);
        temp = temp->next;
    }
    printf("NULL\n");
}

// Prywatna funkcja do zwalniania pamięci zajmowanej przez listę
void List::freeList() {
    Node* temp;
    while (head != nullptr) {
        temp = head;
        head = head->next;
        delete temp;
        this->indexCounter--;
    }
    current = nullptr;
}
