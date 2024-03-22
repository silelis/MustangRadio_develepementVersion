#ifndef __menu_module_function__
   #define __menu_module_function__
   
   #ifndef __keyboard__
   #include</hardware/keyboard/kyeboard.c>
   #endif
   
   #ifndef __timeout__
   #include </core/timer/timeout.c>
   #endif
   
   #ifndef _i2c_HMI_LEDS_
      #include </hardware/I2C_HMI/i2c_HMI_LEDS/i2c_HMI_LEDS.c>
   #endif   
   
   #ifdef _USE_BACKLIGHT_
      #ifndef _i2c_HMI_BACKLIGHT_
            #include </hardware/I2C_HMI/i2c_HMI_BACKLIGHT/i2c_HMI_BACKLIGHT.c>
      #endif
   #endif
   
   //#ifdef _USE_ACTIVE_ANTENA_
  #ifndef __ACTIVE_ANTENA_MODULE_
      #include</modules/active_antena_module/active_antena_module.c>
   #endif
   //#endif
   
   #ifndef __CONFIG_FILE__
   #include<CONFIG_FILE.h>
   #endif
   
   #define MENU_MODULE_TEA5767_input           0
   #define MENU_MODULE_MONKEYBOARD_FM_input    1
   #define MENU_MODULE_MONKEYBOARD_DAB_input   2
   #define MENU_MODULE_MONKEYBOARD_AM_input    3     
   #define MENU_MODULE_VMUSIC3_input           4
   #define MENU_MODULE_VMUSIC3_RND_input       5
   #define MENU_MODULE_BLUETOOTH_input         6   
   #define MENU_MODULE_AUX_input               7
   #define MENU_MODULE_NOTSUPPORTED_input      8
   #define MENU_MODULE_audio_devices_list_len  (MENU_MODULE_NOTSUPPORTED_input-1)
   
   #define  MENU_MODULE_NO_NEED_to_back         0
   #define  MENU_MODULE_VOLUME_menu             1
   #define  MENU_MODULE_PLAY_menu               2
   #define  MENU_MODULE_EQUALIZER_menu          3
   #define  MENU_MODULE_UNSUPPORTED_menu        4
   
   //const unsigned int MENU_MODULE_audio_devices_list[MENU_MODULE_NOTSUPPORTED_input]={
   const int MENU_MODULE_audio_devices_list[MENU_MODULE_NOTSUPPORTED_input]=
       {
   #ifdef _USE_TEA5767_      MENU_MODULE_TEA5767_input,          #else MENU_MODULE_NOTSUPPORTED_input, #endif
   #ifdef _USE_MNONKEYBOARD_DAB_   MENU_MODULE_MONKEYBOARD_FM_input,   #else MENU_MODULE_NOTSUPPORTED_input, #endif
   #ifdef _USE_MNONKEYBOARD_DAB_   MENU_MODULE_MONKEYBOARD_DAB_input,  #else MENU_MODULE_NOTSUPPORTED_input, #endif
   #ifdef _USE_MNONKEYBOARD_DAB_   MENU_MODULE_MONKEYBOARD_AM_input,   #else MENU_MODULE_NOTSUPPORTED_input, #endif
   #ifdef _USE_VMUSIC3_            MENU_MODULE_VMUSIC3_input,          #else MENU_MODULE_NOTSUPPORTED_input, #endif
   #ifdef _USE_VMUSIC3_            MENU_MODULE_VMUSIC3_RND_input,      #else MENU_MODULE_NOTSUPPORTED_input, #endif
   #ifdef _USE_BLUETOOTH_          MENU_MODULE_BLUETOOTH_input,        #else MENU_MODULE_NOTSUPPORTED_input, #endif  
   #ifdef _USE_AUX_                MENU_MODULE_AUX_input               #else MENU_MODULE_NOTSUPPORTED_input  #endif
       };
   
   
   
   #define MENU_MODULE_EQUALISER_MENU_ACTIVE_ANTENA    0
   #define MENU_MODULE_EQUALISER_MENU_Treble           1
   #define MENU_MODULE_EQUALISER_MENU_Middle           2
   #define MENU_MODULE_EQUALISER_MENU_Bass             3
   #define MENU_MODULE_EQUALISER_MENU_Low_HighBoost    4
   #define MENU_MODULE_EQUALISER_MENU_Left_Right       5
   #define MENU_MODULE_EQUALISER_MENU_Front_Rear       6
   #define MENU_MODULE_EQUALISER_MENU_Backlight        7
   #define MENU_MODULE_EQUALISER_MENU_ERROR            8
   #define MENU_MODULE_equaliser_menu_list_len         (MENU_MODULE_EQUALISER_MENU_ERROR-1)
   
   //const unsigned int MENU_MODULE_equaliser_menu_list[MENU_MODULE_EQUALISER_MENU_ERROR] = {
   const int MENU_MODULE_equaliser_menu_list[MENU_MODULE_EQUALISER_MENU_ERROR] =
       {
   //#ifdef _USE_ACTIVE_ANTENA_   MENU_MODULE_EQUALISER_MENU_ACTIVE_ANTENA, #else MENU_MODULE_EQUALISER_MENU_ERROR, #endif
      MENU_MODULE_EQUALISER_MENU_ACTIVE_ANTENA,
       MENU_MODULE_EQUALISER_MENU_Treble,
       MENU_MODULE_EQUALISER_MENU_Middle,
       MENU_MODULE_EQUALISER_MENU_Bass,
       MENU_MODULE_EQUALISER_MENU_Low_HighBoost,
       MENU_MODULE_EQUALISER_MENU_Left_Right,
       MENU_MODULE_EQUALISER_MENU_Front_Rear,
   #ifdef _USE_BACKLIGHT_   MENU_MODULE_EQUALISER_MENU_Backlight #else MENU_MODULE_EQUALISER_MENU_ERROR #endif    
       };
   
   void MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD(void);
   void MENU_MODULE_reset_loop_conditions_BUTTON_KEYBOARD(void);
   void MENU_MODULE_reset_loop_conditions_TIMER_BUTTON(void);
   
   int1 MENU_MODULE_audio_device_init(int);
   #define MENU_MODULE_audio_device_stepUP            0
   #define MENU_MODULE_audio_device_stepDOWN          1
   #define MENU_MODULE_audio_device_auto_tuneUP       2
   #define MENU_MODULE_audio_device_auto_tuneDOWN     4
   //void MENU_MODULE_audio_device_stepUP(int);
   //void MENU_MODULE_audio_device_stepDOWN(int);
   void MENU_MODULE_audio_device_step_action(int, int);
   
   #define MENU_MODULE_EQUALISER_direct_UP      TRUE
   #define MENU_MODULE_EQUALISER_direct_DOWN    FALSE
   //void MENU_MODULE_submenu_action_DOWN(int);
   //void MENU_MODULE_submenu_action_UP(int);
   void MENU_MODULE_submenu_action(int, int1);
   void MENU_MODULE_submenu_set_LED(int);
   void MENU_MODULE_submenu_clear_LED(int);
   
   void MENU_MODULE_check_variable_boarders(signed int8 *, signed int8 , signed int8);
#endif

