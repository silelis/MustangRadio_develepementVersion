/*
 * StepperOptoPowerOFF.h
 *
 *  Created on: Jul 5, 2025
 *      Author: dbank
 */

#ifndef INC_SILELIS_CODE_DEVICE_STEPPEROPTOPOWEROFF_H_
#define INC_SILELIS_CODE_DEVICE_STEPPEROPTOPOWEROFF_H_

//#include "comunicationStructures.h"
#include "comunicationProtocol.h"
#include "i2c_slave_master_queueClass.h"
//#include "SileliS_code/radioMegaStruct.h"
#include "comunication_calculate_checksum.h"

class StepperOptoPowerOFF {
public:
	StepperOptoPowerOFF(/*stepperMotorStruct *stepperMotorData,*/
			i2cQueue4DynamicData *MasterTransmitToSlave_DataQueue);
	virtual ~StepperOptoPowerOFF();

	void setMotorCalibration(void);
	void setMotorGotoAbsolut(uint16_t gotoPosition);
	void setMotorGotoInBoarders(uint16_t gotoPosition);
	void setMotorMoveByAbsolute(int32_t moveBy);
	void setMotorMoveByBoarders(int32_t moveBy);
	void setMotorPercentsAbsolut(StepperPositionInPercents percents);
	void setMotorPercentsBoarders(StepperPositionInPercents percents);
	void setRadioPowerOff(uint8_t radioRestartCountdown);

protected:

private:
	stepperMotorStruct StepperMotorData;
	i2cQueue4DynamicData *pI2C_MasterTransmitToSlave_DataQueue;
	//i2cFrame_stepper stepperComunicationFrame;
	BaseType_t sendDataToI2cTransmitQueue();

};

#endif /* INC_SILELIS_CODE_DEVICE_STEPPEROPTOPOWEROFF_H_ */
