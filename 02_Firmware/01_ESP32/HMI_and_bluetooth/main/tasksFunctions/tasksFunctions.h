#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "./../../../03_Common/comunicationProtocol.h"
#include "./../../../03_Common/comunication_calculate_checksum.h"
//#include "comunicationProtocol.h"
#include "hwConfigFile.h"
#include "tasksFunctions.h"
#include "keyboard/keyboard.h"
#include "Leds/Leds_backlight.h"
#include "StepperOptoPowerOFF/StepperOptoPowerOFF.h"
#include "NVSeeprom/NVSeeprom.h"

   /*
struct hmiDisplay {
	//colorSet sourceLed, equaliserLed, errorLed, backlightLeds;
	hmiLeds leds;
	const uint16_t blinkTime = LED_DISPLAY_BLINK_TIME;
};	   */


void taskFunctionsStaticHandlersInit(void);
void keyboardQueueParametersParser(void*);
void humanMahineDisplayLeds(void *);
void humanMahineBacklightLeds(void *);
void stepperMotor(void *);
void i2cSlaveTransmit(void *);
void i2cSlaveReceive(void *);


static BaseType_t esp32PrepareKbrdDataAndSent_to_QueueSend(const i2cFrame_keyboardFrame * pvItemToQueue, size_t itemSize);