/**************************************************************************/
/*! 
    @file     TDA7418.h
    @author   D. Bankowski (d.bankowski@gmail.com)
    
    @brief    Driver for the TDA7418 3 band car audio processor.
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

#ifndef _TDA7418_
   #define _TDA7418_
   
   /*=========================================================================
       I2C ADDRESS/BITS
       -----------------------------------------------------------------------*/
   #define TDA7418_ADDRESS_Write                      (0x88)  // 1 0 0 0 1 0 0 R/W   
                                                        // R/W  = 1 write addres 
                                                        // R/W  = 0 read addres                                                      
   #define TDA7418_ADDRESS_Read                       (0x89)
   /*=========================================================================*/ 

   /*=========================================================================
   TDA7418 - Subaddress (receive mode)
       -----------------------------------------------------------------------*/
       
   #define TDA7418_SubAddr_Testing_Mode_ON         0b10000000
   #define TDA7418_SubAddr_Testing_Mode_OFF        0b00000000
   #define TDA7418_SubAddr_Auto_Zero_Remain_ON     0b01000000
   #define TDA7418_SubAddr_Auto_Zero_Remain_OFF    0b00000000
   #define TDA7418_SubAddr_Auto_Increment_Mode_ON  0b00100000
   #define TDA7418_SubAddr_Auto_Increment_Mode_OFF 0b00000000
   #define TDA7418_SubAddr_Src                     0x0   //Main Source Selector
   #define TDA7418_SubAddr_Loud                    0x01  //Loudness
   #define TDA7418_SubAddr_Vol                     0x02  //Volume
   #define TDA7418_SubAddr_Tre                     0x03  //Treble
   #define TDA7418_SubAddr_Mid                     0x04  //Middle
   #define TDA7418_SubAddr_Bas                     0x05  //Bass
   #define TDA7418_SubAddr_FC                      0x06  //Middle/Bass Fc Selector
   #define TDA7418_SubAddr_LF                      0x07  //Speaker Attenuator Left Front
   #define TDA7418_SubAddr_LR                      0x08  //Speaker Attenuator Left Right
   #define TDA7418_SubAddr_RR                      0x09  //Speaker Attenuator Right Rear
   #define TDA7418_SubAddr_RF                      0x0A  //Speaker Attenuator Right Front
   #define TDA7418_SubAddr_SW                      0x0B  //Subwoofer Attenuator 
   #define TDA7418_SubAddr_Soft_Mute               0x0C  //Soft Mute / Soft Step
   #define TDA7418_SubAddr_Test_Processor          0x0D  //Testing Audio Processor
   /*=========================================================================*/      
   
   /*=========================================================================
   TDA7418 - Input selector /gain (0) - TDA7418_SubAddr_Src
       -----------------------------------------------------------------------*/
   /* bits 2 to 0 */
   
   #define TDA7418_INPUT_PD_SE4                    0x00  //Source Selector PD/SE4
   #define TDA7418_INPUT_SE1                       0x01  //Source Selector SE1
   #define TDA7418_INPUT_SE2                       0x02  //Source Selector SE2
   #define TDA7418_INPUT_SE3                       0x03  //Source Selector SE3
   #define TDA7418_INPUT_MUTE                      0x04  //Source Selector SE3
   /* bits 2 to 0 */
   
   /* bits 6 to 3 */
   #define TDA7418_INPUT_GAIN_0dB                  (0x0<<3)   //Input Gain 0dB
   #define TDA7418_INPUT_GAIN_1dB                  (0x1<<3)   //Input Gain 1dB   
   #define TDA7418_INPUT_GAIN_2dB                  (0x2<<3)   //Input Gain 2dB
   #define TDA7418_INPUT_GAIN_3dB                  (0x3<<3)   //Input Gain 3dB  
   #define TDA7418_INPUT_GAIN_4dB                  (0x4<<3)   //Input Gain 4dB 
   #define TDA7418_INPUT_GAIN_5dB                  (0x5<<3)   //Input Gain 5dB 
   #define TDA7418_INPUT_GAIN_6dB                  (0x6<<3)   //Input Gain 6dB            
   #define TDA7418_INPUT_GAIN_7dB                  (0x7<<3)   //Input Gain 7dB
   #define TDA7418_INPUT_GAIN_8dB                  (0x8<<3)   //Input Gain 8dB
   #define TDA7418_INPUT_GAIN_9dB                  (0x9<<3)   //Input Gain 9dB 
   #define TDA7418_INPUT_GAIN_10dB                 (0xA<<3)   //Input Gain 10dB 
   #define TDA7418_INPUT_GAIN_11dB                 (0xB<<3)   //Input Gain 11dB 
   #define TDA7418_INPUT_GAIN_12dB                 (0xC<<3)   //Input Gain 12dB 
   #define TDA7418_INPUT_GAIN_13dB                 (0xD<<3)   //Input Gain 13dB 
   #define TDA7418_INPUT_GAIN_14dB                 (0xE<<3)   //Input Gain 14dB 
   #define TDA7418_INPUT_GAIN_15dB                 (0xF<<3)   //Input Gain 15dB 
   /* bits 6 to 3 */   
   
   /* bits 7 */   
   #define TDA7418_DIFFIN_MODE_SINGLE              (0<<7)   //Single Ended Stereo
   #define TDA7418_DIFFIN_MODE_DIFFERENTIAL        (1<<7)   //Differential Stereo
   /* bits 7 */   
   /*=========================================================================*/
   
   /*=========================================================================
   TDA7418 - Loudness (1) - TDA7418_SubAddr_Loud
       -----------------------------------------------------------------------*/
   /* bits 3 to 0 */ 
   #define TDA7418_ATTENUATION_0dB           (0x0)   //Input Gain 0dB
   #define TDA7418_ATTENUATION_1dB           (0x1)   //Input Gain -1dB   
   #define TDA7418_ATTENUATION_2dB           (0x2)   //Input Gain -2dB
   #define TDA7418_ATTENUATION_3dB           (0x3)   //Input Gain -3dB  
   #define TDA7418_ATTENUATION_4dB           (0x4)   //Input Gain -4dB 
   #define TDA7418_ATTENUATION_5dB           (0x5)   //Input Gain -5dB 
   #define TDA7418_ATTENUATION_6dB           (0x6)   //Input Gain -6dB            
   #define TDA7418_ATTENUATION_7dB           (0x7)   //Input Gain -7dB
   #define TDA7418_ATTENUATION_8dB           (0x8)   //Input Gain -8dB
   #define TDA7418_ATTENUATION_9dB           (0x9)   //Input Gain -9dB 
   #define TDA7418_ATTENUATION_10dB          (0xA)   //Input Gain -10dB 
   #define TDA7418_ATTENUATION_11dB          (0xB)   //Input Gain -11dB 
   #define TDA7418_ATTENUATION_12dB          (0xC)   //Input Gain -12dB 
   #define TDA7418_ATTENUATION_13dB          (0xD)   //Input Gain -13dB 
   #define TDA7418_ATTENUATION_14dB          (0xE)   //Input Gain -14dB 
   #define TDA7418_ATTENUATION_15dB          (0xF)   //Input Gain -15dB    
   /* bits 3 to 0 */    

   /* bits 5 to 4 */
   //#define TDA7418_CENTER_FREQ_OFF           //choose on of thise
   #define TDA7418_CENTER_FREQ_400           //choose on of thise
   //#define TDA7418_CENTER_FREQ_800           //choose on of thise
   //#define TDA7418_CENTER_FREQ_2400          //choose on of thise

   #ifdef   TDA7418_CENTER_FREQ_OFF
      #define  TDA7418_CENTER_FREQ           0b00000000 //Filter/Center Frequency OFF
   #endif
   
   #ifdef   TDA7418_CENTER_FREQ_400
      #define  TDA7418_CENTER_FREQ           0b00010000   //Filter/Center Frequency 400Hz
   #endif   

    #ifdef   TDA7418_CENTER_FREQ_800
      #define  TDA7418_CENTER_FREQ           0b00100000   //Filter/Center Frequency 800Hz
   #endif
   
   #ifdef   TDA7418_CENTER_FREQ_2400
      #define  TDA7418_CENTER_FREQ           0b00110000   //Filter/Center Frequency 2400Hz
   #endif
   
   /* bits 5 to 4 */
   
   /* bits 6 */   
   #define TDA7418_SHAPE_LOW_BOOST           0b0000000   //Shape Low Boost
   #define TDA7418_SHAPE_LOW_HIGH_BOOST      0b0100000   //Shape Low Boost & HighBoost
   /* bits 6 */     
   
   /* bits 7 */    
   #define TDA7418_LOUDNESS_SOFT_STEF_ON     0b00000000  //Loudness Soft Step ON
   #define TDA7418_LOUDNESS_SOFT_STEF_OFF    0b10000000  //Loudness Soft Step OFF
   /* bits 7 */       
   /*=========================================================================*/   


   /*=========================================================================
   TDA7418 - Volume / speaker attenuation (2,7,8,9,10,11) - TDA7418_SubAddr_Vol,
            TDA7418_SubAddr_LF, TDA7418_SubAddr_LR, TDA7418_SubAddr_RR,
            TDA7418_SubAddr_RF, TDA7418_SubAddr_SW
       -----------------------------------------------------------------------*/
   /* bits 6 to 0 */ 
   // see const int8 AUDIO_volume_attenuation[41] - in TDA7418.c
   /* bits 6 to 0 */ 

   /* bits 7 */
   
   #define TDA7418_VOLUME_SOFT_STEP_ON          // use one of TDA7418_VOLUME_SOFT_STEP_ON or TDA7418_VOLUME_SOFT_STEP_OFF 
   //#define TDA7418_VOLUME_SOFT_STEP_OFF       // use one of TDA7418_VOLUME_SOFT_STEP_ON or TDA7418_VOLUME_SOFT_STEP_OFF 
   
   #ifdef TDA7418_VOLUME_SOFT_STEP_ON        
      #define TDA7418_VOLUME_SOFT_STEP          0b00000000
   #endif
   
   #ifdef TDA7418_VOLUME_SOFT_STEP_OFF
      #define TDA7418_VOLUME_SOFT_STEP          0b10000000
   #endif
   /* bits 7 */ 
    /*=========================================================================*/ 

   /*=========================================================================
   TDA7418 - Treble filter (3) - TDA7418_SubAddr_Tre
       -----------------------------------------------------------------------*/
   /* bits 4 to 0 */ 
   // see const int8 AUDIO_Treble_Middle_Bass[31]  - in TDA7418.c
   /* bits 4 to 0 */ 
   
   /* bits 6 to 5*/ 
   
   //#define  TDA7418_TREBLE_CENTRAL_FREQUENCY_10_0kHz        // use one of TDA7418_TREBLE_CENTRAL_FREQUENCY_10_0kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_12_5kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_15_0kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_17_5kHz
   #define   TDA7418_TREBLE_CENTRAL_FREQUENCY_12_5kHz     // use one of TDA7418_TREBLE_CENTRAL_FREQUENCY_10_0kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_12_5kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_15_0kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_17_5kHz
   //#define   TDA7418_TREBLE_CENTRAL_FREQUENCY_15_0kHz     // use one of TDA7418_TREBLE_CENTRAL_FREQUENCY_10_0kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_12_5kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_15_0kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_17_5kHz
   //#define   TDA7418_TREBLE_CENTRAL_FREQUENCY_17_5kHz     // use one of TDA7418_TREBLE_CENTRAL_FREQUENCY_10_0kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_12_5kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_15_0kHz or TDA7418_TREBLE_CENTRAL_FREQUENCY_17_5kHz
      
   #ifdef   TDA7418_TREBLE_CENTRAL_FREQUENCY_10_0kHz
      #define  TDA7418_TREBLE_CENTRAL_FREQUENCY          0b00000000
   #endif
   #ifdef   TDA7418_TREBLE_CENTRAL_FREQUENCY_12_5kHz
      #define  TDA7418_TREBLE_CENTRAL_FREQUENCY          0b00100000
   #endif
   #ifdef   TDA7418_TREBLE_CENTRAL_FREQUENCY_15_0kHz
      #define  TDA7418_TREBLE_CENTRAL_FREQUENCY          0b01000000
   #endif
   #ifdef   TDA7418_TREBLE_CENTRAL_FREQUENCY_17_5kHz
      #define  TDA7418_TREBLE_CENTRAL_FREQUENCY          0b01100000
   #endif 
   /* bits 6 to 5*/ 
      
   /* bits 7*/ 
   #define  TDA7418_TREBLE_BIT7                       0b10000000     //must "be" 1
   /* bits 7*/
   /*=========================================================================*/
    
   /*=========================================================================
   TDA7418 - Middle filter (4) - TDA7418_SubAddr_Mid
       -----------------------------------------------------------------------*/
   /* bits 4 to 0 */ 
   // see const int8 AUDIO_Treble_Middle_Bass[31]  - in TDA7418.c
   /* bits 4 to 0 */ 
   
   /* bits 6 to 5*/ 
   //#define   TDA7418_MIDDLE_Q_FACTOR_0_5               // use one of TDA7418_MIDDLE_Q_FACTOR_0_5 or TDA7418_MIDDLE_Q_FACTOR_0_75 or TDA7418_MIDDLE_Q_FACTOR_1 or TDA7418_MIDDLE_Q_FACTOR_1_25
   //#define   TDA7418_MIDDLE_Q_FACTOR_0_75              // use one of TDA7418_MIDDLE_Q_FACTOR_0_5 or TDA7418_MIDDLE_Q_FACTOR_0_75 or TDA7418_MIDDLE_Q_FACTOR_1 or TDA7418_MIDDLE_Q_FACTOR_1_25
   #define   TDA7418_MIDDLE_Q_FACTOR_1                   // use one of TDA7418_MIDDLE_Q_FACTOR_0_5 or TDA7418_MIDDLE_Q_FACTOR_0_75 or TDA7418_MIDDLE_Q_FACTOR_1 or TDA7418_MIDDLE_Q_FACTOR_1_25
   //#define   TDA7418_MIDDLE_Q_FACTOR_1_25              // use one of TDA7418_MIDDLE_Q_FACTOR_0_5 or TDA7418_MIDDLE_Q_FACTOR_0_75 or TDA7418_MIDDLE_Q_FACTOR_1 or TDA7418_MIDDLE_Q_FACTOR_1_25
   
   #ifdef   TDA7418_MIDDLE_Q_FACTOR_0_5
      #define TDA7418_MIDDLE_Q_FACTOR         0b00000000
   #endif
   #ifdef   TDA7418_MIDDLE_Q_FACTOR_0_75
      #define TDA7418_MIDDLE_Q_FACTOR         0b0100000
   #endif
   #ifdef   TDA7418_MIDDLE_Q_FACTOR_1
      #define TDA7418_MIDDLE_Q_FACTOR         0b01000000
   #endif
   #ifdef   TDA7418_MIDDLE_Q_FACTOR_1_25
      #define TDA7418_MIDDLE_Q_FACTOR         0b01100000
   #endif
   /* bits 6 to 5*/ 

   /* bits 7*/
   #define TDA7418_MIDDLE_SOFT_STEP_OFF               // use one of TDA7418_MIDDLE_SOFT_STEP_OFF or TDA7418_MIDDLE_SOFT_STEP_ON
   //#define TDA7418_MIDDLE_SOFT_STEP_ON
   
   #ifdef   TDA7418_MIDDLE_SOFT_STEP_ON
      #define TDA7418_MIDDLE_SOFT_STEP       0b00000000
   #endif
   
   #ifdef   TDA7418_MIDDLE_SOFT_STEP_OFF 
      #define TDA7418_MIDDLE_SOFT_STEP       0b10000000
   #endif
   /* bits 7*/
   /*=========================================================================*/   

   /*=========================================================================
   TDA7418 - Middle / bass Fc select (6) - TDA7418_SubAddr_Bas
       -----------------------------------------------------------------------*/
   /* bits 4 to 0 */ 
   // see const int8 AUDIO_Treble_Middle_Bass[31]  - in TDA7418.c
   /* bits 4 to 0 */ 
   
   /* bits 6 to 5*/
   #define   TDA7418_BASS_Q_FACTOR_1_0          // use one of TDA7418_Bass_Q_FACTOR_1_0  or TDA7418_Bass_Q_FACTOR_1_25 or DA7418_Bass_Q_FACTOR_1_5 or TDA7418_Bass_Q_FACTOR_2_0
   //#define   TDA7418_BASS_Q_FACTOR_1_25         // use one of TDA7418_Bass_Q_FACTOR_1_0  or TDA7418_Bass_Q_FACTOR_1_25 or DA7418_Bass_Q_FACTOR_1_5 or TDA7418_Bass_Q_FACTOR_2_0
   //#define   TDA7418_BASS_Q_FACTOR_1_5          // use one of TDA7418_Bass_Q_FACTOR_1_0  or TDA7418_Bass_Q_FACTOR_1_25 or DA7418_Bass_Q_FACTOR_1_5 or TDA7418_Bass_Q_FACTOR_2_0
   //#define   TDA7418_BASS_Q_FACTOR_2_0          // use one of TDA7418_Bass_Q_FACTOR_1_0  or TDA7418_Bass_Q_FACTOR_1_25 or DA7418_Bass_Q_FACTOR_1_5 or TDA7418_Bass_Q_FACTOR_2_0
   
   #ifdef   TDA7418_BASS_Q_FACTOR_1_0
      #define TDA7418_BASS_Q_FACTOR       0b00000000
   #endif
   #ifdef   TDA7418_BASS_Q_FACTOR_1_25
      #define TDA7418_BASS_Q_FACTOR       0b00100000
   #endif   
    #ifdef  TDA7418_Bass_Q_FACTOR_1_5
      #define TDA7418_BASS_Q_FACTOR       0b01000000
   #endif  
    #ifdef  TDA7418_BASS_Q_FACTOR_2_0
      #define TDA7418_BASS_Q_FACTOR       0b01100000
   #endif   
   /* bits 6 to 5*/

   /* bits 7*/
   #define TDA7418_BASS_SOFT_STEP_OFF        // use one of TDA7418_BASS_SOFT_STEP_OFF or TDA7418_BASS_SOFT_STEP_ON
   //#define TDA7418_BASS_SOFT_STEP_ON
   #ifdef   TDA7418_BASS_SOFT_STEP_OFF
      #define  TDA7418_BASS_SOFT_STEP    0b10000000
   #endif
   #ifdef   TDA7418_BASS_SOFT_STEP_ON
      #define  TDA7418_BASS_SOFT_STEP    0b00000000
   #endif
   /* bits 7*/
   /*=========================================================================*/ 

   /*=========================================================================
   TDA7418 - Bass filter (5) - TDA7418_SubAddr_FC
       -----------------------------------------------------------------------*/
   /* bits 1 to 0 */ 
   //#define TDA7418_MIDDLE_CENTER_FREQUENCY_500Hz      // use one of TDA7418_MIDDLE_CENTER_FREQUENCY_500Hz  or TDA7418_MIDDLE_CENTER_FREQUENCY_1kHz or TDA7418_MIDDLE_CENTER_FREQUENCY_1_5kHz or TDA7418_MIDDLE_CENTER_FREQUENCY_2_5kHz
   //#define TDA7418_MIDDLE_CENTER_FREQUENCY_1kHz      // use one of TDA7418_MIDDLE_CENTER_FREQUENCY_500Hz  or TDA7418_MIDDLE_CENTER_FREQUENCY_1kHz or TDA7418_MIDDLE_CENTER_FREQUENCY_1_5kHz or TDA7418_MIDDLE_CENTER_FREQUENCY_2_5kHz
   #define TDA7418_MIDDLE_CENTER_FREQUENCY_1_5kHz    // use one of TDA7418_MIDDLE_CENTER_FREQUENCY_500Hz  or TDA7418_MIDDLE_CENTER_FREQUENCY_1kHz or TDA7418_MIDDLE_CENTER_FREQUENCY_1_5kHz or TDA7418_MIDDLE_CENTER_FREQUENCY_2_5kHz
   //#define TDA7418_MIDDLE_CENTER_FREQUENCY_2_5kHz    // use one of TDA7418_MIDDLE_CENTER_FREQUENCY_500Hz  or TDA7418_MIDDLE_CENTER_FREQUENCY_1kHz or TDA7418_MIDDLE_CENTER_FREQUENCY_1_5kHz or TDA7418_MIDDLE_CENTER_FREQUENCY_2_5kHz
   
   #ifdef   TDA7418_MIDDLE_CENTER_FREQUENCY_500Hz
      #define  TDA7418_MIDDLE_CENTER_FREQUENCY     0b00000000
   #endif
   #ifdef   TDA7418_MIDDLE_CENTER_FREQUENCY_1kHz
      #define  TDA7418_MIDDLE_CENTER_FREQUENCY     0b00000001
   #endif
   #ifdef   TDA7418_MIDDLE_CENTER_FREQUENCY_1_5kHz
      #define  TDA7418_MIDDLE_CENTER_FREQUENCY     0b00000010
   #endif
   #ifdef   TDA7418_MIDDLE_CENTER_FREQUENCY_2_5kHz
      #define  TDA7418_MIDDLE_CENTER_FREQUENCY     0b00000011
   #endif
   /* bits 1 to 0 */ 
   
   /* bits 3 to 2 */ 
   //#define TDA7418_BASS_CENTER_FREQUENCY_60Hz      // use one of TDA7418_BASS_CENTER_FREQUENCY_60Hz or TDA7418_BASS_CENTER_FREQUENCY_80Hz or TDA7418_BASS_CENTER_FREQUENCY_100Hz or TDA7418_BASS_CENTER_FREQUENCY_200Hz
   //#define TDA7418_BASS_CENTER_FREQUENCY_80Hz      // use one of TDA7418_BASS_CENTER_FREQUENCY_60Hz or TDA7418_BASS_CENTER_FREQUENCY_80Hz or TDA7418_BASS_CENTER_FREQUENCY_100Hz or TDA7418_BASS_CENTER_FREQUENCY_200Hz
   //#define TDA7418_BASS_CENTER_FREQUENCY_100Hz     // use one of TDA7418_BASS_CENTER_FREQUENCY_60Hz or TDA7418_BASS_CENTER_FREQUENCY_80Hz or TDA7418_BASS_CENTER_FREQUENCY_100Hz or TDA7418_BASS_CENTER_FREQUENCY_200Hz
   #define TDA7418_BASS_CENTER_FREQUENCY_200Hz     // use one of TDA7418_BASS_CENTER_FREQUENCY_60Hz or TDA7418_BASS_CENTER_FREQUENCY_80Hz or TDA7418_BASS_CENTER_FREQUENCY_100Hz or TDA7418_BASS_CENTER_FREQUENCY_200Hz

   #ifdef   TDA7418_BASS_CENTER_FREQUENCY_60Hz
      #define  TDA7418_BASS_CENTER_FREQUENCY    0b00000000
   #endif
   #ifdef   TDA7418_BASS_CENTER_FREQUENCY_80Hz
      #define  TDA7418_BASS_CENTER_FREQUENCY    0b00000100
   #endif
   #ifdef   TDA7418_BASS_CENTER_FREQUENCY_100Hz
      #define  TDA7418_BASS_CENTER_FREQUENCY    0b00001000
   #endif
   #ifdef   TDA7418_BASS_CENTER_FREQUENCY_200Hz
      #define  TDA7418_BASS_CENTER_FREQUENCY    0b00001100
   #endif   
   /* bits 3 to 2 */ 

   /* bits 4 to 4 */
   #define TDA7418_BASS_DC_MODE_OFF                       // use one of TDA7418_BASS_DC_MODE_OFF or TDA7418_BASS_DC_MODE_ON
   //#define TDA7418_BASS_DC_MODE_ON                        // use one of TDA7418_BASS_DC_MODE_OFF or TDA7418_BASS_DC_MODE_ON
   
   #ifdef   TDA7418_BASS_DC_MODE_OFF
      #define  TDA7418_BASS_DC_MODE             0b00000000
   #endif
   #ifdef   TDA7418_BASS_DC_MODE_ON
      #define  TDA7418_BASS_DC_MODE             0b00010000
   #endif   
   /* bits 4 to 4 */
      
   /* bits 5 to 5 */  
   #define TDA7418_SMOOTHING_FILTER_OFF               // use one of TDA7418_SMOOTHING_FILTER_OFF or TDA7418_SMOOTHING_FILTER_ON
   //#define TDA7418_SMOOTHING_FILTER_ON                // use one of TDA7418_SMOOTHING_FILTER_OFF or TDA7418_SMOOTHING_FILTER_ON
   
   #ifdef   TDA7418_SMOOTHING_FILTER_OFF
      #define  TDA7418_SMOOTHING_FILTER         0b00000000
   #endif
   #ifdef   TDA7418_SMOOTHING_FILTER_ON
      #define  TDA7418_SMOOTHING_FILTER         0b00100000
   #endif   
   /* bits 5 to 5 */     
   
   /* bits 7 to 6 */
   // any value is possible bits not used
   /* bits 7 to 6 */      
   /*=========================================================================*/    
   
   /*=========================================================================
   TDA7418 - Soft mute and soft step time (12) - TDA7418_SubAddr_Soft_Mute
       -----------------------------------------------------------------------*/   
   /* bits 0 to 0 */
   //#define TDA7418_SOFT_MUTE_OFF                      // use one of TDA7418_SOFT_MUTE_OFF or TDA7418_SOFT_MUTE_ON
   #define TDA7418_SOFT_MUTE_ON                       // use one of TDA7418_SOFT_MUTE_OFF or TDA7418_SOFT_MUTE_ON  
   
   #ifdef   TDA7418_SOFT_MUTE_OFF 
      #define  TDA7418_SOFT_MUTE                0b00000000
   #endif
   #ifdef   TDA7418_SOFT_MUTE_ON 
      #define  TDA7418_SOFT_MUTE                0b00000001
   #endif   
   /* bits 0 to 0 */
   
   /* bits 2 to 1 */   
   //#define TDA7418_SOFT_MUTE_TIME_0_48ms            // use one of TDA7418_SOFT_MUTE_TIME_0_48ms or TDA7418_SOFT_MUTE_TIME_0_96ms or TDA7418_SOFT_MUTE_TIME_123ms
   //#define TDA7418_SOFT_MUTE_TIME_0_96ms            // use one of TDA7418_SOFT_MUTE_TIME_0_48ms or TDA7418_SOFT_MUTE_TIME_0_96ms or TDA7418_SOFT_MUTE_TIME_123ms
   #define TDA7418_SOFT_MUTE_TIME_123ms             // use one of TDA7418_SOFT_MUTE_TIME_0_48ms or TDA7418_SOFT_MUTE_TIME_0_96ms or TDA7418_SOFT_MUTE_TIME_123ms
   
   #ifdef   TDA7418_SOFT_MUTE_TIME_0_48ms
      #define  TDA7418_SOFT_MUTE_TIME           0b00000000
   #endif
   #ifdef   TDA7418_SOFT_MUTE_TIME_0_96ms
      #define  TDA7418_SOFT_MUTE_TIME           0b00000010
   #endif   
   #ifdef   TDA7418_SOFT_MUTE_TIME_123ms
      #define  TDA7418_SOFT_MUTE_TIME           0b00000100
   #endif  
   /* bits 2 to 1 */   
   
   /* bits 5 to 3 */    
   //#define  TDA7418_SOFT_STEP_TIME_0_16ms           // use one of TDA7418_SOFT_STEP_TIME_0_16ms or TDA7418_SOFT_STEP_TIME_0_32ms  ... TDA7418_SOFT_STEP_TIME_10_24ms or TDA7418_SOFT_STEP_TIME_20_48ms
   #define  TDA7418_SOFT_STEP_TIME_0_32ms           // use one of TDA7418_SOFT_STEP_TIME_0_16ms or TDA7418_SOFT_STEP_TIME_0_32ms  ... TDA7418_SOFT_STEP_TIME_10_24ms or TDA7418_SOFT_STEP_TIME_20_48ms
   //#define  TDA7418_SOFT_STEP_TIME_0_64ms           // use one of TDA7418_SOFT_STEP_TIME_0_16ms or TDA7418_SOFT_STEP_TIME_0_32ms  ... TDA7418_SOFT_STEP_TIME_10_24ms or TDA7418_SOFT_STEP_TIME_20_48ms
   //#define  TDA7418_SOFT_STEP_TIME_1_28ms           // use one of TDA7418_SOFT_STEP_TIME_0_16ms or TDA7418_SOFT_STEP_TIME_0_32ms  ... TDA7418_SOFT_STEP_TIME_10_24ms or TDA7418_SOFT_STEP_TIME_20_48ms
   //#define  TDA7418_SOFT_STEP_TIME_2_56ms           // use one of TDA7418_SOFT_STEP_TIME_0_16ms or TDA7418_SOFT_STEP_TIME_0_32ms  ... TDA7418_SOFT_STEP_TIME_10_24ms or TDA7418_SOFT_STEP_TIME_20_48ms
   //#define  TDA7418_SOFT_STEP_TIME_5_12ms           // use one of TDA7418_SOFT_STEP_TIME_0_16ms or TDA7418_SOFT_STEP_TIME_0_32ms  ... TDA7418_SOFT_STEP_TIME_10_24ms or TDA7418_SOFT_STEP_TIME_20_48ms
   //#define  TDA7418_SOFT_STEP_TIME_10_24ms          // use one of TDA7418_SOFT_STEP_TIME_0_16ms or TDA7418_SOFT_STEP_TIME_0_32ms  ... TDA7418_SOFT_STEP_TIME_10_24ms or TDA7418_SOFT_STEP_TIME_20_48ms
   //#define  TDA7418_SOFT_STEP_TIME_20_48ms          // use one of TDA7418_SOFT_STEP_TIME_0_16ms or TDA7418_SOFT_STEP_TIME_0_32ms  ... TDA7418_SOFT_STEP_TIME_10_24ms or TDA7418_SOFT_STEP_TIME_20_48ms

   #ifdef   TDA7418_SOFT_STEP_TIME_0_16ms 
      #define TDA7418_SOFT_STEP_TIME            0b00000000
   #endif
   #ifdef   TDA7418_SOFT_STEP_TIME_0_32ms
      #define TDA7418_SOFT_STEP_TIME            0b00001000
   #endif
   #ifdef   TDA7418_SOFT_STEP_TIME_0_64ms
      #define TDA7418_SOFT_STEP_TIME            0b00010000
   #endif
   #ifdef   TDA7418_SOFT_STEP_TIME_1_28ms
      #define TDA7418_SOFT_STEP_TIME            0b00011000
   #endif
   #ifdef   TDA7418_SOFT_STEP_TIME_2_56ms
      #define TDA7418_SOFT_STEP_TIME            0b00100000
   #endif
   #ifdef   TDA7418_SOFT_STEP_TIME_5_12ms
      #define TDA7418_SOFT_STEP_TIME            0b00101000
   #endif
   #ifdef   TDA7418_SOFT_STEP_TIME_10_24ms
      #define TDA7418_SOFT_STEP_TIME            0b00110000
   #endif   
   #ifdef   TDA7418_SOFT_STEP_TIME_20_48ms
      #define TDA7418_SOFT_STEP_TIME            0b00111000
   #endif   
   /* bits 5 to 3 */     
   
   /* bits 6 to 6 */    
   #define TDA7418_AZ_FUNCTION_OFF                 // use one of TDA7418_AZ_FUNCTION_OFF  or TDA7418_AZ_FUNCTION_ON
   //#define TDA7418_AZ_FUNCTION_ON
   
   #ifdef   TDA7418_AZ_FUNCTION_OFF
      #define  TDA7418_AZ_FUNCTION              0b00000000
   #endif
   #ifdef   TDA7418_AZ_FUNCTION_ON
      #define  TDA7418_AZ_FUNCTION              0b01000000
   #endif   
   /* bits 6 to 6 */
   
   /* bits 7 to 6 */
   // any value is possible bits not used
   /* bits 7 to 6 */      
   /*=========================================================================*/ 
   
   /*=========================================================================
   TDA7418 - Testing audio processor (13) - TDA7418_SubAddr_Test_Processor 
       -----------------------------------------------------------------------*/ 
   
    /* bits 0 to 0 */  
   #define TDA7418_AUDIO_PROCESSOR_TESTING_MODE_OFF        // TDA7418_AUDIO_PROCESSOR_TESTING_MODE_OFF = OPERATION MODE //use one of TDA7418_AUDIO_PROCESSOR_TESTING_MODE_OFF (OPERATION MODE) or TDA7418_AUDIO_PROCESSOR_TESTING_MODE_ON
   //#define TDA7418_AUDIO_PROCESSOR_TESTING_MODE_ON        // TDA7418_AUDIO_PROCESSOR_TESTING_MODE_OFF = OPERATION MODE //use one of TDA7418_AUDIO_PROCESSOR_TESTING_MODE_OFF (OPERATION MODE) or TDA7418_AUDIO_PROCESSOR_TESTING_MODE_ON
   
   #ifdef   TDA7418_AUDIO_PROCESSOR_TESTING_MODE_OFF
      #define  TDA7418_AUDIO_PROCESSOR_TESTING_MODE     0b00000000
      #define  TDA7418_AUDIO_PROCESSOR_OPERATIING_MODE   0b00000000
   #endif
   #ifdef   TDA7418_AUDIO_PROCESSOR_TESTING_MODE_ON
      #define  TDA7418_AUDIO_PROCESSORT_TESTING_MODE     0b00000001
   #endif   
   /* bits 0 to 0 */
   
   //TODO: WRITE DEFINITIONS bits 7 to 1

void TDA7418_HardwareTest(); // The simplest hardware test routine - input: SE1
//void _TDA7418_i2cWritePrepare(); //use before i2c transmission
//void _TDA7418_i2cWriteExecute(); //use when i2c data is ready
void TDA7418__SelectInput(int8, int8, int8); //set TDA7418 input, define input gain and Diffin mode
void TDA7418__Loudness (int8, int8);
//void _TDA7418__Volume_Speaker_Attenuation(int8, int8);
void TDA7418__Volume(int8);
void TDA7418__LeftFront(int8);
void TDA7418__LeftRear(int8);
void TDA7418__RightRear(int8);
void TDA7418__RightFront(int8);
void TDA7418__Subwoofer(int8);
void TDA7418__Treble(int8);
void TDA7418__Middle(int8);
void TDA7418__Bass(int8);
void TDA7418__Middle_Bass_Fc_Select(void);
void TDA7418_Soft_Mut_Step_Time(void);
void _TDA7418_Audio_Processor_Testing_Mode(int8,int8,int8,int8);
void TDA7418_Audio_Processor_Operating_Mode(void);
#endif

