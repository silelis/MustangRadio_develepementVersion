#ifndef __ACTIVE_ANTENA_MODULE_
   #define __ACTIVE_ANTENA_MODULE_

  //#ifdef _USE_ACTIVE_ANTENA_ 
  
  
  
   //#ifndef ACTIVE_ANTENA
   //   #include</MCU_include/hardware/ActiveAntena/ActiveAntena.c>
   //#endif
   #ifndef _i2c_HMI_LEDS_
      #include </MCU_include/hardware/I2C_HMI/i2c_HMI_LEDS/i2c_HMI_LEDS.c>
   #endif
   
  //void ACTIVE_ANTENA_MODULE_active_antena_OFF(void);
  void ACTIVE_ANTENA_MODULE_active_antena_ON(int1, int1, int1, int1, int1);
  //void ACTIVE_ANTENA_MODULE_active_antena_ON(void);
  int8 ACTIVE_ANTENA_MODULE_get_antena_power_setup_from_HMI (void);
  //#endif


#endif
