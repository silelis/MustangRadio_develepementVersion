#pragma once

#include "../MCP23008/MCP23008.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "cmath"
#include <stdio.h>
#include <string.h>
#include "hwConfigFile.h"
//#include "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/03_Common/comunicationStructures.h"
#include "common/comunicationStructures/comunicationStructures.h"
//#include "./../../../03_Common/comunicationStructures.h"
//#include "comunicationStructures.h"

#define MOVE_FORWARD			1
#define MOVE_BACKWARD			0


typedef struct {		//struktura zawieraj¹ca informacje o stanie ledów wykorzystywana podczas komunikacji miêdzy master a slave
	uint16_t beginOffest;
	uint16_t endOffset;
	uint16_t maxPosition;
	uint16_t currentPosition;
	uint16_t gotoPosition=0;
	BaseType_t isCalibrated;
} MotorParameters;

	
class StepperOptoPowerOFF//:private MCP23008
{
public:
	StepperOptoPowerOFF(MCP23008* pointer_MCP23008);
	~StepperOptoPowerOFF();
	void radioPowerOFF(void);
	void measureSliderRange(void);
	void moveXSteps(int32_t stepAmount);
	void moveTo_xPosition(uint16_t xPosition);
	void moveTo_xPercent(float xPercent);
	void moveTo_xPercentIncludeOffset(float xPercent);
	MotorParameters getValue_motorParameters();
	void setValue_motorParameters(const void *src);
	BaseType_t isCalibrated(void);
	BaseType_t isPositionReached(void);
				

protected:
	esp_err_t setDirection(bool direction);
	esp_err_t makeStep();
	uint8_t  readInputs();
	void calibrationSet(void);
	void calibrationReset(void);

	
	
private:
	const char *TAG = "StepperOpto log:";
	esp_err_t enableStepperMotor(); //todo: ta funkcja prywatna
	esp_err_t disableStepperMotor(); //todo: ta funkcja prywatna
	MCP23008* pMCP23008;
	
	MotorParameters motorParameters;
	uint8_t whichDirection;
	bool isStepperDriverEnabled;
};