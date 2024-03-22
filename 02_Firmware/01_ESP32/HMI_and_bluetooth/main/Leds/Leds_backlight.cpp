#include "Leds_backlight.h"

/*---------------------------------------------------------------
 * Konstruktor klasy obsługującej diody HMI.
 * Parameters:
 * int gpioNum - numer GPIO do którego podłaczone są diody
 * uint8_t maxLed - ilość diód (ilość diód podświetlenia + ilośc
 *                  diód HMI)
 * led_pixel_format_t ledPixelFormat - rodzaj diód 
 *                  (np. LED_PIXEL_FORMAT_GRB dla GRB)
 * led_model_t ledModel - typ protokołu diody 
 *                  (np. LED_MODEL_WS2812)
 * Returns:
 * NONE
*---------------------------------------------------------------*/
LEDS_BACKLIGHT::LEDS_BACKLIGHT(/*led_strip_rmt_config_t rtmConfig,*/ int gpioNum, uint8_t maxLed, led_pixel_format_t ledPixelFormat, led_model_t ledModel)
{
	
	ESP_LOGI(TAG, "Leds and background light configuration");
	/* LED strip initialization with the GPIO and pixels number*/

	
	
	this->errorLedAddress = maxLed - 1;
	this->equaliserLedAddress = maxLed - 2;
	this->sourceLedAddress = maxLed - 3;
	led_strip_config_t strip_config =
	{
		.strip_gpio_num = gpioNum,				//16,
		//BLINK_GPIO,
		.max_leds = maxLed,						//4,
		// at least one LED on board
		.led_pixel_format = ledPixelFormat,		//LED_PIXEL_FORMAT_GRB,
		.led_model = ledModel					//LED_MODEL_WS2812
	};
	
	
	led_strip_rmt_config_t rmt_config = 
	{
		.resolution_hz = 10 * 1000 * 1000,		// 10MHz
	};
	ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &this->ledStrip));
	
	this->ledStripClearAll();
	this->ledStripRefresh();
}

/*---------------------------------------------------------------
 * Metoda czyści (wygasza) wszystkie diody.
 * Parameters:
 * NONE
 *                  (np. LED_MODEL_WS2812)
 * Returns:
 * esp_err_t (esp_err_t aka retval) - wynik operacji (0x00 operacja
 *           przebiegła poprawnie)
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripClearAll()
{
	return led_strip_clear(this->ledStrip);
}


/*---------------------------------------------------------------
 * Metoda ustawia kolor na diodzie source (źródło dźwięku)
 * Parameters:
 * uint32_t red - kolor czerwony (0-255)
 * uint32_t green - kolor czerwony (0-255)
 * uint32_t blue - kolor czerwony (0-255)
 * Returns:
 * esp_err_t (esp_err_t aka retval) - wynik operacji (0x00 operacja
 *           przebiegła poprawnie)
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripSet_sourceLed(uint32_t red, uint32_t green, uint32_t blue)
{
	return led_strip_set_pixel(this->ledStrip, this->sourceLedAddress, red, green, blue);
}


/*---------------------------------------------------------------
 * Metoda ustawia kolor na diodzie equaliser (dioda sygnalizująca
 * aktualne menu ustawień radio)
 * Parameters:
 * uint32_t red - kolor czerwony (0-255)
 * uint32_t green - kolor czerwony (0-255)
 * uint32_t blue - kolor czerwony (0-255)
 * Returns:
 * NONE
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripSet_equaliserLed(uint32_t red, uint32_t green, uint32_t blue)
{
	return led_strip_set_pixel(this->ledStrip, this->equaliserLedAddress, red, green, blue);
}


/*---------------------------------------------------------------
 * Metoda ustawia kolor na diodzie sygnalizacji błędu.
 * Parameters:
 * uint32_t red - kolor czerwony (0-255)
 * uint32_t green - kolor czerwony (0-255)
 * uint32_t blue - kolor czerwony (0-255)
 * Returns:
 * esp_err_t (esp_err_t aka retval) - wynik operacji (0x00 operacja
 *           przebiegła poprawnie)
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripSet_errorLed(uint32_t red, uint32_t green, uint32_t blue)
{
	return led_strip_set_pixel(this->ledStrip, this->errorLedAddress, red, green, blue);
}


/*---------------------------------------------------------------
 * Metoda ustawia kolor podświetlenia (wszystkie diody
 * podświetlenia na raz).
 * Parameters:
 * uint32_t red - kolor czerwony (0-255)
 * uint32_t green - kolor czerwony (0-255)
 * uint32_t blue - kolor czerwony (0-255)
 * Returns:
 * esp_err_t (esp_err_t aka retval) - wynik operacji (0x00 operacja
 *           przebiegła poprawnie)
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripSet_backlightLeds(uint32_t red, uint32_t green, uint32_t blue)
{
	esp_err_t retVal = ESP_FAIL;
	for (int8_t i = 0; i < this->sourceLedAddress; i++)
	{
		retVal = led_strip_set_pixel(this->ledStrip, i, red, green, blue);
		if (retVal != ESP_OK)
		{
			break;
		}
	}	
	return retVal;
}


/*---------------------------------------------------------------
 * Metoda odeirza kolor diód (wszystkich na raz). Wysyła dane o
 * kolorze z pamięci do diód.
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t (esp_err_t aka retval) - wynik operacji (0x00 operacja
 *           przebiegła poprawnie)
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripRefresh()
{
	return led_strip_refresh(this->ledStrip);
}

/*---------------------------------------------------------------
 * Destruktior klasy obsługującej diody HMI.
 * Parameters:
 * NONE
 * Returns:
 * NONE
*---------------------------------------------------------------*/
LEDS_BACKLIGHT::~LEDS_BACKLIGHT()
{
	this->ledStripClearAll();
	this->ledStripRefresh();
	ESP_ERROR_CHECK(led_strip_clear(this->ledStrip));
	ESP_ERROR_CHECK(led_strip_del(this->ledStrip));
}
