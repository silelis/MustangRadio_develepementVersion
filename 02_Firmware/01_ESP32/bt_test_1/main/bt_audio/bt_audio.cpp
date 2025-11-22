#include "bt_audio.h"

enum i2sPinStates bt_audio_sink::i2sState;

esp_err_t i2sHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data)
{
    gpio_config_t io_conf = { };
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = ((1ULL << pin_bck) | (1ULL << pin_ws) | (1ULL << pin_data));
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; 
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    esp_err_t retVal = gpio_config(&io_conf);
    if (retVal == ESP_OK)
    {
        printf("I2S pins high impedance mode had been initialized.\n");
        bt_audio_sink::i2sState = highZenabled;
    }
    else
    {
        printf("PROBLEM with I2S pins high impedance mode initialization.\n");
        assert(!ESP_OK);
    }
    return retVal;
}


bt_audio_sink::bt_audio_sink(int pin_bck, int pin_ws, int pin_data){
    this->pin_bck = pin_bck;
    this->pin_ws = pin_ws;
    this->pin_data = pin_data;
}

bt_audio_sink::~bt_audio_sink(void){
    this->btAudioDeinit();
    this->pin_bck = -1;
    this->pin_ws = -1;
    this->pin_data = -1;
}

esp_err_t bt_audio_sink::i2sHighImpedanceDisable(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = ((1ULL << this->pin_bck) | (1ULL << this->pin_ws) | (1ULL << this->pin_data));
    io_conf.mode = GPIO_MODE_DISABLE;
    esp_err_t retVal = gpio_config(&io_conf);
    if (retVal == ESP_OK)
    {
        printf("I2S pins high impedance mode had been deinitialized.\n");
        bt_audio_sink::i2sState = highZdisabled;
    }
    else
    {
        printf("PROBLEM with I2S pins high impedance mode deinitialization.\n");
        assert(!ESP_OK);
    }
    return retVal;
}

void bt_audio_sink::btAudioInit(void){
    this->i2sHighImpedanceDisable();
    bt_audio_sink::i2sState=i2sNotConfigured;
    this->i2s = new I2SStream();
    bt_audio_sink::i2sState=i2sConfigured;
    this->a2dp_sink = new BluetoothA2DPSink(*i2s);
    
    auto cfg = i2s->defaultConfig();
    cfg.pin_bck = this->pin_bck;
    cfg.pin_ws = this->pin_ws;
    cfg.pin_data = this->pin_data;
    this->i2s->begin(cfg);

    this->a2dp_sink->set_auto_reconnect(false);

    this->a2dp_sink->set_on_connection_state_changed(onConnCallback);
    this->a2dp_sink->set_on_audio_state_changed_post(onAudioStateCallback);
    this->a2dp_sink->set_avrc_rn_playstatus_callback(onAVRCPlayStatusCallback);

    this->a2dp_sink->start(BT_AUDIO_NAME);
}

static void onConnCallback(esp_a2d_connection_state_t state, void*){
    switch(state){
        case ESP_A2D_CONNECTION_STATE_DISCONNECTED:     /*!< connection released  */
        
        case ESP_A2D_CONNECTION_STATE_CONNECTING:       /*!< connecting remote device */
        
        case ESP_A2D_CONNECTION_STATE_CONNECTED:        /*!< connection established */
        
        case ESP_A2D_CONNECTION_STATE_DISCONNECTING:    /*!< disconnecting remote device */
    }

}

static void onAudioStateCallback(esp_a2d_audio_state_t state, void*){
    switch(state){
            case ESP_A2D_AUDIO_STATE_SUSPEND:           /*!< audio stream datapath suspended by remote device */
            case ESP_A2D_AUDIO_STATE_STARTED:           /*!< audio stream datapath started */
   
            //case ESP_A2D_AUDIO_STATE_STOPPED: /*aka ESP_A2D_AUDIO_STATE_SUSPEND*/            /*!< @note Deprecated */
            //case ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND: /*aka ESP_A2D_AUDIO_STATE_SUSPEND*/     /*!< @note Deprecated */
    }
}

static void onAVRCPlayStatusCallback(esp_avrc_playback_stat_t playback){
    switch(playback){
        case  ESP_AVRC_PLAYBACK_STOPPED:               /*!< stopped */
        case ESP_AVRC_PLAYBACK_PLAYING:                /*!< playing */
        case ESP_AVRC_PLAYBACK_PAUSED:                 /*!< paused */
        case ESP_AVRC_PLAYBACK_FWD_SEEK:               /*!< forward seek */
        case ESP_AVRC_PLAYBACK_REV_SEEK:               /*!< reverse seek */
        case ESP_AVRC_PLAYBACK_ERROR:                  /*!< error */
    }
}

void bt_audio_sink::btAudioDeinit(void){
    this->btAudioStop();
    delete this->a2dp_sink;
    this->a2dp_sink=nullptr;
    this->i2s->end();
    delete this->i2s;
    this->i2s=nullptr;
    bt_audio_sink::i2sState=i2sNotConfigured;
    i2sHighImpedanceEnabled(this->pin_bck, this->pin_ws, this->pin_data);
}

void bt_audio_sink::btAudioPlay(void){
    this->a2dp_sink->play();
}

void bt_audio_sink::btAudioStop(void){
    this->a2dp_sink->stop();
}

void bt_audio_sink::btAudioPause(void){
    this->a2dp_sink->pause();
}

void bt_audio_sink::btAudioNext(void){
this->a2dp_sink->next();
}
void bt_audio_sink::btAudioPrevious(void){
    this->a2dp_sink->previous();
}

void bt_audio_sink::btAudioFastForward(void){
    this->a2dp_sink->fast_forward();
}

void bt_audio_sink::btAudioFastFRewind(void){
    this->a2dp_sink->rewind();
}