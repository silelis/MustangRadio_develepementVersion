#pragma once

#include "AudioTools.h"
//include "I2SStream.h"
#include "BluetoothA2DPSink.h"
#include "esp_mac.h"


esp_err_t i2sPinsHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data);

class esp32_a2dp_sink{

    public:
        esp32_a2dp_sink(const char *name, int pin_bck, int pin_ws, int in_data);
        
        
    protected:

    private:
            const char *btDeviceName;
            
            int pin_bck;
            int pin_ws;
            int pin_data;

            I2SStream i2s;
            BluetoothA2DPSink a2dp_sink;

            esp_err_t i2sPinsHighImpedanceDisabled(void);

};