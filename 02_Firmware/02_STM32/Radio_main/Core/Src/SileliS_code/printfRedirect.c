/*
 * printf2Uart.c
 *
 *  Created on: Jul 10, 2024
 *      Author: dbank
 */

//#include <printfRedirect.h>
#include <SileliS_code/printfRedirect.h>

//printf to uart redirection
void __io_putchar(uint8_t ch) {
	HAL_UART_Transmit(&huart1, &ch, 1, 0xffff);
}
