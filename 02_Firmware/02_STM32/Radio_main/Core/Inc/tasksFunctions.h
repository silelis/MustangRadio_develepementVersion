/*
 * tasksFunctions.h
 *
 *  Created on: Jul 13, 2024
 *      Author: dbank
 */

#ifndef INC_TASKSFUNCTIONS_H_
#define INC_TASKSFUNCTIONS_H_

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "hwConfigFile.h"
#include "printfRedirect.h"
#include "main.h"
#include <stdio.h>
#include <stdbool.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

void initTaskFunctions(void);

static void esp32IntrrruptRequestCallback(void *pNothing);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

//#ifdef __cplusplus
//}
//#endif

#endif /* INC_TASKSFUNCTIONS_H_ */
