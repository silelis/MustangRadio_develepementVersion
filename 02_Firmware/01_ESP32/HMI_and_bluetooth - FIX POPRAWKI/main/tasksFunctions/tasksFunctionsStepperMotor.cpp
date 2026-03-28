#include "tasksFunctions/tasksFunctionsStepperMotor.h"
#include "tasksFunctions/tasksFunctionsStorage.h"
#include "tasksFunctions/tasksFunctions.h"		//tutaj jest extern NVS* pSTORAGE; //obiekt zapisuj�cy i czytaj�cy dane z NCS ESP32

StepperOptoPowerOFF * pMotor; //obiekt steruj�cy prac� silnika krokowego, jego kra�c�wej i power off radia

// [FIX: Guru Meditation - xPortEnterCriticalTimeout deadlock]
// Definicja semafora - inicjalizowany w stepperMotorDataParser (Core 1).
SemaphoreHandle_t xMotorInitDone = NULL;

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
	// [FIX: Guru Meditation - xPortEnterCriticalTimeout deadlock]
	// PRZYCZYNA BLEDU: i2c_new_master_bus() wywolywane z app_main (Core 0) przypisywalo
	// ISR magistrali I2C master do Core 0. Task stepperMotorCalibration pinned jest do
	// Core 1 i blokowal sie w xQueueReceive czekajac na ten ISR. Jednoczesnie task
	// i2cSlaveTransmit dzialal na Core 0 trzymajac xKernelLock (spinlock FreeRTOS SMP).
	// ISR mastera na Core 0 probowal zdobyc ten sam xKernelLock -> deadlock ->
	// po ~300ms: Interrupt WDT -> Guru Meditation Error (xPortEnterCriticalTimeout).
	// Objawialo sie jako crash po nacisnieciu klawisza Power ON/OFF (ktory startowal
	// kalibracje stepera).
	//
	// NAPRAWA: i2c_new_master_bus() musi byc wywolane z Core 1, tak aby ISR magistrali
	// I2C master byl przypisany do Core 1 - tego samego rdzenia co taski stepera.
	// Eliminuje to cross-core spinlock contention.
	// Ten task (stepperMotorDataParser) jest teraz pinnedToCore1 (patrz app_main.cpp).
	// Semafor jest tworzony w taskFunctionsStaticHandlersInit() (przed startem taskow)
	// aby uniknac sytuacji gdy i2cReceivedDataParser probuje Take() na NULL handle.
	printf("I2C master bus init (Core 1)\n");
	i2cEngin_master *p_i2cMaster = new i2cEngin_master(I2C_MASTER_PORT, I2C_MASTER_PIN_SDA, I2C_MASTER_PIN_SCL);
	assert(p_i2cMaster);

	printf("MCP23008 on I2C master bus init\n");
	MCP23008* p_MCP23008 = new MCP23008(MCP23008_I2C_DEVICE_OPCODE, p_i2cMaster, I2C_MASTER_SPEED);
	assert(p_MCP23008);

	printf("Stepper motor and powerOFF gpio init\n");
	assert(pMotor = new StepperOptoPowerOFF(p_MCP23008));

	// Sygnalizuj gotowsc pMotor dla innych taskow (np. i2cReceivedDataParser)
	xSemaphoreGive(xMotorInitDone);

	configASSERT(xTaskCreatePinnedToCore(stepperMotorCalibration, "StepMotCalib", 26 * 128, NULL, tskIDLE_PRIORITY + 1, &handlerTask_stepperMotorCalibration, TASK_TO_CORE1));
	configASSERT(xTaskCreatePinnedToCore(stepperMotorMove, "StepMotMov", 26 * 128, NULL, tskIDLE_PRIORITY + 1, &handlerTask_stepperMotorMove, TASK_TO_CORE1));
	
	i2cFrame_transmitQueue tempBuffer;
	i2cFrame_stepper loclaStepperMotorFrame;
	for (;;)
	{
		if (pMotor->QueueReceiveFormI2cParsingTask(&tempBuffer, pdMS_TO_TICKS(10)) == pdTRUE)
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
			//sprawdza czy praca tasku zosta�� zako�czona
			if (
				eTaskGetState(handlerTask_stepperMotorCalibration) == eSuspended && pMotor->isCalibrated() &&
				eTaskGetState(handlerTask_stepperMotorMove) == eSuspended && pMotor->isPositionReached()) {
				//na wszelki wypadek sprawdza czy w kolejce do tasku nie ma �adnych nowych danych
				if (pMotor->QueueMessagesWaiting() == 0)
				{
					extern TaskHandle_t handlerTask_stepperMotorDataPasrser;
					//vTaskSuspend(NULL);
					//				vTaskSuspend(handlerTask_stepperMotorDataPasrser);
				}
			}
		}

		
		
		//sprawdza czy slider jest skalibrowany (je�li NIE TO:)
		if (pMotor->isCalibrated() == pdFALSE)
		{
			// Sprawd�, czy task kalibracji nie jest zawieszony															
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