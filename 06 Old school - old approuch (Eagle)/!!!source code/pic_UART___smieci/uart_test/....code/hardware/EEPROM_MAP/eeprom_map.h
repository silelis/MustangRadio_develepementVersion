#ifndef __EEPROM_MAP_
   #define __EEPROM_MAP_
   

   #define  RADIO_1st_run_addr   0x00     //value: 0 - 1;
   #define  RADIO_1st_run_len    0x1

   #define  RADIO_Input_addr     0x01     //val: 0 - 7;
   #define  RADIO_Input_len      0x1   

   #define  Active_Antena_addr   0x02     //val: 0 - 1;
   #define  Active_Antena_len    0x1

   #define  FM_freuency_addr     0x03     //dla Japonii 76 000 000 - 91 000 000 (Hz) plus TV audio at 108 000 000 (Hz) dla USA / EURPOE 87 500 000 Hz - 108 000 000 Hz
   #define  FM_freuency_len      0x4

   #define  EQU_Vol_addr         0x07     //val: 0 - 40
   #define  EUQ_Vol_len          0x1
     
   #define  EQU_Treble_addr      0x08     //val: 0 - 31
   #define  EQU_Treble_len       0x1
   
   #define  EQU_Middle_addr      0x09     //val: 0 - 31
   #define  EQU_Middle_len       0x1
   
   #define  EQU_Bass_addr        0x0A     //val: 0 - 31
   #define  EQU_Bass_len         0x1
   
   #define  EQU_L_H_Boost_addr   0x0B     //value: 0 - 1;
   #define  EQU_L_H_Boost_len    0x1
   
   #define  EQU_Left_Right_addr  0x0C     //value: 0 - 40;
   #define  EQU_Left_Right_len   0x1
   
   #define  EQU_Front_Rear_addr  0x0D     //value: 0 - 40;
   #define  EQU_Front_Rear_len   0x1
     
   #define  Backlight_addr       0x0E     //value: 0 - 63;
   #define  Backlight_len        0x1
   
   // 0x0F wolne
   
   #define  AM_freuency_addr     0x10     //dla EUROPY 522 000 - 1 611 000 (Hz) dla USA 522 000 Hz - 1 710 000 Hz
   #define  AM_freuency_len      0x4
   
   
     
//   #define  DAB_freuency_addr
//   #define  DAB_freuency_len
   
//   #define  Backlight_addr
//   #define  Backlight_len   
   
   
   
  
#endif

