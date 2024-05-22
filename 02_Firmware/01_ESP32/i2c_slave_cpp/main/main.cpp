/* Blink C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

//#include <cstdlib>
#include <thread>
#include "esp_log.h"



//using namespace idf;
//using namespace std;

extern "C" {
#include "i2c_slave.h"

} ;


extern "C" void app_main(void)
{
    /* The functions of GPIO_Output throws exceptions in case of parameter errors or if there are underlying driver
       errors. */
	i2c_slave_init();
	while (1)
	{
		
	}  ;

}
