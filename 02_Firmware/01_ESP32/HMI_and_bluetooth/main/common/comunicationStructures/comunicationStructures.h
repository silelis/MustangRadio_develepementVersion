#pragma once
#include "inttypes.h"

#define LED_DISPLAY_LEDS_QUANTITY	11	//from 0 to LED_DISPLAY_LEDS_QUANTITY-3 are background, last 3 are menu leds
struct ws2812Color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct colorSet {
	struct ws2812Color primary;
	struct ws2812Color secondary;
};

//#define	BACKLIGHT_LED_FROM	0
//#define	BACKLIGHT_LED_TO	LED_DISPLAY_LEDS_QUANTITY-3

struct hmiLeds {//struktura zawierająca informacje o stanie ledów wykorzystywana podczas komunikacji między master a slave
	colorSet sourceLed;
	colorSet equaliserLed;
	colorSet errorLed;
};

union keyboardUnion {
	char array[2];//daje możliwośc odczytania danych jako zwykła tabele char (niezależnie czy są to dane z klawiatury czy enkodera)
	struct //charUint8
	{
		char input;	//informacja, z którego enkodera pochodzą dane (głośnośc czy equalizer)
		int8_t value;		//informacja na temat kierunku rotacji prawo / lewo
	} encoderValue;	//daje możliwość łatwego zapisania/odczytu danych dla enkoderów

	struct //charInt8
	{
		char input;	//zzakodowana ba bitach informacja o tym, który przycisk został wciśnięty i o tym, że nastąpił long press
		uint8_t value;		//informacja, że dane pochodzą z przycisków
	} kbrdValue;//daje możliwość łatwego zapisania/odczytu danych dla klawiatury
};

typedef float StepperPositionInPercents;

struct stepperMotorStruct {
	uint8_t stepperSubcommand;
	union {
		bool stepperCalibration;		//MOTOR_SUBCOMMAND_CALIBRATION
		uint16_t gotoPosition;//MOTOR_SUBCOMMAND_GOTO_ABSOLUT, MOTOR_SUBCOMMAND_GOTO_IN_BOARDERS - position to which motor should move
		int32_t moveBy;	//												 direction -  0 negative move, 1 positive move
		StepperPositionInPercents percents;	//MOTOR_SUBCOMMAND_PERCENTS_ABSOLUT, MOTOR_SUBCOMMAND_PERCENTS_BOARDER
		uint8_t radioRestartCountdown;
	} stepperUnion;
};


struct btAudioStruct{
	uint8_t btAudioSubcommand;
	union{
		bool btAudioDeviceOn_Off;									//0- bt and i2s device "off", 1 - bt and i2s device "on"
		bt_AudioCommand audioCommand;								//sterowanie komendami bt (play, pause, ff, rew, etc)
		i2sPinStates highZstateState;								//przekazywanie informacji o stanie pinów i2s dopiero ak jest highZenablet to można przejśc do kolejnego urządzenia
		bt_onConnCallbackStats onConnCallbackStates;
		bt_onAudioStateCallbackStates onAudioStateCallbackStates;
		bt_onAVRCPlayStatusCallback onAVRCPlayStatusCallbackStates;
	}btAudioUnion;
};

typedef enum 
{
    highZenabled,
    i2sNotConfigured,
    i2sConfigured,
    highZdisabled
}i2sPinStates;

typedef enum {
    BT_ESP_A2D_CONNECTION_STATE_DISCONNECTED = 0, /*!< connection released  */
    BT_ESP_A2D_CONNECTION_STATE_CONNECTING,       /*!< connecting remote device */
    BT_ESP_A2D_CONNECTION_STATE_CONNECTED,        /*!< connection established */
    BT_ESP_A2D_CONNECTION_STATE_DISCONNECTING     /*!< disconnecting remote device */
}bt_onConnCallbackStats;

typedef enum {
    ESP_A2D_AUDIO_STATE_SUSPEND = 0,           /*!< audio stream datapath suspended by remote device */
    ESP_A2D_AUDIO_STATE_STARTED,               /*!< audio stream datapath started */
    ESP_A2D_AUDIO_STATE_STOPPED = ESP_A2D_AUDIO_STATE_SUSPEND,          /*!< @note Deprecated */
    ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND = ESP_A2D_AUDIO_STATE_SUSPEND,   /*!< @note Deprecated */
} bt_onAudioStateCallbackStates;

typedef enum {
    ESP_AVRC_PLAYBACK_STOPPED = 0,                /*!< stopped */
    ESP_AVRC_PLAYBACK_PLAYING = 1,                /*!< playing */
    ESP_AVRC_PLAYBACK_PAUSED = 2,                 /*!< paused */
    ESP_AVRC_PLAYBACK_FWD_SEEK = 3,               /*!< forward seek */
    ESP_AVRC_PLAYBACK_REV_SEEK = 4,               /*!< reverse seek */
    ESP_AVRC_PLAYBACK_ERROR = 0xFF,               /*!< error */
} bt_onAVRCPlayStatusCallback;

typedef enum {
    Play,
    Stop,
    Pause,
    Next,
    Previous,
    FastForward,
    Rewind,
}bt_AudioCommand;