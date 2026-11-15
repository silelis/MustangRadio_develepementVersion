/*
 * menu_radioMenu_periphery_tipCalibration.h
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#ifndef INC_SILELIS_CODE_MENU_RADIOMENU_PERIPHERY_TIPCALIBRATION_H_
#define INC_SILELIS_CODE_MENU_RADIOMENU_PERIPHERY_TIPCALIBRATION_H_


void radioMenuPeripheryTipCalibration_Init(void *context);			//context: radioMenu* (z myList::bindCurrent / execCurrent*)
void radioMenuPeripheryTipCalibration_DestroyInit(void *context);
//brak ReturnInit: do peryferium nigdy się nie "wraca" - wejście zawsze Init, wyjście zawsze DestroyInit


#endif /* INC_SILELIS_CODE_MENU_RADIOMENU_PERIPHERY_TIPCALIBRATION_H_ */
