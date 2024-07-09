#pragma once
#include "inttypes.h"
#include "comunicationStructures.h"
#include <stdio.h>
#include <stdint.h> 

#define I2C_SLAVE_ADDRESS					0x3C	//si468x 0b11001xx, TDA741x	 0b1000100, 24C16 0b1010xxx, TEA5767 0b1100000, MCP23008 0b0100xxx
#define I2C_SLAVE_RX_BUFFER_LEN				512
#define I2C_SLAVE_TX_VARIABLE_LEN			384


#define I2C_COMMAND_GROUP_SYSTEM/*PING*/	0x00
#define I2C_COMMAND_GROUP_KEYBOARD			0x01
#define I2C_COMMAND_GROUP_NVS				0x02


//I2C_COMMAND_GROUP_SYSTEM				0x00


//I2C_COMMAND_GROUP_KEYBOARD			0x01
#define HMI_INPUT_BUTTON					'b'		//oznacza, że przycisk został zwolniony (po osiagnięciu czasu long press lub przed tym czasem)
#define HMI_INPUT_BUTTON_LONG_AND_PRESSED	'B'		//oznacza, że przycisk jest nadal wciśnięty po osiagnięciu czasu long press 
#define HMI_INPUT_VOLUME					'v'		//oznacza, że poruszany jest encoder głośności
#define HMI_INPUT_EQUALISER					'e'		//oznacza, że poruszany jest encoder equalizera
#define LONG_PRESS_BIT_MASK					0b10000000


//I2C_COMMAND_GROUP_NVS					0x02
#define NVS_KEY_i8_test					"test"
#define NVS_KEY_BLOB_MotorParameters	"MotorParam"


/*
//i2c_farame
typedef struct {
	uint8_t frameSize;	  //frameSize = sizeof(frameSize) + 	sizeof(commandGroup)+sizeof(commandData)
	uint8_t commandGroup; //aka I2C_COMMAND_GROUP
	union dataUnion
	{
		union keyboardUnion keyboardData;
		
	}commandData;
		
} i2cFrame;
*/


typedef struct
{	   	
	uint8_t	CRC;			//checksum for i2c farame (commandGroup+dataSize+data)
	uint8_t commandGroup;	//aka I2C_COMMAND_GROUP
	uint8_t dataSize;		//sizeof(data to send)
} i2cFrame_commonHeader;

typedef struct {
	i2cFrame_commonHeader i2cframeCommandHeader;
	union keyboardUnion keyboardData;
} i2cFrame_keyboardFrame;

typedef struct{
	size_t dataSize;
	void *pData;	
} i2cFrame_transmitQueue;