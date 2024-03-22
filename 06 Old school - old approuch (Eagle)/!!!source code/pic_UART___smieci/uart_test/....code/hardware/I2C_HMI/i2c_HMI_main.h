#ifndef _i2c_HMI_main_
   #define _i2c_HMI_main_
   #ifndef _i2c_Slave_buffer_addresses_
      #include <F:/Elektronika_projekty/Car_Radio/testowy/PIC2PIC_slave/core/i2c_Slave_buffer_addresses/i2c_Slave_buffer_addresses.h>
      #include <F:/Elektronika_projekty/Car_Radio/testowy/PIC2PIC_slave/core/i2c_slave_address/i2c_slave_address.h>
      
   #endif
#ifndef _I2C_ENGINE_
   #include </core/I2C_Engine/I2C_Engine.c>
#endif

void _HMI_i2cWriteExecute(void);
void _HMI_i2cWritePrepare (int8);
//void _HMI_i2cReadPrepare (void);
//void _HMI_i2cReadExecute (void);

#endif
   
   
   
   
   

