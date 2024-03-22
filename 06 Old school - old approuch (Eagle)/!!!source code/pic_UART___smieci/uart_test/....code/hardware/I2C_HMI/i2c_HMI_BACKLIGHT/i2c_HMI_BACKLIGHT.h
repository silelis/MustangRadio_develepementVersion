#ifndef _i2c_HMI_BACKLIGHT_
   #define _i2c_HMI_BACKLIGHT_
   #ifndef _i2c_Slave_buffer_addresses_
      #include <F:/Elektronika_projekty/Car_Radio/testowy/PIC2PIC_slave/core/i2c_Slave_buffer_addresses/i2c_Slave_buffer_addresses.h>
      #include <F:/Elektronika_projekty/Car_Radio/testowy/PIC2PIC_slave/core/i2c_slave_address/i2c_slave_address.h>
      
   #endif
#ifndef _I2C_ENGINE_
   #include </core/I2C_Engine/I2C_Engine.c>
#endif
   
#ifndef _i2c_HMI_main_
   #include </hardware/I2C_HMI/i2c_HMI_main.c> 
#endif

   void set_backlight(int8);

#endif
   
   
   
   
   

