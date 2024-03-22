
#ifndef __EEPROM_2416_
   #define __EEPROM_2416_
#endif
#ifndef __EEPROM_ADDRESS_MAP
   #define __EEPROM_ADDRESS_MAP
   #include </core/EEPROM_2416/eeprom_address_map.h.h>
#endif

#ifndef _I2C_ENGINE_
   #include </core/I2C/I2C_Engine.c>
#endif

#define EEPROM_I2C_WRITE_ADDRESS 0xa0    
#define EEPROM_ADDRESS           unsigned int16
#define EEPROM_SIZE              1024



//void write_ext_eeprom(EEPROM_ADDRESS, BYTE, unsigned int16 );
void write_ext_eeprom(EEPROM_ADDRESS, void *, EEPROM_ADDRESS);
//BYTE read_ext_eeprom(EEPROM_ADDRESS);
void read_ext_eeprom(EEPROM_ADDRESS address, void *, EEPROM_ADDRESS);
//#define pic_eeprom
//#define external_eeprom_24C16 //TO DO
//#define max_buffer_size 5
//#define eeprom_write_delay 50
