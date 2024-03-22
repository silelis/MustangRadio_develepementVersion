#ifndef _i2c_Slave_buffer_addresses_
   #define _i2c_Slave_buffer_addresses_
//****************************************************//
/*       i2c COMMANDS protocol and addresses          */
//****************************************************//

// i2c protocle//
     #define command_clrl_all_packs             0x0      //command - size 1/ 0x00           / 0x0             /  0x0            /
     #define command_clrl_x_pack                0x1      //command - size 1/pack_no - size 1/ 0x0             /  0x0            /
     #define command_set_all_packs              0x2      //command - size 1/ 0x0            /colour - size 1  /  0x0            /
     #define command_set_x_pack                 0x3      //command - size 1/pack_no - size 1/colour - size 1  /  0x0            /
     #define command_blink_all_packs            0x4      //command - size 1/ 0x0            /colour_1 - size 1/colour_2 - size 1/
     #define command_blink_x_pack               0x5      //command - size 1/pack_no - size 1/colour_1 - size 1/colour_2 - size 1/
     #define command_set_backlight              0x6      //command - size 1/backlight value - size 1/0x0      /  0x0            /
     #define command_stepper_motor_percen       0x7      //command - size 1/percents value - size 4/ 
     #define command_stepper_motor_abs_val      0x8      //command - size 1/min_value - size 4/ max_value - size 4/ goto value - size 4/
     //#define command_stepper_motor_rel_val      0x9
     #define command_stepper_motor_calibrate    0xA
     // zarezerwowane dla i2c HMI adress        0x10
     
     #define command_address              0x00
     #define pack_no_addres               0x01
     #define colour_1_addres              0x02
     #define colour_2_addres              0x03  
     
     #define command_len                  0x1
     #define pack_no_len                  0x1
     #define colour_no_len                0x1
// i2c protocle//

//****************************************************//
/*       i2c COMMANDS protocol and addresses          */
//****************************************************//


//****************************************************//
/*                  LED i2c protocol                  */
//****************************************************//

/*
//hardware LEDS definition - can be changed
   #define LED1_GREEN   PIN_A0
   #define LED1_RED     PIN_A1   
   #define LED1_BLUE    PIN_A2

   #define LED2_GREEN   PIN_A3
   #define LED2_RED     PIN_A5
   #define LED2_BLUE    PIN_E0
   
   #define LED3_GREEN   PIN_E1
   #define LED3_RED     PIN_E2
   #define LED3_BLUE    PIN_C0
//hardware LEDS definition - can be changed
*/
       
   #define  unset_mask              0b10000000
   #define  colour_BLUE             0b00000001
   #define  colour_GREEN            0b00000010
   #define  colour_RED              0b00000100
   #define  colour_YELLOW           colour_RED|colour_GREEN
   #define  colour_PURPLE           colour_RED|colour_BLUE
   #define  colour_SKYBLUE          colour_GREEN|colour_BLUE
   #define  colour_WHITE            colour_BLUE|colour_GREEN|colour_RED
   #define  colour_BLACK            unset_mask|colour_WHITE
   
   #define  unset_colour_BLUE       unset_mask|0b00000001
   #define  unset_colour_GREEN      unset_mask|0b00000010
   #define  unset_colour_RED        unset_mask|0b00000100
   #define  unset_colour_YELLOW     unset_mask|colour_RED|colour_GREEN
   #define  unset_colour_PURPLE     unset_mask|colour_RED|colour_BLUE
   #define  unset_colour_SKYBLUE    unset_mask|colour_GREEN|colour_BLUE
   #define  unset_colour_WHITE      unset_mask|colour_WHITE
   
   #define  pack_1             0b00000001
   #define  pack_2             0b00000010
   #define  pack_3             0b00000011



   
   #define SB_set_x_pack_no_address                0x01
   #define SB_set_x_pack_no_len                    0x01
   #define SB_set_x_pack_Val_address               0x02   
   #define SB_set_x_pack_Val_len                   0x01    
   
   
   #define set_1st_pack                            0x1
   #define set_2nd_pack                            0x2
   #define set_3rd_pack                            0x3   
   
   #define SB_set_all_leds_address                 0x01
   #define SB_set_all_leds_len                     0x01
   
   #define SB_set_led_number_address               0x01
   #define SB_set_led_number_len                   0x01   
   
   #define SB_set_led_Val_address                  0x02
   #define SB_set_led_Val_len                      0x01
      
   #define SB_set_motor__percent_Val_address       0x01
   #define SB_set_motor__percent_len               0x04
   
   #define SB_set_abs_val_Val_address              0x01
   #define SB_set_abs_val_len                      0x04
   
   #define SB_set_motor_rel_val_min_Val_address    0x01
   #define SB_set_motor_rel_val_min_len            0x04
   
   #define SB_set_motor_rel_val_curr_Val_address   0x05
   #define SB_set_motor_rel_val_curr_len           0x04
   
   #define SB_set_motor_rel_val_max_Val_address    0x09
   #define SB_sets_motor_rel_val_curr_len          0x04
   
   #define SB_set_backlight_Val_address            0x01
   #define SB_set_backlight_len                    0x01
//****************************************************//
/*                  LED i2c protocol                  */
//****************************************************//


//****************************************************//
/*        BACKLIGHTY i2c protocol addresses           */
//****************************************************//
   #define SBL_Val_address               0x01   
   #define SBL_Val_address_len           0x01    

//****************************************************//
/*        BACKLIGHTY i2c protocol addresses           */
//****************************************************//


//****************************************************//
/*      STEPPER MOTOR i2c protocol addresses          */
//****************************************************//
   #define  STP_MOTTOR_percentage_addres  0x01
   #define  STP_MOTTOR_percentage_len     0x04
   
   #define STP_MOTTOR_min_range_address   0x01
   #define STP_MOTTOR_min_range_len       0x04
   #define STP_MOTTOR_max_range_address   0x05
   #define STP_MOTTOR_max_range_len       0x04
   #define STP_MOTTOR_goto_address        0x09
   #define STP_MOTTOR_goto_len            0x04

//****************************************************//
/*      STEPPER MOTOR i2c protocol addresses          */
//****************************************************//

//****************************************************//
/*      ACTIVE ANTENA i2c protocol addresses          */
//****************************************************//

   #define  IS_FM_ANT_ACTIVE_bit       0b00000001
   #define  IS_DAB_ANT_ACTIVE_bit      0b00000010
   #define  IS_FM_DAB_CONVERTER_bit    0b00000100


//****************************************************//
/*      ACTIVE ANTENA i2c protocol addresses          */
//****************************************************//




#endif


