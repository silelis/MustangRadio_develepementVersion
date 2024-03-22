#include </EEPROM_2416.h>

void write_ext_eeprom(EEPROM_ADDRESS address, void *data, unsigned int16 _size_of_data) {
   ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   MasterBuffer[0]   =  (EEPROM_I2C_WRITE_ADDRESS|(BYTE)(address>>7))&0xfe;
   MasterBuffer[1]   =  address;
   memcpy(&MasterBuffer+2, data, _size_of_data);
   I2CWriteLength = 2+_size_of_data;
   I2CReadLength = 0;
   i2cEngine(normal__I2C);
}

void read_ext_eeprom(EEPROM_ADDRESS address, void *data, EEPROM_ADDRESS _size_of_data) {
   ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   ClearBuffers(&SlaveBuffer,I2C_BUFSIZE);
   MasterBuffer[0]   = ((EEPROM_I2C_WRITE_ADDRESS|(BYTE)(address>>7))&0xfe);
   MasterBuffer[1]   = (address);
   MasterBuffer[2]   = ((EEPROM_I2C_WRITE_ADDRESS|(BYTE)(address>>7))|1);
   I2CWriteLength = 3;
   I2CReadLength = _size_of_data;
   i2cEngine(READ__EEPROM);
   memcpy(data, &SlaveBuffer, _size_of_data);
}

void eeprom_erease(void){

   int8 erease_data = 0xff;
   
   for (EEPROM_ADDRESS i =0 ; i<= EEPROM_SIZE; i++)
      {
      write_ext_eeprom(i, &erease_data, 1);
      }
}
