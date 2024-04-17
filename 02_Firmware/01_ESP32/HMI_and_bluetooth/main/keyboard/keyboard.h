#pragma once
#include "cstdio"
#include "hwConfigFile.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "comunicationProtocol.h"

#include "freertos/task.h"
//#include "tasksFunctions/tasksFunctions.h"


#define LONG_PRESS_BIT_MASK					0b10000000
#define ON_PRESS_QUEUE_FEEDER_DELEY_TIME_MS	750

struct kbrdState
{
	volatile uint8_t lastState;
	volatile uint8_t newState;
	uint8_t latchedState;
};

/*union keyboardUnion
{
	char array[2];
	struct //charUint8
	{
		char input;
		int8_t value;
	} encoderValue;
	
	struct //charInt8
	{
		char input;
		uint8_t value;
	}kbrdValue;
};*/

struct encoderState
{
	int8_t stepCounter;
	volatile uint8_t lastState;		//used to debounce
	volatile uint8_t newState;		//used to debounce
	uint8_t lastLatchedState;		//pin state after dobounce used to calculate direction (not requiref if encoders and input would be in different functions)
	uint8_t newLatchedState;		//pin state after dobounce used to calculate direction (not requiref if encoders and input would be in different functions)
	uint8_t pulseIncrement;
	//const int8_t pulsPerDetant = ENCODER_PULSED_PER_DETANT;
	int8_t latchedQEM;				//value which is retrurned to encoder queue
};

//#define CASE_BUTTON_INPUT 'B'
//#define CASE_VOLUME_INPUT 'V'
//#define CASE_EQUIALISER_INPUT 'E'

struct gpioInterruptCallbackStruct				//struct which is passed to interrupts (timer and gpios) callback functions, unfortunately to callback functions only 1 parameter can be passed
{
	encoderState*		pVolEncState;			//pointer to volume encoder
	encoderState*		pEquEncState;			//pointer to equaliser encoder
	kbrdState*			pbuttonsState;			//pointer to buttons struct kbrdState
	gptimer_handle_t	pgptimer;				//pointer to timer
	bool exitTimerInterrupt;					//variable used to determine if alarm callback function should be stopped
	uint16_t debounceTime;						//debounce time and long press counting variable
	//QueueHandle_t callbackKeyboardQueueHandler;
	QueueHandle_t queueHandler_keyboard;		//pointer to keyboard values queue received from main (form class Keyboard constructor)
	TaskHandle_t taskHandler_onPeriodLongButtonPressNotification; //wskaźnik do zadania (taska), które wskazuje (notyfikuje), że nastąpił long button press 
	TaskHandle_t taskHandler_keyboardLongPressOnPressQueueFeeder; //wskaźnik do zadania (taska), które po przekroczenia minimalnego czasu long press i do czasu puszczenia przysicka informuje (wysyła do kolejki dane) o przytrzymaniu prtzycisku
	
	

	//char *keyboardExitValueHandler;				//wskaźnik (pomocniczy) odwołujący sie do zmiennej z klasy kalwaitura przechowującej ostateczną wartośc odczytaną z klawiatury
	keyboardUnion *keyboardExitValueHandler;	//wskaźnik (pomocniczy) odwołujący sie do zmiennej z klasy kalwaitura przechowującej ostateczną wartośc odczytaną z klawiatury, która jest zwracana do kolejki klawiatury
	
	
	
	const int8_t QEM[16] = { 0, -1, 1, 2, 1, 0, 2, -1, -1, 2, 0, 1, 2, 1, -1, 0 };		//rotary encoder state martix
	char whichInput;							//variable used to determine which input in active /buttons/ volume encoder / equaliser encoder
	//const char case_buttonInput = HMI_INPUT_BUTTON; // additional constant 'B' used in switch
	//const char case_volumeInput = HMI_INPUT_VOLUME; /// additional constant 'V' used in switch
	//const char case_equaliserInput = HMI_INPUT_EQUALISER; // additional constant 'E' used in switch
};


static void keyboardLongPressOnPressQueueFeeder(void *);

class KEYBOARD
{
public:
	KEYBOARD(QueueHandle_t, TaskHandle_t);
	~KEYBOARD();
	static void gpio_intr_disableAll();
	static void gpio_intr_enableAll();
	static uint8_t buttonsGetLevel();
	static uint8_t encoderGetLevel(gpio_num_t EncoderPortA, gpio_num_t EncoderPortB);
		
protected:

	
private:
	const char *TAG = "Input(s) log:";
	gpio_config_t iputButtonsConf;
	gpio_config_t iputRotaryEncoderConf;
	gptimer_event_callbacks_t cbs;
	gpioInterruptCallbackStruct gpioInterruptCallback;
		
	gptimer_config_t gpioDebounceTimer_config;
	gptimer_handle_t gptimer = NULL;
	gptimer_alarm_config_t gpioAlarm_config;
	//char onExitHMIValue[HMI_INPUT_COMMAND_LEN];		//zmienna przechowująca ostateczną wartość odczytaną z klawiatury, wartość, która będzie wpisana do kolejki
	keyboardUnion onExitHMIValue;
	kbrdState buttonsState;
	encoderState VolEncState;
	encoderState EquEncState;
	//void keyboardQueueSendResetToDefault(QueueHandle_t );
	friend void keyboardLongPressOnPressQueueFeeder(void *parameters);
};