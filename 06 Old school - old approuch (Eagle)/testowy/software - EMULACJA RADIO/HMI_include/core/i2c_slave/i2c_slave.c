/**************************************************************************/
/*! 
    @file     i2c_slave.c
    @author   D. Bankowski (d.bankowski@gmail.com)
    
    @brief    Driver for i2c slave.
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

#include <i2c_slave.h>


void clr_SSPSTAT(void)     //when data is counted by program and can read new data
{
   SSP1STAT = 0;
   i2c_buffer_counter = 0;
}

void ClearBuffers (unsigned int *buffer, short int buffer_size)
{
   unsigned int8 i;
   for ( i = 0; i < buffer_size; i++ )
   {
      *(buffer+i)= 0x00;
   }
}

//unsigned int set_i2c_interrupt_ready(void)
void set_i2c_interrupt_ready(void)
{
   clr_SSPSTAT();
   i2c_slave_ready();
}
   
   
void i2c_slave_interrupt (void)
{
   unsigned int8 state; //, incoming;
   state = i2c_isr_state();
   if (state < 0x80)
       {
       Master_2_Slave_Buffer[i2c_buffer_counter]= i2c_read();
       i2c_buffer_counter=i2c_buffer_counter+1;
       }
       
   if(state >= 0x80)                      //Master is requesting data do Your operations here
   {
      output_bit( ANTENA_SWITCH_SUPPLY, 1);
      i2c_write(get_active_antenas_supply_setup());
      output_bit( ANTENA_SWITCH_SUPPLY, 0);
   }
   if (SSP1STAT_STOP_SEEN==1)
   {
      i2c_slave_not_ready() ;
   }
}

