#pragma once


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "StepperOptoPowerOFF/StepperOptoPowerOFF.h"
#include "NVSeeprom/NVSeeprom.h"

extern StepperOptoPowerOFF * pMotor; //obiekt steruj�cy prac� silnika krokowego, jego kra�c�wej i power off radia

// [FIX: Guru Meditation - xPortEnterCriticalTimeout deadlock]
// Semafor sygnalizuje, ze pMotor zostal zainicjalizowany na Core 1.
// i2cReceivedDataParser (i inne taski uzywajace pMotor) musza czekac
// na ten semafor zanim po raz pierwszy uzyja pMotor.
extern SemaphoreHandle_t xMotorInitDone;

void stepperMotorDataParser(void *);