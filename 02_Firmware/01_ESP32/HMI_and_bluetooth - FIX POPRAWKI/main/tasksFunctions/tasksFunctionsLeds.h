#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "Leds/Leds_backlight.h"
#include "common/comunicationProtocol/comunicationProtocol.h"
#include "common/comunication_calculate_checksum/comunication_calculate_checksum.h"

extern LEDS_BACKLIGHT *pLedDisplay; //obiekt steruj¹cy prac¹ ledów (diody i backlioght)
void humanMahineDisplayLeds(void *);
void humanMahineBacklightLeds(void *);