#pragma once

#include "hal/gpio_types.h"
#include "comunicationProtocol.h"
#include "driver/gpio.h"

#define CHIP ESP32

#if CHIP == ESP32  
	//task to core assigment
	#define TASK_TO_CORE1		1

	//Please set your hardware details - BEGINNING
	#define I2C_MASTER_PIN_SDA				GPIO_NUM_19
	#define I2C_MASTER_PIN_SCL				GPIO_NUM_18
	#define	I2C_MASTER_SPEED				400000								//DO NOT CHANGE
	#define	I2C_MASTER_RX_BEFFER			10
	#define	I2C_MASTER_TX_BEFFER			10
	#define MCP23008_I2C_DEVICE_OPCODE		0b0100000




	//input buttons and rotary encoder configuration
	#define BUT_0	GPIO_NUM_33						//DO NOT CHANGE
	#define BUT_1	GPIO_NUM_36						//DO NOT CHANGE
	#define BUT_2	GPIO_NUM_39						//DO NOT CHANGE
	#define BUT_3	GPIO_NUM_34						//DO NOT CHANGE
	#define BUT_4	GPIO_NUM_35						//DO NOT CHANGE
	#define BUT_5	GPIO_NUM_32						//DO NOT CHANGE
	#define BUT_6	GPIO_NUM_23						//DO NOT CHANGE
	#define VolBut	BUT_0							//DO NOT CHANGE
	#define EquBut	BUT_6							//DO NOT CHANGE
	#define VolRot_A	GPIO_NUM_2					//DO NOT CHANGE
	#define VolRot_B	GPIO_NUM_4					//DO NOT CHANGE
	#define EquRot_A	GPIO_NUM_17					//DO NOT CHANGE
	#define EquRot_B	GPIO_NUM_5					//DO NOT CHANGE
	#define LED_DISPLAY_GPIO	GPIO_NUM_16
	//input buttons and rotary encoder configuration

	//stepper motor and optotransistor sensors connected to MCP23008
	#define SENSOR_EQU_SIDE_MASK	(1<<7)
	#define SENSOR_VOL_SIDE_MASK	(1<<6)
	#define MOTOR_FAULT_MASK		(1<<5)
	#define MOTOR_DIRECTION_MAKS	(1<<4)
	#define MOTOR_STEP_MASK			(1<<3)
	#define MOTOR_NOT_SLEEP_MASK	(1<<2)
	//#define MUX_SELECT_MASK		(1<<1)
	//#define INTERRUP_REQUEST_MASK	(1<<0)
	//stepper motor and optotransistor sensors connected to MCP23008

	//Please set your hardware details - END
#endif

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
		#define GPIO_DEBOUNCE_TIMER_RESOLUTION_HZ	10000		//DO NOT CHANGE		//10000 Hz 1 tick = 100 us
	#define GPIO_DEBOUNCE_EQUAL						5 /*10*/	//DO NOT CHANGE		//5 wygląda na dobrą wartośc 10 też
		#define GPIO_LONG_PRESS						26000		//DO NOT CHANGE
		#define ENCODER_notFULL_DETANT_TIMEOUT		1300		//DO NOT CHCNGE		//if encoder counts less pulses than ENCODER_PULSED_PER_DETANT	
		#endif // DEBOUNCE_PARAMETERS == 137us_50x
#endif


#define	QueueHandlerMainKeyboard_len	0x32
#define LED_DISPLAY_LEDS_QUANTITY		11
#define LED_DISPLAY_BLINK_TIME			1000


#define NVS_RADIO_CONFIG_NAMESPACE			"RadioConfig"		//not for bluetooth which is controlled via BTstack nvs functions
#define NVS_PARTITION_NAME_DEFAULT			NULL
#define NVS_EREASE_COUNTDOWN_TIME			10

