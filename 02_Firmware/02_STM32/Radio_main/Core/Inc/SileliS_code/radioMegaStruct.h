/*
 * radioMegaStruct.h
 *
 *  Created on: May 5, 2025
 *      Author: dbank
 */

#ifndef INC_SILELIS_CODE_RADIOMEGASTRUCT_H_
#define INC_SILELIS_CODE_RADIOMEGASTRUCT_H_
#include "comunicationStructures.h"
#include "SileliS_code/keyboardToFunction.h"

/*
Radio FM (Frequency Modulation)
Region					Zakres częstotliwości							Skok strojenia
Europa (w tym Polska)	87 500 000 Hz – 108 000 000 Hz (87,5–108 MHz)	100 000 Hz (100 kHz)
Ameryka Północna		88 000 000 Hz – 108 000 000 Hz (88–108 MHz)		200 000 Hz (200 kHz)
Japonia					76 000 000 Hz – 95 000 000 Hz (76–95 MHz)		100 000 Hz (100 kHz)
Rosja (dawniej OIRT)	65 500 000 Hz – 74 000 000 Hz (65,5–74 MHz)		100 000 Hz (100 kHz)

Radio AM (Amplitude Modulation)
Region					Zakres częstotliwości							Skok strojenia
Europa (LW/MW)			148 500 Hz – 1 602 000 Hz (148,5 kHz–1,602 MHz)	9 000 Hz (9 kHz)
Ameryka Północna (MW)	530 000 Hz – 1 710 000 Hz (530 kHz–1,71 MHz)	10 000 Hz (10 kHz)
Krótkofalówka (SW)		2 300 000 Hz – 26 100 000 Hz (2,3–26,1 MHz)		5 000 Hz (5 kHz)
Lotnicze (AM)			118 000 000 Hz – 137 000 000 Hz (118–137 MHz)	25 000 Hz (25 kHz)
*/

enum regionFM{
	fm_USA,
	fm_EU,
	fm_Japan
};

enum regionAM{
	am_USA,
	am_EU
};

enum regionDABplus{
	dab_USA,
	dab_EU
};

struct FM_AM_parameters{
	uint32_t minFreq;
	uint32_t maxFreq;
	uint32_t stepFreq;
	uint32_t currentFreq;
};

struct radioMegaStruct{
	struct {
		_execute_t	ExecutableButtonsArray[EXECUTALBE_BUTTONS_ARRAY_SIZE];
	}control;

	struct {
		hmiLeds leds;
		//StepperPositionInPercents stepperPositionInBoardsPercents;
		stepperMotorStruct stepperMotorData;
	}humanMachineInterface;

	struct{
		struct{
			struct{
				enum regionFM region;
				struct FM_AM_parameters tuneParams;
				//bool activeAntena;
				//antena switch pin
			}FM;

			struct{
				enum regionAM region;
				struct FM_AM_parameters tuneParams;
				//bool activeAntena;
				//antena switch pin
			}AM;

			struct{
				//enum regionDABplus region;
				//bool activeAntena;
				//antena switch pin
			}DABplus;
			struct{

				struct {
				    GPIO_TypeDef* port;
				    uint16_t pin;
				    bool activeAntena;
				} activeAntena;

				struct {
				    GPIO_TypeDef* port;
				    uint16_t pin;
				} switchAntena;
			}antena;
		}radioTuner;

	}audioDevices;

	struct{

	}peripheryDevices;
};



#endif /* INC_SILELIS_CODE_RADIOMEGASTRUCT_H_ */
