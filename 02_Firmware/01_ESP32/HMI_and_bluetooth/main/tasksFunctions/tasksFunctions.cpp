#include "tasksFunctions.h"



static QueueHandle_t handlerQueue_i2cFrameTransmittBuffer;		//wskaźnik do kolejki przechowującej dane jakie mają być wysłane po i2c z ESP32 do STM32
static SemaphoreHandle_t handlerMutex_ledDisplay_Backlight;	//mutex synchronizujący wyświetlanie komunikatów ledów (source, equaliser, error) i podświetlenia (backlight);

static hmiDisplay displayLedsColors; //struktura zawierająca informacje na temat wszystkich stanów (kolorów) diód w wyświetlaczu

static NVS* pSTORAGE; //obiekt zapisujący i czytający dane z NCS ESP32

/*---------------------------------------------------------------
* Funkcja, która poowinna być wywołana jak najwcześniej, a której
* zadaniem jest zainicjowanie handlerów, które sa niezbedne do
* prawidłowego działania tasków. Handlery w obrębie includa są
* GLOBALNE, ale jako, że mają atrybut STATIC nie są widoczna poza
* jewgo obrębem.
* Parameters:
* NONE
* Returns:
* NONE
 *---------------------------------------------------------------*/ 
void taskFunctionsStaticHandlersInit(void)
{
	displayLedsColors.errorLed.primary.blue = 0;
	displayLedsColors.errorLed.primary.green = 0;
	displayLedsColors.errorLed.primary.red = 25;
	displayLedsColors.errorLed.secondary.blue = 0;
	displayLedsColors.errorLed.secondary.green = 0;
	displayLedsColors.errorLed.secondary.red = 0;
	
	
	displayLedsColors.backlightLeds.primary.red = 0;
	displayLedsColors.backlightLeds.primary.green = 0;
	displayLedsColors.backlightLeds.primary.blue = 0;

	
	//tworzenie semafora dla punktu aktualizacji zmiennych przechowujących dane o ledach
	handlerMutex_ledDisplay_Backlight = NULL; //czyści wskaźnik mutex'u dla podświetlenia	i diód sygnalizacyjnych, bo kilka tasków bedzi ekorzystać z linii komunikacyjnej WS2812 		
	assert(handlerMutex_ledDisplay_Backlight = xSemaphoreCreateBinary()); //tworzy mutex dla podświetlenia
	xSemaphoreGive(handlerMutex_ledDisplay_Backlight);	
	
	//tworzenie kolejki bufora nadawczego i2c
	handlerQueue_i2cFrameTransmittBuffer = NULL;
	handlerQueue_i2cFrameTransmittBuffer = xQueueCreate(QueueHandleri2cFrameTransmittBuffer, sizeof(i2cFrame)); 
	assert(handlerQueue_i2cFrameTransmittBuffer);
	
	//tworzy obiekt obsługujący NVS flash radio
	printf("NVS storage init\n");
	//NVS * storage = NULL;
	assert(pSTORAGE = new NVS(NVS_RADIO_CONFIG_NAMESPACE));
	//storage->CAUTION_NVS_ereaseAndInit(NVS_EREASE_COUNTDOWN_TIME);
	
}




/*---------------------------------------------------------------
* Lokalna funkcja potrzebna na etapie programownaia. Poprzez tę 
* funkcję sprawdzam czy do taska parsującego dane z klawiatury
* wpływają popraswen dane i dane sa poprawenie przetważane
* Parameters:
* keyboardUnion DataToParse - unia zawierająca dane na temat stanu
*				klawiszy
* Returns:
* NONE
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
		//if ((DataToParse.encoderValue.value == ENCODER_PULSED_PER_DETANT) || (DataToParse.encoderValue.value == -ENCODER_PULSED_PER_DETANT))
		printf("ENC %c: %d\n", DataToParse.array[0], DataToParse.encoderValue.value);
		break;
	}
	
}






/*---------------------------------------------------------------
* Funkcja sprawdza czy wartości znajdujące się w kolejce klawiatury
* są poprawne (*głównie chodszi tutaj o wartości z enkoderów) i
* jeśli sa poprawne to przekazuje je do kolejki i2cFrame (kolejka
* z której dane są wysyłane do stm32).
* Parameters:
* void *parameters - uchwyt do kolejki zawiwerasjącej dane
*				z przerwania klawiatury
* Returns:
* NONE
*---------------------------------------------------------------*/ 
void keyboardQueueParametersParser(void *parameters)
{
	bool isQueueFeedRequirted;// = pdFALSE;
	BaseType_t queueFeedRetVal;
	keyboardUnion keyboardDataToParse;
	keyboardDataToParse.array[0] = 0;
	keyboardDataToParse.array[1] = 0;
	i2cFrame keyboardDataToI2cTransmittQueue;

	QueueHandle_t handlerParameterAsKeyboard = (QueueHandle_t) parameters;
		
	keyboardDataToI2cTransmittQueue.frameSize = sizeof(keyboardDataToI2cTransmittQueue.frameSize) + sizeof(keyboardDataToI2cTransmittQueue.commandGroup) + sizeof(keyboardDataToI2cTransmittQueue.commandData);
	for (;;)
	{
		isQueueFeedRequirted  = pdFALSE;
		//sprawdza czy w kolejce danych z klawiatury znajdują sie jakiekolwiek dane do parsowania 
		if (xQueueReceive(handlerParameterAsKeyboard, &keyboardDataToParse, portMAX_DELAY))
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
				if (xQueueSend(handlerQueue_i2cFrameTransmittBuffer, &keyboardDataToI2cTransmittQueue, pdMS_TO_TICKS(700)) == pdFAIL) //jeżeli bufor kolejki danych do nadania po i2c jest zapchany (brak komunikacji z stm32) to istnieje możliwość awaryjnego wyłączenia radio lub resetu NVS
				{
					printf("Queue feeding error\n");
					if (keyboardDataToParse.kbrdValue.input == HMI_INPUT_BUTTON)	//long press release button event
					{
						switch (keyboardDataToParse.kbrdValue.value)
						{
						case (LONG_PRESS_BIT_MASK | (0xff & ~(1 << 0))): //BUT0	presses
							printf("Emergency hardware restart\n");
							break;
						case (LONG_PRESS_BIT_MASK | (0xff & ~(1 << 0) & ~(1 << 6))): //BUT0+BUT6 presses
							printf("Emergency NVS reset\n");
							pSTORAGE->CAUTION_NVS_ereaseAndInit(10);
							printf("Please restart device\n");
							break;
						}	 
					}
				}
				//----------------------------------------------------------------------//
				//poniższa funkcja jkest tylko do celów debugowania poprawności programu
				keyboardQueueParametersParserPrintf(keyboardDataToParse);
				//i2cFrame daneZKolejki;
				//xQueueReceive(handlerQueue_i2cFrameTransmittBuffer, &daneZKolejki, portMAX_DELAY);
				//keyboardQueueParametersParserPrintf(daneZKolejki.commandData.keyboardData);	
				//poniższa funkcja jkest tylko do celów debugowania poprawności programu
				//----------------------------------------------------------------------//
			}		
		}
	}
}

/*---------------------------------------------------------------
* Funkcja statyczna (lokalna) której zadaniem jest porównanie
* wartości dwóch kolorów diód led (== nie odnosi sie do tego typu
* danych).
* Parameters:
* const struct ws2812Color *color1 - pierwszy kolor do porównania
* const struct ws2812Color *color2 - drugi kolor do porównania
* Returns:
* bool - TRUE jeżeli kolory sa sobie równe, FALSE jeżeli kolory
*				nie są sobie równe.	  
*---------------------------------------------------------------*/ 
static 	bool areEqual(const struct ws2812Color *color1, const struct ws2812Color *color2) {
	return (bool)(memcmp(color1, color2, sizeof(struct ws2812Color)) == 0) ;
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
	//NVS * pStorage;
	StepperOpto * pMotor=NULL;
	assert(pMotor = new StepperOpto());
	
	//pStorage->get_blob(NVS_KEY_BLOB_MotorParameters, pMotor->)

	//MotorParameters test;
	//test.beginOffest = 1;
	//test.currentPosition = 2;
	//test.endOffset = 50;
	//test.maxPosition = 5000;
	//pMotor->setValue_motorParameters(&test);
	for (;;)
	{
		pMotor->measureSliderRange();
		//vTaskPrioritySet(NULL, tskIDLE_PRIORITY);
		vTaskSuspend(NULL);

	}
}

