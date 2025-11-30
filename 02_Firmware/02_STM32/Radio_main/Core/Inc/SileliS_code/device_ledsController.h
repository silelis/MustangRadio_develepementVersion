/*
 * ledsController.h
 *
 *  Created on: May 14, 2025
 *      Author: dbank
 */
#ifndef LEDSCONTROLLER_H_
#define LEDSCONTROLLER_H_

#include <SileliS_code/menu_radioMegaStruct.h>
#include "comunicationStructures.h"
#include "comunicationProtocol.h"
#include "i2c_slave_master_queueClass.h"
#include "comunication_calculate_checksum.h"

#define COLOR_RED 		{128, 0, 0}
#define COLOR_GREEN 	{0, 128, 0}
#define COLOR_BLUE 		{0, 0, 128}
#define COLOR_BLACK 	{0, 0, 0}
#define COLOR_WHITE 	{128, 128, 128}

class ledsController {
public:

	ledsController(hmiLeds *leds,
			i2cQueue4DynamicData *MasterTransmitToSlave_DataQueue);
	virtual ~ledsController();

	void setLedSourceBlinking(ws2812Color primary, ws2812Color secondary);
	void setLedSourceWithColor(ws2812Color color);
	void setLedSourceCleaned(void);

	void setLedEqualiserBlinking(ws2812Color primary, ws2812Color secondary);
	void setLedEqualiserWithColor(ws2812Color color);
	void setLedEqualiserCleaned(void);

	void setLedErrorBlinking(ws2812Color primary, ws2812Color secondary);
	void setLedErrorWithColor(ws2812Color color);
	void setLedErrorCleaned(void);

	//   void setLedBacklightAllBlinking(ws2812Color primary, ws2812Color secondary);
	//   void setLedBacklightAllWithColor(ws2812Color color);
	//   void setLedBacklightInRangeBlinking(ws2812Color primary, ws2812Color secondary, uint8_t from, uint8_t to);
	//   void setLedBacklightInRangeWithColor(ws2812Color color, uint8_t from, uint8_t to);
	//   void setLedBacklighCleaned(void);

	void setLedAllCleaned(void);

	BaseType_t sendDataToI2cTransmitQueue();

protected:
	enum ledEnum {
		sourceLed, equaliserLed, errorLed //,
	//backlightLeds
	};
	void setLedBlinking(ledEnum whichLed, ws2812Color primary,
			ws2812Color secondary);
	void setLedWithColor(ledEnum whichLed, ws2812Color color);
	void setLedCleaned(ledEnum whichLed);

private:

	hmiLeds *pLeds;

	void setLedColors(ledEnum whichLed, ws2812Color primary,
			ws2812Color secondary);
	i2cQueue4DynamicData *pI2C_MasterTransmitToSlave_DataQueue;

};
#endif /* LEDSCONTROLLER_H_ */
