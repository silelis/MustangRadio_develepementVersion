/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TDA741x_Mute_Pin GPIO_PIN_13
#define TDA741x_Mute_GPIO_Port GPIOC
#define TDA7850_ST_BY_Pin GPIO_PIN_14
#define TDA7850_ST_BY_GPIO_Port GPIOC
#define TDA7850_MUTE_Pin GPIO_PIN_15
#define TDA7850_MUTE_GPIO_Port GPIOC
#define nPS_Sust_STM32_Pin GPIO_PIN_0
#define nPS_Sust_STM32_GPIO_Port GPIOA
#define SdCardDetect_Pin GPIO_PIN_5
#define SdCardDetect_GPIO_Port GPIOC
#define FM_AM_ANT_PS_Pin GPIO_PIN_1
#define FM_AM_ANT_PS_GPIO_Port GPIOB
#define DAB_ANT_PS_Pin GPIO_PIN_2
#define DAB_ANT_PS_GPIO_Port GPIOB
#define si468x_switchDAB_AM_Pin GPIO_PIN_12
#define si468x_switchDAB_AM_GPIO_Port GPIOB
#define si468x_switchFM_Pin GPIO_PIN_6
#define si468x_switchFM_GPIO_Port GPIOC
#define si468x_to_STM_interrupt_Pin GPIO_PIN_7
#define si468x_to_STM_interrupt_GPIO_Port GPIOC
#define nRST_si468x_Pin GPIO_PIN_9
#define nRST_si468x_GPIO_Port GPIOA
#define ESP_to_STM_interrupt_Pin GPIO_PIN_5
#define ESP_to_STM_interrupt_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
