#include "ESP32-A2DP_class.h"

/**/
I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);
 
void setup_a2dp() {
    auto cfg = i2s.defaultConfig();
    cfg.pin_bck = GPIO_NUM_26;
    cfg.pin_ws = GPIO_NUM_27;
    cfg.pin_data = GPIO_NUM_25;
    bool is_active= i2s.begin(cfg);

    a2dp_sink.start("MyMusic");
}
*/
esp_err_t i2sPinsHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data);
{
	gpio_config_t io_conf = { };
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = ((1ULL << pin_bck) | (1ULL << pin_ws) | (1ULL << pin_data));
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


esp_err_t esp32_a2dp_sink::i2sPinsHighImpedanceDisabled(void)
{
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = ((1ULL << this->pin_bck) | (1ULL << this->pin_bck) | (1ULL << this->pin_data));
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

esp32_a2dp_sink::esp32_a2dp_sink(const char *name, int pin_bck, int pin_ws, int pin_data){

         this->pin_bck=pin_bck;
         this->pin_ws=pin_ws;
         this->pin_data=pin_data;
         this->btDeviceName= name;
}