/* C++ exception handling example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <iostream>

#include "driver/gpio.h"

#include "esp_mac.h"

//#include "AudioTools.h" //bin 874 KB (bajtów: 895 840)
//#include "BluetoothA2DPSink.h"

#include "bt_audio/bt_audio.h"

/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void)
{
    i2sHighImpedanceEnabled(GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_25);
    bt_audio_sink bt_audio_sink(GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_25);
    bt_audio_sink.btAudioInit();

    while(1){
        vTaskDelay(pdMS_TO_TICKS(100));
    };
}
