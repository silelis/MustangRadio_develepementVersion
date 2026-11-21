#include "bt_audio.h"
#include "i2c_engine/i2c_engine_slave.h"
#include <new>	// std::nothrow

//i2sPinStates bt_audio_sink::i2sState;

i2sPinStates bt_audio_sink::i2sState;
bt_audio_sink* bt_audio_sink::instance = nullptr;

void bt_audio_sink::seti2cFrame_btAudioHeaderData(i2cFrame_btAudio* btAudioFrame){
    btAudioFrame->i2cframeCommandHeader.commandGroup=I2C_COMMAND_GROUP_BTAUDIO;
    btAudioFrame->i2cframeCommandHeader.dataSize=sizeof(i2cFrame_btAudio);
    btAudioFrame->i2cframeCommandHeader.crcSum=calculate_checksum(btAudioFrame, btAudioFrame->i2cframeCommandHeader.dataSize);
}

BaseType_t bt_audio_sink::PrepareBtDataAndSent_to_i2cQueueSend(i2cFrame_btAudio* dataToSend){
    extern i2cEngin_slave *p_i2cSlave;

    i2cFrame_transmitQueue dataToTransmitQueue;
    dataToTransmitQueue.pData = new (std::nothrow) char[dataToSend->i2cframeCommandHeader.dataSize];	// nothrow: przy braku pamięci nullptr -> assert niżej
    assert(dataToTransmitQueue.pData);
    if (dataToTransmitQueue.pData != NULL){
        memcpy(dataToTransmitQueue.pData,dataToSend, dataToSend->i2cframeCommandHeader.dataSize);
        dataToTransmitQueue.dataSize = dataToSend->i2cframeCommandHeader.dataSize;
        return p_i2cSlave->i2cSendDataToTransisionQueue(&dataToTransmitQueue);
    }
    else{
        return pdFALSE;
    }
}

/*
void bt_audio_sink::sendToMasterI2sPinsState(i2sPinStates i2s_pins_state){
    bt_audio_sink::i2sState=i2s_pins_state;
    i2cFrame_btAudio temp_btAudioFrame;
    temp_btAudioFrame.btAudioData.btAudioSubcommand=BT_SUBCOMMAND_I2S_PINS_STATE;
    temp_btAudioFrame.btAudioData.btAudioUnion.highZstateState = i2s_pins_state;
    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);
    assert(PrepareBtDataAndSent_to_i2cQueueSend(&temp_btAudioFrame));
}
*/

void bt_audio_sink::sendToMasterDeviceOnOffState(void){
    i2cFrame_btAudio temp_btAudioFrame = {};	//jak w sendToMasterConnectionState()
    temp_btAudioFrame.btAudioData.btAudioSubcommand=BT_SUBCOMMAND_deviceOnOff;
    temp_btAudioFrame.btAudioData.btAudioUnion.btAudioDeviceOn_Off = this->btConnectionState.btAudioDeviceOn_Off;
    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);
    BaseType_t frameQueued = PrepareBtDataAndSent_to_i2cQueueSend(&temp_btAudioFrame);
    assert(frameQueued == pdTRUE);
}

void bt_audio_sink::sendToMasterConnectionState(void){
    i2cFrame_btAudio temp_btAudioFrame = {};	//= {}: wyzerowana ramka - brak przypadkowych bajtów (padding, reszta unii) w CRC i w danych dla STM32
    temp_btAudioFrame.btAudioData.btAudioSubcommand=BT_SUBCOMMAND_onConnCallback;
    temp_btAudioFrame.btAudioData.btAudioUnion.onConnCallbackStates = this->btConnectionState.onConnCallbackStates;
    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);
    BaseType_t frameQueued = PrepareBtDataAndSent_to_i2cQueueSend(&temp_btAudioFrame);
    assert(frameQueued == pdTRUE);
}

void bt_audio_sink::sendToMasterAVRCPlayStatus(void){
    i2cFrame_btAudio temp_btAudioFrame = {};	//jak wyżej
    temp_btAudioFrame.btAudioData.btAudioSubcommand=BT_SUBCOMMAND_onAVRCPlayStatusCallback;
    temp_btAudioFrame.btAudioData.btAudioUnion.onAVRCPlayStatusCallbackStates = this->btConnectionState.onAVRCPlayStatusCallbackStates;
    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);
    BaseType_t frameQueued = PrepareBtDataAndSent_to_i2cQueueSend(&temp_btAudioFrame);
    assert(frameQueued == pdTRUE);
}

void bt_audio_sink::sendToMasterI2sPinsState(void){
    i2cFrame_btAudio temp_btAudioFrame = {};	//jak w sendToMasterConnectionState()
    temp_btAudioFrame.btAudioData.btAudioSubcommand=BT_SUBCOMMAND_I2S_PINS_STATE;
    temp_btAudioFrame.btAudioData.btAudioUnion.highZstateState = this->btConnectionState.highZstateState;
    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);
    BaseType_t frameQueued = PrepareBtDataAndSent_to_i2cQueueSend(&temp_btAudioFrame);
    assert(frameQueued == pdTRUE);
}

void bt_audio_sink::setI2sPinsState(i2sPinStates newState){
    bt_audio_sink::i2sState = newState;
    this->btConnectionState.highZstateState = newState;
    this->sendToMasterI2sPinsState();
}



void bt_audio_sink::onConnCallback(esp_a2d_connection_state_t state, void*){
    if (bt_audio_sink::instance == nullptr){	//guard: callback bez obiektu klasy (przed konstruktorem / po destruktorze)
        return;
    }
    bt_audio_sink* self = bt_audio_sink::instance;
    switch(state){
        case ESP_A2D_CONNECTION_STATE_DISCONNECTED:     /*!< connection released  */
            printf("onConnCallback: ESP_A2D_CONNECTION_STATE_DISCONNECTED\r\n");
            self->btConnectionState.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_DISCONNECTED;
            break;
        case ESP_A2D_CONNECTION_STATE_CONNECTING:       /*!< connecting remote device */
            printf("onConnCallback: ESP_A2D_CONNECTION_STATE_CONNECTING\r\n");
            self->btConnectionState.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_CONNECTING;
            break;
        case ESP_A2D_CONNECTION_STATE_CONNECTED:        /*!< connection established */
            printf("onConnCallback: ESP_A2D_CONNECTION_STATE_CONNECTED\r\n");
            self->btConnectionState.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_CONNECTED;
            memcpy(self->btConnectionState.peerAddress, *self->a2dp_sink->get_current_peer_address(), ESP_BD_ADDR_LEN);	//adres ustawiony przez bibliotekę przed wywołaniem callbacku
            self->refreshPeerName();	//nazwa może tu być jeszcze pusta/nieaktualna - dociągana w kolejnych callbackach
            break;
        case ESP_A2D_CONNECTION_STATE_DISCONNECTING:    /*!< disconnecting remote device */
            printf("onConnCallback: ESP_A2D_CONNECTION_STATE_DISCONNECTING\r\n");
            self->btConnectionState.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_DISCONNECTING;
            break;
    }
    self->sendToMasterConnectionState();
}

/*
static void onAudioStateCallback(esp_a2d_audio_state_t state, void*){
    
    i2cFrame_btAudio temp_btAudioFrame;
    switch(state){
            case ESP_A2D_AUDIO_STATE_SUSPEND:           //!< audio stream datapath suspended by remote device
                printf("onAudioStateCallback ESP_A2D_AUDIO_STATE_SUSPEND\r\n");
                break;
            case ESP_A2D_AUDIO_STATE_STARTED:           //< audio stream datapath started 
                printf("onAudioStateCallback ESP_A2D_AUDIO_STATE_STARTED\r\n");
                break;
   
            //case ESP_A2D_AUDIO_STATE_STOPPED: //aka ESP_A2D_AUDIO_STATE_SUSPEND            //!< @note Deprecated 
            //case ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND: //aka ESP_A2D_AUDIO_STATE_SUSPEND     //!< @note Deprecated 
    }

    seti2cFrame_btAudioHeaderData(&temp_btAudioFrame);
    extern i2cEngin_slave *p_i2cSlave;

}*/

void bt_audio_sink::onAVRCPlayStatusCallback(esp_avrc_playback_stat_t playback){
    if (bt_audio_sink::instance == nullptr){	//guard: jak w onConnCallback
        return;
    }
    bt_audio_sink* self = bt_audio_sink::instance;
    switch(playback){
        case  ESP_AVRC_PLAYBACK_STOPPED:               /*!< stopped */
            printf("onAVRCPlayStatusCallback: ESP_AVRC_PLAYBACK_STOPPED\r\n");
            self->btConnectionState.onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_STOPPED;
            break;
        case ESP_AVRC_PLAYBACK_PLAYING:                /*!< playing */
            printf("onAVRCPlayStatusCallback: ESP_AVRC_PLAYBACK_PLAYING\r\n");
            self->btConnectionState.onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_PLAYING;
            break;
        case ESP_AVRC_PLAYBACK_PAUSED:                 /*!< paused */
            printf("onAVRCPlayStatusCallback: ESP_AVRC_PLAYBACK_PAUSED\r\n");
            self->btConnectionState.onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_PAUSED;
            break;
        case ESP_AVRC_PLAYBACK_FWD_SEEK:               /*!< forward seek */
            printf("onAVRCPlayStatusCallback: ESP_AVRC_PLAYBACK_FWD_SEEK\r\n");
            self->btConnectionState.onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_FWD_SEEK;
            break;
        case ESP_AVRC_PLAYBACK_REV_SEEK:               /*!< reverse seek */
            printf("onAVRCPlayStatusCallback: ESP_AVRC_PLAYBACK_REV_SEEK\r\n");
            self->btConnectionState.onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_REV_SEEK;
            break;
        case ESP_AVRC_PLAYBACK_ERROR:                  /*!< error */
            printf("onAVRCPlayStatusCallback: ESP_AVRC_PLAYBACK_ERROR\r\n");
            self->btConnectionState.onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_ERROR;
            break;
    }
    self->refreshPeerName();	//dociąga nazwę telefonu, która przy CONNECTED mogła jeszcze nie dotrzeć
    self->sendToMasterAVRCPlayStatus();
}

void bt_audio_sink::onAudioStateCallback(esp_a2d_audio_state_t state, void*){
    if (bt_audio_sink::instance == nullptr){	//guard: jak w onConnCallback
        return;
    }
    bt_audio_sink::instance->btConnectionState.audioState = state;
}

void bt_audio_sink::onAvrcConnectionStateCallback(bool connected){
    if (bt_audio_sink::instance == nullptr){	//guard: jak wyżej
        return;
    }
    bt_audio_sink::instance->btConnectionState.avrcConnected = connected;
    bt_audio_sink::instance->refreshPeerName();	//AVRC łączy się po A2DP - nazwa telefonu zwykle już dotarła
}

void bt_audio_sink::onVolumeChangeCallback(int volume){
    if (bt_audio_sink::instance == nullptr){	//guard: jak wyżej
        return;
    }
    bt_audio_sink::instance->btConnectionState.volume = volume;
}

void bt_audio_sink::onSampleRateCallback(uint16_t rate){
    if (bt_audio_sink::instance == nullptr){	//guard: jak wyżej
        return;
    }
    bt_audio_sink::instance->btConnectionState.sampleRate = rate;
}

void bt_audio_sink::onPlayPosCallback(uint32_t play_pos){
    if (bt_audio_sink::instance == nullptr){	//guard: jak wyżej
        return;
    }
    bt_audio_sink::instance->btConnectionState.playPosMs = play_pos;
}

void bt_audio_sink::onMetadataCallback(uint8_t id, const uint8_t* text){
    if ((bt_audio_sink::instance == nullptr) || (text == nullptr)){	//guard: jak wyżej + brak tekstu
        return;
    }
    char* target = nullptr;
    size_t targetSize = 0;
    switch(id){
        case ESP_AVRC_MD_ATTR_TITLE:
            target = bt_audio_sink::instance->btConnectionState.title;
            targetSize = sizeof(bt_audio_sink::instance->btConnectionState.title);
            break;
        case ESP_AVRC_MD_ATTR_ARTIST:
            target = bt_audio_sink::instance->btConnectionState.artist;
            targetSize = sizeof(bt_audio_sink::instance->btConnectionState.artist);
            break;
        case ESP_AVRC_MD_ATTR_ALBUM:
            target = bt_audio_sink::instance->btConnectionState.album;
            targetSize = sizeof(bt_audio_sink::instance->btConnectionState.album);
            break;
        default:	//pozostałe atrybuty (nr utworu, liczba utworów, gatunek) - nie przechowywane
            return;
    }
    strncpy(target, (const char*)text, targetSize - 1);
    target[targetSize - 1] = '\0';	//strncpy nie gwarantuje terminatora przy obcięciu
}


esp_err_t bt_audio_sink::i2sHighImpedanceEnabled(int pin_bck, int pin_ws, int pin_data)
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
        if (bt_audio_sink::instance != nullptr){	//obiekt istnieje (konstruktor po ustawieniu instance, Off) - stan + wysyłka do STM32
            bt_audio_sink::instance->setI2sPinsState(highZenabled);
        }
        else{										//wywołanie z app_main przed utworzeniem obiektu - tylko i2sState, bez wysyłki
            bt_audio_sink::i2sState = highZenabled;
        }
        //bt_audio_sink::sendToMasterI2sPinsState(highZenabled);
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
    bt_audio_sink::i2sHighImpedanceEnabled(this->pin_bck, this->pin_ws, this->pin_data);	//klasa sama gwarantuje high-Z pinów I2S do czasu btAudioDeviceOn() (niezależnie od wywołania w app_main)
    bt_audio_sink::instance = this;	//przed btAudioDeviceOff() i przed rejestracją callbacków w btAudioDeviceOn()
    this->btAudioDeviceOff();	//stan początkowy spójny z Off: high-Z (ponownie), highZstateState, btAudioDeviceOn_Off = false; STM32 dostaje OFF, żeby wiedział, co się dzieje
}

bt_audio_sink::~bt_audio_sink(void){
    this->btAudioDeviceOff();
    delete this->a2dp_sink;		//jedyne miejsce usuwania obiektów (delete nullptr jest bezpieczne, gdy On nigdy nie było wywołane)
    this->a2dp_sink = nullptr;
    delete this->i2s;
    this->i2s = nullptr;
    bt_audio_sink::instance = nullptr;	//dopiero po end() i delete - callbacki podczas end() mogą jeszcze korzystać z instance
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
        this->setI2sPinsState(highZdisabled);
        //bt_audio_sink::sendToMasterI2sPinsState(highZdisabled);
    }
    else
    {
        printf("PROBLEM with I2S pins high impedance mode deinitialization.\n");
        assert(!ESP_OK);
    }
    return retVal;
}

void bt_audio_sink::refreshPeerName(void){
    strncpy(this->btConnectionState.peerName, this->a2dp_sink->get_peer_name(), sizeof(this->btConnectionState.peerName) - 1);
    this->btConnectionState.peerName[sizeof(this->btConnectionState.peerName) - 1] = '\0';	//strncpy nie gwarantuje terminatora przy obcięciu
}

void bt_audio_sink::btAudioDeviceOn(void){
    if (this->btConnectionState.btAudioDeviceOn_Off){	//guard: podwójne On (bez Off pomiędzy) - nic nie rób
        return;
    }
    this->i2sHighImpedanceDisable();
    this->setI2sPinsState(i2sNotConfigured);
    //bt_audio_sink::sendToMasterI2sPinsState(i2sNotConfigured);
    if (this->a2dp_sink == nullptr){	//obiekty tworzone RAZ (przy pierwszym On), potem tylko start()/end(false):
										//BluetoothA2DPSink ustawia globalne wskaźniki na siebie (actual_bluetooth_a2dp_sink/common), których stos BT używa - nie może być usuwany przy Off
        this->i2s = new I2SStream();
        this->a2dp_sink = new BluetoothA2DPSinkExt(*i2s);
        this->a2dp_sink->set_auto_reconnect(true);	//podczas On: łączy z ostatnim telefonem przy start() i ponawia przy zerwaniu łącza; Off (end()) wyłącza auto-reconnect
        this->a2dp_sink->set_on_connection_state_changed(onConnCallback);
        //this->a2dp_sink->set_on_audio_state_changed_post(onAudioStateCallback); //nie potrzebuję w mojej aplikacji
        this->a2dp_sink->set_avrc_rn_playstatus_callback(onAVRCPlayStatusCallback);
        this->a2dp_sink->set_on_audio_state_changed(onAudioStateCallback);
        this->a2dp_sink->set_avrc_connection_state_callback(onAvrcConnectionStateCallback);
        this->a2dp_sink->set_avrc_rn_volumechange(onVolumeChangeCallback);
        this->a2dp_sink->set_sample_rate_callback(onSampleRateCallback);
        this->a2dp_sink->set_avrc_rn_play_pos_callback(onPlayPosCallback, 1);	//co 1 s (minimum biblioteki; domyślnie 10 s)
        this->a2dp_sink->set_avrc_metadata_callback(onMetadataCallback);
    }
    auto cfg = i2s->defaultConfig();
    cfg.pin_bck = this->pin_bck;
    cfg.pin_ws = this->pin_ws;
    cfg.pin_data = this->pin_data;
    this->i2s->begin(cfg);
    this->setI2sPinsState(i2sConfigured);			//dopiero po begin(cfg)
    //bt_audio_sink::sendToMasterI2sPinsState(i2sConfigured);
    static const esp_bd_addr_t noAddress = {};
    if (memcmp(this->btConnectionState.peerAddress, noAddress, ESP_BD_ADDR_LEN) != 0){	//znany ostatni telefon (zapamiętany przy CONNECTED)
        this->a2dp_sink->restoreLastConnection(this->btConnectionState.peerAddress);	//end(false) w Off wyczyścił adres w NVS - bez tego brak auto-reconnect po Off -> On
    }
    this->a2dp_sink->start(BT_AUDIO_NAME);	//po end(false) biblioteka pozwala na ponowny start
    this->btConnectionState.btAudioDeviceOn_Off = true;
    this->sendToMasterDeviceOnOffState();	//wysyła btAudioDeviceOn_Off == true (BT_AUDIO_DEVICE_ON)
    //migawka danych wysyłanych w callbackach - STM32 po ponownym Init BT ma aktualny stan, nawet jeśli wcześniej zgubił ramkę lub się zresetował
    this->sendToMasterConnectionState();
    this->sendToMasterAVRCPlayStatus();
    this->sendToMasterI2sPinsState();
}

void bt_audio_sink::btAudioDeviceOff(void){
    if (this->btConnectionState.btAudioDeviceOn_Off){	//guard: Off z konstruktora / podwójne Off / destruktor - BT nie był uruchomiony, nie ma czego zatrzymywać
        this->a2dp_sink->end(false);	//rozłącza telefon, deinit A2DP/AVRC, zatrzymuje task BtAppT, ZATRZYMUJE I2S (out->end());
										//false = kontroler BT i bluedroid zostają -> możliwy ponowny start() (end(true) blokuje BT do resetu)
        esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);	//radio niewidoczne i niepołączalne dla telefonu, gdy BT wyłączony (przy start() biblioteka przywraca tryb connectable/discoverable)
        //dane sesji zerowane po end() (żaden callback ich już nie nadpisze); peerAddress/peerName zostają - potrzebne do auto-reconnect przy następnym On
        //dane wysyłane w callbackach: do STM32 tylko wtedy, gdy callback podczas end() ich nie wysłał (callback najpierw zapisuje do struktury, potem wysyła)
        if (this->btConnectionState.onConnCallbackStates != BT_ESP_A2D_CONNECTION_STATE_DISCONNECTED){	//np. CONNECTING przerwane przez end()
            this->btConnectionState.onConnCallbackStates = BT_ESP_A2D_CONNECTION_STATE_DISCONNECTED;
            this->sendToMasterConnectionState();
        }
        if (this->btConnectionState.onAVRCPlayStatusCallbackStates != BT_ESP_AVRC_PLAYBACK_STOPPED){	//biblioteka przy end() nie woła callbacku play status
            this->btConnectionState.onAVRCPlayStatusCallbackStates = BT_ESP_AVRC_PLAYBACK_STOPPED;
            this->sendToMasterAVRCPlayStatus();
        }
        this->btConnectionState.audioState = ESP_A2D_AUDIO_STATE_SUSPEND;
        this->btConnectionState.sampleRate = 0;
        this->btConnectionState.avrcConnected = false;
        this->btConnectionState.volume = 0;
        this->btConnectionState.playPosMs = 0;
        this->btConnectionState.title[0] = '\0';
        this->btConnectionState.artist[0] = '\0';
        this->btConnectionState.album[0] = '\0';
    }
    this->setI2sPinsState(i2sNotConfigured);
    //bt_audio_sink::sendToMasterI2sPinsState(i2sNotConfigured);
    bt_audio_sink::i2sHighImpedanceEnabled(this->pin_bck, this->pin_ws, this->pin_data);
    //highZenabled ustawione i wysłane wewnątrz i2sHighImpedanceEnabled() (przez instance) - przed wysłaniem OFF do STM32
    this->btConnectionState.btAudioDeviceOn_Off = false;
    this->sendToMasterDeviceOnOffState();	//wysyła btAudioDeviceOn_Off == false (BT_AUDIO_DEVICE_OFF)
}

void bt_audio_sink::btAudioPlay(void){
    if (!this->btConnectionState.btAudioDeviceOn_Off){	//guard: BT wyłączony (przed pierwszym On a2dp_sink == nullptr, po Off AVRC nieaktywne)
        return;
    }
    this->a2dp_sink->play();
}

void bt_audio_sink::btAudioStop(void){
    if (!this->btConnectionState.btAudioDeviceOn_Off){	//guard: BT wyłączony (przed pierwszym On a2dp_sink == nullptr, po Off AVRC nieaktywne)
        return;
    }
    this->a2dp_sink->stop();
}

void bt_audio_sink::btAudioPause(void){
    if (!this->btConnectionState.btAudioDeviceOn_Off){	//guard: BT wyłączony (przed pierwszym On a2dp_sink == nullptr, po Off AVRC nieaktywne)
        return;
    }
    this->a2dp_sink->pause();
}

void bt_audio_sink::btAudioNext(void){
    if (!this->btConnectionState.btAudioDeviceOn_Off){	//guard: BT wyłączony (przed pierwszym On a2dp_sink == nullptr, po Off AVRC nieaktywne)
        return;
    }
this->a2dp_sink->next();
}
void bt_audio_sink::btAudioPrevious(void){
    if (!this->btConnectionState.btAudioDeviceOn_Off){	//guard: BT wyłączony (przed pierwszym On a2dp_sink == nullptr, po Off AVRC nieaktywne)
        return;
    }
    this->a2dp_sink->previous();
}

void bt_audio_sink::btAudioFastForward(void){
    if (!this->btConnectionState.btAudioDeviceOn_Off){	//guard: BT wyłączony (przed pierwszym On a2dp_sink == nullptr, po Off AVRC nieaktywne)
        return;
    }
    this->a2dp_sink->fast_forward();
}

void bt_audio_sink::btAudioRewind(void){
    if (!this->btConnectionState.btAudioDeviceOn_Off){	//guard: BT wyłączony (przed pierwszym On a2dp_sink == nullptr, po Off AVRC nieaktywne)
        return;
    }
    this->a2dp_sink->rewind();
}