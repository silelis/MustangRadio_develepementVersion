/**************************************************************************/
/*! 
    @file     i2c_slave.h
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

#ifndef _I2C_ENGINE_SLAVE_
   #define _I2C_ENGINE_SLAVE_
   
   #if getenv("SFR_VALID:SSPSTAT")
      #byte SSP1STAT = getenv("SFR:SSPSTAT")
      #bit SSP1STAT_STOP_SEEN=SSP1STAT.4 
      #WARNING "PIC_SSPSTAT REGISTER CONFIGURED"
   #else
      #ERROR "PIC_SSPSTAT REGISTER SHOULD BE DEFINED in i2c_slave.h"
   #endif
  
   #define  I2C_SLAVE_BUFSIZE       15
   
   unsigned int8   Master_2_Slave_Buffer[I2C_SLAVE_BUFSIZE];
   unsigned int8   Slave_2_Master_Buffer[I2C_SLAVE_BUFSIZE];
   unsigned int    i2c_buffer_counter;
   
   void clr_SSPSTAT(void);
   void ClearBuffers (unsigned int, short int);
   void set_i2c_interrupt_ready(void);
   void i2c_slave_interrupt (void);
#endif
