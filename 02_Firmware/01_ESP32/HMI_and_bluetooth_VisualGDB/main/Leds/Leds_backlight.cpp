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
	
	configASSERT(this->ParserDataToLedsDataQueue = new i2cQueue4DynamicData(4));
	//ESP_LOGI(TAG, "Leds and background light configuration");
	printf("%s: Leds and background light configuration\n", TAG);
	/* LED strip initialization with the GPIO and pixels number*/

	
	//tworzenie semafora dla punktu aktualizacji zmiennych przechowujących dane o ledach
	this->handlerMutex_ledDisplay_Backlight = NULL; //czyści wskaźnik mutex'u dla podświetlenia	i diód sygnalizacyjnych, bo kilka tasków bedzi ekorzystać z linii komunikacyjnej WS2812 		
	configASSERT(this->handlerMutex_ledDisplay_Backlight = xSemaphoreCreateBinary()); //tworzy mutex dla podświetlenia
	//xSemaphoreGive(this->handlerMutex_ledDisplay_Backlight);
	this->SemaphoreGive();
	
	
	
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

BaseType_t LEDS_BACKLIGHT::SemaphoreGive(void)
{
	return xSemaphoreGive(this->handlerMutex_ledDisplay_Backlight);
}

BaseType_t LEDS_BACKLIGHT::SemaphoreTake(TickType_t xTicksToWait)
{
	return xSemaphoreTake(this->handlerMutex_ledDisplay_Backlight, xTicksToWait);	
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
 * struct ws2812Color colorSet - uint32_t red - kolor czerwony
 *								(0-255)
 *								 uint32_t green - kolor czerwony
 *								 (0-255)
 *								 uint32_t blue - kolor czerwony
 *								 (0-255)
 * Returns:
 * esp_err_t (esp_err_t aka retval) - wynik operacji (0x00 operacja
 *           przebiegła poprawnie)
*---------------------------------------------------------------*/

esp_err_t LEDS_BACKLIGHT::ledStripSet_sourceLed(struct ws2812Color colorSet)
{
	return led_strip_set_pixel(this->ledStrip,
		this->sourceLedAddress,
		(uint32_t) colorSet.red,
		(uint32_t) colorSet.green,
		(uint32_t) colorSet.blue);
}


/*---------------------------------------------------------------
 * Metoda ustawia kolor na diodzie equaliser (dioda sygnalizująca
 * aktualne menu ustawień radio)
 * Parameters:
 * struct ws2812Color colorSet - uint32_t red - kolor czerwony
 *								(0-255)
 *								 uint32_t green - kolor czerwony
 *								 (0-255)
 *								 uint32_t blue - kolor czerwony
 *								 (0-255)
 * Returns:
 * NONE
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripSet_equaliserLed(struct ws2812Color colorSet)
{
	return led_strip_set_pixel(this->ledStrip,
		this->equaliserLedAddress,
		(uint32_t) colorSet.red,
		(uint32_t) colorSet.green,
		(uint32_t) colorSet.blue);
}


/*---------------------------------------------------------------
 * Metoda ustawia kolor na diodzie sygnalizacji błędu.
 * Parameters:
 * struct ws2812Color colorSet - uint32_t red - kolor czerwony
 *								(0-255)
 *								 uint32_t green - kolor czerwony
 *								 (0-255)
 *								 uint32_t blue - kolor czerwony
 *								 (0-255)
 * Returns:
 * esp_err_t (esp_err_t aka retval) - wynik operacji (0x00 operacja
 *           przebiegła poprawnie)
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripSet_errorLed(struct ws2812Color colorSet)
{
	return led_strip_set_pixel(this->ledStrip,
		this->errorLedAddress,
		(uint32_t) colorSet.red,
		(uint32_t) colorSet.green,
		(uint32_t) colorSet.blue);
}


/*---------------------------------------------------------------
 * Metoda ustawia kolor podświetlenia (wszystkie diody
 * podświetlenia na raz).
 * Parameters:
 * struct ws2812Color colorSet - uint32_t red - kolor czerwony
 *								(0-255)
 *								 uint32_t green - kolor czerwony
 *								 (0-255)
 *								 uint32_t blue - kolor czerwony
 *								 (0-255)
 * Returns:
 * esp_err_t (esp_err_t aka retval) - wynik operacji (0x00 operacja
 *           przebiegła poprawnie)
*---------------------------------------------------------------*/
esp_err_t LEDS_BACKLIGHT::ledStripSet_backlightLeds(struct ws2812Color colorSet)
{
	esp_err_t retVal = ESP_FAIL;
	for (int8_t i = 0; i < this->sourceLedAddress; i++)
	{
		retVal = led_strip_set_pixel(this->ledStrip,
			i,
			(uint32_t) colorSet.red,
			(uint32_t) colorSet.green,
			(uint32_t) colorSet.blue);
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
	delete this->ParserDataToLedsDataQueue;
}

/*---------------------------------------------------------------
* Metoda której zadaniem jest porównanie
* wartości kolorów primary i secondary diody led 
* Parameters:
* const struct colorSet *ledColors - wskaźnik do diody led
* Returns:
* bool - TRUE jeżeli kolory sa sobie równe, FALSE jeżeli kolory
*				nie są sobie równe.	  
*---------------------------------------------------------------*/
bool LEDS_BACKLIGHT::areEqual(const struct colorSet *ledColors) {
	//return (bool)(memcmp(&ledColors->, color2, sizeof(struct ws2812Color)) == 0) ;
	return memcmp(&ledColors->primary, &ledColors->secondary, sizeof(struct ws2812Color)) == 0;
}

BaseType_t LEDS_BACKLIGHT::QueueSendDataToLedTask(i2cFrame_transmitQueue * pvItemToQueue)
{
	return this->ParserDataToLedsDataQueue->QueueSend(pvItemToQueue);
}
	
BaseType_t LEDS_BACKLIGHT::QueueReceiveFormI2cParsingTask(i2cFrame_transmitQueue* pvBuffer, TickType_t xTicksToWait)
{
	return this->ParserDataToLedsDataQueue->QueueReceive(pvBuffer, xTicksToWait);
}

void LEDS_BACKLIGHT::QueueDeleteDataFormI2cParsingTask(i2cFrame_transmitQueue structWithPointer)
{
	this->ParserDataToLedsDataQueue->QueueDeleteDataFromPointer(structWithPointer);	
}

UBaseType_t LEDS_BACKLIGHT::QueueMessagesWaiting(void)
{
	return this->ParserDataToLedsDataQueue->QueueMessagesWaiting();
}



void LEDS_BACKLIGHT::blinkTimeMultiplierReset(void)
{
	this->blinkTimeMultiplier = 0;
}
void LEDS_BACKLIGHT::blinkTimeMultiplierIncrement(void)
{
	this->blinkTimeMultiplier++;
}

void LEDS_BACKLIGHT::blinkTimeMultiplierSetMaxValue(void)
{
	this->blinkTimeMultiplier = LED_DISPLAY_BLINK_TIME_MULTIPLIER;	
}

void LEDS_BACKLIGHT::blinkTimeDelayLoop(void) {
	this->blinkTimeMultiplierReset();
	do
	{
		vTaskDelay(pdMS_TO_TICKS(LED_DISPLAY_BLINK_TIME));
		this->blinkTimeMultiplierIncrement();
	} while (this->blinkTimeMultiplier < LED_DISPLAY_BLINK_TIME_MULTIPLIER);
	
}
