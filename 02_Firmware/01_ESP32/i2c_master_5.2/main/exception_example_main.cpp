/* C++ exception handling example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <iostream>





//#include "my_i2c_master/i2c_master_hw_init.h"
#include "my_i2c_master/my_i2c_master.h"






/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void)
{


	i2c_master master(0, GPIO_NUM_19, GPIO_NUM_18, 0);
	//i2c_master_hw_init(1, &i2c_master_config, &bus_handle, GPIO_NUM_21, GPIO_NUM_21, true);
	
	while (true)
	{
		
	}
}
