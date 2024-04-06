#pragma once

//functions required by Mustang radio hardware
#include "driver/gpio.h"
#include "../../main/hwConfigFile.h"

esp_err_t i2sPinsHighImpedanceEnabled(void);
esp_err_t i2sPinsHighImpedanceDisabled(void);

