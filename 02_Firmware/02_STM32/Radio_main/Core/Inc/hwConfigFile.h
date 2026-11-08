/*
 * hwConfig.h
 *
 *  Created on: Jul 10, 2024
 *      Author: dbank
 */

#ifndef INC_HWCONFIG_HWCONFIG_H_
#define INC_HWCONFIG_HWCONFIG_H_

#define FW_VERSION 0.1
#define I2C_STM32_TO_ESP32_ROLE_MASTER

#define PERIPHERY_MENU_TIMEOUT_TASK_DELAY	2000
#define ESP_I2C_BUS_DELAY					7	//[ms] przerwa po transmisji do ESP32 na ponowne uzbrojenie odbioru (dobrane doświadczalnie; okno ESP32 do 1 ticku = 10 ms - KONTEKST 3.5; zmniejszać dopiero po wymuszeniu przełączenia zadania w callbacku ESP32)
#define I2C_MASTER_STALL_PRINT_PERIOD_MS	1000	//co ile [ms] i2cMaster::waitForTransferEnd() wypisuje diagnostykę (stan HAL, ISR, piny SCL/SDA, licznik DMA), gdy transmisja i2c się nie kończy
#define I2C_MASTER_STALL_ASSERT_MS			10000	//po ilu [ms] niekończącej się transmisji i2c i2cMaster::waitForTransferEnd() robi assert(0)



#endif /* INC_HWCONFIG_HWCONFIG_H_ */
