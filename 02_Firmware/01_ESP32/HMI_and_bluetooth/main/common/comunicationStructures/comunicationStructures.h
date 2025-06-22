#pragma once
#include "inttypes.h"

#define LED_DISPLAY_LEDS_QUANTITY	11	//from 0 to LED_DISPLAY_LEDS_QUANTITY-3 are background, last 3 are menu leds
struct ws2812Color
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct colorSet {
	struct ws2812Color primary;
	struct ws2812Color secondary;
};

//#define	BACKLIGHT_LED_FROM	0
//#define	BACKLIGHT_LED_TO	LED_DISPLAY_LEDS_QUANTITY-3

struct hmiLeds{		//struktura zawierająca informacje o stanie ledów wykorzystywana podczas komunikacji między master a slave
	colorSet	sourceLed;
	colorSet	equaliserLed;
	colorSet	errorLed;
	//colorSet	backlightLeds;
	//uint8_t		backlightLedsFrom=BACKLIGHT_LED_FROM;
	//uint8_t		backlightLedsTo=BACKLIGHT_LED_TO;
};

union keyboardUnion
{
	char array[2];			//daje możliwośc odczytania danych jako zwykła tabele char (niezależnie czy są to dane z klawiatury czy enkodera)
	struct //charUint8
	{
		char input;			//informacja, z którego enkodera pochodzą dane (głośnośc czy equalizer)
		int8_t value;		//informacja na temat kierunku rotacji prawo / lewo
	} encoderValue;			//daje możliwość łatwego zapisania/odczytu danych dla enkoderów
	
	struct //charInt8
	{
		char input;			//zzakodowana ba bitach informacja o tym, który przycisk został wciśnięty i o tym, że nastąpił long press
		uint8_t value;		//informacja, że dane pochodzą z przycisków
	}kbrdValue;				//daje możliwość łatwego zapisania/odczytu danych dla klawiatury
};

typedef float StepperPositionInPercents;

union stepperUnion{
	bool stepperCalibration;		//MOTOR_SUBCOMMAND_CALIBRATION
	uint16_t gotoPosition;			//MOTOR_SUBCOMMAND_GOTO_ABSOLUT, MOTOR_SUBCOMMAND_GOTO_IN_BOARDERS - position to which motor should move
	int32_t moveBy;
//	struct{
//		bool direction;				//0 negative, 1 positive
//		uint16_t steps;
//	}moveBy;						//MOTOR_SUBCOMMAND_MOVE_BY_ABSOLUT, MOTOR_SUBCOMMAND_MOVE_BY_BOARDER - steps - how many steps motor should move
									//												 direction -  0 negative move, 1 positive move
	StepperPositionInPercents percents;	//MOTOR_SUBCOMMAND_PERCENTS_ABSOLUT, MOTOR_SUBCOMMAND_PERCENTS_BOARDER
	uint8_t radioRestartCountdown;
};

