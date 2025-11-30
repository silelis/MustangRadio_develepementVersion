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

typedef enum 
{
    highZenabled,
    i2sNotConfigured,
    i2sConfigured,
    highZdisabled
}i2sPinStates;


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

private:
    static i2sPinStates i2sState;
    //static void sendToMasterI2sPinsState(i2sPinStates i2s_pins_state);
    void sendToMastserDeviceOnOffState(bool on_off);
    int pin_bck;
    int pin_ws;
    int pin_data;
    I2SStream *i2s = nullptr;
    BluetoothA2DPSink *a2dp_sink = nullptr;

    
    esp_err_t i2sHighImpedanceDisable(void);

//public: // ale potem ma być private


};