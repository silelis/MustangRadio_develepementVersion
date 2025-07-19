/*
 * StepperOptoPowerOFF.cpp
 *
 *  Created on: Jul 5, 2025
 *      Author: dbank
 */

#include <SileliS_code/StepperOptoPowerOFF.h>
#include <new>

StepperOptoPowerOFF::StepperOptoPowerOFF(stepperMotorStruct* stepperMotorData, i2cQueue4DynamicData* MasterTransmitToSlave_DataQueue)
{

	this->pI2C_MasterTransmitToSlave_DataQueue = MasterTransmitToSlave_DataQueue;

	this->pStepperMotorData = stepperMotorData;

}

StepperOptoPowerOFF::~StepperOptoPowerOFF() {
	// TODO Auto-generated destructor stub
}

BaseType_t StepperOptoPowerOFF::sendDataToI2cTransmitQueue(){

	//tworzy ramke do przesłania
	i2cFrame_stepper stepperToSend;
	stepperToSend.i2cframeCommandHeader.commandGroup = I2C_COMMAND_GROUP_STEPPER;
	stepperToSend.i2cframeCommandHeader.dataSize = sizeof(i2cFrame_stepper);
	//kopiuje dane z radioMegaStruct do ramki
	memcpy(&stepperToSend.stepperData, this->pStepperMotorData, sizeof(stepperMotorStruct));
	stepperToSend.i2cframeCommandHeader.crcSum = (uint8_t) calculate_checksum(&stepperToSend, stepperToSend.i2cframeCommandHeader.dataSize);
	//tworzy ramkę komunikacujną do przesłania
	i2cFrame_transmitQueue dataToTransmitQueue;
	dataToTransmitQueue.pData = new (std::nothrow) i2cFrame_stepper;
	if (dataToTransmitQueue.pData!=nullptr){
		dataToTransmitQueue.slaveDevice7bitAddress = I2C_SLAVE_ADDRESS_ESP32;
		dataToTransmitQueue.dataSize = stepperToSend.i2cframeCommandHeader.dataSize;
		memcpy(dataToTransmitQueue.pData, &stepperToSend, dataToTransmitQueue.dataSize);
		return this->pI2C_MasterTransmitToSlave_DataQueue->QueueSend(&dataToTransmitQueue);
	}
	else{
		assert(dataToTransmitQueue.pData);
		return pdFALSE;
	}
}



void StepperOptoPowerOFF::setMotorCalibration(void){
	this->pStepperMotorData->stepperSubcommand = MOTOR_SUBCOMMAND_CALIBRATION;
	this->pStepperMotorData->stepperUnion.stepperCalibration =1;
	this->sendDataToI2cTransmitQueue();
}

void setMotorGotoAbsolut(uint16_t gotoPosition);
void setMotorGotoInBoarders(uint16_t gotoPosition);
void setMotorMoveByAbsolute(int32_t moveBy);
void setMotorMoveByBoarders(int32_t moveBy);
void setMotorPercentsAbsolut(StepperPositionInPercents percents);
void setMotorPercentsBoarders(StepperPositionInPercents percents);
void setRadioPowerOff(uint8_t radioRestartCountdown);


#define MOTOR_SUBCOMMAND_CALIBRATION			0x00
#define MOTOR_SUBCOMMAND_GOTO_ABSOLUT			0x01
#define MOTOR_SUBCOMMAND_GOTO_IN_BOARDERS		0x02
#define MOTOR_SUBCOMMAND_MOVE_BY_ABSOLUT		0x03
#define MOTOR_SUBCOMMAND_MOVE_BY_BOARDER		0x04
#define MOTOR_SUBCOMMAND_PERCENTS_ABSOLUT		0x05
#define MOTOR_SUBCOMMAND_PERCENTS_BOARDER		0x06
#define MOTOR_SUBCOMMAND_POWER_OFF				0x07
