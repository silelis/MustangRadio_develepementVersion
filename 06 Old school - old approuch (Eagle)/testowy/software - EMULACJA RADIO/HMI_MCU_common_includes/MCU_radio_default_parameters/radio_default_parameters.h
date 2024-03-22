#ifndef __RADIO_DEFAULTS_PARAMETERS__
   #define  __RADIO_DEFAULTS_PARAMETERS__
   
      /*--------------------------------------------*/
      /*-------- RADIO VARIABLE PARAMETERS ---------*/
      /*--------------------------------------------*/
      
//      #ifndef __menu_module_function__
 //        #include</MCU_include/modules/menu_module/menu_module.c>
  //    #endif
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
         
         #define Backlight_MAX_VAL             1
         #define Backlight_DEFAULT_VAL         Backlight_MAX_VAL
         #define Backlight_MIN_VAL             0
         
         
         
          //TODO: DAB_freuency_value
          //TODO: Backlight_value   
         
         
 /*        #ifndef __menu_module_function__
               #include</MCU_include/modules/menu_module/menu_module.c>
         #endif*/
         
         
         #define audio_input_DEFAULT_VAL       0
         #define audio_input_MIN_VAL           0
         #define audio_input_MAX_VAL           MENU_MODULE_NOTSUPPORTED_input
      /*--------------------------------------------*/
      /*-------- RADIO VARIABLE PARAMETERS ---------*/
      /*--------------------------------------------*/

#endif
