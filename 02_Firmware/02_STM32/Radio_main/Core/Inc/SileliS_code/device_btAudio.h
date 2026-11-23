/*
 * device_btAudio.h
 *
 *  Created on: Nov 23, 2026
 *      Author: dbank
 */

#ifndef INC_SILELIS_CODE_DEVICE_BTAUDIO_H_
#define INC_SILELIS_CODE_DEVICE_BTAUDIO_H_

#include <SileliS_code/menu_radioMegaStruct.h>
#include "comunicationProtocol.h"

class btAudioDevice {
public:
	btAudioDevice(btAudioState *btAudioStateData);	//btAudioStateData - wskaźnik do radioStruct.audioDevices.bluetooth (tasksFunctions.cpp)
	virtual ~btAudioDevice();

	void processReceivedData(i2cFrame_btAudio *btAudioFrame);	//wywoływana w locie przez parser (parserFunction::btAudioToBtAudioDevice) - tylko zapis stanu i log, nic blokującego; pData ramki zwalnia task parsera

	//KONWENCJA NAZW:
	// - is...()  - zwraca OD RAZU stan z lokalnej struktury (radioStruct.audioDevices.bluetooth) = ostatni stan przysłany przez ESP32, bez komunikacji I2C,
	// - get...() - (do zrobienia, 4.2 / krok 2) TYLKO wysyła żądanie do ESP32 (zwraca BaseType_t - czy ramka trafiła do kolejki TX), NIE zwraca wartości;
	//              odpowiedź przychodzi asynchronicznie przez parser -> processReceivedData() -> struktura, odczyt przez is...(); sposób potwierdzenia, że odpowiedź już przyszła - do ustalenia
	bool isDeviceOn(void);
	bt_onConnCallbackStats isConnectionState(void);
	bt_onAVRCPlayStatusCallback isPlayStatus(void);
	i2sPinStates isI2sPinsState(void);

protected:

private:
	btAudioState *pBtAudioState;	//stan BT w radioStruct.audioDevices.bluetooth - pisze WYŁĄCZNIE ta klasa
	const char *TAG = "BT audio device log:";
};

#endif /* INC_SILELIS_CODE_DEVICE_BTAUDIO_H_ */
