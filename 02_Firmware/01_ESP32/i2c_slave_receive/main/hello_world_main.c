/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

//#define SOC_I2C_SLAVE_SUPPORT_I2CRAM_ACCESS
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


typedef struct {
	size_t dataSize; //jak wyżej
	void *pData; //jak wyżej
} i2cFrame_transmitQueue;				//jak wyżej


static IRAM_ATTR bool i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *edata, void *user_data)
{
	i2cFrame_transmitQueue receivedFrame;
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;
	receivedFrame.dataSize = 12; //*(size_t*)edata->buffer;
	
	//void* receivedData;
	char*  receivedData = (char*) malloc(receivedFrame.dataSize) ; 
	char* data_start = (char*)edata->buffer;// + sizeof(size_t);

	//memcpy(&receivedData, (char*)edata->buffer + sizeof(size_t), receivedFrame.dataSize);
	//memcpy(receivedData, data_start, receivedFrame.dataSize);
	
	
	
	xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
	
	//void* data = receivedFrame->pData;
	return high_task_wakeup == pdTRUE;
}

void app_main(void)
{
	size_t 	 DATA_LENGTH = 4*sizeof(uint8_t);

	uint8_t *data_rd = (uint8_t *) malloc(DATA_LENGTH);
	uint32_t size_rd = 0;	
	



	i2c_slave_config_t i2c_slv_config = {
		.addr_bit_len = I2C_ADDR_BIT_LEN_7,
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = 0, //TEST_I2C_PORT,
		.send_buf_depth = 256,
		.scl_io_num = GPIO_NUM_22, //I2C_SLAVE_SCL_IO,
		.sda_io_num = GPIO_NUM_21, //I2C_SLAVE_SDA_IO,
		.slave_addr = 0x3c, //0x58,
		
	};

	i2c_slave_dev_handle_t slave_handle;
	ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slv_config, &slave_handle));

	QueueHandle_t s_receive_queue = xQueueCreate(50, sizeof(i2c_slave_rx_done_event_data_t));
	i2c_slave_event_callbacks_t cbs = {
		.on_recv_done = i2c_slave_rx_done_callback,
	};
	ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(slave_handle, &cbs, s_receive_queue));

	i2c_slave_rx_done_event_data_t rx_data;
	
	esp_err_t retVal;
	while (1)
	{
		retVal = i2c_slave_receive(slave_handle, data_rd, 56);
		xQueueReceive(s_receive_queue, &rx_data, portMAX_DELAY);
		//retVal = retVal;
		//rx_data;
		//rx_data;
		
	}

	ESP_ERROR_CHECK(i2c_slave_receive(slave_handle, data_rd, DATA_LENGTH));
	xQueueReceive(s_receive_queue, &rx_data, portMAX_DELAY);
	ESP_ERROR_CHECK(i2c_slave_receive(slave_handle, data_rd, DATA_LENGTH));
	xQueueReceive(s_receive_queue, &rx_data, portMAX_DELAY);
	ESP_ERROR_CHECK(i2c_slave_receive(slave_handle, data_rd, DATA_LENGTH));
	xQueueReceive(s_receive_queue, &rx_data, portMAX_DELAY);
	// Receive done.
	
    printf("Hello world!\n");

	while (1)
	{
		
	}
	
}
