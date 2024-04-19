#include "tasksFunctions.h"


QueueHandle_t handlerQueue_MainKeyboard;				//wskaźnik do kolejki przechowującej wyniki odpowiedzi z klawiatury		
TaskHandle_t handlerTask_keyboardQueueParametersParser; //uchwyt do taska realizującego parsowanie / sprawdzanie danych w kolejce
														//klawiatury "handlerQueue_MainKeyboard"
TaskHandle_t handlerTask_ledDisplay;					//uchwyt do taska wyświetlającego ledy 
TaskHandle_t handlerTask_backlightDisplay;				//uchwyt do taska wyświetlającego backlight
TaskHandle_t handlerTask_stepperMotor;					//uchwyt do taska zarządzającego ruchami silnika krokowego


SemaphoreHandle_t handlerMutex_ledDisplay_Backlight;	//mutex synchronizujący wyświetlanie komunikatów ledów (source, equaliser, error) i podświetlenia (backlight);




hmiDisplay displayLedsColors; //struktura zawierająca informacje na temat wszystkich stanów (kolorów) diód w wyświetlaczu


void keyboardQueueParametersParser(void *nothing)
{
	keyboardUnion dataToParse;
	dataToParse.array[0] = 0;
	dataToParse.array[1] = 0;

	
	for (;;)
	{
		if (xQueueReceive(handlerQueue_MainKeyboard, &dataToParse, portMAX_DELAY))
		{
			switch (dataToParse.array[0])
			{
			case HMI_INPUT_BUTTON:
			case HMI_INPUT_BUTTON_LONG_AND_PRESSED:	
				printf("KBRD %c: %x\n", dataToParse.array[0], dataToParse.kbrdValue.value);
				break;	
			case HMI_INPUT_VOLUME:
			case HMI_INPUT_EQUALISER:
				if ((dataToParse.encoderValue.value == ENCODER_PULSED_PER_DETANT) || (dataToParse.encoderValue.value == -ENCODER_PULSED_PER_DETANT))
				//if ((dataToParse.array[1] % ENCODER_PULSED_PER_DETANT) == 0 && (dataToParse.array[1] != 0))
					printf("ENC %c: %d\n", dataToParse.array[0], dataToParse.encoderValue.value);
				break;
			}
		}
	}
}


static 	bool areEqual(const struct ws2812Color *color1, const struct ws2812Color *color2) {
	return memcmp(color1, color2, sizeof(struct ws2812Color)) == 0;
}

static void init_BacklightColors(LEDS_BACKLIGHT *ledsDisplay)
{	
	xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
	ledsDisplay->ledStripSet_backlightLeds((uint32_t) displayLedsColors.backlightLeds.primary.red,
		(uint32_t) displayLedsColors.backlightLeds.primary.green,
		(uint32_t) displayLedsColors.backlightLeds.primary.blue);
	ledsDisplay->ledStripRefresh();
	
	if (displayLedsColors.backlightLeds.primary.red <= 127)
	{
		displayLedsColors.backlightLeds.secondary.red = displayLedsColors.backlightLeds.primary.red + 25;
	}
	else
	{
		displayLedsColors.backlightLeds.secondary.red = displayLedsColors.backlightLeds.primary.red - 25;
	}
	if (displayLedsColors.backlightLeds.primary.green <= 127)
	{
		displayLedsColors.backlightLeds.secondary.green = displayLedsColors.backlightLeds.primary.green + 25;
	}
	else
	{
		displayLedsColors.backlightLeds.secondary.green = displayLedsColors.backlightLeds.primary.green - 25;
	}
	
	if (displayLedsColors.backlightLeds.primary.blue <= 127)
	{
		displayLedsColors.backlightLeds.secondary.blue = displayLedsColors.backlightLeds.primary.blue + 25;
	}
	else
	{
		displayLedsColors.backlightLeds.secondary.blue = displayLedsColors.backlightLeds.primary.blue - 35;
	}
	xSemaphoreGive(handlerMutex_ledDisplay_Backlight);	
}


void humanMahineBacklightLeds(void *ledsDisplay)
{
	LEDS_BACKLIGHT *leds = static_cast<LEDS_BACKLIGHT*>(ledsDisplay);		
	init_BacklightColors(leds);
		
	for (;;)
	{
		vTaskSuspend(NULL);
		xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		leds->ledStripSet_backlightLeds((uint32_t) displayLedsColors.backlightLeds.secondary.red,
			(uint32_t) displayLedsColors.backlightLeds.secondary.green,
			(uint32_t) displayLedsColors.backlightLeds.secondary.blue);
		leds->ledStripRefresh();
		xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
		vTaskDelay(pdMS_TO_TICKS(150));
		
		xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		leds->ledStripSet_backlightLeds((uint32_t) displayLedsColors.backlightLeds.primary.red,
			(uint32_t) displayLedsColors.backlightLeds.primary.green,
			(uint32_t) displayLedsColors.backlightLeds.primary.blue);
		leds->ledStripRefresh();
		xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
		//vTaskSuspend(NULL);	
	}
}
	
	
void humanMahineDisplayLeds(void *ledsDisplay)
{
	LEDS_BACKLIGHT *leds = static_cast<LEDS_BACKLIGHT*>(ledsDisplay);

	uint8_t ledsPrimarySecondary = 0;
	for (;;)
	{
		xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		//leds->ledStripSet_backlightLeds((uint32_t) displayLedsColors.backlightLeds.primary.red,
		//	(uint32_t) displayLedsColors.backlightLeds.primary.green,
		//	(uint32_t) displayLedsColors.backlightLeds.primary.blue);
		
		if (areEqual(&displayLedsColors.equaliserLed.primary, &displayLedsColors.equaliserLed.secondary)&&
					areEqual(&displayLedsColors.errorLed.primary, &displayLedsColors.errorLed.secondary)&&
					areEqual(&displayLedsColors.sourceLed.primary, &displayLedsColors.sourceLed.secondary))		//if all leds primary == secondary
		{
			
			
											
			leds->ledStripSet_equaliserLed((uint32_t) displayLedsColors.equaliserLed.primary.red,
														(uint32_t) displayLedsColors.equaliserLed.primary.green,
														(uint32_t) displayLedsColors.equaliserLed.primary.blue);
			leds->ledStripSet_errorLed((uint32_t) displayLedsColors.errorLed.primary.red,
														(uint32_t) displayLedsColors.errorLed.primary.green,
														(uint32_t) displayLedsColors.errorLed.primary.blue);
			leds->ledStripSet_sourceLed((uint32_t) displayLedsColors.sourceLed.primary.red,
														(uint32_t) displayLedsColors.sourceLed.primary.green,
														(uint32_t) displayLedsColors.sourceLed.primary.blue);
			//leds->ledStripSet_backlightLeds((uint32_t) displayLedsColors.backlightLeds.primary.red,
			//	(uint32_t) displayLedsColors.backlightLeds.primary.green,
			//	(uint32_t) displayLedsColors.backlightLeds.primary.blue);
			leds->ledStripRefresh();

			
			xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
			vTaskSuspend(NULL);																					//all primary == secondary so suspend this task
		}
		else		
		{																										//if at least one led primary != secondary
			
			switch (ledsPrimarySecondary)
			{
			case 0:
				leds->ledStripSet_equaliserLed((uint32_t) displayLedsColors.equaliserLed.primary.red,
					(uint32_t) displayLedsColors.equaliserLed.primary.green,
					(uint32_t) displayLedsColors.equaliserLed.primary.blue);
				leds->ledStripSet_errorLed((uint32_t) displayLedsColors.errorLed.primary.red,
					(uint32_t) displayLedsColors.errorLed.primary.green,
					(uint32_t) displayLedsColors.errorLed.primary.blue);
				leds->ledStripSet_sourceLed((uint32_t) displayLedsColors.sourceLed.primary.red,
					(uint32_t) displayLedsColors.sourceLed.primary.green,
					(uint32_t) displayLedsColors.sourceLed.primary.blue);
				ledsPrimarySecondary = 1; //next time leds flip to secondary color
				break;
			case 1:
				leds->ledStripSet_equaliserLed((uint32_t) displayLedsColors.equaliserLed.secondary.red,
					(uint32_t) displayLedsColors.equaliserLed.secondary.green,
					(uint32_t) displayLedsColors.equaliserLed.secondary.blue);
				leds->ledStripSet_errorLed((uint32_t) displayLedsColors.errorLed.secondary.red,
					(uint32_t) displayLedsColors.errorLed.secondary.green,
					(uint32_t) displayLedsColors.errorLed.secondary.blue);
				leds->ledStripSet_sourceLed((uint32_t) displayLedsColors.sourceLed.secondary.red,
					(uint32_t) displayLedsColors.sourceLed.secondary.green,
					(uint32_t) displayLedsColors.sourceLed.secondary.blue);
				ledsPrimarySecondary = 0; //next time leds flip to primary color
				break;
				
			}
			
			//leds->ledStripSet_backlightLeds((uint32_t) displayLedsColors.backlightLeds.primary.red,
			//	(uint32_t) displayLedsColors.backlightLeds.primary.green,
			//	(uint32_t) displayLedsColors.backlightLeds.primary.blue);
			
			leds->ledStripRefresh();
			xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
			vTaskDelay(pdMS_TO_TICKS(LED_DISPLAY_BLINK_TIME));													//wait 1s till change color
		}
	}
}


void stepperMotor(void *TaskParameters)
{
	NVS * pStorage;
	StepperOpto * pMotor;
	
	{
		motorTaskParam *parameter = (motorTaskParam*) TaskParameters;
		pMotor = parameter->motorPointer;
		pStorage = parameter->storagePointer;
	}

	//pStorage->get_blob(NVS_KEY_BLOB_MotorParameters, pMotor->)

	MotorParameters test;
	test.beginOffest = 1;
	test.currentPosition = 2;
	test.endOffset = 50;
	test.maxPosition = 5000;
	pMotor->setValue_motorParameters(&test);
	for (;;)
	{
		pMotor->measureSliderRange();
		//vTaskPrioritySet(NULL, tskIDLE_PRIORITY);
		vTaskSuspend(NULL);

	}
}

