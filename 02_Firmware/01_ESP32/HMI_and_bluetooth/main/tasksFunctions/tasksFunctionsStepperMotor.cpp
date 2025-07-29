#include "tasksFunctions/tasksFunctionsStepperMotor.h"
#include "tasksFunctions/tasksFunctions.h"		//tutaj jest extern NVS* pSTORAGE; //obiekt zapisuj¹cy i czytaj¹cy dane z NCS ESP32

StepperOptoPowerOFF * pMotor; //obiekt steruj¹cy prac¹ silnika krokowego, jego krañcówej i power off radia


static TaskHandle_t handlerTask_stepperMotorCalibration = NULL;
static TaskHandle_t handlerTask_stepperMotorMove = NULL;

static void stepperMotorCalibration(void* nothing)
{
	uint16_t beginOffest;
	uint16_t endOffset;
	
	if (pSTORAGE->get_u16(NVS_KEY_MOTOR_BEGIN_OFFSET, &beginOffest) != ESP_OK) {
		beginOffest = 0;
	} 	
	if (esp_err_t retVal1 = pSTORAGE->get_u16(NVS_KEY_MOTOR_END_OFFSET, &endOffset) != ESP_OK)
	{
		endOffset = UINT16_MAX;	
	}
	vTaskSuspend(handlerTask_stepperMotorCalibration);
	for (;;)
	{
		if (pMotor->isCalibrated() == pdFALSE)
		{
			pMotor->measureSliderRange(beginOffest, endOffset);
		}
		
	}
}

static void stepperMotorMove(void* nothing)
{
	vTaskSuspend(handlerTask_stepperMotorMove);
	for (;;)
	{
		if (pMotor->isPositionReached() != pdTRUE)
		{
			pMotor->moveToVolatileDestinationPosition();
		}
		
	}
}


void stepperMotorDataParser(void *TaskParameters)
{
	configASSERT(xTaskCreatePinnedToCore(stepperMotorCalibration, "StepMotCalib", 3048, NULL, tskIDLE_PRIORITY + 1, &handlerTask_stepperMotorCalibration, TASK_TO_CORE1));
	configASSERT(xTaskCreatePinnedToCore(stepperMotorMove, "StepMotMov", 3048, NULL, tskIDLE_PRIORITY + 1, &handlerTask_stepperMotorMove, TASK_TO_CORE1));
	
	i2cFrame_transmitQueue tempBuffer;
	i2cFrame_stepper loclaStepperMotorFrame;
	for (;;)
	{
		if (pMotor->QueueReceiveFormI2cParsingTask(&tempBuffer, pdMS_TO_TICKS(100)) == pdTRUE)
		{
			memcpy(&loclaStepperMotorFrame, tempBuffer.pData, sizeof(i2cFrame_stepper));
			pMotor->QueueDeleteDataFormI2cParsingTask(tempBuffer);
			
			switch (loclaStepperMotorFrame.stepperData.stepperSubcommand)
			{
			case MOTOR_SUBCOMMAND_CALIBRATION:			//0x00
				pMotor->calibrationReset();	
				break;
			case MOTOR_SUBCOMMAND_GOTO_ABSOLUT:		//0x01
				pMotor->volatileDestinationBy_GotoAbsolutRange(loclaStepperMotorFrame.stepperData.stepperUnion.gotoPosition);
				break;
			case MOTOR_SUBCOMMAND_GOTO_IN_BOARDERS:	//0x02
				pMotor->volatileDestinationBy_GotoBoardertRange(loclaStepperMotorFrame.stepperData.stepperUnion.gotoPosition);
				break;
			case MOTOR_SUBCOMMAND_MOVE_BY_ABSOLUT:		//0x03
				pMotor->volatileDestinationBy_MoveByAbsoluteRange(loclaStepperMotorFrame.stepperData.stepperUnion.moveBy);	
				break;
			case MOTOR_SUBCOMMAND_MOVE_BY_BOARDER:		//0x04
				pMotor->volatileDestinationBy_MoveByBoarderRange(loclaStepperMotorFrame.stepperData.stepperUnion.moveBy);	
				break;
			case MOTOR_SUBCOMMAND_PERCENTS_ABSOLUT:	//0x05
				pMotor->volatileDestinationBy_PercentageAbsoluteRange(loclaStepperMotorFrame.stepperData.stepperUnion.percents);	
				break;
			case MOTOR_SUBCOMMAND_PERCENTS_BOARDER:	//0x06
				pMotor->volatileDestinationBy_PercentageBoarderRange(loclaStepperMotorFrame.stepperData.stepperUnion.percents);
				break;
			case MOTOR_SUBCOMMAND_POWER_OFF:		//0x07
				pMotor->radioPowerOffInSecond(loclaStepperMotorFrame.stepperData.stepperUnion.radioRestartCountdown);
				break;
			default:
				assert(0);
			}	
		}
		else
		{
			//sprawdza czy praca tasku zosta³¹ zakoñczona
			if (
				eTaskGetState(handlerTask_stepperMotorCalibration) == eSuspended && pMotor->isCalibrated() &&
				eTaskGetState(handlerTask_stepperMotorMove) == eSuspended && pMotor->isPositionReached()) {
				//na wszelki wypadek sprawdza czy w kolejce do tasku nie ma ¿adnych nowych danych
				if (pMotor->QueueMessagesWaiting() == 0)
				{
					extern TaskHandle_t handlerTask_stepperMotorDataPasrser;
					//vTaskSuspend(NULL);
					//				vTaskSuspend(handlerTask_stepperMotorDataPasrser);
				}
			}
		}

		
		
		//sprawdza czy slider jest skalibrowany (jeœli NIE TO:)
		if (pMotor->isCalibrated() == pdFALSE)
		{
			// SprawdŸ, czy task kalibracji nie jest zawieszony															
			if (eTaskGetState(handlerTask_stepperMotorCalibration) == eSuspended)
			{
				vTaskResume(handlerTask_stepperMotorCalibration);
			}
		}
		else //((pMotor->isCalibrated() == pdTRUE)
		{
			if (eTaskGetState(handlerTask_stepperMotorCalibration) != eSuspended)
			{
				vTaskSuspend(handlerTask_stepperMotorCalibration);
			}
			else if (!pMotor->isPositionReached() && eTaskGetState(handlerTask_stepperMotorMove) == eSuspended)
			{
				vTaskResume(handlerTask_stepperMotorMove);
			}
			else if (pMotor->isPositionReached() && (eTaskGetState(handlerTask_stepperMotorMove) != eSuspended))
			{
				vTaskSuspend(handlerTask_stepperMotorMove);
			}		
		}
	}
}