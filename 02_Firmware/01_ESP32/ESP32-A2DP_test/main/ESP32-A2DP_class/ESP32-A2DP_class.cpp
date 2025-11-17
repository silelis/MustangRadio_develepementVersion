#include "ESP32-A2DP_class.h"

/*
I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);
 
void setup_a2dp() {
    auto cfg = i2s.defaultConfig();
    cfg.pin_bck = GPIO_NUM_26;
    cfg.pin_ws = GPIO_NUM_27;
    cfg.pin_data = GPIO_NUM_25;
    bool is_active= i2s.begin(cfg);

    a2dp.set_on_connection_state_changed(onConn);
    a2dp.set_on_audio_state_changed_post(onAudioState);
    a2dp.set_avrc_rn_playstatus_callback(onAVRCPlayStatus);
    a2dp.start("MyMusic");
    a2dp_sink.start("MyMusic");
}
*/

i2sPinStates esp32_a2dp_sink::i2sState;

esp_err_t i2sPinsHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data)
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
		esp32_a2dp_sink::i2sState = highZenabled;
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
		esp32_a2dp_sink::i2sState = highZdisabled;
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
//         this->btDeviceName= name;

}

void esp32_a2dp_sink::enableBluetoothModuleAndI2S(const char *btDeviceName){
	this->i2sPinsHighImpedanceDisabled();

	auto cfg = i2s->defaultConfig();
    cfg.pin_bck =this->pin_bck;
    cfg.pin_ws = this->pin_ws;
    cfg.pin_data = this->pin_data;
    bool is_active= i2s->begin(cfg);

	if (is_active == false){
		esp32_a2dp_sink::i2sState = i2sNotConfigured;
		assert(0);
	}
	else{
		esp32_a2dp_sink::i2sState = i2sConfigured;
			a2dp_sink->set_on_connection_state_changed(this->btCallback_onConn);
 			a2dp_sink->set_on_audio_state_changed_post(this->btCallback_onAudioState);
  			a2dp_sink->set_avrc_rn_playstatus_callback(this->btCallback_onAVRCPlayStatus);
  			a2dp_sink->start(btDeviceName);
	}
}

void esp32_a2dp_sink::btCallback_onConn(esp_a2d_connection_state_t state, void*){

}
void esp32_a2dp_sink::btCallback_onAudioState(esp_a2d_audio_state_t state, void*){

}
void esp32_a2dp_sink::btCallback_onAVRCPlayStatus(esp_avrc_playback_stat_t playback, void*){

}
