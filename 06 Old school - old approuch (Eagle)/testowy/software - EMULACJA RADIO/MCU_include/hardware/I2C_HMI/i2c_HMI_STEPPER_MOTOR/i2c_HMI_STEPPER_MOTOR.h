#ifndef _i2c_HMISTEPPER_MOTOR_
   #define _i2c_HMISTEPPER_MOTOR_

/*   #ifndef _i2c_Slave_buffer_addresses_
      #include </HMI_MCU_common_includes/HMI_i2c_Slave_buffer_addresses/i2c_Slave_buffer_addresses.h>
      #include </HMI_MCU_common_includes/HMI_i2c_slave_address/i2c_slave_address.h>  
   #endif*/
   
   #ifndef _I2C_ENGINE_
      #include </MCU_include/core/I2C_Engine/I2C_Engine.c>
   #endif
   
   #ifndef _i2c_HMI_main_
      #include </MCU_include/hardware/I2C_HMI/i2c_HMI_main.c> 
   #endif
   
void STEPPER_MOTOR_send_percentage(float percents);
void STEPPER_MOTOR_send_absolut(int32, int32, int32);

#endif
