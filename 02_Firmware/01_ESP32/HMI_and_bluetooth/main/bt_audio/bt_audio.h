#pragma once

#include "hwConfigFile.h"

#include "esp_mac.h"
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include "driver/gpio.h"


/*
SDKConfig required to set:
CONFIG_BT_ENABLED=y
CONFIG_BTDM_CTRL_MODE_BLE_ONLY=n
CONFIG_BTDM_CTRL_MODE_BR_EDR_ONLY=y
CONFIG_BTDM_CTRL_MODE_BTDM=n
CONFIG_BT_BLUEDROID_ENABLED=y
CONFIG_BT_CLASSIC_ENABLED=y
CONFIG_BT_A2DP_ENABLE=y
CONFIG_BT_SPP_ENABLED=y
CONFIG_BT_BLE_ENABLED=n
more info:
https://github.com/pschatzmann/ESP32-A2DP/wiki/Espressif-IDF-as-a-Component*/

#include  "common/comunication_calculate_checksum/comunication_calculate_checksum.h"
#include  "common/comunicationProtocol/comunicationProtocol.h"
#include  "common/comunicationStructures/comunicationStructures.h"

/*
typedef enum 
{
    highZenabled,
    i2sNotConfigured,
    i2sConfigured,
    highZdisabled
}i2sPinStates;*/


//BluetoothA2DPSink z dostępem do chronionej set_last_connection(): end(false) w Off czyści w NVS adres ostatniego telefonu,
//co wyłączało auto-reconnect po Off -> On; przed start() przywracamy adres zapamiętany w btConnectionState.peerAddress
class BluetoothA2DPSinkExt : public BluetoothA2DPSink {
public:
    using BluetoothA2DPSink::BluetoothA2DPSink;	//te same konstruktory (m.in. BluetoothA2DPSink(AudioStream&))
    void restoreLastConnection(esp_bd_addr_t bda){ this->set_last_connection(bda); }
};

class bt_audio_sink{
public:
    bt_audio_sink(int pin_bck, int pin_ws, int pin_data);
    ~bt_audio_sink(void);
    static esp_err_t i2sHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data);

protected:
    void btAudioPlay(void);
    void btAudioStop(void);
    void btAudioPause(void);
    void btAudioNext(void);
    void btAudioPrevious(void);
    void btAudioFastForward(void);
    void btAudioRewind(void);

public:
    void btAudioDeviceOn(void);
    void btAudioDeviceOff(void);
    //wysyłka danych z btConnectionState do STM32 - wołane z callbacków, docelowo także na żądanie STM32
    void sendToMasterDeviceOnOffState(void);		//BT_SUBCOMMAND_STATE_DEVICE_ON_OFF z btConnectionState.btAudioDeviceOn_Off
    void sendToMasterConnectionState(void);		//BT_SUBCOMMAND_STATE_CONNECTION z btConnectionState.onConnCallbackStates
    void sendToMasterAVRCPlayStatus(void);		//BT_SUBCOMMAND_STATE_AVRC_PLAY_STATUS z btConnectionState.onAVRCPlayStatusCallbackStates
    void sendToMasterI2sPinsState(void);		//BT_SUBCOMMAND_STATE_I2S_PINS z btConnectionState.highZstateState

private:
    static i2sPinStates i2sState;	//i2sPinStates - wspólny enum z comunicationStructures.h (wysyłany do STM32)
    static bt_audio_sink* instance;	//jedyny obiekt klasy - dla callbacków biblioteki bez parametru kontekstu (void* obj); ustawiany w konstruktorze
    //pomocnicze do wysyłki ramek BT do STM32 - statyczne, bo wołane także z callbacków
    static void seti2cFrame_btAudioHeaderData(i2cFrame_btAudio* btAudioFrame);
    static BaseType_t PrepareBtDataAndSent_to_i2cQueueSend(i2cFrame_btAudio* dataToSend);
    //callbacki biblioteki ESP32-A2DP - metody statyczne, aby miały dostęp do prywatnego btConnectionState przez instance
    static void onConnCallback(esp_a2d_connection_state_t state, void*);
    static void onAVRCPlayStatusCallback(esp_avrc_playback_stat_t playback);
    static void onAudioStateCallback(esp_a2d_audio_state_t state, void*);	//-> btConnectionState.audioState
    static void onAvrcConnectionStateCallback(bool connected);				//-> btConnectionState.avrcConnected
    static void onVolumeChangeCallback(int volume);							//-> btConnectionState.volume (0-127)
    static void onSampleRateCallback(uint16_t rate);						//-> btConnectionState.sampleRate
    static void onPlayPosCallback(uint32_t play_pos);						//-> btConnectionState.playPosMs
    static void onMetadataCallback(uint8_t id, const uint8_t* text);		//-> btConnectionState.title / artist / album
    //static void sendToMasterI2sPinsState(i2sPinStates i2s_pins_state);
    int pin_bck;
    int pin_ws;
    int pin_data;
    I2SStream *i2s = nullptr;
    BluetoothA2DPSinkExt *a2dp_sink = nullptr;

    
    esp_err_t i2sHighImpedanceDisable(void);
    void setI2sPinsState(i2sPinStates newState);	//jedyne miejsce zmiany stanu pinów: i2sState + btConnectionState.highZstateState + wysyłka do STM32
    void refreshPeerName(void);	//kopiuje nazwę telefonu z biblioteki do btConnectionState.peerName (nazwa przychodzi asynchronicznie po CONNECTED)
	struct{
				bool btAudioDeviceOn_Off = false;
				bt_AudioCommand audioSinkCommand = Play;													//0
				i2sPinStates highZstateState = highZenabled;												//0, ustawiane przez setI2sPinsState() - wszystkie 4 stany
				bt_onConnCallbackStats onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_DISCONNECTED;	//0
		//		bt_onAudioStateCallbackStates onAudioStateCallbackStates;
				bt_onAVRCPlayStatusCallback onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_STOPPED;	//0
				//A2DP
				esp_a2d_audio_state_t audioState = ESP_A2D_AUDIO_STATE_SUSPEND;		//0, set_on_audio_state_changed
				uint16_t sampleRate = 0;												//[Hz] set_sample_rate_callback
				//urządzenie zdalne (telefon) - uzupełniane przy ESP_A2D_CONNECTION_STATE_CONNECTED
				char peerName[ESP_BT_GAP_MAX_BDNAME_LEN + 1] = {};					//get_peer_name()
				esp_bd_addr_t peerAddress = {};										//get_current_peer_address() (MAC, 6 bajtów)
				//AVRC
				bool avrcConnected = false;											//set_avrc_connection_state_callback
				int volume = 0;														//0-127, set_avrc_rn_volumechange
				uint32_t playPosMs = 0;												//[ms] set_avrc_rn_play_pos_callback
				char title[64] = {};													//set_avrc_metadata_callback (ESP_AVRC_MD_ATTR_TITLE), obcinane do 63 znaków
				char artist[64] = {};												//set_avrc_metadata_callback (ESP_AVRC_MD_ATTR_ARTIST)
				char album[64] = {};													//set_avrc_metadata_callback (ESP_AVRC_MD_ATTR_ALBUM)
	}btConnectionState;

//public: // ale potem ma być private


};