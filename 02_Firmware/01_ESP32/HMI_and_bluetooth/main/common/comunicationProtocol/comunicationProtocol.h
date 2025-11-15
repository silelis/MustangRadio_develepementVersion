#pragma once
#include "inttypes.h"
#ifdef  /*TOOLCHAIN_ENVIRONMENT == __esp32__ */ ESP_PLATFORM
	#include "D:\!!!__GitHUB_repositories\MustangRadio_develepementVersion\02_Firmware\01_ESP32\HMI_and_bluetooth\main\common\comunicationStructures/comunicationStructures.h"
	#include "../../hwConfigFile.h"
#elif /*TOOLCHAIN_ENVIRONMENT == __stm32__ */ __ARM_ARCH
#include "comunicationStructures/comunicationStructures.h"
#include "hwConfigFile.h"
#else
	#error "TOOLCHAIN_ENVIRONMENT which is unknown!!!!"
#endif
#include <stdio.h>
#include <stdint.h> 

//#define STM32_2_ESP32_I2C_IN_SEQUENCE  - nie działa po stronie STM32 i nie wiem jak naprawić nie właczać!!!

#ifdef I2C_STM32_TO_ESP32_ROLE_MASTER
struct i2cFrame_transmitQueue {
	uint8_t slaveDevice7bitAddress;	//pole zawiera informację z którego urządzenia slave (ares urządzenia) pochodzą odczytane po i2c dane
	size_t dataSize;//pole zawiera informację o długości przesłanych danych (m.in. na podstawie tej informacji w sposób dynamiczny tworzone są zmienne przechowujące otrzymane dane
	void *pData;//wskaźnik do miejsca w pamięci RAM (zarezerwowanej dynamicznie), gdzie przechowywane są otrzymane po i2c dane
	i2cFrame_transmitQueue() :
			slaveDevice7bitAddress(0), dataSize(0), pData(nullptr) {
	}
};
#else
	typedef struct{
		size_t dataSize;					//jak wyżej
		void *pData;						//jak wyżej
	} i2cFrame_transmitQueue;				//jak wyżej

#endif

typedef struct {
	uint8_t crcSum;		//checksum for i2c farame (commandGroup+dataSize+data)
	uint8_t commandGroup;	//aka I2C_COMMAND_GROUP
	size_t dataSize;		//sizeof(data to send)
} i2cFrame_commonHeader;//nagłówek każdej ramki komunikacyjnej i2c zpomiędzy stm32 i ESP32

#define I2C_SLAVE_ADDRESS_ESP32					0x3C	//si468x 0b11001xx, TDA741x	 0b1000100, 24C16 0b1010xxx, TEA5767 0b1100000, MCP23008 0b0100xxx
#define ESP32_SLAVE_RECEIVE_BUFFER_LEN			52		//wygląda na to, że 52 to max

//DEFINICJE I STRUKTURY KOMUNIKACYJNE POMIĘDZY STM32 i ESP32
#define I2C_COMMAND_GROUP_SYSTEM			0x00
#define I2C_COMMAND_GROUP_KEYBOARD			0x01	//DONE
#define I2C_COMMAND_GROUP_NVS				0x02
#define I2C_COMMAND_GROUP_LEDS				0x03	//DONE
#define I2C_COMMAND_GROUP_STEPPER			0x04	//DONE

//I2C_COMMAND_GROUP_KEYBOARD			0x01
#define HMI_INPUT_BUTTON					'b'		//oznacza, że przycisk został zwolniony (po osiagnięciu czasu long press lub przed tym czasem)
#define HMI_INPUT_BUTTON_LONG_AND_PRESSED	'B'		//oznacza, że przycisk jest nadal wciśnięty po osiagnięciu czasu long press 
#define HMI_INPUT_VOLUME					'v'		//oznacza, że poruszany jest encoder głośności
#define HMI_INPUT_EQUALISER					'e'		//oznacza, że poruszany jest encoder equalizera
#define LONG_PRESS_BIT_MASK					0b10000000

typedef struct {
	i2cFrame_commonHeader i2cframeCommandHeader;
	union keyboardUnion keyboardData;
} i2cFrame_keyboardFrame;

//I2C_COMMAND_GROUP_LEDS				0x03
typedef struct {
	i2cFrame_commonHeader i2cframeCommandHeader;
	struct hmiLeds ledsData;
} i2cFrame_hmiLeds;

//I2C_COMMAND_GROUP_STEPPER			0x04
#define MOTOR_SUBCOMMAND_CALIBRATION			0x00
#define MOTOR_SUBCOMMAND_GOTO_ABSOLUT			0x01
#define MOTOR_SUBCOMMAND_GOTO_IN_BOARDERS		0x02
#define MOTOR_SUBCOMMAND_MOVE_BY_ABSOLUT		0x03
#define MOTOR_SUBCOMMAND_MOVE_BY_BOARDER		0x04
#define MOTOR_SUBCOMMAND_PERCENTS_ABSOLUT		0x05
#define MOTOR_SUBCOMMAND_PERCENTS_BOARDER		0x06
#define MOTOR_SUBCOMMAND_POWER_OFF				0x07

typedef struct {
	i2cFrame_commonHeader i2cframeCommandHeader;
	//uint8_t stepperSubcommand;
	//union stepperUnion stepperData;
	struct stepperMotorStruct stepperData;
} i2cFrame_stepper;

//I2C_COMMAND_GROUP_SYSTEM				0x00
//#define SYSTEM_KEEPALIV				0x01
//#define SYSTEM_CRC_ERROR				0x02
//#define SYSTEM_CRC_OK					0x03

//I2C_COMMAND_GROUP_NVS					0x02
#define NVS_KEY_IS_INITIALISED			(const char[]){0x01,0x01}
#define NVS_KEY_MOTOR_BEGIN_OFFSET		(const char[]){0x01,0x02}
#define NVS_KEY_MOTOR_END_OFFSET				(const char[]){0x01,0x03}

#define NVS_KEY_IS_INITIALISED_ANSWER   "YES"
#define NVS_KEY_i8_test					"test"
#define NVS_KEY_BLOB_MotorParameters	"MotorParam"

