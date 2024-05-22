/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"


#include "driver/i2c.h"

static esp_err_t i2c_slave_init(void)
{
	int i2c_slave_port = 0; //I2C_SLAVE_NUM;
	i2c_config_t conf_slave = {
		.sda_io_num = 21,
		//.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_io_num = 22,
		//.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.mode = I2C_MODE_SLAVE,
		.slave.addr_10bit_en = 0,
		.slave.slave_addr = 0x3C,
		 //ESP_SLAVE_ADDR,
	};
	esp_err_t err = i2c_param_config(i2c_slave_port, &conf_slave);
	if (err != ESP_OK) {
		return err;
	}
	return i2c_driver_install(i2c_slave_port, conf_slave.mode, 512, 512, 0);
}

void app_main(void)
{
    printf("Hello world!\n");
	i2c_slave_init();


    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}
