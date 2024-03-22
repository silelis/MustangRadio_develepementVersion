#ifndef __EEPROM_MODULE_
   #define __EEPROM_MODULE_
   
   #ifndef __EEPROM_2416_
      #include </MCU_include/hardware/EEPROM_2416/EEPROM_2416.c>
   #endif
   
   #ifndef __EEPROM_MAP_
      #include </MCU_include/hardware/EEPROM_MAP/eeprom_map.h>
   #endif

   void check_read_value_int8(signed int8 , signed int8 , signed int8 , signed int8 );
   void check_read_value_int32(signed int32, signed int32, signed int32, signed int32);
   void check_temp_value_int8(signed int8, signed int8, signed int8);
   void read_and_check_value_int8 (EEPROM_ADDRESS, void *, EEPROM_ADDRESS , int8 , int8 , int8 );
   void read_and_check_value_int32(EEPROM_ADDRESS, void *, EEPROM_ADDRESS , int32, int32, int32);
#endif
