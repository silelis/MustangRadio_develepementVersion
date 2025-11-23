#pragma once

#include "led_strip.h"
#include "esp_log.h"
#include <string.h> // dla memcmp
#include <stdbool.h> // dla typu bool
//#include "D:\!!!__GitHUB_repositories\MustangRadio_develepementVersion\02_Firmware\03_Common\comunicationStructures.h"
#include "D:\!!!__GitHUB_repositories\MustangRadio_develepementVersion\02_Firmware\01_ESP32\HMI_and_bluetooth\main\common\comunicationStructures\comunicationStructures.h"
#include "D:\!!!__GitHUB_repositories\MustangRadio_develepementVersion\02_Firmware\01_ESP32\HMI_and_bluetooth\main\common/i2c_slave_master_queueClass/i2c_slave_master_queueClass.h"


#define LED_DISPLAY_BLINK_TIME_MULTIPLIER	7
#define LED_DISPLAY_BLINK_TIME				100
class LEDS_BACKLIGHT
{
public:
	LEDS_BACKLIGHT(/*led_strip_rmt_config_t rtmConfig,*/ int gpioNum, uint8_t maxLed, led_pixel_format_t ledPixelFormat, led_model_t ledModel);
	~LEDS_BACKLIGHT();
	
	esp_err_t ledStripSet_sourceLed(struct ws2812Color colorSet);
	esp_err_t ledStripSet_equaliserLed(struct ws2812Color colorSet);
	esp_err_t ledStripSet_errorLed(struct ws2812Color colorSet);
	esp_err_t ledStripSet_backlightLeds(struct ws2812Color colorSet);
	esp_err_t ledStripRefresh();
	esp_err_t ledStripClearAll();
	bool areEqual(const struct colorSet *ledColors);// struct ws2812Color *color1, const struct ws2812Color *color2)
	
	BaseType_t SemaphoreGive(void);
	BaseType_t SemaphoreTake(TickType_t xTicksToWait);
	BaseType_t QueueSendDataToLedTask(i2cFrame_transmitQueue * pvItemToQueue);
	BaseType_t QueueReceiveFormI2cParsingTask(i2cFrame_transmitQueue* pvBuffer, TickType_t xTicksToWait);
	void QueueDeleteDataFormI2cParsingTask(i2cFrame_transmitQueue structWithPointer);
	UBaseType_t QueueMessagesWaiting(void);
	
	void blinkTimeMultiplierSetMaxValue(void);
	void blinkTimeDelayLoop();
protected:
	
private:
	const char *TAG = "Led Bglight log:";
	uint32_t errorLedAddress;
	uint32_t equaliserLedAddress;
	uint32_t sourceLedAddress;
	led_strip_handle_t ledStrip;
	SemaphoreHandle_t handlerMutex_ledDisplay_Backlight; //mutex synchronizuj�cy wy�wietlanie komunikat�w led�w (source, equaliser, error) i pod�wietlenia (backlight);
	i2cQueue4DynamicData* ParserDataToLedsDataQueue;
	uint8_t blinkTimeMultiplier;
	void blinkTimeMultiplierReset(void);
	void blinkTimeMultiplierIncrement(void);
};