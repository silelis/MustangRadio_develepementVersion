/*
 * printf2Uart.c
 *
 *  Created on: Jul 10, 2024
 *      Author: dbank
 */

//#include <printfRedirect.h>
#include <SileliS_code/printfRedirect.h>
#include "FreeRTOS.h"	//__assert_func: vTaskPrioritySet, xTaskGetSchedulerState
#include "task.h"

//printf to uart redirection
void __io_putchar(uint8_t ch) {
	HAL_UART_Transmit(&huart1, &ch, 1, 0xffff);
}

/********************************************************************
 * @brief  Własna obsługa assert() - zastępuje __assert_func z newlib
 *
 * Makro assert() z <assert.h> przy niespełnionym warunku woła
 * __assert_func(). Linker bierze tę definicję zamiast bibliotecznej
 * (obiekt z libc.a dołączany tylko dla nierozwiązanych symboli) -
 * działa dla wszystkich assert() w plikach C i C++, bez zmian w kodzie.
 *
 * Różnica względem newlib: przed zatrzymaniem obniża priorytet taska
 * do tskIDLE_PRIORITY. Komunikaty feedPrintf sprzed assert idą przez
 * kolejkę do printfTask (prio 0) - bez obniżenia task z assert o
 * wyższym priorytecie (np. taski i2c +2) kręciłby się w while(1)
 * i printfTask nigdy by ich nie wypisał (np. "TX ERROR ... err: 0x04").
 *
 * @param  [file] [const char*] plik z assert
 * @param  [line] [int] linia
 * @param  [func] [const char*] funkcja (może być NULL)
 * @param  [failedexpr] [const char*] niespełniony warunek
 *
 * @note   Komunikat assert wypisywany od razu (stderr -> _write ->
 * 			__io_putchar, blokująco), komunikaty feedPrintf - po nim.
 * @note   assert w przerwaniu (IPSR != 0) lub przed startem schedulera
 * 			- bez zmiany priorytetu (jak newlib: komunikat + stop).
 * @note   NIE dotyczy configASSERT() FreeRTOS (FreeRTOSConfig.h).
 *******************************************************************/
void __assert_func(const char *file, int line, const char *func, const char *failedexpr) {
	fprintf(stderr, "assertion \"%s\" failed: file \"%s\", line %d%s%s\n", failedexpr, file, line,
			(func != NULL) ? ", function: " : "", (func != NULL) ? func : "");
	if ((__get_IPSR() == 0U) && (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)) {
		vTaskPrioritySet(NULL, tskIDLE_PRIORITY);	//task z assert nie blokuje printfTask (time slicing z prio 0)
	}
	while (1) {
	}
}
