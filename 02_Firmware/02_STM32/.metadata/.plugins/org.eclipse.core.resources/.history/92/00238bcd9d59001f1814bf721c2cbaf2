/*
 * list.cpp
 *
 *  Created on: Aug 12, 2024
 *      Author: dbank
 */

#include <SileliS_code/list.h>

// Konstruktor: inicjalizuje pustą listę
List::List() : head(nullptr), current(nullptr) {}

// Destruktor: zwalnia pamięć zajmowaną przez listę
List::~List() {
    freeList();
}

// Prywatna funkcja pomocnicza do tworzenia nowego węzła
Node* List::createNode(int data) {
    Node* newNode = new Node;  // Alokacja pamięci dla nowego węzła
    newNode->data = data;      // Ręczne przypisanie danych
    newNode->next = nullptr;   // Ręczne ustawienie wskaźnika na nullptr
    return newNode;
}

// Dodawanie elementu na początku listy
void List::addAtBeginning(int data) {
    Node* newNode = createNode(data);
    newNode->next = head;
    head = newNode;
}

// Dodawanie elementu na końcu listy
void List::addAtEnd(int data) {
    Node* newNode = createNode(data);
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

// Ustawienie wskaźnika current na pierwszy element listy
void List::resetToFirst() {
    current = head;
}

// Przesunięcie wskaźnika current na następny element
void List::moveToNext() {
    if (current != nullptr) {
        current = current->next;
    }
}

// Zwrócenie aktualnego elementu
int List::getCurrentElement() {
    if (current != nullptr) {
        return current->data;
    } else {
        fprintf(stderr, "Nie ma bieżącego elementu.\n");
        return -1;  // Można również użyć innego wskaźnika błędu
    }
}

// Sprawdzenie, czy wskaźnik current jest na końcu listy
bool List::isAtEnd() {
    return current == nullptr;
}

// Usuwanie pierwszego wystąpienia elementu z listy
void List::removeElement(int data) {
    if (head == nullptr) {
        fprintf(stderr, "Lista jest pusta!\n");
        return;
    }

    Node* temp = head;
    Node* prev = nullptr;

    if (temp != nullptr && temp->data == data) {
        head = temp->next;
        if (current == temp) {
            current = temp->next;
        }
        delete temp;
        return;
    }

    while (temp != nullptr && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == nullptr) {
        fprintf(stderr, "Element %d nie został znaleziony na liście.\n", data);
        return;
    }

    prev->next = temp->next;
    if (current == temp) {
        current = temp->next;
    }
    delete temp;
}

// Wydrukowanie elementów listy
void List::printList() {
    Node* temp = head;
    while (temp != nullptr) {
        printf("%d -> ", temp->data);
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
    }
    current = nullptr;
}
