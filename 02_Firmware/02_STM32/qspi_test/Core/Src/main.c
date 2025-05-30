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
#include "i2c.h"
#include "octospi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*W25Q32 memory parameters*/





uint8_t writebuf[] = "Hello world from QSPI";
uint8_t Readbuf[100];




/**************************************************************************/
/*!
    @brief  Configure OCTO-SPI as Memory Mapped Mode.
            Winbond W25Q128JVSIQ specific setting.
            * Address size is 24bit(16MBytes).
            * Initially sets QUAD-MODE(not need quadmode command).
            * MAX 133MHz CLK.
            * Support "XIP",thus suitable for MemoryMappedMode.
    @param  None.
    @retval None.
*/
/**************************************************************************/


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
  MX_USART1_UART_Init();
  MX_OCTOSPI1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */



//		  HAL_StatusTypeDef ret;
//		  uint8_t rxbuf[100];
//		  uint8_t txbuf[] = "TEST";
//		  ret=W25Q128_OCTO_SPI_Init(&hospi1);
//
//		  //ret=W25Q128_OSPI_Erase_Chip(&hospi1);
//		  printf("Erased\r\n");
//		  //ret=W25Q128_OSPI_Write(&hospi1,txbuf,0x7FFFFEC/*0x3FFFFEC*/,sizeof(txbuf));
//		  ret=W25Q128_OSPI_Read(&hospi1, rxbuf,0x7FFFFEC,sizeof(txbuf));
//		  ret=W25Q128_OSPI_EnableMemoryMappedMode(&hospi1);
//		  HAL_Delay(1000);
//
//		  /*__IO*/ uint8_t *mem_addr;
//		  //mem_addr = (uint8_t *)(0x08010000);
//		  mem_addr = (uint8_t *)(0x090000000+0x7FFFFEC);
//		  uint8_t* ala = mem_addr;
//		  memcpy(rxbuf, mem_addr, sizeof(txbuf));

  //HAL_Delay(0x8ff);
  printf("Hello world!!!\r\n");
  char receiveData[8];
  memset(receiveData[0],0xff,8);
  HAL_StatusTypeDef result;				//1000100
  //result= HAL_I2C_IsDeviceReady(&hi2c1, 0b1010010<<1, 13, HAL_MAX_DELAY);
  result= HAL_I2C_IsDeviceReady(&hi2c1, /*0b1000100<<1*/0x3C<<1, 13, HAL_MAX_DELAY);
  //result=HAL_I2C_Mem_Read(&hi2c1, 0b1010010<<1, 0x00, 1,(uint8_t *) &receiveData,1, HAL_MAX_DELAY);
  //result= HAL_I2C_Master_Receive(&hi2c1, 0x3C<<1, (uint8_t *)&receiveData, 8, HAL_MAX_DELAY);
  //result= HAL_I2C_Master_Receive(&hi2c4, 0xA0, receiveData, 8, HAL_MAX_DELAY);
 // printf("%s\r\n", map[0]);
  HAL_Delay(0x1ff);
  result= HAL_I2C_Master_Receive(&hi2c1, 0x3C<<1, &receiveData, 6, 0xffff);

  /* USER CODE END 2 */

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
