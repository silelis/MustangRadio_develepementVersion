#pragma once


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "StepperOptoPowerOFF/StepperOptoPowerOFF.h"
#include "NVSeeprom/NVSeeprom.h"

extern StepperOptoPowerOFF * pMotor; //obiekt steruj¹cy prac¹ silnika krokowego, jego krañcówej i power off radia

void stepperMotorDataParser(void *);