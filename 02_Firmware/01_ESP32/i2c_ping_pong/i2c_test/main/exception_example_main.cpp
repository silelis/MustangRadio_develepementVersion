/* C++ exception handling example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <iostream>
#include <iomanip> 


//#include <iostream>
#include <inttypes.h>
#include "cstdio"
#include "driver/gpio.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c_slave.h"

typedef enum {
    I2C_SLAVE_EVT_RX,
    I2C_SLAVE_EVT_TX
} i2c_slave_event_t;

#define REC_BUFFER_LEN  100
uint8_t theBUFFER[REC_BUFFER_LEN];

QueueHandle_t /*DRAM_ATTR*/ i2cSlaveEventQueue =  xQueueCreate( 6, sizeof(i2c_slave_event_t) );

static bool IRAM_ATTR i2c_slave_receive_cb(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_rx_done_event_data_t *evt_data, void *user_data)
{
    i2c_slave_event_t evt = I2C_SLAVE_EVT_RX;
    BaseType_t xTaskWoken = 0;
    xQueueSendFromISR(i2cSlaveEventQueue , &evt, &xTaskWoken);
    return xTaskWoken;
}

uint32_t write_len;
static bool IRAM_ATTR i2c_slave_request_cb(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_request_event_data_t *evt_data, void *arg)
{
    i2c_slave_event_t evt = I2C_SLAVE_EVT_TX;
    BaseType_t xTaskWoken = 0;

    uint8_t data_buffer[9]="TataTata";

    i2c_slave_write(i2c_slave, (const uint8_t*) data_buffer, 8, &write_len, 10);

    xQueueSendFromISR(i2cSlaveEventQueue , &evt, &xTaskWoken);
    vTaskDelay(pdMS_TO_TICKS(100));
    return xTaskWoken;
}

gpio_num_t i2cSlave_intRequestPin;

esp_err_t interruptRequestSet(void)
{
    vTaskDelay(pdMS_TO_TICKS(100));
	return gpio_set_level(i2cSlave_intRequestPin, 0); // interrupt request is SET when pin goes low
}

esp_err_t interruptRequestReset(void)
{
    vTaskDelay(pdMS_TO_TICKS(100));
	return gpio_set_level(i2cSlave_intRequestPin, 1); // interrupt request is RESET when pin goes high
}


/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void)
{
    using namespace std;
    cout << "app_main done" << endl;

	i2cSlave_intRequestPin = GPIO_NUM_0;

    i2cSlaveEventQueue  =  xQueueCreate( 20, sizeof(i2c_slave_event_t) );

    assert(i2cSlaveEventQueue );

	gpio_config_t I2C_slave_IntRequestPinConfig;
	I2C_slave_IntRequestPinConfig.intr_type = GPIO_INTR_DISABLE;
	I2C_slave_IntRequestPinConfig.mode = GPIO_MODE_OUTPUT;
	I2C_slave_IntRequestPinConfig.pin_bit_mask = 0x1 << i2cSlave_intRequestPin;
	I2C_slave_IntRequestPinConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	I2C_slave_IntRequestPinConfig.pull_up_en = GPIO_PULLUP_ENABLE;
	interruptRequestReset(); // ustawiam wyjście na wysokie przed inicjalizacją GPIO, aby nie wywołać niepotrzebnie interrupt request
	ESP_ERROR_CHECK(gpio_config(&I2C_slave_IntRequestPinConfig));

    i2c_slave_config_t i2c_slv_config;// = {
    i2c_slv_config.i2c_port = 0,
    i2c_slv_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_slv_config.scl_io_num = GPIO_NUM_22;
    i2c_slv_config.sda_io_num = GPIO_NUM_21;
    i2c_slv_config.addr_bit_len = I2C_ADDR_BIT_LEN_7;
    i2c_slv_config.slave_addr = 0x3C;
    i2c_slv_config.send_buf_depth = 2*52;
    i2c_slv_config.receive_buf_depth = 100;
    i2c_slv_config.flags.allow_pd = 0;
    i2c_slv_config.flags.enable_internal_pullup = true;
    i2c_slv_config.intr_priority = 3;
//

i2c_slave_dev_handle_t slave_handle; //aka context

i2c_slave_event_callbacks_t cbs;// = {
    cbs.on_receive = i2c_slave_receive_cb;
    cbs.on_request = i2c_slave_request_cb;


//ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(slave_handle, &cbs, NULL));
ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slv_config, &slave_handle));
ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(slave_handle, &cbs, NULL));

interruptRequestSet();
interruptRequestReset();

    while(1){
        vTaskDelay(pdMS_TO_TICKS(3000));
        interruptRequestSet();
        interruptRequestReset();
    }
}
