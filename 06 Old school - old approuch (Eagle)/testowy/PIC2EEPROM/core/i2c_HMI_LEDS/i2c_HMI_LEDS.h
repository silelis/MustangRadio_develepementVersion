#ifndef _i2c_HMI_LEDS_
   #define _i2c_HMI_LEDS_
   #ifndef _i2c_Slave_buffer_addresses_
      #include <F:/Elektronika_projekty/Car_Radio/testowy/PIC2PIC_slave/core/i2c_Slave_buffer_addresses/i2c_Slave_buffer_addresses.h>
      #include <F:/Elektronika_projekty/Car_Radio/testowy/PIC2PIC_slave/core/i2c_slave_address/i2c_slave_address.h>
      
   #endif
#ifndef _I2C_ENGINE_
   #include </core/I2C_Engine/I2C_Engine.c>
#endif
   
   #define led_ON    1
   #define led_OFF   0
   
   #define  audio_led_pack       set_1st_pack
   #define  equalizer_led_pack   set_2nd_pack
   #define  AANTENA_led_pack     set_3rd_pack
   
   #define  audio_FM_R                 LED1_RED_no
   #define  audio_DAB_G                LED1_GREEN_no
   #define  audio_MP3_B                LED1_BLUE_no
   #define  audio_MP3_RND_R            LED1_RED_no
   #define  audio_MP3_RND_G            LED1_GREEN_no
   #define  audio_AUX_R                LED1_RED_no
   #define  audio_AUX_B                LED1_BLUE_no
   
   #define  equalizer_AANTENA_R        LED2_RED_no
   #define  equalizer_TREBLE_G         LED2_GREEN_no
   #define  equalizer_MIDDLE_B         LED2_BLUE_no
   #define  equalizer_BASS_R           LED2_RED_no
   #define  equalizer_BASS_G           LED2_GREEN_no
   #define  equalizer_L_H_BOOST_R      LED2_RED_no
   #define  equalizer_L_H_BOOST_B      LED2_BLUE_no
   #define  equalizer_LEFT_RIGHT_G     LED2_GREEN_no
   #define  equalizer_LEFT_RIGHT_B     LED2_BLUE_no
   #define  equalizer_FRONT_READ_R     LED2_RED_no
   #define  equalizer_FRONT_READ_G     LED2_GREEN_no
   #define  equalizer_FRONT_READ_B     LED2_BLUE_no
   
   #define  AANTENA_R                  LED3_RED_no          //ACTIVE ANTENA LED on/off
   

/*void set_all_leds(unsigned int8);
void set_one_led(unsigned int8, unsigned int8);
void set_leds_pack(unsigned int8, unsigned int8);*/

void set_all_pack_colour(int8);
void set_x_pack_colour(int8, int8);
void set_blink_all_packs(int8, int8);
void set_blink_x_pack(int8, int8, int8);
void clrl_all_packs(void);
void clrl_x_pack(int8);
#endif
   
   
   
   
   

