#pragma once

#include "led_strip.h"
#include "esp_log.h"

#define LED_DISPLAY_BLINK_TIME	1000

class LEDS_BACKLIGHT
{
public:
	LEDS_BACKLIGHT(/*led_strip_rmt_config_t rtmConfig,*/ int gpioNum, uint8_t maxLed, led_pixel_format_t ledPixelFormat, led_model_t ledModel);
	~LEDS_BACKLIGHT();
	
	esp_err_t ledStripSet_sourceLed(uint32_t red, uint32_t green, uint32_t blue);
	esp_err_t ledStripSet_equaliserLed(uint32_t red, uint32_t green, uint32_t blue);
	esp_err_t ledStripSet_errorLed(uint32_t red, uint32_t green, uint32_t blue);
	esp_err_t ledStripSet_backlightLeds(uint32_t red, uint32_t green, uint32_t blue);
	esp_err_t ledStripRefresh();
	esp_err_t ledStripClearAll();
	
protected:
	
private:
	const char *TAG = "Led Bglight log:";
	uint32_t errorLedAddress;
	uint32_t equaliserLedAddress;
	uint32_t sourceLedAddress;
	led_strip_handle_t ledStrip;
};