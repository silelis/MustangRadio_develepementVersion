#ifndef __Actice_antena_select__
   #define __Actice_antena_select__
   
   #ifndef _i2c_Slave_buffer_addresses_
      #include </HMI_MCU_common_includes/HMI_i2c_Slave_buffer_addresses/i2c_Slave_buffer_addresses.h>
   #endif

  #ifndef   IS_FM_ANT_ACTIVE
   #define  IS_FM_ANT_ACTIVE              PIN_B0
  #endif
  #ifndef   IS_DAB_ANT_ACTIVE
   #define  IS_DAB_ANT_ACTIVE             PIN_B1 
  #endif
  #ifndef   IS_FM_DAB_CONVERTER_ACTIVE
   #define IS_FM_DAB_CONVERTER_ACTIVE    PIN_B2
  #endif
  #ifndef   ANTENA_SWITCH_SUPPLY
   #define ANTENA_SWITCH_SUPPLY    PIN_B3
  #endif   
   
   
   int8 get_active_antenas_supply_setup(void);
   
#endif
