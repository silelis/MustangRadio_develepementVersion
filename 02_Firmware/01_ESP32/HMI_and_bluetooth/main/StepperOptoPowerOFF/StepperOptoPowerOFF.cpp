#include "StepperOptoPowerOFF.h"

/*---------------------------------------------------------------
 * Konstruktor klasy obsługującej silnik krokowy i jego krańcówki.
 * Parameters:
 * MCP23008* p_MCP23008 - obiekt sterujący pracą ekspandera GPIO
 *						typu MCP23008
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
StepperOptoPowerOFF::StepperOptoPowerOFF(MCP23008* pointer_MCP23008)
	//: MCP23008(MCP23008_I2C_DEVICE_OPCODE, I2C_MASTER_PIN_SDA, I2C_MASTER_PIN_SCL, I2C_MASTER_SPEED, I2C_MASTER_RX_BEFFER, I2C_MASTER_TX_BEFFER)
{
	configASSERT(this->ParserDataToStepperMotorDataQueue = new i2cQueue4DynamicData(4));
	
	pMCP23008 = pointer_MCP23008; 
	this->pMCP23008->writeIODIR(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK);
	this->pMCP23008->writeIPOL(0b0);
	this->pMCP23008->writeGPINTEN(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK); //interrupt pin is not supported
	this->pMCP23008->writeINTCON(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK); //interrupt pin is not supported
	this->pMCP23008->writeDEFVAL(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK); //interrupt pin is not supported
	this->pMCP23008->writeIOCON(0b00110000); //interrupt pin is not supported
	this->pMCP23008->writeGPPU(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK); //only inputs pulled-up
	
	this->pMCP23008->writeOLAT(0b00000000); //all GPIOs down to save energy
	this->motorParameters.maxPosition = 0;
	this->calibrationReset();
	
	//this->calibrationErrorCounter = 0; //todo:zmieni  status tego zale nie od odczytanej waro ci this->maxPosition o ile odczyta to 0 a jak nie ma tej waro ci this->maxPosition to +1
}

/*---------------------------------------------------------------
 * Destruktor klasy obsługującej silnik krokowy i jego krańcówki.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
StepperOptoPowerOFF::~StepperOptoPowerOFF()
{
	this->pMCP23008->writeOLAT(0b00000000); //all GPIOs down to save energy
	this->pMCP23008->writeGPPU(0b00000000); //pull-down enabled to save energy
	delete this->ParserDataToStepperMotorDataQueue;
}

/*---------------------------------------------------------------
 * Metoda ustawia stan włączający na sterowniku silnika krokowego
 * przez co na cewkach pojawia się zasilanie (silnik się grzeje
 * dlatego zalecane jest, aby po skończonej jeździe silnika
 * wyłaczyć te zasilanie).
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t retVal - 0x00 = OK
 *---------------------------------------------------------------*/
esp_err_t StepperOptoPowerOFF::enableStepperMotor()
{
	uint8_t data = this->pMCP23008->readOLAT();
	data |= MOTOR_NOT_SLEEP_MASK;						//set bit ONE
	esp_err_t retVal = this->pMCP23008->writeOLAT(data);
	vTaskDelay(pdMS_TO_TICKS(20));
	this->isStepperDriverEnabled = pdTRUE;
	return retVal;
}

/*---------------------------------------------------------------
 * Metoda ustawia stan wyłączający na sterowniku silnika krokowego
 * przez co na cewkach znika zasilanie (jest to wymagane, aby
 * silnik się nie grzał).
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t retVal - informacja o poprawności wykonania operacji
 *           (0x00 = OK)
 *---------------------------------------------------------------*/
esp_err_t StepperOptoPowerOFF::disableStepperMotor()
{
	uint8_t data = this->pMCP23008->readOLAT();
	//data ^= MOTOR_NOT_SLEEP_MASK;
	data &= ~MOTOR_NOT_SLEEP_MASK; //set bit ZERO
	this->isStepperDriverEnabled = pdFALSE;
	esp_err_t retVal = this->pMCP23008->writeOLAT(data);
	vTaskDelay(pdMS_TO_TICKS(20));
	return retVal;
}

/*---------------------------------------------------------------
 * Metoda czyta stany logiczne na czujnikach krańcówki i wyjściu
 * sterownika silników krokowych sygnalizującego błąd.
 * Parameters:
 * NONE
 * Returns:
 * uint8_t value - stan bitów (wejść) SENSOR_EQU_SIDE_MASK,
 *         SENSOR_VOL_SIDE_MASK, MOTOR_FAULT_MASK na ekspanderze
 *         MCP2300
 *---------------------------------------------------------------*/
uint8_t  StepperOptoPowerOFF::readInputs()
{
	return this->pMCP23008->readGPIO()&(SENSOR_EQU_SIDE_MASK | SENSOR_VOL_SIDE_MASK | MOTOR_FAULT_MASK);
}

/*---------------------------------------------------------------
 * Metoda dokonuje "kalibracji" suwaka silnika krokowego, tzn.
 * wylicza ile kroków jest od jednego położenia krańcowego do
 * drugiego (jest to konieczne ze względu na fakt, że lutowanie
 * czujiników krańcowych ma swoją tolerancję pozycjonowania.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void StepperOptoPowerOFF::measureSliderRange(uint16_t begginOffset, uint16_t endOffset)
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
				this->motorParameters.currentPosition = 0;
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
				this->motorParameters.currentPosition++;
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
			this->motorParameters.maxPosition = this->motorParameters.currentPosition;
			this->disableStepperMotor();
			break;
		} 
	}
	this->motorParameters.beginOffest = begginOffset;
	if (endOffset==UINT16_MAX)
	{
		this->motorParameters.endOffset = this->motorParameters.maxPosition;
	}
	else
	{
		this->motorParameters.endOffset = endOffset;
	}
	//vTaskDelay(1000);
	this->calibrationSet();
}

/*---------------------------------------------------------------
 * Metoda ustawiakierunek ruchu karetki silnika krokowego.
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t retVal - informacja o poprawności wykonania operacji
 *           (0x00 = OK)
 *---------------------------------------------------------------*/
esp_err_t StepperOptoPowerOFF::setDirection(bool direction)
{
	uint8_t data = this->pMCP23008->readOLAT();
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
	//vTaskDelay(pdMS_TO_TICKS(350));
	esp_err_t retVal = this->pMCP23008->writeOLAT(data);
	vTaskDelay(pdMS_TO_TICKS(350));
	return retVal;
}

/*---------------------------------------------------------------
 * Robi jeden krok silnika krokowego.
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t retVal - informacja o poprawności wykonania operacji
 *           (0x00 = OK)
 *---------------------------------------------------------------*/
esp_err_t StepperOptoPowerOFF::makeStep()
{
	uint8_t data = this->pMCP23008->readOLAT();
	data ^= MOTOR_STEP_MASK;
	vTaskDelay(pdMS_TO_TICKS(1));
	return this->pMCP23008->writeOLAT(data);
	
}

/*---------------------------------------------------------------
 * Robi n kroków silnika krokowego.
 * Parameters:
 * int32_t stepAmount - ilość kroków jakie silnik ma wykonać.
 *             + / - ma znaczenie i określa ilość kierunek ruchu.
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void StepperOptoPowerOFF::moveXSteps(int32_t stepAmount)
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
			if ((inputData & SENSOR_VOL_SIDE_MASK))
			{
				stepAmount = 0; //osi gni to lewy czujnik, nie mo na dalej i c do ty u
				this->motorParameters.currentPosition = 0; //zerowanie pozycji na wszelki wypadek
			}
			else
			{
				this->makeStep();
				this->motorParameters.currentPosition--;
				stepAmount++;
			}
		}
		else if (this->whichDirection == MOVE_FORWARD)
		{
			if ((inputData & SENSOR_EQU_SIDE_MASK))
			{
				stepAmount = 0;
				this->motorParameters.currentPosition = this->motorParameters.maxPosition;
			}
			else
			{
				this->makeStep();
				this->motorParameters.currentPosition++;
				stepAmount--;
			}
		}
	} while (stepAmount);
	this->disableStepperMotor();
}

/*---------------------------------------------------------------
 * Przesuwa karetkę silnika krokowego do określonej pozycji w
 * zakresie <0 motorParameters.maxPosition>
 * Parameters:
 * uint16_t xPosition - docelowa pozycja karetki silnika krokowego
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void StepperOptoPowerOFF::moveTo_xPosition(uint16_t xPosition)
{
	int32_t stepAmount;
	if (xPosition < 0)
		xPosition = 0;
	if (xPosition > this->motorParameters.maxPosition)
		xPosition = this->motorParameters.maxPosition;
	
	#warning "Sprawdzić czy rzutowanie typów działa poprawnie"
	stepAmount = (int32_t)xPosition - (int32_t)this->motorParameters.currentPosition;
	this->moveXSteps(stepAmount);
}

void StepperOptoPowerOFF::moveToVolatileDestinationPosition(void)
{
	
	uint8_t lastDirection = 3;
	this->enableStepperMotor();
	uint8_t inputsStates;
	for (;;)
	{
		
		{
			//określa w którym kierunku powinien poruszać się motor - w kierunku ujemnym
			if (this->motorParameters.volatileDestinationPosition < this->motorParameters.currentPosition)
			{
				if (lastDirection != MOVE_BACKWARD)
				{
					lastDirection = MOVE_BACKWARD;
					this->setDirection(MOVE_BACKWARD);
				}
			}
			//określa w którym kierunku powinien poruszać się motor - w kierunku dodatnim
			if (this->motorParameters.volatileDestinationPosition > this->motorParameters.currentPosition)
			{
				if (lastDirection != MOVE_FORWARD)
				{
					lastDirection = MOVE_FORWARD;
					this->setDirection(MOVE_FORWARD);
				}
			}	
			
		}

		
		{
			//czyta wejscia i sprawdza czy krańcówki nie zostały osignięte
			inputsStates = this->readInputs();
			
			//jeśli osiągnięto 0 to koryguje wartość obecnej pozycji na 0
			if (!(inputsStates & SENSOR_VOL_SIDE_MASK))
			{
				this->motorParameters.currentPosition = 0;
			}
			
			//jeśli osiagnieto max to koryguje warośc obecnej pozycji na max
			if (!(inputsStates & SENSOR_EQU_SIDE_MASK))
			{
				this->motorParameters.currentPosition =  this->motorParameters.maxPosition;
			}
		}
		
		 	

		 //osprawdza czy motor osiągnął zadaną pozycję
		if (this->motorParameters.currentPosition != this->motorParameters.volatileDestinationPosition)
		{
			switch (this->whichDirection)
			{
			case MOVE_BACKWARD:
				this->motorParameters.currentPosition--;
				break;
			case MOVE_FORWARD:
				this->motorParameters.currentPosition++;
				break;
			}
			this->makeStep();
		
		}
		//jeśli pozycja osiągnięta to wyłącza steronik silnika krokowego
		else
		{
			this->disableStepperMotor();
		}
	}
}


BaseType_t StepperOptoPowerOFF::isPositionReached(void)
{
	if (this->motorParameters.volatileDestinationPosition == this->motorParameters.currentPosition)
	{
		this->disableStepperMotor();
		return pdTRUE;
	}
	else
	{
		return pdFALSE;		
	}
}


/*---------------------------------------------------------------
 * Przesuwa karetkę silnika krokowego do pozycji x%, mierzonej w
 * zakresie od 0 (to 0%) do motorParameters.maxPosition(to 100%),
 * gdzie np. float 51.1 oznacza 51.1% (wartość z float na % jest
 * przeliczana w metodzie).
 * Parameters:
 * float xPercent - wartość na jaką ma być ustawiony slider (float
 *         51,1 to 51,1% - przeliczenie na % odbywa się w metodzie). 
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void StepperOptoPowerOFF::moveTo_xPercent(float xPercent)
{
	uint16_t xPosition = (uint16_t) std::round(this->motorParameters.maxPosition*(xPercent / 100));
	this->moveTo_xPosition(xPosition);
}

/*---------------------------------------------------------------
 * Przesuwa karetkę silnika krokowego do pozycji x%, mierzonej w
 * zakresie od this->motorParameters.beginOffest (to 0%) do 
 * this->motorParameters.endOffset (to 100%), gdzie np. float 51.1
 * oznacza 51.1% (wartość z float na % jest przeliczana w metodzie).
 * Parameters:
 * float xPercent - wartość na jaką ma być ustawiony slider (float
 *         51,1 to 51,1% - przeliczenie na % odbywa się w metodzie). 
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void StepperOptoPowerOFF::moveTo_xPercentIncludeOffset(float xPercent)
{
	uint16_t xPosition = (uint16_t) std::round((this->motorParameters.endOffset - this->motorParameters.beginOffest)*(xPercent / 100));
	xPosition = xPosition + this->motorParameters.beginOffest;
	this->moveTo_xPosition(xPosition);
}

/*---------------------------------------------------------------
 * Metoda zwraca wartość motorParameters.
 * Parameters:
 * NONE
 * Returns:
 * MotorParameters motorParameters - zwraca wartośc całej struktury.
 *---------------------------------------------------------------*/
MotorParameters StepperOptoPowerOFF::getValue_motorParameters()
{
	return this->motorParameters;
}

/*---------------------------------------------------------------
 * Metoda ustawia wartość motorParameters. Ponieważ w algorytnie
 * nie ma zaimplementowanych operacji matematycznych (+, -, *, /,
 * =, ==) dla typu danych MotorParameters, to przypisanie wartości
 * odnywa się poprzez kopiowanie pamięci.
 * Parameters:
 * const void *src - adres pamieci zawierający dane (wartość
 *          struktury) jaka ma być wpisana do this->motorParameters.
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void StepperOptoPowerOFF::setValue_motorParameters(const void *src)
{
	memcpy(&this->motorParameters, src, sizeof(this->motorParameters));
}

void StepperOptoPowerOFF::radioPowerOFF(void)
{
	uint8_t data = this->pMCP23008->readOLAT();
	data |= POWER_SUPPLU_SUSTAIN_MASK; //set bit ONE
	/*return*/ this->pMCP23008->writeOLAT(data);
}

void StepperOptoPowerOFF::calibrationSet(void)
{
	this->motorParameters.isCalibrated = pdTRUE;
}

void StepperOptoPowerOFF::calibrationReset(void)
{
	this->motorParameters.isCalibrated = pdFALSE;
}

BaseType_t StepperOptoPowerOFF::isCalibrated(void)
{
	return this->motorParameters.isCalibrated;
}



BaseType_t StepperOptoPowerOFF::QueueSendDataToMotorDataQueue(i2cFrame_transmitQueue * pvItemToQueue)
{
	return this->ParserDataToStepperMotorDataQueue->QueueSend(pvItemToQueue);
}
	
BaseType_t StepperOptoPowerOFF::QueueReceiveFormI2cParsingTask(i2cFrame_transmitQueue* pvBuffer, TickType_t xTicksToWait)
{
	return this->ParserDataToStepperMotorDataQueue->QueueReceive(pvBuffer, xTicksToWait);
}

void StepperOptoPowerOFF::QueueDeleteDataFormI2cParsingTask(i2cFrame_transmitQueue structWithPointer)
{
	this->ParserDataToStepperMotorDataQueue->QueueDeleteDataFromPointer(structWithPointer);	
}