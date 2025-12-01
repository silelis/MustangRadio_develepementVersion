/*
 * menu_menuPrototype.h
 *
 *  Created on: Nov 29, 2025
 *      Author: dbank
 */

#ifndef INC_SILELIS_CODE_MENU_MENUPROTOTYPE_H_
#define INC_SILELIS_CODE_MENU_MENUPROTOTYPE_H_

#include <SileliS_code/menu_bindKeyboardToFunction.h>
#include "SileliS_code/myPrintfTask.h"
#include "comunicationStructures.h"

class menuPrototype: protected bindKeyboardToFunction {
public:
	menuPrototype();
	~menuPrototype();
	virtual void deviceInit();
	virtual void deviceDeInit();
	virtual void deviceBackInit();

protected:
	virtual void deviceOn(void);
	virtual void deviceOff(void);


private:
    struct HumanMachineInterfaceStruct {
        hmiLeds leds;
        static bool backlight;
        stepperMotorStruct stepperMotorData;
    };
    HumanMachineInterfaceStruct humanMachineInterface;
};

#endif /* INC_SILELIS_CODE_MENU_MENUPROTOTYPE_H_ */
