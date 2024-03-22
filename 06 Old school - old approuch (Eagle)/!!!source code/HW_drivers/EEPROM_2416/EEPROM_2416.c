#include </core/EEPROM_2416/EEPROM_2416.h>

/*#ifndef __EEPROM_ADDRESS_MAP
   #define __EEPROM_ADDRESS_MAP
   #include </core/EEPROM_2416/eeprom_address_map.h.h>
#endif

#ifndef _I2C_ENGINE_
   #include </core/I2C/I2C_Engine.c>
#endif*/




void write_ext_eeprom(EEPROM_ADDRESS address, void *data, unsigned int16 _size_of_data) {
   ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   MasterBuffer[0]   =  (EEPROM_I2C_WRITE_ADDRESS|(BYTE)(address>>7))&0xfe;
   MasterBuffer[1]   =  address;
   memcpy(&MasterBuffer+2, data, _size_of_data);
   I2CWriteLength = 2+_size_of_data;
   I2CReadLength = 0;
   i2cEngine(normal_I2C);
}


void read_ext_eeprom(EEPROM_ADDRESS address, void *data, EEPROM_ADDRESS _size_of_data) {
   ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   ClearBuffers(&SlaveBuffer,I2C_BUFSIZE);
   MasterBuffer[0]   = ((EEPROM_I2C_WRITE_ADDRESS|(BYTE)(address>>7))&0xfe);
   MasterBuffer[1]   =(address);
   MasterBuffer[2]   =((EEPROM_I2C_WRITE_ADDRESS|(BYTE)(address>>7))|1);
   I2CWriteLength = 3;
   I2CReadLength = _size_of_data;
   i2cEngine(READ_EEPROM);
   memcpy(data, &SlaveBuffer, _size_of_data);
}



