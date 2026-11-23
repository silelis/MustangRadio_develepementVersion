/*
 * device_btAudio.cpp
 *
 *  Created on: Nov 23, 2026
 *      Author: dbank
 */

#include <SileliS_code/device_btAudio.h>
#include "SileliS_code/myPrintfTask.h"

extern myPrintfTask *pPrintf;

/********************************************************************
 * @brief  Konstruktor klasy btAudioDevice
 *
 * Klasa btAudioDevice obsługuje po stronie STM32 urządzenie Bluetooth
 * audio, które fizycznie znajduje się w ESP32. Przechowuje stan BT
 * przysłany przez ESP32 (ramki I2C_COMMAND_GROUP_BTAUDIO) w strukturze
 * radioStruct.audioDevices.bluetooth. Konstruktor zapamiętuje wskaźnik
 * do tej struktury i ustawia stan startowy.
 *
 * @param  [btAudioStateData] [btAudioState*] Wskaźnik do struktury
 * 			stanu BT - radioStruct.audioDevices.bluetooth
 * 			(tasksFunctions.cpp).
 *
 * @return [NONE]
 *
 * @note   Stan startowy (do czasu pierwszej ramki z ESP32): BT OFF,
 * 			piny I2S w high-Z (highZenabled), brak połączenia
 * 			(DISCONNECTED), odtwarzanie zatrzymane (STOPPED) - taki stan
 * 			ESP32 ustawia w swoim konstruktorze.
 * @warning Do struktury stanu pisze WYŁĄCZNIE ta klasa; pozostałe
 * 			moduły tylko czytają przez metody is...().
 *******************************************************************/
btAudioDevice::btAudioDevice(btAudioState *btAudioStateData) {
	assert(btAudioStateData);
	this->pBtAudioState = btAudioStateData;

	this->pBtAudioState->btAudioDeviceOn_Off = BT_AUDIO_DEVICE_OFF;
	this->pBtAudioState->highZstateState = highZenabled;
	this->pBtAudioState->onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_DISCONNECTED;
	this->pBtAudioState->onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_STOPPED;
}

/********************************************************************
 * @brief  Destruktor klasy btAudioDevice
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   Pusty - struktura stanu należy do radioStruct, a nie do
 * 			klasy, więc nie jest tu zwalniana.
 * @warning NONE
 *******************************************************************/
btAudioDevice::~btAudioDevice() {
}

/********************************************************************
 * @brief  Zapisuje do struktury stanu BT dane z ramki otrzymanej z
 * 			ESP32
 *
 * Na podstawie pola btAudioSubcommand zapisuje odpowiednie pole unii
 * btAudioUnion do struktury radioStruct.audioDevices.bluetooth i
 * loguje zmianę:
 * - BT_SUBCOMMAND_STATE_DEVICE_ON_OFF		-> btAudioDeviceOn_Off,
 * - BT_SUBCOMMAND_STATE_CONNECTION			-> onConnCallbackStates,
 * - BT_SUBCOMMAND_STATE_AVRC_PLAY_STATUS	-> onAVRCPlayStatusCallbackStates,
 * - BT_SUBCOMMAND_STATE_I2S_PINS			-> highZstateState.
 *
 * @param  [btAudioFrame] [i2cFrame_btAudio*] Wskaźnik do ramki BT
 * 			otrzymanej z ESP32 (pData ramki z kolejki parsowania).
 *
 * @return NONE
 *
 * @note   Wywoływana w locie przez parser
 * 			(parserFunction::btAudioToBtAudioDevice) w tasku
 * 			i2cMasterParseReceivedData - dlatego tylko zapis stanu i
 * 			log, nic blokującego (blokada opóźniłaby klawiaturę i odbiór
 * 			kolejnych ramek z ESP32). Reakcje na zmianę stanu - w menu.
 * @warning Metoda NIE zwalnia pamięci ramki - pData zwalnia task
 * 			parsera (deleteAlocatedDataAfterParsing) po powrocie z
 * 			parseReceivedData(). Nieznany subcommand -> printf + assert(0).
 *******************************************************************/
void btAudioDevice::processReceivedData(i2cFrame_btAudio *btAudioFrame) {
	switch (btAudioFrame->btAudioData.btAudioSubcommand) {
	case BT_SUBCOMMAND_STATE_DEVICE_ON_OFF:
		this->pBtAudioState->btAudioDeviceOn_Off = btAudioFrame->btAudioData.btAudioUnion.btAudioDeviceOn_Off;
		pPrintf->feedPrintf("%s device on/off: %d", this->TAG,
				(int) this->pBtAudioState->btAudioDeviceOn_Off);
		break;
	case BT_SUBCOMMAND_STATE_CONNECTION:
		this->pBtAudioState->onConnCallbackStates = btAudioFrame->btAudioData.btAudioUnion.onConnCallbackStates;
		pPrintf->feedPrintf("%s connection state: %d", this->TAG,
				(int) this->pBtAudioState->onConnCallbackStates);
		break;
	case BT_SUBCOMMAND_STATE_AVRC_PLAY_STATUS:
		this->pBtAudioState->onAVRCPlayStatusCallbackStates = btAudioFrame->btAudioData.btAudioUnion.onAVRCPlayStatusCallbackStates;
		pPrintf->feedPrintf("%s play status: %d", this->TAG,
				(int) this->pBtAudioState->onAVRCPlayStatusCallbackStates);
		break;
	case BT_SUBCOMMAND_STATE_I2S_PINS:
		this->pBtAudioState->highZstateState = btAudioFrame->btAudioData.btAudioUnion.highZstateState;
		pPrintf->feedPrintf("%s i2s pins state: %d", this->TAG,
				(int) this->pBtAudioState->highZstateState);
		break;
	default:
		pPrintf->feedPrintf("%s unknown btAudioSubcommand value: 0x%x", this->TAG,
				btAudioFrame->btAudioData.btAudioSubcommand);
		assert(0);
		break;
	}
}

/********************************************************************
 * @brief  Zwraca stan włączenia BT z lokalnej struktury stanu
 *
 * @param  NONE
 *
 * @return [bool]
 * @retval [BT_AUDIO_DEVICE_ON] BT w ESP32 włączony
 * @retval [BT_AUDIO_DEVICE_OFF] BT w ESP32 wyłączony
 *
 * @note   Metoda is...(): zwraca OD RAZU ostatni stan przysłany przez
 * 			ESP32 (radioStruct.audioDevices.bluetooth), bez komunikacji
 * 			I2C.
 * @warning Wartość może nie odpowiadać bieżącemu stanowi ESP32, jeśli
 * 			ESP32 jeszcze nie przysłał zmiany (komunikacja I2C jest
 * 			asynchroniczna).
 *******************************************************************/
bool btAudioDevice::isDeviceOn(void) {
	return this->pBtAudioState->btAudioDeviceOn_Off;
}

/********************************************************************
 * @brief  Zwraca stan połączenia BT z telefonem z lokalnej struktury
 * 			stanu
 *
 * @param  NONE
 *
 * @return [bt_onConnCallbackStats] stan połączenia A2DP
 * 			(DISCONNECTED / CONNECTING / CONNECTED / DISCONNECTING)
 *
 * @note   Metoda is...(): jak isDeviceOn().
 * @warning Jak isDeviceOn().
 *******************************************************************/
bt_onConnCallbackStats btAudioDevice::isConnectionState(void) {
	return this->pBtAudioState->onConnCallbackStates;
}

/********************************************************************
 * @brief  Zwraca stan odtwarzania (AVRC) z lokalnej struktury stanu
 *
 * @param  NONE
 *
 * @return [bt_onAVRCPlayStatusCallback] stan odtwarzania (STOPPED /
 * 			PLAYING / PAUSED / FWD_SEEK / REV_SEEK / ERROR)
 *
 * @note   Metoda is...(): jak isDeviceOn().
 * @warning Jak isDeviceOn().
 *******************************************************************/
bt_onAVRCPlayStatusCallback btAudioDevice::isPlayStatus(void) {
	return this->pBtAudioState->onAVRCPlayStatusCallbackStates;
}

/********************************************************************
 * @brief  Zwraca stan pinów I2S ESP32 z lokalnej struktury stanu
 *
 * @param  NONE
 *
 * @return [i2sPinStates] stan pinów I2S ESP32 na wspólnej szynie DAC
 *
 * @note   Metoda is...(): jak isDeviceOn(). Przełączenie DAC na inne
 * 			źródło audio dopiero przy highZenabled (wspólna szyna I2S).
 * @warning Jak isDeviceOn().
 *******************************************************************/
i2sPinStates btAudioDevice::isI2sPinsState(void) {
	return this->pBtAudioState->highZstateState;
}
