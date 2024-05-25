#include "i2c_slave_hw_init.h"


esp_err_t i2c_slave_init(i2c_mode_t mode, uint16_t slave_7bit_Address, int port, int SDA_pin, int SCL_pin, size_t rxBufferLen, size_t txButterLen)
{
	int i2c_slave_port = port; //I2C_SLAVE_NUM;
	i2c_config_t conf_slave = {
		.sda_io_num = SDA_pin,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_io_num = SCL_pin,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.mode = mode,	//,
		.slave.addr_10bit_en = 0,
		.slave.slave_addr = slave_7bit_Address,
		 //ESP_SLAVE_ADDR,
	};
	esp_err_t err = i2c_param_config(i2c_slave_port, &conf_slave);
	if (err != ESP_OK) {
		return err;
	}
	return i2c_driver_install(i2c_slave_port, conf_slave.mode, rxBufferLen, txButterLen, 0);
}