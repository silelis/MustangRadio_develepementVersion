#pragma once

#include "AudioTools.h"
// include "I2SStream.h"
#include "BluetoothA2DPSink.h"
#include "esp_mac.h"

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
    esp32_a2dp_sink(const char *name, int pin_bck, int pin_ws, int pin_data);
    void enableBluetoothModuleAndI2S(const char *btDeviceName);
//   void disableBluetoothModuleAndI2S(void);
//    ~esp32_a2dp_sink();

protected:
private:

    static i2sPinStates i2sState;

    int pin_bck;
    int pin_ws;
    int pin_data;

    I2SStream *i2s;
    BluetoothA2DPSink *a2dp_sink;

    //https://chatgpt.com/c/691b01a1-7ea8-832d-9445-2e339e19c0e1

    void btCallback_onConn(esp_a2d_connection_state_t state, void*);
    void btCallback_onAudioState(esp_a2d_audio_state_t state, void*);
    void btCallback_onAVRCPlayStatus(esp_avrc_playback_stat_t playback, void*);

    esp_err_t i2sPinsHighImpedanceDisabled(void);
};
