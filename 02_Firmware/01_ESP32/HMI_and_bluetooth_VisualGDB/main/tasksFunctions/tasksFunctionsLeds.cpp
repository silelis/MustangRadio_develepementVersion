#include "tasksFunctionsLeds.h"

LEDS_BACKLIGHT *pLedDisplay;

void humanMahineBacklightLeds(void *lnothing)
{
	colorSet baclightLedsLocal;
	baclightLedsLocal.primary = { 120, 120, 120 };
	const uint8_t COLOR_DIFFERENCE = 50;
		
	//xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
	pLedDisplay->SemaphoreTake(portMAX_DELAY);
	pLedDisplay->ledStripSet_backlightLeds(baclightLedsLocal.primary);
	pLedDisplay->ledStripRefresh();
	
	if (baclightLedsLocal.primary.red <= 127)
	{
		baclightLedsLocal.secondary.red = baclightLedsLocal.primary.red + COLOR_DIFFERENCE;
	}
	else
	{
		baclightLedsLocal.secondary.red = baclightLedsLocal.primary.red - COLOR_DIFFERENCE;
	}
	if (baclightLedsLocal.primary.green <= 127)
	{
		baclightLedsLocal.secondary.green = baclightLedsLocal.primary.green + COLOR_DIFFERENCE;
	}
	else
	{
		baclightLedsLocal.secondary.green = baclightLedsLocal.primary.green - COLOR_DIFFERENCE;
	}
	
	if (baclightLedsLocal.primary.blue <= 127)
	{
		baclightLedsLocal.secondary.blue = baclightLedsLocal.primary.blue + COLOR_DIFFERENCE;
	}
	else
	{
		baclightLedsLocal.secondary.blue = baclightLedsLocal.primary.blue - COLOR_DIFFERENCE;
	}
	//xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
	pLedDisplay->SemaphoreGive();
	
	
		
	for (;;)
	{
		vTaskSuspend(NULL);
		//xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		pLedDisplay->SemaphoreTake(portMAX_DELAY);
		pLedDisplay->ledStripSet_backlightLeds(baclightLedsLocal.secondary);
		pLedDisplay->ledStripRefresh();
		//xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
		pLedDisplay->SemaphoreGive();
		vTaskDelay(pdMS_TO_TICKS(150));
		
		//xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		pLedDisplay->SemaphoreTake(portMAX_DELAY);
		pLedDisplay->ledStripSet_backlightLeds(baclightLedsLocal.primary);
		pLedDisplay->ledStripRefresh();
		//xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
		pLedDisplay->SemaphoreGive();
	}
}

void humanMahineDisplayLeds(void *nothiong)
{
	hmiLeds ledsLocal;
	const uint16_t blinkTime = LED_DISPLAY_BLINK_TIME;
	
	ledsLocal.errorLed.primary = { 128, 0, 0 };
	ledsLocal.errorLed.secondary = { 0, 0, 0 };	
	
	ledsLocal.sourceLed.primary = { 0, 0, 0 };	
	ledsLocal.sourceLed.secondary = { 0, 0, 0 };	
	
	ledsLocal.equaliserLed.primary = { 0, 0, 0 };	
	ledsLocal.equaliserLed.secondary = { 0, 0, 0 };
	
	
	uint8_t ledsPrimarySecondary = 0;
	i2cFrame_transmitQueue tempBuffer;
	
	for (;;)
	{
		//xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		if (pLedDisplay->QueueReceiveFormI2cParsingTask(&tempBuffer, pdMS_TO_TICKS(LED_DISPLAY_BLINK_TIME)) == pdTRUE)
		{
			//i2cFrame_hmiLeds* tempHmiLedsFrame;
			//tempHmiLedsFrame = (i2cFrame_hmiLeds*) tempBuffer.pData; 
			//memcpy(&ledsLocal, &tempHmiLedsFrame->ledsData, sizeof(hmiLeds));
			memcpy(&ledsLocal, tempBuffer.pData + sizeof(i2cFrame_commonHeader), sizeof(hmiLeds));
			pLedDisplay->QueueDeleteDataFormI2cParsingTask(tempBuffer);
		}
		
		
		
		pLedDisplay->SemaphoreTake(portMAX_DELAY);
			
		if (pLedDisplay->areEqual(&ledsLocal.equaliserLed)&&
			pLedDisplay->areEqual(&ledsLocal.errorLed)&&
			pLedDisplay->areEqual(&ledsLocal.sourceLed))		//if all leds primary == secondary
		{
			pLedDisplay->ledStripSet_sourceLed(ledsLocal.sourceLed.primary);								
			pLedDisplay->ledStripSet_equaliserLed(ledsLocal.equaliserLed.primary);
			pLedDisplay->ledStripSet_errorLed(ledsLocal.errorLed.primary);

			pLedDisplay->ledStripRefresh();

			
			//xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
			pLedDisplay->SemaphoreGive();
			if (pLedDisplay->QueueMessagesWaiting() == 0)
			{
				extern TaskHandle_t handlerTask_ledDisplay;
				//vTaskSuspend(NULL); //all primary == secondary so suspend this task
				vTaskSuspend(handlerTask_ledDisplay);
			}
		}
		else		
		{
			//if at least one led primary != secondary
			
			switch (ledsPrimarySecondary)
			{
			case 0:
				pLedDisplay->ledStripSet_equaliserLed(ledsLocal.equaliserLed.primary);
				pLedDisplay->ledStripSet_errorLed(ledsLocal.errorLed.primary);
				pLedDisplay->ledStripSet_sourceLed(ledsLocal.sourceLed.primary);
				ledsPrimarySecondary = 1; //next time leds flip to secondary color
				break;
			case 1:
				pLedDisplay->ledStripSet_equaliserLed(ledsLocal.equaliserLed.secondary);
				pLedDisplay->ledStripSet_errorLed(ledsLocal.errorLed.secondary);
				pLedDisplay->ledStripSet_sourceLed(ledsLocal.sourceLed.secondary);
				ledsPrimarySecondary = 0; //next time leds flip to primary color
				break;
				
			}
			
			pLedDisplay->ledStripRefresh();
			pLedDisplay->SemaphoreGive();
			pLedDisplay->blinkTimeDelayLoop();
			
			
			
			//vTaskDelay(pdMS_TO_TICKS(LED_DISPLAY_BLINK_TIME));													//wait 1s till change color
		}
	}
}