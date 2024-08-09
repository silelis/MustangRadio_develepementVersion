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
};

struct colorSet {
	struct ws2812Color primary;
	struct ws2812Color secondary;
};