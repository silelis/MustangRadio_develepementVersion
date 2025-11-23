#pragma once

#include "esp_mac.h"
#include "AudioTools.h" //bin 874 KB (bajtów: 895 840)
#include "BluetoothA2DPSink.h"
#include "driver/gpio.h"
// #include "ESP32-A2DP/src/BluetoothA2DP.h"
// #include "arduino-audio-tools/src/AudioTools.h" //bin 874 KB (bajtów: 895 840)

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

#include "hwConfigFile.h"
#include  "common/comunication_calculate_checksum/comunication_calculate_checksum.h"
#include  "common/comunicationProtocol/comunicationProtocol.h"
#include  "common/comunicationStructures/comunicationStructures.h"

esp_err_t i2sHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data);




class bt_audio_sink{
    friend esp_err_t i2sHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data);

public:
    bt_audio_sink(int pin_bck, int pin_ws, int pin_data);
    ~bt_audio_sink(void);

    void btAudioPlay(void);
    void btAudioStop(void);
    void btAudioPause(void);
    void btAudioNext(void);
    void btAudioPrevious(void);
    void btAudioFastForward(void);
    void btAudioRewind(void);

private:
    static i2sPinStates i2sState;
    int pin_bck;
    int pin_ws;
    int pin_data;
    I2SStream *i2s = nullptr;
    BluetoothA2DPSink *a2dp_sink = nullptr;

    esp_err_t i2sHighImpedanceDisable(void);

public: // ale potem ma być private
    void btAudioDeviceOn(void);
    void btAudioDeviceOff(void);
};