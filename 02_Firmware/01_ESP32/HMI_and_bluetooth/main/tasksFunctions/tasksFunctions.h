#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "comunicationProtocol.h"
#include "hwConfigFile.h"
#include "tasksFunctions.h"
#include "keyboard/keyboard.h"
#include "Leds\Leds_backlight.h"
#include "StepperOpto/StepperOpto.h"
#include "NVSeeprom/NVSeeprom.h"


struct hmiDisplay {
	colorSet sourceLed, equaliserLed, errorLed, backlightLeds;
	const uint16_t blinkTime = LED_DISPLAY_BLINK_TIME;
};

typedef struct {
	StepperOpto * motorPointer;
	NVS * storagePointer;
} motorTaskParam;



void taskFunctionsStaticHandlersInit(void);
void keyboardQueueParametersParser(void*);
void humanMahineDisplayLeds(void *);
void humanMahineBacklightLeds(void *);
void stepperMotor(void *);
