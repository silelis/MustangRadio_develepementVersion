#include </active_antena_module.h>

//#ifdef _USE_ACTIVE_ANTENA_ 

#define ACTIVE_ANTENA_MODULE_active_antena_ON_TEA_5756()                ACTIVE_ANTENA_MODULE_active_antena_ON(Active_FM_antena_hardware_setup, EQU_use_active_antena_val ,0, 0, 0)
#define ACTIVE_ANTENA_MODULE_active_antena_ON_MONKEYBOARD_FM_AM()       ACTIVE_ANTENA_MODULE_active_antena_ON(Active_FM_antena_hardware_setup, EQU_use_active_antena_val ,Active_converter_antena_hardware_setup, Active_DAB_antena_hardware_setup, 0)
#define ACTIVE_ANTENA_MODULE_active_antena_ON_MONKEYBOARD_DAB()         ACTIVE_ANTENA_MODULE_active_antena_ON(Active_FM_antena_hardware_setup, 0                         ,Active_converter_antena_hardware_setup, Active_DAB_antena_hardware_setup, EQU_use_active_antena_val)
#define ACTIVE_ANTENA_MODULE_active_antena_ON_MONKEYBOARD_CONVERTER()   ACTIVE_ANTENA_MODULE_active_antena_ON(0,                               0                         ,Active_converter_antena_hardware_setup, 0, 0)

   void ACTIVE_ANTENA_MODULE_active_antena_ON(int1 FM_hardware_setup, int1 FM_eeprom_value, int1 Converter_hardware_setup, int1 DAB_hardware_setup, int1 DAB_eeprom_value, unsigned int antena_voltage_propagation_time = 150)
   {  
      if ((FM_hardware_setup && FM_eeprom_value)||(DAB_hardware_setup && DAB_eeprom_value))
      {
         dbg_printf("Ta linijka powinna byc wykasowana\r\n");
         //dbg_return_carriage()
         set_x_pack_colour(pack_3, colour_RED);
      }
      output_bit (FM_ANTENA_POWR_ON_OFF_pin, FM_hardware_setup *FM_eeprom_value);
      output_bit (DAB_FM_CONVERTER_POWR_ON_OFF_pin, Converter_hardware_setup);
      output_bit (DAB_ANTENA_POWR_ON_OFF_pin, DAB_hardware_setup * DAB_eeprom_value);
      delay_ms(antena_voltage_propagation_time);
      
   }
   
   void ACTIVE_ANTENA_MODULE_active_antena_OFF(void)
   {
      set_x_pack_colour(pack_3, colour_BLACK);
      ACTIVE_ANTENA_MODULE_active_antena_ON(0,0,0,0,0,0);
   }
//#endif

int8 ACTIVE_ANTENA_MODULE_get_antena_power_setup_from_HMI(void)
{
   disable_interrupts(GLOBAL);
   //dbg_printf("HMI antena bug: chyba mozesz go juz poprawic. To samo co w TEA5767 by³o");
   //dbg_return_carriage();
   //int8 data;
   
   
  i2c_ClearMasterBuffer();
  i2c_ClearSlaveBuffers();
  
  
  I2CWriteLength = 1; //"1" beacause i2c after start should write adress
  I2CReadLength = 1;
  i2c_MasterBuffer[0] = i2c_slave_address+1;
  i2cEngine(FALSE);

   
/*   _HMI_i2cWritePrepare ();
   if (__try_i2c_bus()==0)
   {
   
      //int8 data;
      i2c_start();
      i2c_write(i2c_slave_address+1);
      data=i2c_read(0);
      i2c_stop();
      
      return data;
   }
   else
   {
      dbg_printf("HMI i2c error. ");
      data= 0;
   }*/
   
   
   
   delay_ms(HMI_software_delay_ms);
   enable_interrupts(GLOBAL);
   return i2c_SlaveBuffer[0];
   //return data;
}

