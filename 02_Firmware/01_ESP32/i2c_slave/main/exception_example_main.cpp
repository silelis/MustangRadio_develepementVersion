/* C++ exception handling example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <cstring>
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "soc/dport_access.h"

static const char *TAG = "i2c-slave";

#define DATA_LENGTH 512    /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 128 /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define DELAY_TIME_BETWEEN_ITEMS_MS \
  20 /*!< delay time between different test items */

#define I2C_SLAVE_SDA_IO GPIO_NUM_22
#define I2C_SLAVE_SCL_IO GPIO_NUM_21

#define I2C_SLAVE_NUM I2C_NUM_0
#define I2C_SLAVE_TX_BUF_LEN 256  //(2 * DATA_LENGTH)
#define I2C_SLAVE_RX_BUF_LEN 256  //(2 * DATA_LENGTH)

#define ESP_SLAVE_ADDR 0x04
#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */
#define ACK_CHECK_EN 0x1           /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

#define SLAVE_REQUEST_WAIT_MS 100

const uint8_t testCmd[15] = { 
	0x00,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x07,
	0x08,
	0x09,
	0x0A,
	0x0B,
	0x0C,
	0x0D,
	0x0E
 };

uint8_t outBuff[256];
uint16_t outBuffLen = 0;

uint8_t inBuff[256];
uint16_t inBuffLen = 0;

esp_err_t i2c_slave_init() {
	i2c_port_t i2c_slave_port = I2C_SLAVE_NUM;
	i2c_config_t conf_slave;
	conf_slave.sda_io_num = I2C_SLAVE_SDA_IO;
	//conf_slave.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf_slave.scl_io_num = I2C_SLAVE_SCL_IO;
	//conf_slave.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf_slave.mode = I2C_MODE_SLAVE;
	conf_slave.slave.addr_10bit_en = 0;
	conf_slave.slave.slave_addr = 0b0111100;
	i2c_param_config(i2c_slave_port, &conf_slave);
	return i2c_driver_install(i2c_slave_port,
		conf_slave.mode,
		I2C_SLAVE_RX_BUF_LEN,
		6,
		0);
}

extern "C" void app_main(void)
{
	esp_err_t retVal = i2c_slave_init();
	assert(!retVal);
	char data[] = "Dawid";
	int ile = 0;
	ile = i2c_slave_write_buffer(I2C_SLAVE_NUM,	(const uint8_t * )&data, sizeof(data)*2, portMAX_DELAY);
	ile = ile + 1;
	ile = i2c_slave_write_buffer(I2C_SLAVE_NUM, (const uint8_t *)&data, sizeof(data) * 2, portMAX_DELAY);
	ile = ile + 1;
	ile = i2c_slave_write_buffer(I2C_SLAVE_NUM, (const uint8_t *)&data, sizeof(data) * 2, portMAX_DELAY);
	ile = ile + 1;
	ile = i2c_slave_write_buffer(I2C_SLAVE_NUM, (const uint8_t *)&data, sizeof(data) * 2, portMAX_DELAY);
	ile = ile + 1;
	//rtc_reg_get
	for (;;) {
		vTaskDelay(10);
		printf("%d", ile);
	}
	
		
}
