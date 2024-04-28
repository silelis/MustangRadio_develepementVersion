#pragma once

#include "../MCP23008/MCP23008.h"
#include "hwConfigFile.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "cmath"
#include <stdio.h>
#include <string.h>

#define MOVE_FORWARD			1
#define MOVE_BACKWARD			0

typedef struct {
	uint16_t beginOffest;
	uint16_t endOffset;
	uint16_t maxPosition;
	uint16_t currentPosition;

} MotorParameters;

	
class StepperOptoPowerOFF:private MCP23008
{
public:
	StepperOptoPowerOFF();
	~StepperOptoPowerOFF();
	
	//esp_err_t enableESP32InterruptNotification();
	//esp_err_t disableESP32InterruptNotification();
	
	//esp_err_t ;		//TODO: nie wiem czy w ostatecznej wersji hardware będzie to wykorzystywane, zależy od sposobu w jaki rozwiąże przełączanie I2S
	//esp_err_t disableMuxSelect();		//TODO: nie wiem czy w ostatecznej wersji hardware będzie to wykorzystywane, zależy od sposobu w jaki rozwiąże przełączanie I2S
	

	//esp_err_t measureSliderRange();
	
	void radioPowerOFF(void);
	void measureSliderRange(void);
	void moveXSteps(int32_t stepAmount);
	void moveTo_xPosition(uint16_t xPosition);
	void moveTo_xPercent(float xPercent);
	void moveTo_xPercentIncludeOffset(float xPercent);
	MotorParameters getValue_motorParameters();
	void setValue_motorParameters(const void *src);
				

protected:
	esp_err_t setDirection(bool direction);
	esp_err_t makeStep();
	uint8_t  readInputs();	
	
private:
	const char *TAG = "StepperOpto log:";
	esp_err_t enableStepperMotor(); //todo: ta funkcja prywatna
	esp_err_t disableStepperMotor(); //todo: ta funkcja prywatna
	
	MotorParameters motorParameters;
	//uint16_t maxPosition;
	//uint16_t currentPosition;
	//uint16_t begginOffest;
	//uint16_t endOffset;
	uint8_t whichDirection;
	bool isStepperDriverEnabled;
	//uint8_t calibrationErrorCounter;
};