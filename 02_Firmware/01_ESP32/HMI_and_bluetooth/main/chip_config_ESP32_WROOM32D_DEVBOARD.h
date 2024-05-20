#pragma once

//task to core assigment
#define TASK_TO_CORE1		1

//Please set your hardware details - BEGINNING
#define I2C_MASTER_PIN_SDA				GPIO_NUM_19
#define I2C_MASTER_PIN_SCL				GPIO_NUM_18
#define I2C_SLAVE_PIN_SDA				GPIO_NUM_21
#define I2C_SLAVE_PIN_SCL				GPIO_NUM_22
#define	I2C_MASTER_SPEED				400000								//DO NOT CHANGE
#define	I2C_MASTER_RX_BEFFER			10
#define	I2C_MASTER_TX_BEFFER			10
#define MCP23008_I2C_DEVICE_OPCODE		0b0100000

//i2s pins assigment
#define I2S_PIN_BCK		GPIO_NUM_26
#define I2S_PIN_WS		GPIO_NUM_27
#define I2S_PIN_OUT		GPIO_NUM_25

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

//stepper motor and optotransistor sensors connected to MCP23008
#define SENSOR_EQU_SIDE_MASK		(1<<7)
#define SENSOR_VOL_SIDE_MASK		(1<<6)
#define MOTOR_FAULT_MASK			(1<<5)
#define MOTOR_DIRECTION_MAKS		(1<<4)
#define MOTOR_STEP_MASK				(1<<3)
#define MOTOR_NOT_SLEEP_MASK		(1<<2)
#define POWER_SUPPLU_SUSTAIN_MASK	(1<<1)
//#define INTERRUP_REQUEST_MASK	(1<<0)
