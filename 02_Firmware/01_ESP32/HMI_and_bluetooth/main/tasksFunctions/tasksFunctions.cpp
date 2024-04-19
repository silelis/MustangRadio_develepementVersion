#include "tasksFunctions.h"


QueueHandle_t handlerQueue_MainKeyboard;				//wskaźnik do kolejki przechowującej wyniki odpowiedzi z klawiatury		
QueueHandle_t handlerQueue_i2cFrameTransmittBuffer;		//wskaźnik do kolejki przechowującej dane jakie mają być wysłane po i2c z ESP32 do STM32






TaskHandle_t handlerTask_keyboardQueueParametersParser; //uchwyt do taska realizującego parsowanie / sprawdzanie danych w kolejce
														//klawiatury "handlerQueue_MainKeyboard"
TaskHandle_t handlerTask_ledDisplay;					//uchwyt do taska wyświetlającego ledy 
TaskHandle_t handlerTask_backlightDisplay;				//uchwyt do taska wyświetlającego backlight
TaskHandle_t handlerTask_stepperMotor;					//uchwyt do taska zarządzającego ruchami silnika krokowego





SemaphoreHandle_t handlerMutex_ledDisplay_Backlight;	//mutex synchronizujący wyświetlanie komunikatów ledów (source, equaliser, error) i podświetlenia (backlight);




hmiDisplay displayLedsColors; //struktura zawierająca informacje na temat wszystkich stanów (kolorów) diód w wyświetlaczu



/*---------------------------------------------------------------
* Lokalna funkcja potrzebna na etapie programownaia. Poprzez tę 
* funkcję sprawdzam czy do taska parsującego dane z klawiatury
* *wpływają popraswen dane i dane sa poprawenie przetważane.
*---------------------------------------------------------------*/ 
static void keyboardQueueParametersParserPrintf(keyboardUnion DataToParse)
{
	switch (DataToParse.array[0])
	{
	case HMI_INPUT_BUTTON:
	case HMI_INPUT_BUTTON_LONG_AND_PRESSED:	
		printf("KBRD %c: %x\n", DataToParse.array[0], DataToParse.kbrdValue.value);
					
								
		break;	
	case HMI_INPUT_VOLUME:
	case HMI_INPUT_EQUALISER:
		if ((DataToParse.encoderValue.value == ENCODER_PULSED_PER_DETANT) || (DataToParse.encoderValue.value == -ENCODER_PULSED_PER_DETANT))
			printf("ENC %c: %d\n", DataToParse.array[0], DataToParse.encoderValue.value);
		break;
	}
	
}



void keyboardQueueParametersParser(void *parameters)
{
	bool isQueueFeedRequirted = pdFALSE;
	BaseType_t queueFeedRetVal;
	keyboardUnion keyboardDataToParse;
	keyboardDataToParse.array[0] = 0;
	keyboardDataToParse.array[1] = 0;
	i2cFrame keyboardDataToI2cTransmittQueue;
	taskParameters_keyboardQueueParametersParser* handlerStruct = (taskParameters_keyboardQueueParametersParser*) parameters;
	
	keyboardDataToI2cTransmittQueue.frameSize = sizeof(keyboardDataToI2cTransmittQueue.frameSize) + sizeof(keyboardDataToI2cTransmittQueue.commandGroup) + sizeof(keyboardDataToI2cTransmittQueue.commandData);
	for (;;)
	{
		//sprawdza czy w kolejce danych z klawiatury znajdują sie jakiekolwiek dane do parsowania 
		if (xQueueReceive(handlerStruct->handlerQueue_mainKeyboard, &keyboardDataToParse, portMAX_DELAY))
		{
			//sprawdza czy dane do parsowania pochodzą z przyciskow (short press/ long on press/ long press release)
			if ((keyboardDataToParse.array[0] == HMI_INPUT_BUTTON) || (keyboardDataToParse.array[0] == HMI_INPUT_BUTTON_LONG_AND_PRESSED)) {
				isQueueFeedRequirted  = pdTRUE;
			}
			
			//sprawdza czy dane do parsowania pochodzą z enkoderów
			if ((keyboardDataToParse.array[0] == HMI_INPUT_VOLUME) || (keyboardDataToParse.array[0] == HMI_INPUT_EQUALISER))
			{
				//sprawdza czy dane z enkoderów sa poprawne, tzn. czy są równie +/- encoder detant
				if ((keyboardDataToParse.encoderValue.value == ENCODER_PULSED_PER_DETANT) || (keyboardDataToParse.encoderValue.value == -ENCODER_PULSED_PER_DETANT)) {
					isQueueFeedRequirted  = pdTRUE;
				}
			}
			
		//jeśli dane są poprawne to następuje przesłanie danych do kolejki buffora nadawczego i2c
		if (isQueueFeedRequirted  == pdTRUE)
		{
			keyboardDataToI2cTransmittQueue.commandGroup = I2C_COMMAND_GROUP_KEYBOARD;
			memcpy(&keyboardDataToI2cTransmittQueue.commandData.keyboardData, &keyboardDataToParse, sizeof(keyboardUnion));
			assert(xQueueSend(handlerStruct->handlerQueue_i2cFrameTransmitt, &keyboardDataToI2cTransmittQueue, pdMS_TO_TICKS(5000)/*portMAX_DELAY*/));
			
			/*----------------------------------------------------------------------*/
			//poniższa funkcja jkest tylko do celów debugowania poprawności programu
			//i2cFrame daneZKolejki;
			//xQueueReceive(handlerStruct->handlerQueue_i2cFrameTransmitt, &daneZKolejki, portMAX_DELAY);
			keyboardQueueParametersParserPrintf(keyboardDataToParse);
			//keyboardQueueParametersParserPrintf(daneZKolejki.commandData.keyboardData);	
			//poniższa funkcja jkest tylko do celów debugowania poprawności programu
			/*----------------------------------------------------------------------*/
			}		
		}
		isQueueFeedRequirted  = pdFALSE;
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

