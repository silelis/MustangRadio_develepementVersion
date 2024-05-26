//#pragma once

#ifndef __MAIN_H
#define __MAIN_H
#endif // !__MAIN_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdlib.h>
#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
	
	esp_err_t my_i2c_slave_init(i2c_mode_t mode, uint16_t slave_7bit_Address, int port, int SDA_pin, int SCL_pin, size_t rxBufferLen, size_t txButterLen);
	 
#ifdef __cplusplus
}	 
#endif // __cplusplus