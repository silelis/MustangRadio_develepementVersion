#pragma once
#include "inttypes.h"
#include "comunicationStructures.h"

#define HMI_INPUT_BUTTON			'B'
#define HMI_INPUT_VOLUME			'V'
#define HMI_INPUT_EQUALISER			'E'
//#define HMI_INPUT_COMMAND_LEN		2


/*struct ws2812Color
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
};*/




//NVS keys definition
#define NVS_KEY_i8_test					"test"
#define NVS_KEY_BLOB_MotorParameters	"MotorParam"


//NVS keys definition