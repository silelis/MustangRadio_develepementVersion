#ifndef __PRINTF_REDIRECT__
#define __PRINTF_REDIRECT__

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"
#include <stdio.h>

void __io_putchar(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif /* __PRINTF_REDIRECT__ */
