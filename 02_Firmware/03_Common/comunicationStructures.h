#pragma once
#include "inttypes.h"

struct ws2812Color
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
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



struct colorSet {
	struct ws2812Color primary;
	struct ws2812Color secondary;
};
