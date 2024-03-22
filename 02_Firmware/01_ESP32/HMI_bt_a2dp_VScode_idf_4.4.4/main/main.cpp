/* Blink C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <cstdlib>
#include <thread>
#include "esp_log.h"
//#include "gpio_cxx.hpp"


//esp-idf-tools-setup-offline-4.4.4.exe <-- this ESP-IDF is working

#include "BluetoothA2DPSink.h"


BluetoothA2DPSink a2dp_sink;

void setup() {
    i2s_pin_config_t my_pin_config = {
    	.mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = 26,
        .ws_io_num = 27,
        .data_out_num = 25,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    a2dp_sink.set_pin_config(my_pin_config);
    a2dp_sink.start("MyMusic2312");
}

void loop(){
   delay( 500 );
}


using namespace std;

extern "C" void app_main(void)
{
      printf("stopping.\n");
	  setup();
	  while(true){
	    loop();
	  }


}
