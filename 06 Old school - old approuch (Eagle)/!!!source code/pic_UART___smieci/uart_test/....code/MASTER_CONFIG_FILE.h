#define master_fw_version  "fw_v0_1_ "



/*#ifndef __menu_module_function__
   #include</modules/menu_module/menu_module.c>
#endif*/

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
/*-------- RADIO VARIABLE PARAMETERS ---------*/
/*--------------------------------------------*/
   #define FM_freuency_DEFAULT_VAL       87500000
   #define FM_freuency_step              100000
   #if defined US_EU_FM_RADIO
      #define FM_freuency_MIN_VAL        87500000
      #define FM_freuency_MAX_VAL        108000000
   #elif defined JAPAN_FM_RADIO
      #define FM_freuency_MIN_VAL        76000000
      #define FM_freuency_MAX_VAL        91000000
   #endif
   
   #if defined EU_AM_RADIO
      #define AM_freuency_DEFAULT_VAL       522000    //TODO: SPRAWDZIÆ PRZELICZNIK CZÊSTOTLIWOŒCI                                
      #define AM_freuency_step              9000
      #define AM_freuency_MIN_VAL           522000    //TODO: SPRAWDZIÆ PRZELICZNIK CZÊSTOTLIWOŒCI
      #define AM_freuency_MAX_VAL           1611000   //TODO: SPRAWDZIÆ PRZELICZNIK CZÊSTOTLIWOŒCI
   #elif defined US_AM_RADIO
      #define AM_freuency_DEFAULT_VAL       522000    //TODO: SPRAWDZIÆ PRZELICZNIK CZÊSTOTLIWOŒCI                                
      #define AM_freuency_step              10000
      #define AM_freuency_MIN_VAL           522000    //TODO: SPRAWDZIÆ PRZELICZNIK CZÊSTOTLIWOŒCI
      #define AM_freuency_MAX_VAL           1710000   //TODO: SPRAWDZIÆ PRZELICZNIK CZÊSTOTLIWOŒCI  
   #endif   
   
   #define audio_input_DEFAULT_VAL       0
   #define audio_input_MIN_VAL           0
   #define audio_input_MAX_VAL           MENU_MODULE_NOTSUPPORTED_input
   
   
   #define active_antena_MIN_VAL         0
   #define active_antena_MAX_VAL         1
   
   #define EQU_VOL_DEFAULT_VAL           5  
   #define EQU_VOL_MIN_VAL               0
   #define EQU_VOL_MAX_VAL               40  //const int8 AUDIO_speaker_LeftRight_FrontRear[41], wiêc 41-1
   
   #define EQU_Treble_DEFAULT_VAL        0    
   #define EQU_Treble_MIN_VAL            0   
   #define EQU_Treble_MAX_VAL            30  //const int8 AUDIO_Treble_Middle_Bass[31], wiêc 31-1
   
   #define EQU_Middle_DEFAULT_VAL        0
   #define EQU_Middle_MIN_VAL            0
   #define EQU_Middle_MAX_VAL            30  //const int8 AUDIO_Treble_Middle_Bass[31], wiêc 31-1
   
   #define EQU_Bass_DEFAULT_VAL          0
   #define EQU_Bass_MIN_VAL              0
   #define EQU_Bass_MAX_VAL              30  //const int8 AUDIO_Treble_Middle_Bass[31], wiêc 31-1  
   
   #define EQU_L_H_Boost_DEFAULT_VAL     0
   #define EQU_L_H_Boost_MIN_VAL         0
   #define EQU_L_H_Boost_MAX_VAL         1   //const int8 AUDIO_low_high_boost[2], wiêc 2-1
   
   #define EQU_Left_Right_DEFAULT_VAL    0  
   #define EQU_Left_Right_MIN_VAL        -6 //0xFA      //-6 w hex
   #define EQU_Left_Right_MAX_VAL        6         //const int8 AUDIO_speaker_LeftRight_FrontRear[7], wiêc 7-1
   
   #define EQU_Front_Rear_DEFAULT_VAL    0
   #define EQU_Front_Rear_MIN_VAL        -6 //0xFA      //-6 w hex
   #define EQU_Front_Rear_MAX_VAL        6   //const int8 AUDIO_speaker_LeftRight_FrontRear[7], wiêc 7-1   
   
   #define Backlight_DEFAULT_VAL         5
   #define Backlight_MIN_VAL             0
   #define Backlight_MAX_VAL             10
   
   
    //TODO: DAB_freuency_value
    //TODO: Backlight_value   
   
/*--------------------------------------------*/
/*-------- RADIO VARIABLE PARAMETERS ---------*/
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

#if device=="PIC16F1939"
   #define  _I2C_SDA             PIN_D1
   #define  _I2C_SCL             PIN_D0
#endif

#define ActiveAntena          PIN_B5
/*--------------------------------------------*/
/*-------- PIC MCU pins configuration --------*/
/*--------------------------------------------*/




#endif
