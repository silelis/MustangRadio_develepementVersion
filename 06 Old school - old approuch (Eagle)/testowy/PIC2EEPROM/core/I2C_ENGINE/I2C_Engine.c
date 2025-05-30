/**************************************************************************/
/*! 
    @file     tea5767.c
    @author   D. Bankowski (d.bankowski@gmail.com)
    
    @brief    Driver for the TEA5767 FM receiver.
    @section LICENSE
    Software License Agreement (BSD License)
    Copyright (c) 2015, D. Bankowski
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

#define _I2C_ENGINE_
//#include </core/I2C_ENGINE/I2C_Engine.h>
#include <I2C_Engine.h>

/*extern volatile unsigned int   MasterBuffer[I2C_BUFSIZE];
extern volatile unsigned int   SlaveBuffer[I2C_BUFSIZE];
volatile unsigned int32 I2CReadLength; //I2C zmieni� na 
volatile unsigned int32 I2CWriteLength;*/


unsigned int   MasterBuffer[I2C_BUFSIZE];
unsigned int   SlaveBuffer[I2C_BUFSIZE];


unsigned int32 I2CReadLength; //I2C zmieni� na 
unsigned int32 I2CWriteLength;

//#define UARTReadLength I2CReadLength;
//#define UARTRWriteLength I2CWriteLength;

void ClearBuffers (unsigned int *buffer, short int buffer_size)
{
   unsigned int8 i;
   for ( i = 0; i < buffer_size; i++ )
   {
      *(buffer+i)= 0x00;
   }
}

void i2cInit(void) {
   output_float(_I2C_SCL);
   output_float(_I2C_SDA);
}

int1 __i2cDeviceStatus() {
   int1 ack;
   i2c_start();            // If the write command is acknowledged,
   delay_us(I2C_DELAY_US);
   ack = i2c_write(MasterBuffer[0]);  // then the device is ready.
   delay_us(I2C_DELAY_US);
   i2c_stop();
   delay_us(I2C_DELAY_US);
   return ack;             // 0 means ACK, 1 means NO ACK, 2 means there was a collision if in Multi_Master Mode. This does not return an ACK if using i2c in slave mode
}

void i2cEngine(int1 eeprom_read = FALSE) //, unsigned int8 = 0)
{
   disable_interrupts(GLOBAL);
   
   int8  i2c_ACK=1;
   unsigned int i2c_tryout_error = 0;
   while(i2c_ACK!=0&&i2c_tryout_error <100)
   {i2c_ACK = __i2cDeviceStatus();
   i2c_tryout_error = i2c_tryout_error +1;
   }
   
   if (i2c_ACK==0)
   {
   i2c_start();
   delay_us(I2C_DELAY_US);
   unsigned int8 i;
   if (I2CWriteLength>0)
   {     
      for(i=0;i<I2CWriteLength; i++)
      {
         i2c_write(MasterBuffer[i]); //write to device from buffer
         delay_us(I2C_DELAY_US);
      }
   }
      if (eeprom_read == TRUE)
   {
      i2c_start();
      delay_us(I2C_DELAY_US);
      i2c_write(MasterBuffer[2]);
      delay_us(I2C_DELAY_US);
   }   
   
   if (I2CReadLength > 0)
   {
      for(i=0;i<I2CReadLength; i++)
      {
         SlaveBuffer[i]=i2c_read(); ///write to buffer from device
         delay_us(I2C_INTERBYTE_DELAY_US);
      }
   }
   i2c_stop();
   delay_us(I2C_DELAY_US);
   if (I2CReadLength == 0)
      {dbg_printf("I2C w.addr:%x ACK ", MasterBuffer[0]);}
   else
      {dbg_printf("I2C r.addr:%x ACK ", MasterBuffer[0]);}
   //I2CWriteLength = 0;
   //I2CReadLength = 0;
   break;
   }
   else if (i2c_ACK==1)
   {
   dbg_printf("I2C addr:%x !ACK ", MasterBuffer[0]);
   break;
   }
   else if (i2c_ACK==2)
   {
   dbg_printf("I2C adr:%x COLISION ", MasterBuffer[0]);
   break;
   }
   I2CWriteLength = 0;
   I2CReadLength = 0;
   enable_interrupts(GLOBAL); 
}
