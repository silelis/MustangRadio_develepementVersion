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
	/*struct colorSet {
		struct ws2812Color primary;
		struct ws2812Color secondary;
	}*/ colorSet sourceLed, equaliserLed, errorLed, backlightLeds;

	const uint16_t blinkTime = LED_DISPLAY_BLINK_TIME;
};

typedef struct {
	StepperOpto * motorPointer;
	NVS * storagePointer;
} motorTaskParam;


extern QueueHandle_t handlerQueue_MainKeyboard;
extern TaskHandle_t handlerTask_keyboardQueueParametersParser;
extern TaskHandle_t handlerTask_ledDisplay;
extern TaskHandle_t handlerTask_backlightDisplay;	
extern TaskHandle_t handlerTask_stepperMotor;	
extern hmiDisplay displayLedsColors;
extern SemaphoreHandle_t handlerMutex_ledDisplay_Backlight;






void keyboardQueueParametersParser(void*);
void humanMahineDisplayLeds(void *);
void humanMahineBacklightLeds(void *);
void stepperMotor(void *);