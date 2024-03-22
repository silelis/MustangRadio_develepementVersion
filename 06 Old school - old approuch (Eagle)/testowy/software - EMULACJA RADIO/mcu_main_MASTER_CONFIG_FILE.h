#define master_fw_version  "MCU_fw_v0_1_ "



#ifndef __CONFIG_FILE__
#define __CONFIG_FILE__

#define _USE_LOGPORT_               //chode one of _USE_LOGPORT_ (on RB7 aka on ICSP pin) or DEBUG_ENABLED
//#define DEBUG_ENABLED             //chode one of _USE_LOGPORT_ or DEBUG_ENABLED


/*--------------------------------------------*/
/*---------- Hardware configuration ----------*/
/*--------------------------------------------*/
#define US_EU_FM_RADIO           //use one of US_EU_FM_RADIO or JAPAN_FM_RADIO
//#define JAPAN_FM_RADIO

#define EU_AM_RADIO              //use one of EU_AM_RADIO  or US_AM_RADIO
//#define US_AM_RADIO

#define _USE_BACKLIGHT_
//#define _USE_ACTIVE_ANTENA_      //importowanie ACTIVE ANTENA

#define _USE_TEA5767_
#define _USE_MNONKEYBOARD_DAB_
#define _USE_VMUSIC3_
#define _USE_AUX_
#define _USE_BLUETOOTH_
/*--------------------------------------------*/
/*---------- Hardware configuration ----------*/
/*--------------------------------------------*/

/*--------------------------------------------*/
/*-------- PIC MCU pins configuration --------*/
/*--------------------------------------------*/
#define _USE_debounce_function_
#define  Encoder_1_plus       pin_A0
#define  Encoder_1_minus      pin_A1
#define  Encoder_2_plus       pin_A2
#define  Encoder_2_minus      pin_A3
#define  button1              pin_A4
#define  button2              pin_A5
#define  button3              pin_E0
#define  button4              pin_E1
#define  keyboard_interrupt   pin_B0
//define button5        //jeœli go u¿yjesz to nale¿y go dopisaæ w ifie funkcji read_keyboard_value
//define button6        //jeœli go u¿yjesz to nale¿y go dopisaæ w ifie funkcji read_keyboard_value


#if device=="PIC18F4520"
   #define _I2C_SCL  PIN_C3
   #define _I2C_SDA  PIN_C4
#endif

#if device=="PIC18F4620"
   #define _I2C_SCL  PIN_C3
   #define _I2C_SDA  PIN_C4
#endif

/*#if device=="PIC16F1939"
   #define  _I2C_SDA             PIN_D1
   #define  _I2C_SCL             PIN_D0
#endif*/

#define FM_ANTENA_POWR_ON_OFF_pin         PIN_B5
#define DAB_FM_CONVERTER_POWR_ON_OFF_pin  PIN_B4
#define DAB_ANTENA_POWR_ON_OFF_pin         PIN_D7

/*--------------------------------------------*/
/*-------- PIC MCU pins configuration --------*/
/*--------------------------------------------*/




#endif
