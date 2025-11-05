#include "keyboard.h"

static TaskHandle_t handlerTask_keyboardLongPressOnPressQueueFeeder; //uchwyt do taska, który po przekroczenia minimalnego czasu long press i do
																		//czasu puszczenia przysicka informuje (wysyła do kolejki dane) o przytrzymaniu prtzycisku

/*---------------------------------------------------------------
 * Zadaniem funkcji jest cyliczne wysyłanie do kolejki klawiatury
 * "queueHandler_keyboard" informacji na temat kombinajci klawiszy,
 * jakie były wciśnięte w momencie upłynięcia czasu "long press".
 * Funkcja przestaje wysyłać tę informację w momencie puszczenia
 * klawiszy. Wtedy przerwanie zawiesza działanie tej funkcji.
 * Parameters:
 * void *object	- funkcja przyjmuje wskaźnik do obiektu klasy KEYBOARD,
 *				  ale poniewarz jest ona wywoływana jako task (zadanie
 *				  freertos) przyjmuje go jako void*, a dopiero w ciele
 *				  funkcji następuje rzutowanie na obiekt klasy.
 * Returns:
 * NONE
 *---------------------------------------------------------------*/ 
/*static*/ void keyboardLongPressOnPressQueueFeeder(void *object)
{
	KEYBOARD* instance = static_cast<KEYBOARD*>(object);
	keyboardUnion valueToQueue;
	valueToQueue.kbrdValue.input = HMI_INPUT_BUTTON_LONG_AND_PRESSED;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	vTaskSuspend(NULL);
	while (true)
	{
		valueToQueue.kbrdValue.value = instance->buttonsState.latchedState;
		//xQueueSend(instance->gpioInterruptCallback.queueHandler_keyboard, valueToQueue.array, portMAX_DELAY);
		xQueueSendFromISR(instance->gpioInterruptCallback.queueHandler_keyboard, valueToQueue.array, &xHigherPriorityTaskWoken);
		vTaskDelay(pdMS_TO_TICKS(ON_PRESS_QUEUE_FEEDER_DELEY_TIME_MS));
	}
}




/*---------------------------------------------------------------
 * Funckja obsługująca przerwanie od zmiany stanu na GPIO klawiatury.
 * Zadaniem funkcji jest zresetowanie wszystkich zmiennycxh do stanu
 * poczatkowego, wyłaczenie przerwań od zmiany stanów na GPIO (ze
 * względu na drgania styków) oraz właczenie timera, który w ramach
 * przerwań dokonuje filtracji sygnałów ze względu na drgania styków
 * oraz całej obsługi klawiszy.
 * Parameters:
 * void* arg1 - wskaźnik void do struktury gpioInterruptCallbackStruct
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
static void IRAM_ATTR button_isr_handler(void* arg1)
{
	KEYBOARD::gpio_intr_disableAll(); //disable GPIOs interrupts to avoid GPIO interrupt nesting

	//new input interrupt starts reset buttonsState VolEncState EquEncState debounce time
	gpioInterruptCallbackStruct* _gpioInterruptCallback = (gpioInterruptCallbackStruct*)arg1;
	_gpioInterruptCallback->whichInput = HMI_INPUT_BUTTON; //_gpioInterruptCallback->case_buttonInput;
	_gpioInterruptCallback->debounceTime = 0;
	_gpioInterruptCallback->exitTimerInterrupt = pdTRUE;
	_gpioInterruptCallback->pbuttonsState->lastState = KEYBOARD::buttonsGetLevel();
	_gpioInterruptCallback->pbuttonsState->latchedState = 0; //////
	
	
	if (gptimer_start((gptimer_handle_t)_gpioInterruptCallback->pgptimer) != ESP_OK) {
		//starts debounce timer
		KEYBOARD::gpio_intr_enableAll();
	}
//	ESP_ERROR_CHECK(gptimer_start((gptimer_handle_t)_gpioInterruptCallback->pgptimer)); //starts debounce timer
}	


/*---------------------------------------------------------------
 * Funckja obsługująca przerwanie od zmiany stanu na GPIO enkoder
 * inkrementacyjnego głośności.
 * Zadaniem funkcji jest zresetowanie wszystkich zmiennych do stanu
 * poczatkowego, wyłaczenie przerwań od zmiany stanów na GPIO (ze
 * względu na drgania styków) oraz właczenie timera, który w ramach
 * przerwań dokonuje filtracji sygnałów ze względu na drgania styków
 * oraz całej obsługi klawiszy.
 * Parameters:
 * void* arg1 - wskaźnik void do struktury gpioInterruptCallbackStruct
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
static void IRAM_ATTR volume_isr_handler(void* arg1)
{
	KEYBOARD::gpio_intr_disableAll(); //disable GPIOs interrupts to avoid GPIO interrupt nesting

	//new input interrupt starts reset buttonsState VolEncState EquEncState debounce time
	gpioInterruptCallbackStruct* _gpioInterruptCallback = (gpioInterruptCallbackStruct*)arg1;
	_gpioInterruptCallback->whichInput = HMI_INPUT_VOLUME;//_gpioInterruptCallback->case_volumeInput;
	_gpioInterruptCallback->debounceTime = 0;
	_gpioInterruptCallback->exitTimerInterrupt = pdTRUE;
	
	
	
	
	_gpioInterruptCallback->pVolEncState->lastState = KEYBOARD::encoderGetLevel(VolRot_A, VolRot_B);
	_gpioInterruptCallback->pVolEncState->latchedQEM = 0;
	_gpioInterruptCallback->pVolEncState->pulseIncrement = 0;
	
	
	
	if (gptimer_start((gptimer_handle_t)_gpioInterruptCallback->pgptimer) != ESP_OK) {
		//starts debounce timer
		KEYBOARD::gpio_intr_enableAll();
	}
	//ESP_ERROR_CHECK(gptimer_start((gptimer_handle_t)_gpioInterruptCallback->pgptimer)); //starts debounce timer
}



/*---------------------------------------------------------------
 * Funckja obsługująca przerwanie od zmiany stanu na GPIO enkoder
 * inkrementacyjnego equalisera.
 * Zadaniem funkcji jest zresetowanie wszystkich zmiennych do stanu
 * poczatkowego, wyłaczenie przerwań od zmiany stanów na GPIO (ze
 * względu na drgania styków) oraz właczenie timera, który w ramach
 * przerwań dokonuje filtracji sygnałów ze względu na drgania styków
 * oraz całej obsługi klawiszy.
 * Parameters:
 * void* arg1 - wskaźnik void do struktury gpioInterruptCallbackStruct
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
static void IRAM_ATTR equaliser_isr_handler(void* arg1)
{
	KEYBOARD::gpio_intr_disableAll(); //disable GPIOs interrupts to avoid GPIO interrupt nesting

	//new input interrupt starts reset buttonsState VolEncState EquEncState debounce time
	gpioInterruptCallbackStruct* _gpioInterruptCallback = (gpioInterruptCallbackStruct*)arg1;
	_gpioInterruptCallback->whichInput = HMI_INPUT_EQUALISER;//_gpioInterruptCallback->case_equaliserInput;
	_gpioInterruptCallback->debounceTime = 0;
	_gpioInterruptCallback->exitTimerInterrupt = pdTRUE;
	
	_gpioInterruptCallback->pEquEncState->lastState = KEYBOARD::encoderGetLevel(EquRot_A, EquRot_B);
	_gpioInterruptCallback->pEquEncState->latchedQEM = 0;
	_gpioInterruptCallback->pEquEncState->pulseIncrement = 0;

	
	if(gptimer_start((gptimer_handle_t)_gpioInterruptCallback->pgptimer) != ESP_OK) {		  //starts debounce timer
		KEYBOARD::gpio_intr_enableAll();
	}
	//ESP_ERROR_CHECK(gptimer_start((gptimer_handle_t)_gpioInterruptCallback->pgptimer)); //starts debounce timer
}

/*---------------------------------------------------------------
 * Funckja obsługująca przerwanie od timera, kórego zadaniuem jest
 * odfiltrowanie drgania styków oraz detekcja sygnałów sterujących
 * radiem , jakie zostały użyte.
 * Funkcja wywoływana może być przez:
 * - button_isr_handler,
 * - volume_isr_handler,
 * -equaliser_isr_handler.
 * W zależnościi od funkcji przez którą debounceAndGpiosCheckCallback
 * została uruchomiona następuje obsługa stosownych interfejsów
 * wejściowych, odpowiednio:
 * - kalwiszy,
 * - enkodera głośności,
 * - enkodera equalizera.
 * Ostatecznie funkcja poprzez mechanizm kolejek przekazuje do XXXXXXXXXXXXXXXXXXXXXXXXXX
 * informację dotyczącą użyutych interfejsów wejściowych (klawiatyra
 * enkodery).
 * Parameters:
 * void* arg1 - wskaźnik void do struktury gpioInterruptCallbackStruct
 * Returns:
 * NONE
 *--------------------------------------------------------------*/
static bool debounceAndGpiosCheckCallback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
	//copy adress of alarm interrupt callback function parameters struct
	gpioInterruptCallbackStruct* _gpioInterruptCallback = (gpioInterruptCallbackStruct*)user_ctx;
	gpio_num_t port_A, port_B;
	encoderState* pEncoderActive;

	/*if (_gpioInterruptCallback->whichInput == HMI_INPUT_VOLUME)//'V')
	{
		port_A = VolRot_A;
		port_B = VolRot_B;
		pEncoderActive = _gpioInterruptCallback->pVolEncState;
	}
	else if(_gpioInterruptCallback->whichInput == HMI_INPUT_EQUALISER)//'E')
	{
		port_A = EquRot_A;
		port_B = EquRot_B;	
		pEncoderActive = _gpioInterruptCallback->pEquEncState;
	}*/
	
	switch (_gpioInterruptCallback->whichInput)
	{
		case HMI_INPUT_BUTTON:
			break;
		case HMI_INPUT_VOLUME://'V'
			port_A = VolRot_A;
			port_B = VolRot_B;
			pEncoderActive = _gpioInterruptCallback->pVolEncState;
			break;
		case HMI_INPUT_EQUALISER://'E')
			port_A = EquRot_A;
			port_B = EquRot_B;	
			pEncoderActive = _gpioInterruptCallback->pEquEncState;
			break;	
	}
	
	
	//char keyboardExitValue[HMI_INPUT_COMMAND_LEN];
		
	switch (_gpioInterruptCallback->whichInput)
	{
	case HMI_INPUT_BUTTON:
		_gpioInterruptCallback->exitTimerInterrupt = pdFALSE;
	
		//get newState for all input devices (buttons and encoders)
		_gpioInterruptCallback->pbuttonsState->newState = KEYBOARD::buttonsGetLevel();
	
		//1. Debounce section BEGINNING
		//==========================
		//1.1 BUTTONS - Debounce section BEGINNING
		//------------------------------------
		if (_gpioInterruptCallback->pbuttonsState->lastState == _gpioInterruptCallback->pbuttonsState->newState)
		{
			
			if (_gpioInterruptCallback->debounceTime < GPIO_LONG_PRESS) {
				_gpioInterruptCallback->debounceTime++;
				//_gpioInterruptCallback->debounceTime = GPIO_LONG_PRESS;
			}
		}
		else
		{
			_gpioInterruptCallback->pbuttonsState->lastState = _gpioInterruptCallback->pbuttonsState->newState;
			_gpioInterruptCallback->debounceTime = 0;
		}
		//------------------------------
		//1.1 BUTTONS - Debounce section END	
		//========================================
		//1. Debounce section END	
	
		//2. Contacts in idle (DEBOUNCED) state - BEGGINING
		//**********************************************
	
		//2.1 Check ending conditions for stop the timer - BEGINNING
		//======================================================
	
		//2.1.1 button(s) stop condition - BEGGINING
		//------------------------------------
		//Thise condition are: all buttons released and in idle (debounced) state 	
		if ((_gpioInterruptCallback->debounceTime >= GPIO_DEBOUNCE_EQUAL) && (_gpioInterruptCallback->pbuttonsState->newState == 0b01111111))
		{
			_gpioInterruptCallback->exitTimerInterrupt = pdTRUE;
		}
		//------------------------------------
		//2.1.1 button(s) stop condition - END
		//======================================================
		//2.1 Check ending conditions for stop the timer - END
	
	

		//2.2 Section for checking the latched value of buttons and encoders - BEGGINING
		//==========================================================================
		//2.2.1 Section for checking the latched value of buttons- BEGGINING
		//------------------------------------------------------------
		if (_gpioInterruptCallback->exitTimerInterrupt != pdTRUE)
		{	
			if (_gpioInterruptCallback->debounceTime >= GPIO_DEBOUNCE_EQUAL)
			{
				if (!(_gpioInterruptCallback->pbuttonsState->latchedState>>7))		//jeśli nastąpił LONG PRESS to więcej nie powinno zatrzaskiwać klawiszy (jest to warunek, aby dobrze działało odczytywanie długiwgo wciśnięcia w lub więcej klawiszy na raz.
				{
					_gpioInterruptCallback->pbuttonsState->latchedState = _gpioInterruptCallback->pbuttonsState->newState;
					if (_gpioInterruptCallback->debounceTime /*>*/ == GPIO_LONG_PRESS)
					{
						_gpioInterruptCallback->pbuttonsState->latchedState = _gpioInterruptCallback->pbuttonsState->latchedState | LONG_PRESS_BIT_MASK; // set "1" in LONG_PRESS_BIT_MASK aka 0b10000000 means long button press
						xTaskResumeFromISR(_gpioInterruptCallback->taskHandler_onPeriodLongButtonPressNotification); //Uruchamnia zadanie odpowiadające za powiadomienie o długim naciśnięciu przycisku(ów)
						xTaskResumeFromISR/*vTaskResume*/(handlerTask_keyboardLongPressOnPressQueueFeeder);
					}
				}
			}		
		}
		//------------------------------------------------------------	
		//2.2.1 Section for checking the latched value of buttons- END
		//==========================================================================
		//2.2 Section for checking the latched value of buttons and encoders - ENDING
		//**********************************************
		//2. Contacts in idle (DEBOUNCED) state - END
		break;
	case HMI_INPUT_EQUALISER:
	case HMI_INPUT_VOLUME:

		_gpioInterruptCallback->exitTimerInterrupt = pdFALSE;
	
		//get newState for all input devices (buttons and encoders)
		pEncoderActive->newState = KEYBOARD::encoderGetLevel(port_A, port_B);
	
		//1. Debounce section BEGINNING
		//==========================
		//1.2 VOLUME ENCODER - Debounce section BEGINNING
		//-------------------------------------------
		if (pEncoderActive->lastState == pEncoderActive->newState)
		{
			_gpioInterruptCallback->debounceTime++;
		}
		else
		{
			pEncoderActive->lastState = pEncoderActive->newState;
			_gpioInterruptCallback->debounceTime = 0;
		}	
		//-------------------------------------
		//1.2 VOLUME ENCODER - Debounce section END
		//========================================
		//1. Debounce section END	
	
	
	
		//2. Contacts in idle (DEBOUNCED) state - BEGGINING
		//**********************************************
	
		//2.1 Check ending conditions for stop the timer - BEGINNING
		//======================================================	
	
		//2.1.2 volume encoder stop condition - BEGGINING
		//-----------------------------------------	
				
		if ((pEncoderActive->pulseIncrement == ENCODER_PULSED_PER_DETANT) || (pEncoderActive->newState == 0x11 /*DETANT1_GPIO_STATE*/) || (_gpioInterruptCallback->debounceTime >= ENCODER_notFULL_DETANT_TIMEOUT))
		{
//			if (pEncoderActive->pulseIncrement == ENCODER_PULSED_PER_DETANT)
//			{

//				#ifdef DEBUG_ENCODER_TABLE
//				pEncoderActive->table[pEncoderActive->i] = pEncoderActive->latchedQEM;
//				pEncoderActive->i++;
//				if (pEncoderActive->i == 100)
//					pEncoderActive->i = 0;			
//				#endif

//			}
			_gpioInterruptCallback->exitTimerInterrupt = pdTRUE;			
		}
		
		
		
		
		//-----------------------------------
		//2.1.2 volume encoder stop condition - END	
		//======================================================
		//2.1 Check ending conditions for stop the timer - END
	
	

		//2.2 Section for checking the latched value of buttons and encoders - BEGGINING
		//==========================================================================
		//2.2.2 Section for checking the latched value of Volume encoder- BEGGINING
		//------------------------------------------------------------	
		if (_gpioInterruptCallback->exitTimerInterrupt != pdTRUE)
		{
			pEncoderActive->newLatchedState = pEncoderActive->newState;
					
			if ((_gpioInterruptCallback->debounceTime >= GPIO_DEBOUNCE_EQUAL)&&(pEncoderActive->newLatchedState != pEncoderActive->lastLatchedState))
			{
				
				//QEM [OLD_states * 4 + NEW_states];
				pEncoderActive->latchedQEM += _gpioInterruptCallback->QEM[4*pEncoderActive->lastLatchedState +
																								pEncoderActive->newLatchedState];
				
				
				pEncoderActive->lastLatchedState = pEncoderActive->newLatchedState;
				pEncoderActive->pulseIncrement++;
				
			}
		}
		//------------------------------------------------------------		
		//2.2.2 Section for checking the latched value of Volume encoder- END	
		//==========================================================================
		//2.2 Section for checking the latched value of buttons and encoders - ENDING
	
		//**********************************************
		//2. Contacts in idle (DEBOUNCED) state - END
	
		break;
	}

	
	//3. Exiting button and encoders timer interrupt- BEGGINING
	//*********************************************************	
	
	
	if (_gpioInterruptCallback->exitTimerInterrupt == pdTRUE)
	{
		ESP_ERROR_CHECK(gptimer_stop((gptimer_handle_t)_gpioInterruptCallback->pgptimer)); //debounce finished encoders do not move, buttons released so timer should stop;

		
		_gpioInterruptCallback->keyboardExitValueHandler->array[0] = _gpioInterruptCallback->whichInput;
		switch (_gpioInterruptCallback->whichInput)
		{
		case HMI_INPUT_BUTTON:
			_gpioInterruptCallback->keyboardExitValueHandler->array[1] = (char) _gpioInterruptCallback->pbuttonsState->latchedState;
			vTaskSuspend(handlerTask_keyboardLongPressOnPressQueueFeeder);
			break;
		case HMI_INPUT_EQUALISER:
			_gpioInterruptCallback->keyboardExitValueHandler->array[1] = _gpioInterruptCallback->pEquEncState->latchedQEM;
			break;
		case HMI_INPUT_VOLUME:
			_gpioInterruptCallback->keyboardExitValueHandler->array[1] = _gpioInterruptCallback->pVolEncState->latchedQEM;
			break;
		}

		BaseType_t xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;

		xQueueSendFromISR(_gpioInterruptCallback->queueHandler_keyboard, _gpioInterruptCallback->keyboardExitValueHandler, &xHigherPriorityTaskWoken);

//for test purpouses only
//		char exitValue111[HMI_INPUT_COMMAND_LEN];
//		xQueueReceive(_gpioInterruptCallback->queueHandler_keyboard, &exitValue111, (TickType_t) 10);
//		uint8_t convert = (uint8_t) exitValue111[1] ;
		
		KEYBOARD::gpio_intr_enableAll();
	}
	//***************************************************
	//3. Exiting button and encoders timer interrupt- END

	
	return pdTRUE;
}

/*-----------------------------------------------------
 * Konstruktor klasy KEYBOARD obsługującej przyciski oraz enkodery inkrementacyjne.
 * Przy tym konstruktorze informacje dotyczące porów wejściowych, do którcyh podłączone są przyciski i enkopdery podawana jest poprzez:
 * #define BUT_0	GPIO_NUM_33						//DO NOT CHANGE
 * #define BUT_1	GPIO_NUM_36						//DO NOT CHANGE
 * #define BUT_2	GPIO_NUM_39						//DO NOT CHANGE
 * #define BUT_3	GPIO_NUM_34						//DO NOT CHANGE
 * #define BUT_4	GPIO_NUM_35						//DO NOT CHANGE
 * #define BUT_5	GPIO_NUM_32						//DO NOT CHANGE
 * #define BUT_6	GPIO_NUM_23						//DO NOT CHANGE
 * #define VolBut	BUT_0							//DO NOT CHANGE
 * #define EquBut	BUT_6							//DO NOT CHANGE
 * #define VolRot_A	GPIO_NUM_2						//DO NOT CHANGE
 * #define VolRot_B	GPIO_NUM_4						//DO NOT CHANGE
 * #define EquRot_A	GPIO_NUM_17						//DO NOT CHANGE
 * #define EquRot_B	GPIO_NUM_5						//DO NOT CHANGE
 * zdefiniowane w pliku: hwConfigFile.h.
 * Parameters:
 * QueueHandle_t queueHandler_Keyboard				- kolejka do której przerwanie
 *					od klatiatury pisze odczytane wartości klaiszy/ enkoderów, a z
 *					której task keyboardQueueParametersParser sprawdza czy wartości
 *					są poprawne
 * TaskHandle_t taskHandler_onPeriodLongButtonPressNotificationn - uchwyt do taska
 *					sprawdzającego czy w kolejce queueHandler_Keyboard znajdują się
 *					sprawidłowe dane 
 * Returns:
 * NONE
 *-----------------------------------------------------------*/
KEYBOARD::KEYBOARD(QueueHandle_t queueHandler_Keyboard, TaskHandle_t taskHandler_onPeriodLongButtonPressNotification)
{
	//ESP_LOGI(this->TAG, "Create input GPIOs debounce timer configuration");
	printf("%s Create input GPIOs debounce timer configuration\n", this->TAG);
	//configures the timer, which is responsible for checking whether the contacts vibrate (debounceing), and after stabilizing contacts, checks how long the button was pressed
	gpioDebounceTimer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
	gpioDebounceTimer_config.direction = GPTIMER_COUNT_UP;
	gpioDebounceTimer_config.resolution_hz = GPIO_DEBOUNCE_TIMER_RESOLUTION_HZ;
	
	
	gpioDebounceTimer_config.intr_priority = DEBOUNCE_TIMER_PRIORITY;
	gpioDebounceTimer_config.flags.intr_shared = pdTRUE;
	
	//ESP_ERROR_CHECK(
	assert(!gptimer_new_timer(&this->gpioDebounceTimer_config, &gptimer));
	
	//configure alarms (interrupts) for timer	
	gpioAlarm_config.alarm_count = 1;
	gpioAlarm_config.reload_count = 0;
	this->gpioAlarm_config.flags.auto_reload_on_alarm	= pdTRUE;
	ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &this->gpioAlarm_config));
	
	
	//starting task that feed queue about button long press	in progress
	printf("%s Starting 'keyboardLongPressOnPressQueueFeeder' task.\n", this->TAG);
	handlerTask_keyboardLongPressOnPressQueueFeeder	 = NULL;
	assert(xTaskCreate(keyboardLongPressOnPressQueueFeeder, "Long press feeder", 128 * 10, this, tskIDLE_PRIORITY, &handlerTask_keyboardLongPressOnPressQueueFeeder));
						 

	//interrupts (GPIOs and timer) callback function data passing structure
	this->gpioInterruptCallback.pbuttonsState = &buttonsState;
	this->gpioInterruptCallback.pEquEncState = &EquEncState;
	this->gpioInterruptCallback.pVolEncState = &VolEncState;
	this->gpioInterruptCallback.pgptimer = gptimer;
	this->gpioInterruptCallback.queueHandler_keyboard = queueHandler_Keyboard;
	this->gpioInterruptCallback.keyboardExitValueHandler = &this->onExitHMIValue;
	this->gpioInterruptCallback.taskHandler_onPeriodLongButtonPressNotification = taskHandler_onPeriodLongButtonPressNotification;
	this->gpioInterruptCallback.taskHandler_keyboardLongPressOnPressQueueFeeder = handlerTask_keyboardLongPressOnPressQueueFeeder;
	
	cbs.on_alarm = debounceAndGpiosCheckCallback;
	ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, &this->gpioInterruptCallback));
	ESP_ERROR_CHECK(gptimer_enable(gptimer));

	printf("%s Create button inputs GPIO configuration.\n", this->TAG);
	//ESP_LOGI(this->TAG, "Create button inputs GPIO configuration");
	this->iputButtonsConf.pin_bit_mask = ((1ULL << VolBut) | (1ULL << BUT_1) | (1ULL << BUT_2) | (1ULL << BUT_3) | (1ULL << BUT_4) | (1ULL << BUT_5) | (1ULL << EquBut));
	this->iputButtonsConf.mode = GPIO_MODE_INPUT;
	this->iputButtonsConf.pull_up_en = GPIO_PULLUP_DISABLE;
	this->iputButtonsConf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	this->iputButtonsConf.intr_type = GPIO_INTR_NEGEDGE;
	gpio_config(&this->iputButtonsConf);
	
	printf("%s Create rotary encoder inputs GPIO configuration.\n", this->TAG);
	//ESP_LOGI(this->TAG, "Create rotary encoder inputs GPIO configuration");
	this->iputRotaryEncoderConf.pin_bit_mask = ((1ULL << VolRot_A) | (1ULL << VolRot_B) | (1ULL << EquRot_A) | (1ULL << EquRot_B));
	this->iputRotaryEncoderConf.mode = GPIO_MODE_INPUT;
	this->iputRotaryEncoderConf.pull_up_en = GPIO_PULLUP_DISABLE;
	this->iputRotaryEncoderConf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	this->iputRotaryEncoderConf.intr_type = GPIO_INTR_ANYEDGE;
	gpio_config(&this->iputRotaryEncoderConf);
	 
	
	
	
//#error poprawić poniższy kod, bo nie jest już potrzebna sekwencja resetu, ale sekwencja detekcji wciśniętego przycisku już tak
	//jedyne miejsce w którym jest możliwe wysłanie do main MCU reset to default, to początek uruchamianai radio,
	//jeśli kombinacja klawiszy But1 + But2 + But3 zostanie naciśnięta to do kolejki klawiszy radio zostanie przesłana odpowienia wartość.
//	this->keyboardQueueSendResetToDefault(this->gpioInterruptCallback.queueHandler_keyboard);
	
	
	do
	{
		//ESP_LOGI(this->TAG, "Release Power ON/OFF to continue initializing \r\n ");
		printf("%s Release Power ON/OFF to continue initializing \r\n", this->TAG);
		//button = buttonsGetLevel();
	} while (!(buttonsGetLevel() & 0b00000001));		//czeka zanim przycisk Power On/Off zostanie zwolniony po właczeniu radio
		
	
	//gpio_install_isr_service(ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_LOWMED);	
	gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
	
	
	
	//nie wiem czy tutaj czy nie lepiej w interrupcie od GPIO???
	this->VolEncState.pulseIncrement = 0;
	this->EquEncState.pulseIncrement = 0;
	this->VolEncState.lastLatchedState = encoderGetLevel(VolRot_A, VolRot_B);
	this->EquEncState.lastLatchedState = encoderGetLevel(EquRot_A, EquRot_B);
	
	gpio_isr_handler_add(VolBut, button_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(BUT_1, button_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(BUT_2, button_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(BUT_3, button_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(BUT_4, button_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(BUT_5, button_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(EquBut, button_isr_handler, &this->gpioInterruptCallback);
	
	gpio_isr_handler_add(VolRot_A, volume_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(VolRot_B, volume_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(EquRot_A, equaliser_isr_handler, &this->gpioInterruptCallback);
	gpio_isr_handler_add(EquRot_B, equaliser_isr_handler, &this->gpioInterruptCallback);
	
	this->gpio_intr_enableAll();
	
}


/*----------------------------------------------
 * Funkcja wyłącza przerwania od wszystkich GPIO
 * klawiatury i enkoderów.
 *---------------------------------------------*/
void KEYBOARD::gpio_intr_disableAll()
{
	gpio_intr_disable(VolBut);
	gpio_intr_disable(BUT_1);
	gpio_intr_disable(BUT_2);
	gpio_intr_disable(BUT_3);
	gpio_intr_disable(BUT_4);
	gpio_intr_disable(BUT_5);
	gpio_intr_disable(EquBut);
	gpio_intr_disable(VolRot_A);
	gpio_intr_disable(VolRot_B);
	gpio_intr_disable(EquRot_A);
	gpio_intr_disable(EquRot_B);
}

/*-------------------------------------------------------
 * Funkcja sprawdza wartość na wejściach GPIO do kórych
 * podłączono enkoder inkrementacyjny.
 * Parameters:
 * - gpio_num_t EncoderPortA - GPIO do kórego podłączono
 *				pin A enkodera,
 * - gpio_num_t EncoderPortB - GPIO do kórego podłączono
 *				pin B enkodera.
 *	Returns:
 *	- uint8_t - odczytany stan na GPIO enkodera, jako
 *				0b000000xy, gszie x to stan piny A, y
 *				stan pinu B.
 *-------------------------------------------------------*/
uint8_t KEYBOARD::encoderGetLevel(gpio_num_t EncoderPortA, gpio_num_t EncoderPortB)
{
	return ((uint8_t)(gpio_get_level(EncoderPortA) << 0x1) | (uint8_t)(gpio_get_level(EncoderPortB) << 0x0));
}

/*-------------------------------------------------------
 * Funkcja sprawdza wartość na wejściach GPIO do kórych
 * podłączono przyciski.
 * Parameters:
 * NONE
 *	Returns:
 *	- uint8_t - odczytany stan na GPIO przycisków, jako
 *				0b0xxxxxxx, gdzie x to odczytany stan na
 *				poszczególnych GPIO przypisanych do
 *				przyciskow.
 *-------------------------------------------------------*/
uint8_t KEYBOARD::buttonsGetLevel()
{
	return ((((uint8_t) gpio_get_level(VolBut)) << 0) |
							(((uint8_t) gpio_get_level(BUT_1)) << 0x1) | 
							(((uint8_t) gpio_get_level(BUT_2)) << 0x2) | 
							(((uint8_t) gpio_get_level(BUT_3)) << 0x3) | 
							(((uint8_t) gpio_get_level(BUT_4)) << 0x4) | 
							(((uint8_t) gpio_get_level(BUT_5)) << 0x5) | 
							(((uint8_t) gpio_get_level(EquBut)) << 0x6));
		}

/*---------------------------------------------
 * Funkcja włącza przerwania od wszystkich GPIO
 * klawiatury i enkoderów.
 *---------------------------------------------*/
void KEYBOARD::gpio_intr_enableAll()
{
	gpio_intr_enable(VolBut);
	gpio_intr_enable(BUT_1);
	gpio_intr_enable(BUT_2);
	gpio_intr_enable(BUT_3);
	gpio_intr_enable(BUT_4);
	gpio_intr_enable(BUT_5);
	gpio_intr_enable(EquBut);
	gpio_intr_enable(VolRot_A);
	gpio_intr_enable(VolRot_B);
	gpio_intr_enable(EquRot_A);
	gpio_intr_enable(EquRot_B);
}


/*--------------------------------------------
 * Destrukrot klasy klawiatury
 *------------------------------------------*/
KEYBOARD::~KEYBOARD()
{
		//todo:
		// - zobaczyć co trzeba NULLować, żeby zrobić soft reset
	this->gpio_intr_disableAll();
	gpio_reset_pin(VolBut);
	gpio_reset_pin(BUT_1);
	gpio_reset_pin(BUT_2);
	gpio_reset_pin(BUT_3);
	gpio_reset_pin(BUT_4);
	gpio_reset_pin(BUT_5);
	gpio_reset_pin(EquBut);
	gpio_reset_pin(VolRot_A);
	gpio_reset_pin(VolRot_B);
	gpio_reset_pin(EquRot_A);
	gpio_reset_pin(EquRot_B);
	
	
	gpioAlarm_config.alarm_count = 0;// NULL;
	gpioAlarm_config.reload_count = 0;//NULL;
	this->gpioAlarm_config.flags.auto_reload_on_alarm	= 0;//NULL;
	ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &this->gpioAlarm_config));
	gptimer_stop(gptimer);
	gptimer_del_timer(gptimer);
	
	vTaskSuspend(handlerTask_keyboardLongPressOnPressQueueFeeder);
	vTaskDelete(handlerTask_keyboardLongPressOnPressQueueFeeder);
	
}


//void KEYBOARD::keyboardQueueSendResetToDefault(QueueHandle_t keyboardQueue)
//{
//	uint8_t button;
//	const uint8_t keyboardResetvaluePattern = 0b01110001;		//wartość zwracana przez buttonsGetLevel(), gdy wciśnięte śa przycisk But1, But2, But3
//	const uint8_t buttonOnOffBitPattern	= 0b00000001;			//bit 1 to bit przycisku Power On/ Off
//	
//	do
//	{
//		ESP_LOGI(this->TAG, "Release Power ON/OFF to go fortrher to continue initializing \r\n or press 'reset to default' key set.");
//		button = buttonsGetLevel();
//	} while (!(button & buttonOnOffBitPattern));		//czeka zanim przycisk Power On/Off zostanie zwolniony po właczeniu radio
//		
//	if (button == keyboardResetvaluePattern)	//sprzqwdza czy podczas właczenia radio użytkownik trzymał włączone But1 + But2 + But3,
//												//co jest oznaką, że użytkownik podczas startu dał komendę reset to default.
//	{
//
//		ESP_LOGI(this->TAG, "Reset to default!!!");
//
//		this->onExitHMIValue[0] = HMI_INPUT_BUTTON;//this->gpioInterruptCallback.case_buttonInput;
//		this->onExitHMIValue[1] = button;
//		
//		xQueueSend(keyboardQueue, this->onExitHMIValue, portMAX_DELAY); //wpisanie do kolejki warości przycisków oznaczającej RESET TO DEFAULT
//	}
//}