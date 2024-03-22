#pragma once

#include "../MCP23008/MCP23008.h"
#include "hwConfigFile.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "cmath"

/*#define SENSOR_EQU_SIDE_MASK	(1<<7)
#define SENSOR_VOL_SIDE_MASK	(1<<6)
#define MOTOR_FAULT_MASK		(1<<5)
#define MOTOR_DIRECTION_MAKS	(1<<4)
#define MOTOR_STEP_MASK			(1<<3)
#define MOTOR_NOT_SLEEP_MASK	(1<<2)
//#define MUX_SELECT_MASK		(1<<1)
//#define INTERRUP_REQUEST_MASK	(1<<0)*/
#define MOVE_FORWARD			1
#define MOVE_BACKWARD			0



	
class StepperOptoInterMux:private MCP23008
{
public:
	StepperOptoInterMux();
	~StepperOptoInterMux();
	
	//esp_err_t enableESP32InterruptNotification();
	//esp_err_t disableESP32InterruptNotification();
	
	//esp_err_t enableMuxSelect();		//TODO: nie wiem czy w ostatecznej wersji hardware bêdzie to wykorzystywane, zale¿y od sposobu w jaki rozwi¹¿e prze³¹czanie I2S
	//esp_err_t disableMuxSelect();		//TODO: nie wiem czy w ostatecznej wersji hardware bêdzie to wykorzystywane, zale¿y od sposobu w jaki rozwi¹¿e prze³¹czanie I2S
	

	//esp_err_t measureSliderRange();
	
	void measureSliderRange();
	void moveXSteps(int32_t stepAmount);
	void moveTo_xPosition(uint16_t xPosition);
	void moveTo_xPercent(float xPercent);
	
				

protected:
	esp_err_t setDirection(bool direction);
	esp_err_t makeStep();
	uint8_t  readInputs();	
	
private:
	const char *TAG = "StepperOptoInterMux log:";
	esp_err_t enableStepperMotor(); //todo: ta funkcja prywatna
	esp_err_t disableStepperMotor(); //todo: ta funkcja prywatna
	
	uint16_t maxPosition;
	uint16_t currentPosition;
	uint8_t whichDirection;
	bool isStepperDriverEnabled;
	//uint8_t calibrationErrorCounter;
};