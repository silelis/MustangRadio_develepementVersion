#include "bt_audio.h"
#include "i2c_engine/i2c_engine_slave.h"

i2sPinStates bt_audio_sink::i2sState;

static void seti2cFrame_btAudioHeaderData(i2cFrame_btAudio* btAudioFrame){
    btAudioFrame->i2cframeCommandHeader.commandGroup=I2C_COMMAND_GROUP_BTAUDIO;
    btAudioFrame->i2cframeCommandHeader.dataSize=sizeof(i2cFrame_btAudio);
    btAudioFrame->i2cframeCommandHeader.crcSum=calculate_checksum(btAudioFrame, btAudioFrame->i2cframeCommandHeader.dataSize);
}

static void sendDataToI2CMaster(i2cFrame_btAudio* dataToSend){
    extern i2cEngin_slave *p_i2cSlave
    
}

static void onConnCallback(esp_a2d_connection_state_t state, void*){
        i2cFrame_btAudio temp_btAudioFrame;
        switch(state){
        case ESP_A2D_CONNECTION_STATE_DISCONNECTED:     /*!< connection released  */
            printf("onConnCallback: ESP_A2D_CONNECTION_STATE_DISCONNECTED\r\n");
            temp_btAudioFrame.btAudioData.btAudioUnion.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_DISCONNECTED;
            break;
        case ESP_A2D_CONNECTION_STATE_CONNECTING:       /*!< connecting remote device */
            printf("onConnCallback: ESP_A2D_CONNECTION_STATE_CONNECTING\r\n");
            temp_btAudioFrame.btAudioData.btAudioUnion.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_CONNECTING;
            break;
        case ESP_A2D_CONNECTION_STATE_CONNECTED:        /*!< connection established */
            printf("onConnCallback: ESP_A2D_CONNECTION_STATE_CONNECTED\r\n");
            temp_btAudioFrame.btAudioData.btAudioUnion.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_CONNECTED
            break;
        case ESP_A2D_CONNECTION_STATE_DISCONNECTING:    /*!< disconnecting remote device */
            printf("onConnCallback: ESP_A2D_CONNECTION_STATE_DISCONNECTING\r\n");
            temp_btAudioFrame.btAudioData.btAudioUnion.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_DISCONNECTING;
            break;
    }
    temp_btAudioFrame.btAudioData.btAudioSubcommand=BT_SUBCOMMAND_onConnCallback;
    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);

    
    #error "tutaj skończyłem"
}

static void onAudioStateCallback(esp_a2d_audio_state_t state, void*){
    
    i2cFrame_btAudio temp_btAudioFrame;
    switch(state){
            case ESP_A2D_AUDIO_STATE_SUSPEND:           /*!< audio stream datapath suspended by remote device */
                printf("onAudioStateCallback ESP_A2D_AUDIO_STATE_SUSPEND\r\n");
                break;
            case ESP_A2D_AUDIO_STATE_STARTED:           /*!< audio stream datapath started */
                printf("onAudioStateCallback ESP_A2D_AUDIO_STATE_STARTED\r\n");
                break;
   
            //case ESP_A2D_AUDIO_STATE_STOPPED: /*aka ESP_A2D_AUDIO_STATE_SUSPEND*/            /*!< @note Deprecated */
            //case ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND: /*aka ESP_A2D_AUDIO_STATE_SUSPEND*/     /*!< @note Deprecated */
    }

    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);
    extern i2cEngin_slave *p_i2cSlave;

}

static void onAVRCPlayStatusCallback(esp_avrc_playback_stat_t playback){
    i2cFrame_btAudio temp_btAudioFrame;

    switch(playback){
        case  ESP_AVRC_PLAYBACK_STOPPED:               /*!< stopped */
            printf("onAVRCPlayStatusCallback ESP_AVRC_PLAYBACK_STOPPED\r\n");
            break;
        case ESP_AVRC_PLAYBACK_PLAYING:                /*!< playing */
            printf("onAVRCPlayStatusCallback ESP_AVRC_PLAYBACK_PLAYING\r\n");
            break;
        case ESP_AVRC_PLAYBACK_PAUSED:                 /*!< paused */
            printf("onAVRCPlayStatusCallback ESP_AVRC_PLAYBACK_PAUSED\r\n");
            break;
        case ESP_AVRC_PLAYBACK_FWD_SEEK:               /*!< forward seek */
            printf("onAVRCPlayStatusCallback ESP_AVRC_PLAYBACK_FWD_SEEK\r\n");
            break;
        case ESP_AVRC_PLAYBACK_REV_SEEK:               /*!< reverse seek */
            printf("onAVRCPlayStatusCallback ESP_AVRC_PLAYBACK_REV_SEEK\r\n");
            break;
        case ESP_AVRC_PLAYBACK_ERROR:                  /*!< error */
            printf("onAVRCPlayStatusCallback ESP_AVRC_PLAYBACK_ERROR\r\n");
            break;
    }
    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);
    extern i2cEngin_slave *p_i2cSlave;
}


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
    this->btAudioDeviceOff();
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

void bt_audio_sink::btAudioDeviceOn(void){
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
    //this->a2dp_sink->set_on_audio_state_changed_post(onAudioStateCallback); //nie potrzebuję w mojej aplikacji
    this->a2dp_sink->set_avrc_rn_playstatus_callback(onAVRCPlayStatusCallback);
    this->a2dp_sink->start(BT_AUDIO_NAME);
}

void bt_audio_sink::btAudioDeviceOff(void){
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

void bt_audio_sink::btAudioRewind(void){
    this->a2dp_sink->rewind();
}