//#pragma once

#ifndef __I2C_MASTER_HW_INIT
#define __I2C_MASTER_HW_INIT
#endif // !__I2C_MASTER_HW_INIT

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdlib.h>
#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
	
	esp_err_t i2c_master_hw_init(i2c_port_num_t portNumber, i2c_master_bus_config_t *i2c_master_config, i2c_master_bus_handle_t *bus_handle, gpio_num_t pinSDA, gpio_num_t pinSCL, uint32_t pullUpEnable);
	 
#ifdef __cplusplus
}	 
#endif // __cplusplus