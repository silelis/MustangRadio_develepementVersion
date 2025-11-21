#pragma once

#include "AudioTools.h"
// include "I2SStream.h"
#include "BluetoothA2DPSink.h"
#include "esp_mac.h"

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
https://github.com/pschatzmann/ESP32-A2DP/wiki/Espressif-IDF-as-a-Component
*/
enum i2sPinStates
{
    highZenabled,
    i2sNotConfigured,
    i2sConfigured,
    highZdisabled,
};

esp_err_t i2sPinsHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data);

class esp32_a2dp_sink
{
    friend esp_err_t i2sPinsHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data);

public:
    esp32_a2dp_sink(int pin_bck, int pin_ws, int pin_data);
    void enableBluetoothModuleAndI2S(const char *btDeviceName);
//   void disableBluetoothModuleAndI2S(void);
//    ~esp32_a2dp_sink();
    void audioPlay();
    void audioPause();
    void audioStop();
    void aidioFF();
    void audioRev();


protected:
private:

    static i2sPinStates i2sState;

    int pin_bck;
    int pin_ws;
    int pin_data;

    I2SStream *i2s=nullptr;
    BluetoothA2DPSink *a2dp_sink=nullptr;


    /*
    //https://chatgpt.com/c/691b01a1-7ea8-832d-9445-2e339e19c0e1
    // pointer to active instance
    static esp32_a2dp_sink* instance;
    // static callback wrappers
    static void s_btCallback_onConn(esp_a2d_connection_state_t state, void* param);
    static void s_btCallback_onAudioState(esp_a2d_audio_state_t state, void* param);
    static void s_btCallback_onAVRCPlayStatus(esp_avrc_playback_stat_t playback);
    //bluetooth callbacks
    void btCallback_onConn(esp_a2d_connection_state_t state, void* param);
    void btCallback_onAudioState(esp_a2d_audio_state_t state, void* param);
    void btCallback_onAVRCPlayStatus(esp_avrc_playback_stat_t playback);
*/
    esp_err_t i2sPinsHighImpedanceDisabled(void);
};


esp32_a2dp_sink::esp32_a2dp_sink(int pin_bck, int pin_ws, int pin_data){

//    esp32_a2dp_sink::instance = this;

    this->pin_bck=pin_bck;
    this->pin_ws=pin_ws;
    this->pin_data=pin_data;
//         this->btDeviceName= name;

}