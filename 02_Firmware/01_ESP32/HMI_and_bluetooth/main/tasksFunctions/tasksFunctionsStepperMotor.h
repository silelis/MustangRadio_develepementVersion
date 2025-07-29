#pragma once


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "StepperOptoPowerOFF/StepperOptoPowerOFF.h"
#include "NVSeeprom/NVSeeprom.h"

extern StepperOptoPowerOFF * pMotor; //obiekt steruj�cy prac� silnika krokowego, jego kra�c�wej i power off radia

void stepperMotorDataParser(void *);