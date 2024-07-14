#pragma once

#include "hal/gpio_types.h"
#include "./../../../03_Common/comunicationProtocol.h"
#include "./../../../03_Common/toolchainEnvironmentNumbers.h"
#include "driver/gpio.h"

#define TOOLCHAIN_ENVIRONMENT		__esp32__
#define chip_ESP32_WROOM32D_DEVBOARD	0

#define CHIP chip_ESP32_WROOM32D_DEVBOARD		//select Your chip/ devboard type

#if CHIP == chip_ESP32_WROOM32D_DEVBOARD  
#include "chip_config_ESP32_WROOM32D_DEVBOARD.h"
#endif


#define DEBOUNCE_TIMER_PRIORITY			0									//w ESP-IDF 5.2+ wprowadzono nowy parametr timera (prioryter timera), który we moim kodzie odpowiada za debouncowanie styków 
#define DEBOUNCE_PARAMETERS				DEBOUNCE_PARAMETERS_100us_20x		//PLEASE choose one of: "DEBOUNCE_PARAMETERS_100us_20x" or "DEBOUNCE_PARAMETERS_275us_20x" or "DEBOUNCE_PARAMETERS_137us_40x"
#define ENCODER_PULSED_PER_DETANT		4									//PLEASE choose one of: "4" or "2" based on rotary encoder datasheet
#define DETANT1_GPIO_STATE				0x11								//PLEASE choose: set according to encoder datasheet

//debounceTimer configuration
#define DEBOUNCE_PARAMETERS_275us_20x	1		//DO NOT CHANGE
//#define DEBOUNCE_PARAMETERS_137us_40x	2		//DO NOT CHANGE
#define DEBOUNCE_PARAMETERS_100us_20x	3		//DO NOT CHANGE
	
//#define DEBOUNCE_PARAMETERS		DEBOUNCE_PARAMETERS_275us_20x //PLEASE choose one of: "EBOUNCE_PARAMETERS_275us_20x" or "DEBOUNCE_PARAMETERS_137us_40x"	

#ifdef DEBOUNCE_PARAMETERS
	#if DEBOUNCE_PARAMETERS == 1
		#define GPIO_DEBOUNCE_TIMER_RESOLUTION_HZ	3636		//DO NOT CHANGE		//3636 Hz 1 tick = 275us
		#define GPIO_DEBOUNCE_EQUAL					12 //7	//20	//DO NOT CHANGE
		#define GPIO_LONG_PRESS						10000		//DO NOT CHANGE
		#define ENCODER_notFULL_DETANT_TIMEOUT		1000		//DO NOT CHCNGE		//if encoder counts less pulses than ENCODER_PULSED_PER_DETANT
	#endif

	#if DEBOUNCE_PARAMETERS == 2
		#define GPIO_DEBOUNCE_TIMER_RESOLUTION_HZ	7272		//DO NOT CHANGE		//7272 Hz 1 tick = 137 us
		#define GPIO_DEBOUNCE_EQUAL					40			//DO NOT CHANGE
		#define GPIO_LONG_PRESS						20000		//DO NOT CHANGE
		#define ENCODER_notFULL_DETANT_TIMEOUT		1000		//DO NOT CHCNGE		//if encoder counts less pulses than ENCODER_PULSED_PER_DETANT	
	#endif // DEBOUNCE_PARAMETERS == 137us_50x

	#if DEBOUNCE_PARAMETERS == 3
		#define GPIO_DEBOUNCE_TIMER_RESOLUTION_HZ	20000		//DO NOT CHANGE		//10000 Hz 1 tick = 100 us
		#define GPIO_DEBOUNCE_EQUAL					18//5 /*10*/	//DO NOT CHANGE		//5 wygląda na dobrą wartośc 10 też
		#define GPIO_LONG_PRESS						20000		//DO NOT CHANGE
		#define ENCODER_notFULL_DETANT_TIMEOUT		1300		//DO NOT CHCNGE		//if encoder counts less pulses than ENCODER_PULSED_PER_DETANT	
		#endif // DEBOUNCE_PARAMETERS == 137us_50x
#endif


#define	MAIN_KEYBOARD_QUEUE_LEN						16
#define I2C_SLAVESET_BUFFER_KEYBOARD_LEN			MAIN_KEYBOARD_QUEUE_LEN	-1
//#define I2C_SLAVE_QUEUESET_COMBINED_LEN				I2C_SLAVESET_BUFFER_KEYBOARD_LEN			
#define LED_DISPLAY_LEDS_QUANTITY					11
#define LED_DISPLAY_BLINK_TIME						1000


#define NVS_RADIO_CONFIG_NAMESPACE			"RadioConfig"		//not for bluetooth which is controlled via BTstack nvs functions
#define NVS_PARTITION_NAME_DEFAULT			NULL
#define NVS_EREASE_COUNTDOWN_TIME			10

