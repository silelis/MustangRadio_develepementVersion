#pragma once
#include "inttypes.h"
#include "comunicationStructures.h"


#define I2C_COMMAND_GROUP_PING				0x00
#define I2C_COMMAND_GROUP_KEYBOARD			0x01
#define I2C_COMMAND_GROUP_NVS				0x02



//I2C_COMMAND_GROUP_KEYBOARD			0x01
#define HMI_INPUT_BUTTON					'b'		//oznacza, ¿e przycisk zosta³ zwolniony (po osiagniêciu czasu long press lub przed tym czasem)
#define HMI_INPUT_BUTTON_LONG_AND_PRESSED	'B'		//oznacza, ¿e przycisk jest nadal wciœniêty po osiagniêciu czasu long press 
#define HMI_INPUT_VOLUME					'v'		//oznacza, ¿e poruszany jest encoder g³oœnoœci
#define HMI_INPUT_EQUALISER					'e'		//oznacza, ¿e poruszany jest encoder equalizera



//NVS keys definition	+
//I2C_COMMAND_GROUP_NVS					0x02
#define NVS_KEY_i8_test					"test"
#define NVS_KEY_BLOB_MotorParameters	"MotorParam"


//NVS keys definition