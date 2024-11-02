/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE BEGIN Includes */
//#include "hwConfigFile.h"
#include "freertos.h"
#include <semphr.h>
#include <time.h>
//#include "printfRedirect.h"
//#include "tasksFunctions.h"
//#include "SileliS_code/tasksFunctions.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define I2C_SLAVE_ADDRESS_ESP32					0x3C
#define I2C_SLAVE_ADDRESS_FAKE					0x3d
SemaphoreHandle_t CountingSemaphore;
#define ESP_delay		7

static bool esp32I2cInitialised = false;
SemaphoreHandle_t i2c_semap;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(esp32I2cInitialised==false){
		esp32I2cInitialised=true;
	}
	else{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(CountingSemaphore, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

}
static TaskHandle_t taskHandle_taskSendFake;
static TaskHandle_t taskHandle_taskSend;
static TaskHandle_t taskHandle_taskReceive;

int losuj_od_1_do_20() {
    return rand() % 20 + 1;
}

void taskReceive(void *nothink){
	HAL_StatusTypeDef retVal;
	char recData[250];
	size_t len;
	while(1){
		if(xSemaphoreTake(CountingSemaphore, portMAX_DELAY)==pdTRUE){
			xSemaphoreTake(i2c_semap, portMAX_DELAY);
			while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){};
			vTaskDelay(pdMS_TO_TICKS(ESP_delay));
			retVal = HAL_I2C_Master_Receive_DMA(&hi2c1, I2C_SLAVE_ADDRESS_ESP32<<1, (uint8_t*) &len, sizeof(size_t));

			while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){};
			vTaskDelay(pdMS_TO_TICKS(ESP_delay));
			retVal = HAL_I2C_Master_Receive_DMA(&hi2c1, I2C_SLAVE_ADDRESS_ESP32<<1, (uint8_t*) recData, len);
			vTaskDelay(pdMS_TO_TICKS(ESP_delay));
			while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){};
			HAL_UART_Transmit(&huart1, recData, 9, 200);
			HAL_UART_Transmit(&huart1, "\r", 1, 200);
			xSemaphoreGive(i2c_semap);
		}
	}

}

void taskSendFake(void *nothink){
	HAL_StatusTypeDef retVal;
	while(1){
		xSemaphoreTake(i2c_semap, portMAX_DELAY);
		while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){};
		vTaskDelay(pdMS_TO_TICKS(ESP_delay));
		  //retVal = HAL_I2C_Master_Transmit(&hi2c1, I2C_SLAVE_ADDRESS_ESP32, "Dawid", 6, 2000);
		  retVal =HAL_I2C_Master_Transmit_DMA(&hi2c1, I2C_SLAVE_ADDRESS_FAKE<<1, "Dawid", 6);
		  HAL_UART_Transmit(&huart1, "FAKE\r\n", 6, 200);
		  vTaskDelay(pdMS_TO_TICKS(ESP_delay));
		  while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){};
		  xSemaphoreGive(i2c_semap);
		  vTaskDelay(pdMS_TO_TICKS(losuj_od_1_do_20()*100));
		  /*for(uint32_t i=0;i<0xffffff;i++){

		  }*/
	}
}


void taskSend(void *nothink){
	HAL_StatusTypeDef retVal;
	while(1){
		xSemaphoreTake(i2c_semap, portMAX_DELAY);
		while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){};
		vTaskDelay(pdMS_TO_TICKS(ESP_delay));
		  //retVal = HAL_I2C_Master_Transmit(&hi2c1, I2C_SLAVE_ADDRESS_ESP32<<1, "Dawid", 6, 2000);
		  retVal =HAL_I2C_Master_Transmit_DMA(&hi2c1, I2C_SLAVE_ADDRESS_ESP32<<1, "Dawid", 6);
		  HAL_UART_Transmit(&huart1, "Transmition1\r\n", 14, 200);
		  vTaskDelay(pdMS_TO_TICKS(ESP_delay));
		  while(HAL_I2C_GetState(&hi2c1)!= HAL_I2C_STATE_READY){};
		  xSemaphoreGive(i2c_semap);
		  vTaskDelay(pdMS_TO_TICKS(losuj_od_1_do_20()*100));
		  /*for(uint32_t i=0;i<0xffffff;i++){

		  }*/
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  srand(time(NULL)); // Inicjalizacja generatora losowego

  while(esp32I2cInitialised!=true){
	  HAL_UART_Transmit(&huart1, "1\r\n", 3, 200);
  }

  CountingSemaphore = xSemaphoreCreateCounting(20, 0);
  i2c_semap = xSemaphoreCreateBinary();
  xSemaphoreGive(i2c_semap);
  xTaskCreate(taskSend, "i2cSend", 5*128, NULL, tskIDLE_PRIORITY+5, &taskHandle_taskSend);
  xTaskCreate(taskSendFake, "i2cSendFake", 5*128, NULL, tskIDLE_PRIORITY+5, &taskHandle_taskSendFake);
  xTaskCreate(taskReceive, "i2cRec", 5*128, NULL, tskIDLE_PRIORITY+5, &taskHandle_taskReceive);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /*AXI clock gating */
  RCC->CKGAENR = 0xFFFFFFFF;

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 35;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
