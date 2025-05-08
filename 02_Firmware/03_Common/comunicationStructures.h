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

struct hmiLeds{		//struktura zawierająca informacje o stanie ledów wykorzystywana podczas komunikacji między master a slave
	colorSet	sourceLed;
	colorSet	equaliserLed;
	colorSet	errorLed;
	colorSet	backlightLeds;
	uint8_t		backlightLedsFrom=0;
	uint8_t		backlightLedsTo=LED_DISPLAY_LEDS_QUANTITY-3;
};

typedef struct {		//struktura zawierająca informacje o stanie ledów wykorzystywana podczas komunikacji między master a slave
	uint16_t beginOffest;
	uint16_t endOffset;
	uint16_t maxPosition;
	uint16_t currentPosition;

} MotorParameters;

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


