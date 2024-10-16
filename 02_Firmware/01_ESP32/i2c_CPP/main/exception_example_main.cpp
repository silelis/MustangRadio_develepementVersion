/* C++ exception handling example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <iostream>
//#include "driver/i2c_slave.h"
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/i2c_slave.h"
#include "freertos/queue.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

using std::cout;
using std::endl;
using std::runtime_error;

/* A simple class which may throw an exception from constructor */
/*class Throwing
{
public:
    Throwing(int arg)
    : m_arg(arg)
    {
        cout << "In constructor, arg=" << arg << endl;
        if (arg == 0) {
            throw runtime_error("Exception in constructor");
        }
    }

    ~Throwing()
    {
        cout << "In destructor, m_arg=" << m_arg << endl;
    }

protected:
    int m_arg;
};

*/


typedef struct {
	size_t dataSize; //jak wyżej
	void *pData; //jak wyżej
} i2cFrame_transmitQueue;				//jak wyżej


static IRAM_ATTR bool i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *edata, void *user_data)
{
	i2cFrame_transmitQueue receivedFrame;
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;
	receivedFrame.dataSize = *(size_t*)edata->buffer;
	
	//void* receivedData;
	char*  receivedData = new char[receivedFrame.dataSize]; //(char*) malloc(receivedFrame.dataSize); 
	char* data_start = (char*)edata->buffer + sizeof(size_t);

	//memcpy(&receivedData, (char*)edata->buffer + sizeof(size_t), receivedFrame.dataSize);
	//memcpy(receivedData, data_start, receivedFrame.dataSize);
	
	memcpy(receivedData, data_start, receivedFrame.dataSize);
	
	receivedFrame.pData =  receivedData;
	
	//xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
	if (xQueueSendFromISR(receive_queue, &receivedFrame, &high_task_wakeup) == pdFAIL)
	{
		delete[]receivedFrame.pData;
		assert(0);
	}
	
//void* data = receivedFrame->pData;
	return high_task_wakeup == pdTRUE;
}


extern "C" void app_main(void)
{
    cout << "app_main starting" << endl;

	
	size_t 	 DATA_LENGTH = 4*sizeof(uint8_t);

	uint8_t *data_rd = (uint8_t *) malloc(DATA_LENGTH);
	uint32_t size_rd = 0;	
	



	i2c_slave_config_t i2c_slv_config;// = {
	i2c_slv_config.addr_bit_len = I2C_ADDR_BIT_LEN_7;//,
	i2c_slv_config.clk_source = I2C_CLK_SRC_DEFAULT;//,
	i2c_slv_config.i2c_port = 0;//,
		//TEST_I2C_PORT,
	i2c_slv_config.send_buf_depth = 256;//,
	i2c_slv_config.scl_io_num = GPIO_NUM_22;//,
		//I2C_SLAVE_SCL_IO,
	i2c_slv_config.sda_io_num = GPIO_NUM_21;//,
		//I2C_SLAVE_SDA_IO,
	i2c_slv_config.slave_addr = 0x3c;//,
		//0x58,

		
//	};

	i2c_slave_dev_handle_t slave_handle;
	ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slv_config, &slave_handle));

	//QueueHandle_t s_receive_queue = xQueueCreate(50, sizeof(i2c_slave_rx_done_event_data_t));
	QueueHandle_t s_receive_queue = xQueueCreate(50, sizeof(i2cFrame_transmitQueue));
	
	
	i2c_slave_event_callbacks_t cbs = {
		.on_recv_done = i2c_slave_rx_done_callback,
	};
	ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(slave_handle, &cbs, s_receive_queue));

	i2cFrame_transmitQueue rx_data;
	//char* receivedString;
	esp_err_t retVal;
	retVal = i2c_slave_receive(slave_handle, data_rd, 512);
	while (1)
	{
		//retVal = i2c_slave_receive(slave_handle, data_rd, 512);
		if (xQueueReceive(s_receive_queue, &rx_data, portMAX_DELAY) == pdPASS) {
			i2c_slave_receive(slave_handle, data_rd, 512);
			printf("%s\r\n", (char*)   rx_data.pData);
			delete[]rx_data.pData;
			
		}
	}

	
	
	
  //  try {
  //      /* This will succeed */
   //     Throwing obj1(42);
    //
    //    /* This will throw an exception */
  //      Throwing obj2(0);
  //
  //      cout << "This will not be printed" << endl;
  //  } catch (const runtime_error &e) {
  //      cout << "Exception caught: " << e.what() << endl;
  //  }

    cout << "app_main done" << endl;
}
