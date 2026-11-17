/*
 * menu_radioMenu_periphery_tipCalibrationLeftRight.h
 *
 *  Created on: 17 lis 2026
 *      Author: dbank
 */

#ifndef INC_SILELIS_CODE_MENU_RADIOMENU_PERIPHERY_TIPCALIBRATIONLEFTRIGHT_H_
#define INC_SILELIS_CODE_MENU_RADIOMENU_PERIPHERY_TIPCALIBRATIONLEFTRIGHT_H_

//kalibracja wskazówki (stepper motor): dwa osobne węzły menu (Left / Right), wspólna część jako funkcje static w .cpp

void radioMenuPeripheryTipCalibrationLeft_Init(void *context);			//context: radioMenu* (z myList::bindCurrent / execCurrent*)
void radioMenuPeripheryTipCalibrationLeft_DestroyInit(void *context);
void radioMenuPeripheryTipCalibrationRight_Init(void *context);
void radioMenuPeripheryTipCalibrationRight_DestroyInit(void *context);
//brak ReturnInit: do peryferium nigdy się nie "wraca" - wejście zawsze Init, wyjście zawsze DestroyInit


#endif /* INC_SILELIS_CODE_MENU_RADIOMENU_PERIPHERY_TIPCALIBRATIONLEFTRIGHT_H_ */
