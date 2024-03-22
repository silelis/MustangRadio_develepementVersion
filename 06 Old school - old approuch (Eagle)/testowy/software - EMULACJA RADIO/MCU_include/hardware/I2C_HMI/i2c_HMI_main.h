#ifndef _i2c_HMI_main_
   #define _i2c_HMI_main_
   #ifndef _i2c_Slave_buffer_addresses_
      #include </HMI_MCU_common_includes/HMI_i2c_Slave_buffer_addresses/i2c_Slave_buffer_addresses.h>
      #include </HMI_MCU_common_includes/HMI_i2c_slave_address/i2c_slave_address.h>  
      
   #endif
#ifndef _I2C_ENGINE_
   #include </MCU_include/core/I2C_Engine/I2C_Engine.c>
#endif

#define HMI_software_delay_ms    5     //delay required to HMI compute last data

void _HMI_i2cWriteExecute(void);
void _HMI_i2cWritePrepare (int8);
//void _HMI_i2cReadPrepare (void);
//void _HMI_i2cReadExecute (void);

#endif
   
   
   
   
   

