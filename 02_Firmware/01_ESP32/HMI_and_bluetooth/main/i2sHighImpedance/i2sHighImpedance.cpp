#include "i2sHighImpedance.h"


esp_err_t i2sPinsHighImpedanceEnabled(void)
{
	gpio_config_t io_conf = { };
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = ((1ULL << I2S_PIN_BCK) | (1ULL << I2S_PIN_WS) | (1ULL << I2S_PIN_OUT));
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE; 
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	esp_err_t retVal = gpio_config(&io_conf);
	if (retVal == ESP_OK)
	{
		printf("I2S pins high impedance mode had been initialized.\n");
	}
	else
	{
		printf("PROBLEM with I2S pins high impedance mode initialization.\n");
		assert(!ESP_OK);
	}
	return retVal;
}


esp_err_t i2sPinsHighImpedanceDisabled(void)
{
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = ((1ULL << I2S_PIN_BCK) | (1ULL << I2S_PIN_WS) | (1ULL << I2S_PIN_OUT));
	io_conf.mode = GPIO_MODE_DISABLE;
	//io_conf.pull_up_en = GPIO_PULLUP_DISABLE; 
	//io_conf.pull_down_en = GPIO_PULLUP_DISABLE;
	esp_err_t retVal = gpio_config(&io_conf);
	if (retVal == ESP_OK)
	{
		printf("I2S pins high impedance mode had been deinitialized.\n");
	}
	else
	{
		printf("PROBLEM with I2S pins high impedance mode deinitialization.\n");
		assert(!ESP_OK);
	}
	return retVal;
}