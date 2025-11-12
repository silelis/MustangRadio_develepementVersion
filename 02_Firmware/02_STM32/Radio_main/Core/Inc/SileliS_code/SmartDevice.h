#ifndef SMARTDEVICE_H
#define SMARTDEVICE_H

template<typename T>
class SmartDevice {
private:
	T *ptr;

public:
	// Konstruktor domyślny, inicjalizujący wskaźnik jako nullptr
	SmartDevice() :
			ptr(nullptr) {
	}

	// Konstruktor z istniejącym wskaźnikiem
	explicit SmartDevice(T *p) :
			ptr(p) {
	}

	// Konstruktor, który tworzy obiekt typu T przy użyciu new
	explicit SmartDevice(const T &obj) :
			ptr(new T(obj)) {
	}

	// Destruktor, który zwalnia pamięć
	~SmartDevice() {
		delete ptr;
	}

	// Metoda do ustawiania wskaźnika na nowy obiekt
	void SetPointer(T *p) {
		if (ptr != p) {
			delete ptr;
			ptr = p;
		}
	}

	// Metoda do ustawiania wskaźnika na istniejący obiekt
	void SetPointer(T &obj) {
		if (ptr != &obj) {
			delete ptr;
			ptr = &obj;
		}
	}

	// Operator dostępu do obiektu
	T& operator*() const {
		return *ptr;
	}

	// Operator dostępu do wskaźnika
	T* operator->() const {
		return ptr;
	}

	// Operator przenoszenia
	SmartDevice(SmartDevice &&other) noexcept :
			ptr(other.ptr) {
		other.ptr = nullptr;
	}

	SmartDevice& operator=(SmartDevice &&other) noexcept {
		if (this != &other) {
			delete ptr;
			ptr = other.ptr;
			other.ptr = nullptr;
		}
		return *this;
	}

	// Zakaz kopiowania
	SmartDevice(const SmartDevice&) = delete;
	SmartDevice& operator=(const SmartDevice&) = delete;
};

#endif // SMARTDEVICE_H

/*
 * #include <iostream>
 #include "SmartDevice.h"

 class TestClass {
 public:
 TestClass() { std::cout << "TestClass default constructor\n"; }
 TestClass(const TestClass&) { std::cout << "TestClass copy constructor\n"; }
 TestClass(TestClass&&) noexcept { std::cout << "TestClass move constructor\n"; }
 ~TestClass() { std::cout << "TestClass destructor\n"; }
 void Show() const {
 std::cout << "TestClass instance\n";
 }
 };

 int main() {
 TestClass obj1; // Istniejący obiekt
 TestClass obj2; // Inny istniejący obiekt

 // Konstruktor domyślny
 SmartDevice<TestClass> sd1;

 // Konstruktor z wskaźnikiem
 SmartDevice<TestClass> sd2(new TestClass());

 // Konstruktor, który tworzy obiekt typu T
 SmartDevice<TestClass> sd3(obj1);

 // Użycie metody SetPointer do przypisania nowego wskaźnika
 sd1.SetPointer(new TestClass());

 // Użycie metody SetPointer do przypisania istniejącego obiektu
 sd2.SetPointer(obj2);

 sd1->Show();
 sd2->Show();
 sd3->Show();

 return 0;
 }
 */
*/
