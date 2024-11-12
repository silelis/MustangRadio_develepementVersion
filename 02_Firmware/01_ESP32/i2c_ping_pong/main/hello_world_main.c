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
#include "driver/gpio.h"
#include "driver/i2c_slave.h"
#include "driver/i2c_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define I2C_SLAVE_ADDRESS_ESP32					0x3C
#define ESP32_SLAVE_RECEIVE_BUFFER_LEN			20

#include "soc/i2c_struct.h"
#include "hal/i2c_hal.h"
#if SOC_I2C_SUPPORTED
#include "hal/i2c_ll.h"
#endif

int losuj_od_1_do_20() {
	return rand() % 20 + 1;
}

i2c_slave_dev_handle_t slave_handle;
QueueHandle_t s_receive_queue;	
TaskHandle_t handlerTask_i2cSlaveReceive; //uchwyt do taska obsługującego wysyłanie danych z i2c slave to i2c master
TaskHandle_t handlerTask_i2cSlaveTransmit; //uchwyt do taska obsługującego transmisję z i2c slave to i2c master
extern volatile i2c_dev_t I2C0;

uint32_t rx_fifo_end_addrLast;
//i2c_dev_t I2C0_last;
//i2c_hal_context_t *hal1;

enum i2cCallbackState
{
	recpeptionNotToMe,
	recpeptionToMe,
	transmition
};
enum i2cCallbackState rxToEsp32 = recpeptionNotToMe;
static IRAM_ATTR bool i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *edata, void *user_data)
{
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;
	xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);

	if (I2C0.int_status.trans_complete == 0)
	{
		if (rx_fifo_end_addrLast != I2C0.fifo_st.rx_fifo_end_addr)
		{
			rxToEsp32 = recpeptionToMe;
		}
		else
		{
			rxToEsp32 = recpeptionNotToMe;
		}
		
	}
	else
	{
		rxToEsp32 = transmition;	
	}
	rx_fifo_end_addrLast = I2C0.fifo_st.rx_fifo_end_addr;
	
	/*if (high_task_wakeup == pdTRUE) {
		// Wybudź zadanie o wyższym priorytecie
		portYIELD_FROM_ISR(high_task_wakeup);
	}	  */
	return high_task_wakeup == pdTRUE;
}




esp_err_t interruptRequestSet(void)
{
	return gpio_set_level(GPIO_NUM_0, 0); //interrupt request is SET when pin goes low
}

esp_err_t interruptRequestReset(void)
{
	return gpio_set_level(GPIO_NUM_0, 1); //interrupt request is RESET when pin goes high
}

void i2cSlaveTransmit(void* nothink)
{		vTaskDelay(pdMS_TO_TICKS(500));
	size_t len = 9;
	while (1)
	{
		i2c_slave_transmit(slave_handle, (const uint8_t*) &len, sizeof(size_t), 10000);
		i2c_slave_transmit(slave_handle, (const uint8_t*) "Received\n", 9, 10000);
		interruptRequestSet();
		vTaskDelay(pdMS_TO_TICKS(2));
		interruptRequestReset();
		vTaskDelay(pdMS_TO_TICKS(losuj_od_1_do_20()*100));
	}
	
	
}

void i2cSlaveReceive(void* nothink)
{
	//uint8_t *data_rd = (uint8_t *) malloc(ESP32_SLAVE_RECEIVE_BUFFER_LEN);
	//uint8_t *data_rd =  new uint8_t[ESP32_SLAVE_RECEIVE_BUFFER_LEN];
	
	uint8_t* data_rd = (uint8_t *) malloc(ESP32_SLAVE_RECEIVE_BUFFER_LEN);// malloc(ESP32_SLAVE_RECEIVE_BUFFER_LEN * sizeof(uint8_t));
	//memset(data_rd, 0, ESP32_SLAVE_RECEIVE_BUFFER_LEN);
	uint32_t size_rd = 0;
	i2c_slave_rx_done_event_data_t rx_data;
	i2c_slave_receive(slave_handle, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN);
	while (1)
	{
		memset(data_rd, 0, ESP32_SLAVE_RECEIVE_BUFFER_LEN);	
		
		if (xQueueReceive(s_receive_queue, &rx_data, portMAX_DELAY) == pdTRUE)
		{
			I2C0.int_status.trans_complete;
			I2C0.int_raw.slave_tran_comp;

			//slave_handle wysyła dane do Master
			I2C0.int_raw.trans_complete;// = 1; slave_handle wysyła dane do Master
			
			
			//I2C0.status_reg.slave_rw; //=1; slave_handle wysyła dane do Master
			//I2C0.int_raw.tx_send_empty; //=1 slave_handle wysyła dane do Master
			
			 //fake
			I2C0.int_raw.trans_complete; //=0
			
			I2C0.int_status.trans_complete;	 //=0
			I2C0.int_raw.slave_tran_comp;	 //=1
			I2C0.fifo_st.rx_fifo_end_addr;//bez zmian	
			I2C0.status_reg.tx_fifo_cnt;//sie zmieniło	 o 1
			
			//transmition
			I2C0.int_raw.trans_complete; //=0
			
			I2C0.int_status.trans_complete; //=0
			I2C0.int_raw.slave_tran_comp; //=1
			I2C0.fifo_st.rx_fifo_end_addr; //wzrasta
			
			if (rxToEsp32 == recpeptionToMe)
			//if (data_rd[0] != 0)
			{
				printf("%s\n", data_rd);
			}
			else if (rxToEsp32 == recpeptionNotToMe)
			{
				printf("Not to me\n");	
			}
			else if (rxToEsp32 ==transmition)
			{
				printf("Transmititng\n");	 
			}
			//ESP_ERROR_CHECK(i2c_slave_receive(slave_handle, data_rd, 6));
			//printf("%s\r\n", data_rd);
			//i2c_slave_receive(slave_handle, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN);
		}
		i2c_slave_receive(slave_handle, data_rd, ESP32_SLAVE_RECEIVE_BUFFER_LEN);
	}
}




void app_main(void)
{
    printf("Hello world!\n");
	srand(time(NULL)); // Inicjalizacja generatora losowego


	gpio_config_t I2C_slave_IntRequestPinConfig;
	I2C_slave_IntRequestPinConfig.intr_type = GPIO_INTR_DISABLE;
	I2C_slave_IntRequestPinConfig.mode = GPIO_MODE_OUTPUT;
	I2C_slave_IntRequestPinConfig.pin_bit_mask = 1<<GPIO_NUM_0;
	I2C_slave_IntRequestPinConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	I2C_slave_IntRequestPinConfig.pull_up_en = GPIO_PULLUP_ENABLE; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	interruptRequestReset(); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!							
	ESP_ERROR_CHECK(gpio_config(&I2C_slave_IntRequestPinConfig)); //KOLEJNOŚĆ!!!!!!!
	
	static i2c_slave_config_t i2c_config_slave;	
	i2c_config_slave.addr_bit_len =  I2C_ADDR_BIT_LEN_7;
	i2c_config_slave.clk_source = I2C_CLK_SRC_DEFAULT;
	i2c_config_slave.i2c_port = 0;
	i2c_config_slave.send_buf_depth = 1024;
	i2c_config_slave.scl_io_num = GPIO_NUM_22;
	i2c_config_slave.sda_io_num = GPIO_NUM_21;
	i2c_config_slave.slave_addr = I2C_SLAVE_ADDRESS_ESP32;
	//i2c_config_slave.intr_priority  = 0;
	
	ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_config_slave, &slave_handle));

	s_receive_queue = xQueueCreate(10, sizeof(i2c_slave_rx_done_event_data_t));	  //!!!!!!!!!!!długość kolejki
	i2c_slave_event_callbacks_t cbs = {
		.on_recv_done = i2c_slave_rx_done_callback,
	};
	
	ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(slave_handle, &cbs, s_receive_queue));
	configASSERT(xTaskCreate(i2cSlaveReceive, "I2C slave rx", 128 * 8, NULL, tskIDLE_PRIORITY + 2, &handlerTask_i2cSlaveReceive));
	

	rx_fifo_end_addrLast = I2C0.fifo_st.rx_fifo_end_addr;
	vTaskDelay(pdMS_TO_TICKS(1200));
	interruptRequestReset(); //ustawiam wyjście na wysokie przed inicjalizacją GPIO, aby nie wywołać niepotrzebnie interrupt request
	vTaskDelay(pdMS_TO_TICKS(200));
	interruptRequestSet();
	
	configASSERT(xTaskCreate(i2cSlaveTransmit, "I2C slave tx", 128 * 8, NULL, tskIDLE_PRIORITY+2, &handlerTask_i2cSlaveTransmit));
	

	while (1)
{
	} 
}
