/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

#include "ESP32-A2DP_class/ESP32-A2DP_class.h"

extern "C" void app_main(void)
{
//    printf("Hello world!\n");

/*
    I2SStream i2s;
    BluetoothA2DPSink a2dp_sink(i2s);
 
    auto cfg = i2s.defaultConfig();
    cfg.pin_bck = GPIO_NUM_26;//26;//14;
    cfg.pin_ws = GPIO_NUM_27;//27;//15;
    cfg.pin_data = GPIO_NUM_25;//25;//22;
    i2s.begin(cfg);

    a2dp_sink.start("MyMusic");*/

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
