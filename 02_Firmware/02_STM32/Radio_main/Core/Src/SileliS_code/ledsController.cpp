/*
 * ledsController.cpp
 *
 *  Created on: May 14, 2025
 *      Author: dbank
 */

#include "SileliS_code/ledsController.h"


ledsController::ledsController(hmiLeds* leds, i2cQueue4DynamicData* MasterTransmitToSlave_DataQueue) {
	// TODO Auto-generated constructor stub
	this->pLeds = leds;
	this->pI2C_MasterTransmitToSlave_DataQueue = MasterTransmitToSlave_DataQueue;
//	this->setLedBlinking(sourceLed, this->color.Red, this->color.Black);
	this->setLedAllCleaned();
}

void ledsController::setLedColors(ledEnum whichLed, ws2812Color primary, ws2812Color secondary){
	colorSet*pSetLed;
	switch (whichLed) {
	        case sourceLed:
	        	pSetLed=&this->pLeds->sourceLed;
	            break;
	        case equaliserLed:
	        	pSetLed=&this->pLeds->equaliserLed;
	        	break;
	        case errorLed:
	        	pSetLed=&this->pLeds->errorLed;
	            break;
//	        case backlightLeds:
//	        	pSetLed=&this->pLeds->backlightLeds;
//	            break;
	    }
	pSetLed->primary=primary;
	pSetLed->secondary=secondary;
}

//OK
void ledsController::setLedBlinking(ledEnum whichLed, ws2812Color primary, ws2812Color secondary){
	this->setLedColors(whichLed, primary, secondary);
}

//OK
void ledsController::setLedWithColor(ledEnum whichLed, ws2812Color color){
	this->setLedColors(whichLed, color, color);
}

//OK
void ledsController::setLedCleaned(ledEnum whichLed){
	this->setLedColors(whichLed, COLOR_BLACK, COLOR_BLACK);
}


//OK
void ledsController::setLedSourceBlinking(ws2812Color primary, ws2812Color secondary){
	this->setLedBlinking(sourceLed, primary, secondary);
}

//OK
void ledsController::setLedSourceWithColor(ws2812Color color){
	this->setLedSourceBlinking(color, color);
}

//OK
void ledsController::setLedSourceCleaned(void){
	this->setLedSourceWithColor(COLOR_BLACK);
}

//OK
void ledsController::setLedEqualiserBlinking(ws2812Color primary, ws2812Color secondary){
	this->setLedBlinking(equaliserLed, primary, secondary);
}

//OK
void ledsController::setLedEqualiserWithColor(ws2812Color color){
	this->setLedEqualiserBlinking(color, color);
}

//OK
void ledsController::setLedEqualiserCleaned(void){
	this->setLedEqualiserWithColor(COLOR_BLACK);
}

//OK
void ledsController::setLedErrorBlinking(ws2812Color primary, ws2812Color secondary){
	this->setLedBlinking(errorLed, primary, secondary);
}

//OK
void ledsController::setLedErrorWithColor(ws2812Color color){
	this->setLedErrorBlinking(color, color);
}

//OK
void ledsController::setLedErrorCleaned(void){
	this->setLedErrorWithColor(COLOR_BLACK);
}

/*
void ledsController::setLedBacklightAllBlinking(ws2812Color primary, ws2812Color secondary){
	this->setLedBacklightInRangeBlinking(primary, secondary, BACKLIGHT_LED_FROM, BACKLIGHT_LED_TO);
}*/

/*
void ledsController::setLedBacklightAllWithColor(ws2812Color color){
	this->setLedBacklightInRangeWithColor(color, BACKLIGHT_LED_FROM, BACKLIGHT_LED_TO);
}*/


/*
void ledsController::setLedBacklightInRangeBlinking(ws2812Color primary, ws2812Color secondary, uint8_t from, uint8_t to){
	this->setLedBlinking(backlightLeds, primary, secondary);
	if (from>to){
		uint8_t tempFrom;
		uint8_t tempTo;

		tempFrom=to;
		tempTo=from;

		from=tempFrom;
		to=tempTo;
	}
	if (from<BACKLIGHT_LED_FROM){
		from = BACKLIGHT_LED_FROM;
	}
	if (to>BACKLIGHT_LED_TO){
		to = BACKLIGHT_LED_TO;
	}
	pLeds->backlightLedsFrom=from;
	pLeds->backlightLedsTo=to;
}*/

/*
void ledsController::setLedBacklightInRangeWithColor(ws2812Color color, uint8_t from, uint8_t to){
	this->setLedBacklightInRangeBlinking(color, color, from, to);
}*/

/*
void ledsController::setLedBacklighCleaned(void){
	this->setLedBacklightAllWithColor(COLOR_BLACK);
}*/

void ledsController::setLedAllCleaned(void){
	this->setLedSourceCleaned();
	this->setLedEqualiserCleaned();
	this->setLedErrorCleaned();
//	this->setLedBacklighCleaned();
}
BaseType_t ledsController::sendDataToI2cTransmitQueue(){

		//tworzy ramke do przesłania
		i2cFrame_hmiLeds hmiLedsToSend;
		hmiLedsToSend.i2cframeCommandHeader.commandGroup = I2C_COMMAND_GROUP_LEDS;
		hmiLedsToSend.i2cframeCommandHeader.dataSize = sizeof(i2cFrame_hmiLeds);
		//kopiuje dane z radioMegaStruct do ramki
		memcpy(&hmiLedsToSend.ledsData, this->pLeds, sizeof(hmiLeds));
		hmiLedsToSend.i2cframeCommandHeader.crcSum = (uint8_t) calculate_checksum(&hmiLedsToSend, hmiLedsToSend.i2cframeCommandHeader.dataSize/*sizeof(i2cFrame_hmiLeds)*/);
		//tworzy ramkę komunikacujną do przesłania
		i2cFrame_transmitQueue dataToTransmitQueue;
		dataToTransmitQueue.pData = new i2cFrame_hmiLeds; //new i2cFrame_hmiLeds[1];//malloc(sizeof(i2cFrame_hmiLeds));
		assert(dataToTransmitQueue.pData);
		dataToTransmitQueue.slaveDevice7bitAddress = I2C_SLAVE_ADDRESS_ESP32;
		dataToTransmitQueue.dataSize = hmiLedsToSend.i2cframeCommandHeader.dataSize/*sizeof(i2cFrame_hmiLeds)*/;
		memcpy(dataToTransmitQueue.pData, &hmiLedsToSend, dataToTransmitQueue.dataSize);
		return this->pI2C_MasterTransmitToSlave_DataQueue->QueueSend(&dataToTransmitQueue);
}



ledsController::~ledsController() {
	// TODO Auto-generated destructor stub
}
