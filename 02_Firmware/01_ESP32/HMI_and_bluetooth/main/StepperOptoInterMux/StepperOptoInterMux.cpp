#include "StepperOptoInterMux.h"


StepperOptoInterMux::StepperOptoInterMux()
	: MCP23008(MCP23008_I2C_DEVICE_OPCODE, I2C_MASTER_PIN_SDA, I2C_MASTER_PIN_SCL, I2C_MASTER_SPEED, I2C_MASTER_RX_BEFFER, I2C_MASTER_TX_BEFFER)
{
	this->writeIODIR(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK);
	this->writeIPOL(0b0);
	this->writeGPINTEN(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK);		//interrupt pin is not supported
	this->writeINTCON(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK);		//interrupt pin is not supported
	this->writeDEFVAL(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK);		//interrupt pin is not supported
	this->writeIOCON(0b00110000);															//interrupt pin is not supported
	this->writeGPPU(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK);		//only inputs pulled-up
	
	this->writeOLAT(0b00000000); //all GPIOs down to save energy
	this->maxPosition = 0;
	
	//this->calibrationErrorCounter = 0; //todo:zmieniæ status tego zale¿nie od odczytanej waroœci this->maxPosition o ile odczyta to 0 a jak nie ma tej waroœci this->maxPosition to +1
}

StepperOptoInterMux::~StepperOptoInterMux()
{
	this->writeOLAT(0b00000000); //all GPIOs down to save energy
	this->writeGPPU(0b00000000);		//pull-down enabled to save energy
}

/*
esp_err_t StepperOptoInterMux::enableESP32InterruptNotification()
{
	uint8_t data = this->readOLAT();
	data |= INTERRUP_REQUEST_MASK;					//set bit ONE
	return this->writeOLAT(data);	
}*/

/*
esp_err_t StepperOptoInterMux::disableESP32InterruptNotification()
{
	uint8_t data = this->readOLAT();
	//data ^= INTERRUP_REQUEST_MASK;
	data &= ~INTERRUP_REQUEST_MASK; //set bit ZERO
	return this->writeOLAT(data);
}*/

/*
esp_err_t StepperOptoInterMux::enableMuxSelect()
{
	uint8_t data = this->readOLAT();
	data |= MUX_SELECT_MASK;						//set bit ONE
	return this->writeOLAT(data);
}*/
	
/*
esp_err_t StepperOptoInterMux::disableMuxSelect()
{
	uint8_t data = this->readOLAT();
	//data ^= MUX_SELECT_MASK;
	data &= ~MUX_SELECT_MASK; //set bit ZERO
	return this->writeOLAT(data);
}*/


esp_err_t StepperOptoInterMux::enableStepperMotor()
{
	uint8_t data = this->readOLAT();
	data |= MOTOR_NOT_SLEEP_MASK;						//set bit ONE
	esp_err_t retVal = this->writeOLAT(data);
	vTaskDelay(pdMS_TO_TICKS(20));
	this->isStepperDriverEnabled = pdTRUE;
	return retVal;
}

esp_err_t StepperOptoInterMux::disableStepperMotor()
{
	uint8_t data = this->readOLAT();
	//data ^= MOTOR_NOT_SLEEP_MASK;
	data &= ~MOTOR_NOT_SLEEP_MASK; //set bit ZERO
	this->isStepperDriverEnabled = pdFALSE;
	return this->writeOLAT(data);
}


uint8_t  StepperOptoInterMux::readInputs()
{
	return this->readGPIO()&(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK);
}


void StepperOptoInterMux::measureSliderRange()
{
	uint8_t inputsStates;
	//uint16_t measuredMaxPosition = 0;
	enum Calibration_state
	{
		stepper_motor_calibrated_NONE,
		stepper_motor_calibrated_LEFT,
		stepper_motor_calibrated_RIGHT,
		//stepper_motor_calibrated_FINISHED
	};
	Calibration_state calibrationState = stepper_motor_calibrated_NONE;
	
	this->enableStepperMotor();
	this->setDirection(MOVE_BACKWARD);
	while (1)
	{
		//switch ((uint8_t) calibrationState)
		//{
		if (calibrationState == stepper_motor_calibrated_NONE) 
		{
			inputsStates = this->readInputs();
			
			if (inputsStates & SENSOR_VOL_SIDE_MASK)
			{
				this->makeStep();
				//this->currentPosition--;
			}
			else
			{
				this->currentPosition = 0;
				calibrationState = stepper_motor_calibrated_LEFT;
				this->setDirection(MOVE_FORWARD);
			}
		}
		if (calibrationState == stepper_motor_calibrated_LEFT)
		{
			inputsStates = this->readInputs();
			if (inputsStates & SENSOR_EQU_SIDE_MASK)
			{
				this->makeStep();
				this->currentPosition++;
			}
			else
			{
				calibrationState = stepper_motor_calibrated_RIGHT;
				this->setDirection(MOVE_BACKWARD);
				//this->currentPosition = 0;
			}
		}
		if (calibrationState == stepper_motor_calibrated_RIGHT) 
		{
			this->maxPosition = this->currentPosition;
			this->disableStepperMotor();
			break;
		} 
	}

}

esp_err_t StepperOptoInterMux::setDirection(bool direction)
{
	uint8_t data = this->readOLAT();
	switch ((uint8_t)direction)
	{
	case MOVE_BACKWARD:
		data &= ~MOTOR_DIRECTION_MAKS;		//set bit ZERO
		this->whichDirection = MOVE_BACKWARD;
		break;
	case MOVE_FORWARD:
		data |= MOTOR_DIRECTION_MAKS;		//set bit ONE
		this->whichDirection = MOVE_FORWARD;
		break;		
	}
	vTaskDelay(pdMS_TO_TICKS(350));
	return this->writeOLAT(data);
}

esp_err_t StepperOptoInterMux::makeStep()
{
	uint8_t data = this->readOLAT();
	data ^= MOTOR_STEP_MASK;
	vTaskDelay(pdMS_TO_TICKS(8));
	return this->writeOLAT(data);
	
}

void StepperOptoInterMux::moveXSteps(int32_t stepAmount)
{
	uint8_t inputData;
	if ((stepAmount < 0)&&(this->whichDirection != MOVE_BACKWARD))
	{
		this->setDirection(MOVE_BACKWARD);
	}
	if ((stepAmount >0)&&(this->whichDirection != MOVE_FORWARD))
	{
		this->setDirection(MOVE_FORWARD);
	}
	this->enableStepperMotor();
	
	do
	{
		inputData = this->readInputs();
		if (this->whichDirection == MOVE_BACKWARD)
		{
			if (!(inputData & SENSOR_VOL_SIDE_MASK))
			{
				stepAmount = 0; //osi¹gniêto lewy czujnik, nie mo¿na dalej iœc do ty³u
				this->currentPosition = 0; //zerowanie pozycji na wszelki wypadek
			}
			else
			{
				this->makeStep();
				this->currentPosition--;
				stepAmount++;
			}
		}
		else if (this->whichDirection == MOVE_FORWARD)
		{
			if (!(inputData & SENSOR_EQU_SIDE_MASK))
			{
				stepAmount = 0;
				this->currentPosition = this->maxPosition;
			}
			else
			{
				this->makeStep();
				this->currentPosition++;
				stepAmount--;
			}
		}
	} while (stepAmount);
	this->disableStepperMotor();
}


void StepperOptoInterMux::moveTo_xPosition(uint16_t xPosition)
{
	int32_t stepAmount;
	if (xPosition < 0)
		xPosition = 0;
	if (xPosition > this->maxPosition)
		xPosition = this->maxPosition;
	
	stepAmount = xPosition - this->currentPosition;
	this->moveXSteps(stepAmount);
}

void StepperOptoInterMux::moveTo_xPercent(float xPercent)
{
	uint16_t xPosition = (uint16_t) std::round(this->maxPosition*(xPercent / 100));
	this->moveTo_xPosition(xPosition);
}