#include "tasksFunctions.h"
#include "i2c_engine/i2c_engine.h"



//static QueueHandle_t handlerQueue_i2cSlaveSetBuffer_keyboard;		//wskaźnik do kolejki przechowującej dane jakie mają być wysłane po i2c z ESP32 do STM32
static SemaphoreHandle_t handlerMutex_ledDisplay_Backlight;	//mutex synchronizujący wyświetlanie komunikatów ledów (source, equaliser, error) i podświetlenia (backlight);

static hmiDisplay displayLedsColors;	//struktura zawierająca informacje na temat wszystkich stanów (kolorów) diód w wyświetlaczu
static NVS* pSTORAGE;					//obiekt zapisujący i czytający dane z NCS ESP32
static LEDS_BACKLIGHT *pLedDisplay;		//obiekt sterujący pracą ledów (diody i backlioght)
static 	StepperOptoPowerOFF * pMotor;	//obiekt sterujący pracą silnika krokowego, jego krańcówej i power off radia
static i2cEngin_slave *p_i2cSlave;		//obiekt sterujący komunikacją z stm32 po szynie i2c


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
	
		
	
	//tworzenie obiektu i2cSlave komunikującewgo się STM32 po szynie i2c
	printf("I2C slave bus init\n");
	p_i2cSlave = nullptr;
	assert(p_i2cSlave = 	new i2cEngin_slave(I2C_SLAVE_PORT, I2C_SLAVE_PIN_SDA, I2C_SLAVE_PIN_SCL, I2C_SLAVE_ADDRESS, I2C_ADDR_BIT_LEN_7, I2C_SLAVE_INTERRUP_REQUEST_PIN));
	
	
	//assert(p_i2cSlave);
	
	//tworzenie kolejki bufora nadawczego i2c
	//handlerQueue_i2cSlaveSetBuffer_keyboard = NULL;
	//configASSERT(handlerQueue_i2cSlaveSetBuffer_keyboard = xQueueCreate(I2C_SLAVESET_BUFFER_KEYBOARD_LEN, sizeof(i2cFrame_keyboardFrame))); 
	//configASSERT(handlerQueue_i2cSlaveSetBuffer_keyboard);
	//configASSERT(p_i2cSlave->addQueueToSet(handlerQueue_i2cSlaveSetBuffer_keyboard));

	
	//tworzenie semafora dla punktu aktualizacji zmiennych przechowujących dane o ledach
	handlerMutex_ledDisplay_Backlight = NULL; //czyści wskaźnik mutex'u dla podświetlenia	i diód sygnalizacyjnych, bo kilka tasków bedzi ekorzystać z linii komunikacyjnej WS2812 		
	configASSERT(handlerMutex_ledDisplay_Backlight = xSemaphoreCreateBinary()); //tworzy mutex dla podświetlenia
	xSemaphoreGive(handlerMutex_ledDisplay_Backlight);	
	
	//tworzy obiekt obsługujący NVS flash radio
	printf("NVS storage init\n");
	assert(pSTORAGE = new NVS(NVS_RADIO_CONFIG_NAMESPACE));
	//storage->CAUTION_NVS_ereaseAndInit(NVS_EREASE_COUNTDOWN_TIME);
	
	//tworzy obiekt obsługujący ledy sygnalizacyjne i podświetlenia
	printf("Backlight and display leds init\n");
	assert(pLedDisplay = new LEDS_BACKLIGHT(LED_DISPLAY_GPIO, LED_DISPLAY_LEDS_QUANTITY, LED_PIXEL_FORMAT_GRB, LED_MODEL_WS2812));
	pLedDisplay->ledStripClearAll();
	
	//tworzy obiekt obsługujący szyne i2c master komunikującą się z  MCP23008
	printf("I2C master bus init\n");
	i2cEngin_master *p_i2cMaster = new i2cEngin_master(I2C_MASTER_PORT, I2C_MASTER_PIN_SDA, I2C_MASTER_PIN_SCL);
	assert(p_i2cMaster);
	// sprawdza czy MCP23008 jest dostępny na szynie i2c
	assert(!p_i2cMaster->i2cPing(MCP23008_I2C_DEVICE_OPCODE));
	//tworzy obiekt obsługujący transmisję danych z MCP23008
	printf("MCP23008 on I2C master bus init\n");
	MCP23008* p_MCP23008 = new MCP23008(MCP23008_I2C_DEVICE_OPCODE, p_i2cMaster, I2C_MASTER_SPEED);
	assert(p_MCP23008);
	
	
	
	//tworzy obiekt obsługujący silnik krokowy, krańsówki i power off radia
	printf("Stepper motor and powerOFF gpio init\n");
	assert(pMotor = new StepperOptoPowerOFF(p_MCP23008));
}


/*---------------------------------------------------------------
* Lokalna funkcja odpowiadająca za wyłaczenia zasilania hardware'u
* wykonane przez ESP32
* Parameters:
* NONE
* Returns:
* NONE
*---------------------------------------------------------------*/ 
static void hardwarePowerOFF(void)
{
	pMotor->radioPowerOFF();
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
* Lokalna funkcja, ktora sprawdza czy dane otrzymane z obsługi
* przerwania klawiatury są poprawne (chodzi głównie o dane z
* enkodera, gdzie jeśli wartość nie jest równa +detand lub -detand
* to nastąpił błąd debounceowania enkodera i uszkodzonych danych
* nie warto wysyłać do MCU mastera).
* Parameters:
* keyboardUnion keyboardDataToCheck - unia zawierająca dane z
*		klawiatury jakie należy sprawdziź pod kątem ich poprawności
* Returns:
* bool - pdTrue jeśli dane są poprawne, pdFalse jeśli dane nie są
*		poprawne.
*---------------------------------------------------------------*/ 

static bool keyboardQueueParameters_isComunicationWithI2CMasterRequired(keyboardUnion keyboardDataToCheck)
{
	//sprawdza czy dane do parsowania pochodzą z przyciskow (short press/ long on press/ long press release)
	if ((keyboardDataToCheck.array[0] == HMI_INPUT_BUTTON) || (keyboardDataToCheck.array[0] == HMI_INPUT_BUTTON_LONG_AND_PRESSED))
	{
		return pdTRUE; 
	}
			
	//sprawdza czy dane do parsowania pochodzą z enkoderów
	if ((keyboardDataToCheck.array[0] == HMI_INPUT_VOLUME) || (keyboardDataToCheck.array[0] == HMI_INPUT_EQUALISER))
	{
		//sprawdza czy dane z enkoderów sa poprawne, tzn. czy są równie +/- encoder detant
		if ((keyboardDataToCheck.encoderValue.value == ENCODER_PULSED_PER_DETANT) || (keyboardDataToCheck.encoderValue.value == -ENCODER_PULSED_PER_DETANT)) {
			return pdTRUE;
		}
	}
	return pdFALSE;
}


static void keyboardQueueParameters_isEmergencyResetRequired(keyboardUnion keyboardDataToCheck)
{
	printf("Queue feeding error\n");
	if (keyboardDataToCheck.kbrdValue.input == HMI_INPUT_BUTTON)	//long press release button event
	{
		switch (keyboardDataToCheck.kbrdValue.value)
		{
		case (LONG_PRESS_BIT_MASK | (0xff & ~(1 << 0))):			//BUT0	presses	= emergency reset
			printf("Emergency hardware restart\n");
			hardwarePowerOFF();
			break;
		case (LONG_PRESS_BIT_MASK | (0xff & ~(1 << 0) & ~(1 << 6))): //BUT0+BUT6 presses = NVS reset + emergency reset
			printf("Emergency NVS reset\n");
			pSTORAGE->CAUTION_NVS_ereaseAndInit(10);
			printf("Please restart device\n");
			break;
		}
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
	QueueHandle_t handlerParameterAsKeyboard = (QueueHandle_t) parameters; //uchwyt który przekazuje fo taska parametr z funkcji MAIN (handlerQueue_MainKeyboard
	//bool isComunicationWithI2CMasterRequired;// = pdFALSE;
	//BaseType_t queueFeedRetVal;
	keyboardUnion keyboardDataToParse;		//bufor do którego będa kopiowane dane z kolejki klawiatury, i który bedzi eporzetwarzany w pętli for
	//keyboardDataToParse.array[0] = 0;		//zerowanei bufora
	//keyboardDataToParse.array[1] = 0;		//zerowanei bufora
	i2cFrame_keyboardFrame kbrdDataToI2CSlaveTransmittQueueTemoraryVariable;
		
	//keyboardDataToI2cTransmittQueue.frameSize = sizeof(keyboardDataToI2cTransmittQueue.frameSize) + sizeof(keyboardDataToI2cTransmittQueue.commandGroup) + sizeof(keyboardDataToI2cTransmittQueue.commandData);
	kbrdDataToI2CSlaveTransmittQueueTemoraryVariable.i2cframeCommandHeader.dataSize = sizeof(keyboardUnion/*keyboardDataToParse*/);
	for (;;)
	{
		if (xQueueReceive(handlerParameterAsKeyboard, &keyboardDataToParse, portMAX_DELAY))
		{
			if (keyboardQueueParameters_isComunicationWithI2CMasterRequired(keyboardDataToParse))		//sprawdza czy dane z przerwania klawiatury są poprawne, jeśli tak to przystępuje do ich przesłania do kolejki nadawczej I2C slave
			{
				memcpy(&kbrdDataToI2CSlaveTransmittQueueTemoraryVariable.keyboardData, &keyboardDataToParse, sizeof(keyboardUnion));
				
				kbrdDataToI2CSlaveTransmittQueueTemoraryVariable.i2cframeCommandHeader.crcSum = (uint8_t) calculate_checksum(&kbrdDataToI2CSlaveTransmittQueueTemoraryVariable/*&keyboardDataToParse*/, sizeof(i2cFrame_keyboardFrame/*keyboardDataToParse*/));
				//memcpy(&kbrdDataToI2CSlaveTransmittQueueTemoraryVariable.keyboardData, &keyboardDataToParse, sizeof(keyboardUnion));
				//if (xQueueSend(handlerQueue_i2cSlaveSetBuffer_keyboard, &kbrdDataToI2CSlaveTransmittQueueTemoraryVariable, pdMS_TO_TICKS(700)) == pdFAIL) //jeżeli bufor kolejki danych do nadania po i2c jest zapchany (brak komunikacji z stm32) to istnieje możliwość awaryjnego wyłączenia radio lub resetu NVS
				//if (p_i2cSlave->transmitQueueSend(&kbrdDataToI2CSlaveTransmittQueueTemoraryVariable, sizeof(i2cFrame_keyboardFrame))!=ESP_OK)
				
				if (p_i2cSlave->pQueueObject->QueueSend(&kbrdDataToI2CSlaveTransmittQueueTemoraryVariable, sizeof(i2cFrame_keyboardFrame))!=ESP_OK)
				
				{
					keyboardQueueParameters_isEmergencyResetRequired(kbrdDataToI2CSlaveTransmittQueueTemoraryVariable.keyboardData/*keyboardDataToParse*/);
				}
				
				//----------------------------------------------------------------------//
				//poniższa funkcja jest tylko do celów debugowania poprawności programu
				keyboardQueueParametersParserPrintf(keyboardDataToParse);
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


void humanMahineBacklightLeds(void *lnothing)
{
	init_BacklightColors(pLedDisplay);
		
	for (;;)
	{
		vTaskSuspend(NULL);
		xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		pLedDisplay->ledStripSet_backlightLeds((uint32_t) displayLedsColors.backlightLeds.secondary.red,
			(uint32_t) displayLedsColors.backlightLeds.secondary.green,
			(uint32_t) displayLedsColors.backlightLeds.secondary.blue);
		pLedDisplay->ledStripRefresh();
		xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
		vTaskDelay(pdMS_TO_TICKS(150));
		
		xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		pLedDisplay->ledStripSet_backlightLeds((uint32_t) displayLedsColors.backlightLeds.primary.red,
			(uint32_t) displayLedsColors.backlightLeds.primary.green,
			(uint32_t) displayLedsColors.backlightLeds.primary.blue);
		pLedDisplay->ledStripRefresh();
		xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
		//vTaskSuspend(NULL);	
	}
}
	
	
void humanMahineDisplayLeds(void *nothiong)
{
	uint8_t ledsPrimarySecondary = 0;
	for (;;)
	{
		xSemaphoreTake(handlerMutex_ledDisplay_Backlight, portMAX_DELAY);
		
		if (areEqual(&displayLedsColors.equaliserLed.primary, &displayLedsColors.equaliserLed.secondary)&&
					areEqual(&displayLedsColors.errorLed.primary, &displayLedsColors.errorLed.secondary)&&
					areEqual(&displayLedsColors.sourceLed.primary, &displayLedsColors.sourceLed.secondary))		//if all leds primary == secondary
		{
											
			pLedDisplay -> ledStripSet_equaliserLed((uint32_t) displayLedsColors.equaliserLed.primary.red,
														(uint32_t) displayLedsColors.equaliserLed.primary.green,
														(uint32_t) displayLedsColors.equaliserLed.primary.blue);
			pLedDisplay->ledStripSet_errorLed((uint32_t) displayLedsColors.errorLed.primary.red,
														(uint32_t) displayLedsColors.errorLed.primary.green,
														(uint32_t) displayLedsColors.errorLed.primary.blue);
			pLedDisplay->ledStripSet_sourceLed((uint32_t) displayLedsColors.sourceLed.primary.red,
														(uint32_t) displayLedsColors.sourceLed.primary.green,
														(uint32_t) displayLedsColors.sourceLed.primary.blue);
			pLedDisplay->ledStripRefresh();

			
			xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
			vTaskSuspend(NULL);																					//all primary == secondary so suspend this task
		}
		else		
		{																										//if at least one led primary != secondary
			
			switch (ledsPrimarySecondary)
			{
			case 0:
				pLedDisplay->ledStripSet_equaliserLed((uint32_t) displayLedsColors.equaliserLed.primary.red,
					(uint32_t) displayLedsColors.equaliserLed.primary.green,
					(uint32_t) displayLedsColors.equaliserLed.primary.blue);
				pLedDisplay->ledStripSet_errorLed((uint32_t) displayLedsColors.errorLed.primary.red,
					(uint32_t) displayLedsColors.errorLed.primary.green,
					(uint32_t) displayLedsColors.errorLed.primary.blue);
				pLedDisplay->ledStripSet_sourceLed((uint32_t) displayLedsColors.sourceLed.primary.red,
					(uint32_t) displayLedsColors.sourceLed.primary.green,
					(uint32_t) displayLedsColors.sourceLed.primary.blue);
				ledsPrimarySecondary = 1; //next time leds flip to secondary color
				break;
			case 1:
				pLedDisplay->ledStripSet_equaliserLed((uint32_t) displayLedsColors.equaliserLed.secondary.red,
					(uint32_t) displayLedsColors.equaliserLed.secondary.green,
					(uint32_t) displayLedsColors.equaliserLed.secondary.blue);
				pLedDisplay->ledStripSet_errorLed((uint32_t) displayLedsColors.errorLed.secondary.red,
					(uint32_t) displayLedsColors.errorLed.secondary.green,
					(uint32_t) displayLedsColors.errorLed.secondary.blue);
				pLedDisplay->ledStripSet_sourceLed((uint32_t) displayLedsColors.sourceLed.secondary.red,
					(uint32_t) displayLedsColors.sourceLed.secondary.green,
					(uint32_t) displayLedsColors.sourceLed.secondary.blue);
				ledsPrimarySecondary = 0; //next time leds flip to primary color
				break;
				
			}
			
			pLedDisplay->ledStripRefresh();
			xSemaphoreGive(handlerMutex_ledDisplay_Backlight);
			vTaskDelay(pdMS_TO_TICKS(LED_DISPLAY_BLINK_TIME));													//wait 1s till change color
		}
	}
}


void stepperMotor(void *TaskParameters)
{
	//NVS * pStorage;
//	StepperOpto * pMotor=NULL;
//	assert(pMotor = new StepperOpto());
	
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

void i2cSlaveTransmit(void *nothing)
{
	for (;;)
	{
		p_i2cSlave->slaveTransmit();		
	}	
}