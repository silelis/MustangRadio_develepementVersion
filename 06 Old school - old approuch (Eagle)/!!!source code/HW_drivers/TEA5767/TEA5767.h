/**************************************************************************/
/*! 
    @file     tea5767.h
    @author   D. Bankowski (d.bankowski@gmail.com)
    
    @brief    Driver for the TEA5767 FM receiver.
    @section LICENSE
    Software License Agreement (BSD License)
    Copyright (c) 2017, D. Bankowski
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#ifndef _TEA5767_H_
   #define _TEA5767_H_ 
   
   
   /*       konfiguracja odbiornika wybierz jedn¹ z opcji       */
   
   #ifndef TEA5767_REGION
      #define TEA5767_REGION        "region_US_EUROPE"
      //#define TEA5767_REGION      "region_JAPAN"
   #endif
   
   #ifndef  TEA5767_crystal  
      //#define TEA5767_crystal    "_13_MHz"              //PLLREF=0 XTAL=0
      #define TEA5767_crystal    "_32_768_kHz"          //PLLREF=0 XTAL=1
      //#define TEA5767_crystal    "_6._5_MHz"            //PLLREF=1 XTAL=0
   #endif
   
   #ifndef TEA5767_search_level                          // SSL1 SSL0 Search stop level: 01 = low level (ADC = 5) 
                                                         //                              10 = mid level (ADC = 7)
                                                         //                              11 = high level (ADC = 10)
      #define  TEA5767_search_level       "LOW"
      //#define  TEA5767_search_level     "MID"
      //#define  TEA5767_search_level     "HIGH"
   #endif   
   
   
   #ifndef TEA5767_SPEAKERS_MODE
      //#define     TEA5767_SPEAKERS_MODE       "FORCE_MONO"
      #define     TEA5767_SPEAKERS_MODE       "FORCE_STEREO"
   #endif   
   
   #ifndef TEA5767_High_Low_Side_Injection
      #define  TEA5767_High_Low_Side_Injection  "HIGH"
      //#define  TEA5767_High_Low_Side_Injection  "LOW"
   #endif
   
   #ifndef TEA5767_SoftwareProgramablePort1        //Software programmable port 1: if SWP1 = 1 then port 1 is HIGH; if SWP1 = 0 then port 1 is LOW
      //#define TEA5767_SoftwareProgramablePort1     "HIGH"
      #define TEA5767_SoftwareProgramablePort1     "LOW"
   #endif
   
   #ifndef TEA5767_SoftwareProgramablePort2        //Software programmable port 2: if SWP2 = 1 then port 1 is HIGH; if SWP2 = 0 then port 1 is LOW
      //#define TEA5767_SoftwareProgramablePort2     "HIGH"
      #define TEA5767_SoftwareProgramablePort2     "LOW"
   #endif   
   
   #ifndef  TEA5767_Soft_Mute  
      #define TEA5767_Soft_Mute        "YES"
      //#define TEA5767_Soft_Mute      "NO"
   #endif
   
   #ifndef  TEA5767_High_Cut_Control  
      #define TEA5767_High_Cut_Control   "ON"
      //#define TEA5767_High_Cut_Control   "OFF"
   #endif
   
   #ifndef  TEA5767_Stereo_Noise_Cancelling  
      //#define TEA5767_Stereo_Noise_Cancelling   "ON"
      #define TEA5767_Stereo_Noise_Cancelling     "OFF"
   #endif   

   #ifndef  TEA5767_Search_Indicator  
      #define TEA5767_Search_Indicator          "pin_SWPORT1_is output_for_the_ready"            //Search Indicator: if SI = 1 then pin SWPORT1 is output for the ready flag; if SI = 0 then pin SWPORT1 is software programmable port 1
      //#define TEA5767_Search_Indicator            "pin_SWPORT1_is_software_programmable_port_1"    //Search Indicator: if SI = 1 then pin SWPORT1 is output for the ready flag; if SI = 0 then pin SWPORT1 is software programmable port 1
   #endif   
   
   #ifndef  TEA5767_DTC_time  
      #define TEA5767_DTC_time       "75us"        //if DTC = 1 then the de-emphasis time constant is 75 us; if DTC = 0 then the de-emphasis time constant is 50 us
      //#define TEA5767_DTC_time       "50us"        //if DTC = 1 then the de-emphasis time constant is 75 us; if DTC = 0 then the de-emphasis time constant is 50 us
   #endif
   
   /*       konfiguracja odbiornika wybierz jedn¹ z opcji       */

   /*=========================================================================
       I2C ADDRESS/BITS
       -----------------------------------------------------------------------*/
   #define TEA5767_ADDRESS_WRITE                      (0xC0)  // 1100000b
                                                        // b = 0 write addres 
                                                        // b = 1 read addres                                                      
   #define TEA5767_ADDRESS_READ                       (0x01)
   /*=========================================================================*/   
   
   /*=========================================================================
       I2C bytes/ bit adresses
       -----------------------------------------------------------------------*/

   /*=========================================================================
       WRITE BYTE 1
       MUTE | SEARCHMODE | PLL13 | PLL12 | PLL11 | PLL10 | PLL9 | PLL8
       -----------------------------------------------------------------------*/
   #define TEA5767_WRITE_byte_1        0

   #define TEA5767_MUTE_bit            7
   #define TEA5767_SEARCHMODE_bit      6
   #define TEA5767_PLL13_bit           5
   #define TEA5767_PLL12_bit           4
   #define TEA5767_PLL11_bit           3
   #define TEA5767_PLL10_bit           2
   #define TEA5767_PLL9_bit            1
   #define TEA5767_PLL8_bit            0
   #define TEA5767_PLL13_8_bit_mask    0x3F

   //bit:7
   #define TEA5767_MUTE_ON             1   // 1 = mute,  (mute enabled after reset)
   #define TEA5767_MUTE_OFF            0    // 0 = output enabled,
   //bit:6
   #define TEA5767_SEARCHMODE_enable   1     // 1 = Search mode enabled
   #define TEA5767_SEARCHMODE_disable  0     // 0 = Search mode disabled
   //bit:5-0
   //      PLL val is salculated for frequency by tea5767CalculatePLL 


   /*=========================================================================
       WRITE BYTE 2
       PLL7 | PLL6 | PLL5 | PLL4 | PLL3 | PLL2 | PLL1 | PLL0    
       -----------------------------------------------------------------------*/
   #define TEA5767_WRITE_byte_2     1

   #define TEA5767_PLL7_bit         7
   #define TEA5767_PLL6_bit         6
   #define TEA5767_PLL5_bit         5
   #define TEA5767_PLL4_bit         4
   #define TEA5767_PLL3_bit         3
   #define TEA5767_PLL2_bit         2
   #define TEA5767_PLL1_bit         1
   #define TEA5767_PLL0_bit         0
   #define TEA5767_PLL7_0_bit_mask  0xFF
   //bit:7-0
   //      PLL val is salculated for frequency by tea5767CalculatePLL   


   /*=========================================================================
       WRITE BYTE 3    
       SUD | SSL1 | SSL0 | HLSI | MS | MR | ML | SWP1
       -----------------------------------------------------------------------*/
   #define TEA5767_WRITE_byte_3        2

   #define TEA5767_SearchUpDown_bit                7
   #define TEA5767_SearchStopLevel_1_bit           6
   #define TEA5767_SearchStopLevel_0_bit           5
   #define TEA5767_HighLowSideInjection_bit        4
   #define TEA5767_Mono2Stereo_bit                 3
   #define TEA5767_MuteRight_bit                   2
   #define TEA5767_MuteLeft_bit                    1
   #define TEA5767_SoftwareProgrammablePort1_bit   0

   //bit:7
   #define TEA5767_SEARCHUP            1  // 1 = search up, 0 = search down
   #define TEA5767_SEARCHDOWN          0  // 1 = search up, 0 = search down
      //#define TEA5767_WRITE3_SEARCHDIRECTION              (1<<7)  // 1 = search up, 0 = search down
   //bit:6-5
   #if      TEA5767_search_level=="LOW"
      #define TEA5767_SEARCHSTOPLEVEL          (0x20)  // 0 1 low; level ADC output = 5
   #elif    TEA5767_search_level=="MID"
      #define TEA5767_SEARCHSTOPLEVEL          (0x40)  // 1 0 mid; level ADC output = 7
   #elif    TEA5767_search_level=="HIGH" 
      #define TEA5767_SEARCHSTOPLEVEL          (0x60)  // 1 1 high; level ADC output = 10
   #else
      #error "TEA5767_search_level" NOT DEFINED
   #endif
   //bit:4
   #if   TEA5767_High_Low_Side_Injection=="HIGH"
      #define  TEA5767_HLSI_value            1 //High/Low Side Injection: if HLSI = 1 then high side LO injection; if HLSI = 0 then low side LO injection
   #elif TEA5767_High_Low_Side_Injection=="LOW"
      #define  TEA5767_HLSI_value            0 //High/Low Side Injection: if HLSI = 1 then high side LO injection; if HLSI = 0 then low side LO injection
   #else
      #error "TEA5767_High_Low_Side_Injection" NOT DEFINED
   #endif
   //bit:3   
   #if   TEA5767_SPEAKERS_MODE=="FORCE_MONO"
         #define TEA5767_MONOTOSTEREO          1     //(1<<3)  // 1 = force mono, 0 = stereo on
   #elif TEA5767_SPEAKERS_MODE=="FORCE_STEREO"
         //#define TEA5767_WRITE3_STEREOTOMONO                 (0<<3)  // 1 = force mono, 0 = stereo on
         #define TEA5767_MONOTOSTEREO          0     //(0<<3)    // 1 = force mono, 0 = stereo on
   #else
         #error   "TEA5767_WRITE3_MONOTOSTEREO" NOT DEFINED
   #endif
   //bit 2
   #define TEA5767_MUTERIGHT_ON                 TEA5767_MUTE_ON     // 1 = mute right audio, 0 = enabled
   #define TEA5767_MUTERIGHT_OFF                TEA5767_MUTE_OFF    // 1 = mute right audio, 0 = enabled
   //bit 1
   #define TEA5767_MUTELEFT_ON                  TEA5767_MUTE_ON     // 1 = mute right audio, 0 = enabled
   #define TEA5767_MUTELEFT_OFF                 TEA5767_MUTE_OFF    // 1 = mute right audio, 0 = enabled
   //bit:0
   #if TEA5767_SoftwareProgramablePort1=="HIGH"
      #define  TEA5767_SWP1_value              1                   //Software programmable port 1: if SWP1 = 1 then port 1 is HIGH; if SWP1 = 0 then port 1 is LOW
   #elif TEA5767_SoftwareProgramablePort1=="LOW"
      #define  TEA5767_SWP1_value              0                   //Software programmable port 1: if SWP1 = 1 then port 1 is HIGH; if SWP1 = 0 then port 1 is LOW   
   #else
   #endif


   /*=========================================================================
       WRITE BYTE 4
       SWP2 | STBY | BL | XTAL | SMUTE | HCC | SNC | SI
       -----------------------------------------------------------------------*/
   #define TEA5767_WRITE_byte_4                 3
   
   #define TEA5767_SoftwareProgrammablePort2_bit   7
   #define TEA5767_StandBy_bit                     6
   #define TEA5767_BANDLIMITS_bit                  5
   #define TEA5767_XTAL_bit                        4
   #define TEA5767_SoftMUTE_bit                    3
   #define TEA5767_HihgCutControl_bit              2
   #define TEA5767_StereoNoiceCancelling_bit       1
   #define TEA5767_SearchIndicator_bit             0
   
   //bit:7
   #ifndef TEA5767_SoftwareProgramablePort2=="HIGH"
      #define TEA5767_SWP2_value                    1  //Software programmable port 2: if SWP2 = 1 then port 2 is HIGH; if SWP2 = 0 then port 2 is LOW           1
   #elif TEA5767_SoftwareProgramablePort2=="LOW"
      #define TEA5767_SWP2_value                    0  //Software programmable port 2: if SWP2 = 1 then port 2 is HIGH; if SWP2 = 0 then port 2 is LOW  
   #else
      #error "TEA5767_SoftwareProgramablePort2" NOT DEFINED
   #endif 
   //bit:6
   #define TEA5767_STBY_enable                  1  //Standby: if STBY = 1 then in Standby mode; if STBY = 0 then not in Standby mode
   #define TEA5767_STBY_disable                 0  //Standby: if STBY = 1 then in Standby mode; if STBY = 0 then not in Standby mode
   //bit:5
   #if TEA5767_REGION=="region_US_EUROPE"
      #define TEA5767_BANDLIMITS_value          0  // 1 = Japanese FM band, 0 = US/Europe
      #define TEA5767_FM_LIMIT_DOWN                (87500000)
      #define TEA5767_FM_LIMIT_UP                  (108000000)    
      #WARNING TEA5767 regfion US_EUROPE from TEA5767_FM_LIMIT_DOWN to TEA5767_FM_LIMIT_UP
   #elif TEA5767_REGION=="region_JAPAN"
      #define TEA5767_BANDLIMITS_value          1  // 1 = Japanese FM band, 0 = US/Europe
      #define TEA5767_FM_LIMIT_DOWN                (76000000)
      #define TEA5767_FM_LIMIT_UP                  (91000000)
      #WARNING TEA5767 regfion JAPAN from TEA5767_FM_LIMIT_DOWN to TEA5767_FM_LIMIT_UP 
   #else
      #ERROR   "TEA5767_REGION" NOT DEFINED
   #endif
   
   //bit:4
   #if TEA5767_crystal=="_13_MHz"                        //PLLREF=0 XTAL=0  
   //#ifdef crystal_13_MHz                               //PLLREF=0 XTAL=0
      #define TEA5767_XTAL_value                0
      #define  TEA5767_crystal_PLL_value      13000000
   //#endif
   //#ifdef crystal_32_768_kHz                           //PLLREF=0 XTAL=1
   #elif TEA5767_crystal=="_32_768_kHz"                  //PLLREF=0 XTAL=1
      #define TEA5767_XTAL_value                1
      #define  TEA5767_crystal_PLL_value      32768      
   //#endif
   //#ifdef crystal_6_5_MHz                              //PLLREF=1 XTAL=0
   #elif TEA5767_crystal=="_6._5_MHz"                    //PLLREF=1 XTAL=0 
      #define TEA5767_XTAL_value                0
      #define  TEA5767_crystal_PLL_value      6500000
   #else
      #error  "TEA5767_crystal" NOT DEFINED 
   #endif
   //bit3
   #if   TEA5767_Soft_Mute=="YES"
      #define TEA5767_SMute_value                    1  //Soft Mute: if SMUTE = 1 then soft mute is ON; if SMUTE = 0 then soft mute is OFF
   #elif TEA5767_Soft_Mute=="NO"
      #define TEA5767_SMute_value                    0  //Soft Mute: if SMUTE = 1 then soft mute is ON; if SMUTE = 0 then soft mute is OFF
   #else
      #error  "TEA5767_Soft_Mute" NOT DEFINED
   #endif
   //bit:2
   #if   TEA5767_High_Cut_Control=="ON"
      #define TEA5767_HCC_value                       1     //High Cut Control: if HCC = 1 then high cut control is ON; if HCC = 0 then high cut control is OFF
   #elif TEA5767_High_Cut_Control=="OFF"
      #define TEA5767_HCC_value                       0     //High Cut Control: if HCC = 1 then high cut control is ON; if HCC = 0 then high cut control is OFF
   #else
      #error  "TEA5767_High_Cut_Control" NOT DEFINED   
   #endif
   //bit:1
   #if   TEA5767_Stereo_Noise_Cancelling=="ON"
      #define TEA5767_SNC_value                       1     //Stereo Noise Cancelling: if SNC = 1 then stereo noise cancelling is ON; if SNC = 0 then stereo noise cancelling is OFF
   #elif TEA5767_Stereo_Noise_Cancelling=="OFF"
      #define TEA5767_SNC_value                       0     //Stereo Noise Cancelling: if SNC = 1 then stereo noise cancelling is ON; if SNC = 0 then stereo noise cancelling is OFF
   #else
      #error  "TEA5767_Stereo_Noise_Cancellinge" NOT DEFINED
   #endif
   //bit:0
   #if   TEA5767_Search_Indicator=="pin_SWPORT1_is output_for_the_ready"
      #define TEA5767_SI_output_ready__port1          1     //Search Indicator: if SI = 1 then pin SWPORT1 is output for the ready flag; if SI = 0 then pin SWPORT1 is software programmable port 1   
   #elif TEA5767_Search_Indicator=="pin_SWPORT1_is_software_programmable_port_1"
      #define TEA5767_SI_output_ready__port1          0     //Search Indicator: if SI = 1 then pin SWPORT1 is output for the ready flag; if SI = 0 then pin SWPORT1 is software programmable port 1   
   #else
      #error  "TEA5767_Search_Indicator" NOT DEFINED   
   #endif


   /*=========================================================================
       WRITE BYTE 5
       PLLREF | DTC | -- | -- | -- | -- | -- | --
       -----------------------------------------------------------------------*/   
   #define TEA5767_WRITE_byte_5 4
   
   #define TEA5767_PLLREF_bit                  7
   #define TEA5767_De_emphasisTimeConstant_bit       6
   
   //bit:7
   #if TEA5767_crystal=="_13_MHz"                        //PLLREF=0 XTAL=0  
   //#ifdef crystal_13_MHz                                       //PLLREF=0 XTAL=0
      #define TEA5767_PLLREF_value                0  
   //#endif
   //#ifdef crystal_32_768_kHz                                   //PLLREF=0 XTAL=1
   #elif TEA5767_crystal=="_32_768_kHz"                  //PLLREF=0 XTAL=1
      #define TEA5767_PLLREF_value                0  
   //#endif
   //#ifdef crystal_6_5_MHz                                      //PLLREF=1 XTAL=0
   #elif TEA5767_crystal=="_6._5_MHz"                    //PLLREF=1 XTAL=0 
      #define TEA5767_PLLREF_value                1
   #else
      #error  "TEA5767_crystal" NOT DEFINED
   #endif
   //bit:6
   
   #if   TEA5767_DTC_time=="75us"        //if DTC = 1 then the de-emphasis time constant is 75 us; if DTC = 0 then the de-emphasis time constant is 50 us
      #define TEA5767_DTC__value                1  //if DTC = 1 then the de-emphasis time constant is 75 µs; if DTC = 0 then the de-emphasis time constant is 50 µs
   #elif TEA5767_DTC_time=="50us"        //if DTC = 1 then the de-emphasis time constant is 75 us; if DTC = 0 then the de-emphasis time constant is 50 us     
      #define TEA5767_DTC__value                 0  //if DTC = 1 then the de-emphasis time constant is 75 µs; if DTC = 0 then the de-emphasis time constant is 50 µs
   #else
      #error  "TEA5767_DTC_time" NOT DEFINED
   #endif
   //bit:5-0
   //    not used; position is don't care

  
   /*=========================================================================
       READ BYTE 1
       RF | BLF | PLL13 | PLL12 | PLL11 | PLL 10 | PLL9 | PLL8
       -----------------------------------------------------------------------*/
   #define TEA5767_READ_byte_1        0
   
   #define TEA5767_ReadyFlag_bit      7
   #define TEA5767_BandLimitFlag_bit  6
   //PLL - same as write
/* #define TEA5767_PLL13_bit           5
   #define TEA5767_PLL12_bit           4
   #define TEA5767_PLL11_bit           3
   #define TEA5767_PLL10_bit           2
   #define TEA5767_PLL9_bit            1
   #define TEA5767_PLL8_bit            0*/  
   
   //bit:7
   #define  TEA5767_StationFound_or_LimitReached   1     //Ready Flag: if RF = 1 then a station has been found or the band limit has been reached; if RF = 0 then no station has been found
   #define  TEA5767_NO_StationFound                0     //Ready Flag: if RF = 1 then a station has been found or the band limit has been reached; if RF = 0 then no station has been found
   //bit:6
   #define  TEA5767_BandLimit_reached              1     //Band Limit Flag: if BLF = 1 then the band limit has been reached; if BLF = 0 then the band limit has not been reached
   #define  TEA5767_BandLimit_NOT_reached          0     //Band Limit Flag: if BLF = 1 then the band limit has been reached; if BLF = 0 then the band limit has not been reached   

   #define TEA5767_search_status_NOTfound_NOTreached   0b00  // no station found no band limit reached
   //#define TEA5767_search_status                   0b01  // not possible
   #define TEA5767_search_status_found_NOTreached     0b10  // station found no band limit reached
   #define TEA5767_search_status_found_reached        0b11  // station found band limit reached

//bit5-0
   //depend on TEA5767 - can be calculated back to humen value by tea5767GetFrequency and tea5767GetFrequencySignificant

   /*=========================================================================
       READ BYTE 2
       PLL7 | PLL6 | PLL5 | PLL4 | PLL3 | PLL2 | PLL1 | PLL0
       -----------------------------------------------------------------------*/   
   #define TEA5767_READ_byte_2      1
   //PLL - same as write
/*   #define TEA5767_PLL7_bit         7
   #define TEA5767_PLL6_bit         6
   #define TEA5767_PLL5_bit         5
   #define TEA5767_PLL4_bit         4
   #define TEA5767_PLL3_bit         3
   #define TEA5767_PLL2_bit         2
   #define TEA5767_PLL1_bit         1
   #define TEA5767_PLL0_bit         0*/
   
   //bit:7-0
   //depend on TEA5767 - can be calculated back to humen value by tea5767GetFrequency and tea5767GetFrequencySignificant   


   /*=========================================================================
       READ BYTE 3
       STEREO | IF6 | IF5 | IF4 | IF3 | IF2 | IF1 | IF0
       -----------------------------------------------------------------------*/   
   
   #define TEA5767_READ_byte_3 2
   
   #define TEA5767_STEREO_MONO_reception_bit 7
   #define TEA5767_IF6_bit                   6
   #define TEA5767_IF5_bit                   5
   #define TEA5767_IF4_bit                   4
   #define TEA5767_IF3_bit                   3
   #define TEA5767_IF2_bit                   2
   #define TEA5767_IF1_bit                   1
   #define TEA5767_IF0_bit                   0
   
   //bit:7
   #define TEA5767_STEREO_reception          1  //Stereo indication: if STEREO = 1 then stereo reception; if STEREO = 0 then mono reception
   #define TEA5767_MONO_reception            0  //Stereo indication: if STEREO = 1 then stereo reception; if STEREO = 0 then mono reception
   //bit:6-0
   //PLL[13:8] IF counter result

    
   /*=========================================================================
       READ BYTE 4
       LEV3 | LEV2 | LEV1 | LEV0 | CI3 | CI2 | CI1 | -
       -----------------------------------------------------------------------*/    
   
   #define TEA5767_READ_byte_4               3
   
   #define TEA5767_LEV3_bit                  7
   #define TEA5767_LEV2_bit                  6
   #define TEA5767_LEV1_bit                  5
   #define TEA5767_LEV0_bit                  4
   #define TEA5767_CI3_bit                   3
   #define TEA5767_CI2_bit                   2
   #define TEA5767_CI1_bit                   1
   #define TEA5767_0_bit                     0
   //#define   //reserved for future extensions; these bits are internally set to logic 0
   
   //bit:7-4
   #define TEA5767_station_level_low         5                 // SSL1 SSL0 Search stop level: low level (ADC = 5) mid level (ADC = 7) high level (ADC = 10)
   #define TEA5767_station_level_medium      7                 // SSL1 SSL0 Search stop level: low level (ADC = 5) mid level (ADC = 7) high level (ADC = 10)
   #define TEA5767_station_level_high        10                 // SSL1 SSL0 Search stop level: low level (ADC = 5) mid level (ADC = 7) high level (ADC = 10)
   //bit:3-1
   //Chip Identification: these bits have to be set to logic 0
   //bit:0
   //reserved for future extensions; these bits are internally set to logic 0
   
   
   /*=========================================================================
       READ BYTE 5
       - | - | - | - | - | - | - | -
       -----------------------------------------------------------------------*/
  
   #define TEA5767_READ_byte_5 4
   
   //#define   //reserved for future extensions; these bits are internally set to logic 0
   
   //bit:7-0
   //reserved for future extensions; these bits are internally set to logic 0

   /*=========================================================================*/ 
   void           tea5767SendData                  (unsigned int8 *); 
   unsigned int32 tea5767CalculatePLL              (unsigned int32);   
   void           tea5767_init                     (unsigned int32);
   void           tea5767_set_frequency            (unsigned int32);
   void           tea5767_SetCommand_SendData      (unsigned int8, int8, unsigned int8);
   void           tea5767_auto_search              (int1, unsigned int32);

   #define        tea5767_Mute(on_off_value)            (tea5767_SetCommand_SendData(TEA5767_WRITE_byte_1, on_off_value, TEA5767_MUTE_bit))

   #define        tea5767_force_MonoStereo(mono_stereo_value)  (tea5767_SetCommand_SendData(TEA5767_WRITE_byte_3 , mono_stereo_value, TEA5767_Mono2Stereo_bit))
   #define        tea5767_Mute_Right(on_off_value)             (tea5767_SetCommand_SendData(TEA5767_WRITE_byte_3, on_off_value, TEA5767_MuteRight_bit))
   #define        tea5767_Mute_Left(on_off_value)              (tea5767_SetCommand_SendData(TEA5767_WRITE_byte_3, on_off_value, TEA5767_MuteLeft_bit))


   #define        tea5767_StandBy(enable_disable_value)        (tea5767_SetCommand_SendData(TEA5767_WRITE_byte_4, enable_disable_value, TEA5767_StandBy_bit))
   
   //te funkcje sprawdziæ
   // sprawdziæ czy zmienne zwi¹zane z PLL freq musz¹ byæ unsigned int32 czy wystarczy unsigned int16
   void           tea5767ReadData                     (void);
   unsigned int32 tea5767ReadPLL_frequency            (void);
   unsigned int32 tea5767GetFrequency                 (void);
   unsigned int32 tea5767GetFrequency_humanClosest    (void);
   unsigned int8  tea5767GetStationQualityLevel       (void);
   int1           tea5767GetStereoMonoReceptionStatus (void);
   unsigned int8  tea5767GetSearchStatus              (void);
   
   
//   #define        tea5767_Mute_ALL            tea5767_SetCommand_SendData(TEA5767_WRITE_byte_1, 1, TEA5767_MUTE_bit);      // TEA5767_MUTE_ON TEA5767_MUTE_OFF
//   void           tea5767SetFreq                   (unsigned int32); //do not set Active anthena ON

//   unsigned int32 tea5767GetFrequency              (void);
//   unsigned int32 tea5767GetFrequencySignificant   (void);
//   void           tea5767ReadData                  (void);
//   void           tea5767Mute                      (void);
//   //void           tea5767Standby                   (int8);
//   void           tea5767Standby                   (int8);   
//   void           tea5767ForceMono                 (int8);  //state  ON- mono OFF-stereo
//   void           tea5767MuteRight                 (int8);  //state  ON- mute OFF-no mute
//   void           tea5767MuteLeft                  (int8);  //state  ON- mute OFF-no mute
//   void           tea5767Scan                      (unsigned int32, int8, int8);
//   void           tea5767SetBit2ScanOFF            (void);
//   BYTE           tea5767StereoReception           (void);
//   BYTE           tea5767ScanModeStatus            (void);
#endif


