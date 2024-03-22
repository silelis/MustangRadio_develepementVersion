/**************************************************************************/
/*! 
    @file     i2c_engine.c
    @author   D. Bankowski (d.bankowski@gmail.com)
    
    @brief    Driver for i2c master.
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
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.ty
*/
/**************************************************************************/

#include </I2C_Engine.h>

#ifndef _TRANSMISSION_BUFFERS_
   #include </MCU_include/core/transmission_buffers/transmission_buffers.c>
#endif

//unsigned int32 I2CReadLength; //I2C zmieniæ na 
//unsigned int32 I2CWriteLength;
unsigned int16 I2CReadLength; 
unsigned int16 I2CWriteLength;

void i2cInit(void) {
   output_float(_I2C_SCL);
   output_float(_I2C_SDA);
}

int8 __i2cDeviceStatus() {
   int8 ack;
   i2c_start();            // If the write command is acknowledged,
   delay_us(I2C_DELAY_US);
   ack = i2c_write(i2c_MasterBuffer[0]&0xFE);  // then the device is ready. PING should be always of write address so 0xFE 0b11111110 is required on ping
   delay_us(I2C_DELAY_US);
   i2c_stop();
   delay_us(I2C_DELAY_US);
   return ack;             // 0 means ACK, 1 means NO ACK, 2 means there was a collision if in Multi_Master Mode. This does not return an ACK if using i2c in slave mode
}

int8 __try_i2c_bus(void)
{
   int8  i2c_try=1;
   unsigned int16 i2c_tryout_error = 0;
   while(i2c_try!=0&&i2c_tryout_error <100) //i2c_tryout_error<100
   {
      i2c_try = __i2cDeviceStatus();
      
      if (i2c_try!= 0)
      {
         dbg_printf("I2C w.addr:%x error try(s):%Lu status:%d", i2c_MasterBuffer[0], i2c_tryout_error, i2c_try);
         //dbg_return_carriage();
      }
      i2c_tryout_error = i2c_tryout_error +1;
      delay_us(254);
   }
   return i2c_try;
}

void i2cEngine(int1 eeprom_read = FALSE) //, unsigned int8 = 0)
{
   disable_interrupts(GLOBAL);
   
   int8  i2c_ACK=__try_i2c_bus();
   
   if (i2c_ACK==0)
   {
   i2c_start();
   delay_us(I2C_DELAY_US);
   
   
   
   //unsigned int8 i;
   unsigned int16 i;

   /*
   #if (I2CReadLength >mcu_Reception_BUFSIZE)
      #error 'B³¹d krytyczny. Nie mo¿esz wysy³aæ wiêcej danych ni¿ pojemnoœæ SlaveBuffer[mcu_Reception_BUFSIZE]'
   #endif*/
   
   
   if (I2CWriteLength>0)
   {     
      for(i=0;i<I2CWriteLength; i++)
      {
         i2c_write(i2c_MasterBuffer[i]); //write to device from buffer
         delay_us(I2C_DELAY_US);
      }
   }
      if (eeprom_read == TRUE)
   {
      i2c_start();
      delay_us(I2C_DELAY_US);
      i2c_write(i2c_MasterBuffer[2]);
      delay_us(I2C_DELAY_US);
   }   
   
   if (I2CReadLength > 0)
   {  
      int1 state;
      for(i=0;i<I2CReadLength; i++)
      {  
      
         if (i<(I2CReadLength-1))
            state = TRUE;
         else
            state = FALSE;
         i2c_SlaveBuffer[i]=i2c_read(state);
         delay_us(I2C_INTERBYTE_DELAY_US);
      }
   }
   i2c_stop();
   delay_us(I2C_DELAY_US);
   if (I2CReadLength == 0)
      {
         dbg_printf("I2C w.addr:%x ACK", i2c_MasterBuffer[0]);
         //dbg_return_carriage();
      }
   else
      {
         dbg_printf("I2C r.addr:%x ACK", i2c_MasterBuffer[0]);
         //dbg_return_carriage();
      }
   break;
   }
   else if (i2c_ACK==1)
   {
   dbg_printf("I2C addr:%x !ACK", i2c_MasterBuffer[0]);
   //dbg_return_carriage();
   break;
   }
   else if (i2c_ACK==2)
   {
   dbg_printf("I2C adr:%x COLISION", i2c_MasterBuffer[0]);
   //dbg_return_carriage();
   break;
   }
   I2CWriteLength = 0;
   I2CReadLength = 0;
   enable_interrupts(GLOBAL); 
}
