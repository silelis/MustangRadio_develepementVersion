#ifndef _TRANSMISSION_BUFFERS_
   #define _TRANSMISSION_BUFFERS_
   
   //#define mcu_Transmission_BUFSIZE         65
   //#define mcu_Reception_BUFSIZE           255 //257 by�o 257 z jakiego� powodu, ale UART dzia�a mi z 255 //poza tym wsz�dzie s� tak powyliczane p�tke (na int8)
   
   //i2c buffer size (MAX):
   // device:                    TEA5767        TDA7418       HMI      EEPROM
   //- write (I2CWriteLength):      6              3           15      4+2=6
   //- read  (I2CReadLength):       5              0           1        4
   const unsigned int16 i2c_mcu_Transmission_BUFSIZE=16;
   const unsigned int16 i2c_mcu_Reception_BUFSIZE=16; //257 by�o 257 z jakiego� powodu, ale UART dzia�a mi z 255 //poza tym wsz�dzie s� tak powyliczane p�tke (na int8)

   unsigned int   i2c_MasterBuffer[i2c_mcu_Transmission_BUFSIZE ];
   unsigned int   i2c_SlaveBuffer[i2c_mcu_Reception_BUFSIZE];

//   void ClearBuffers (unsigned int, short int);
   void i2c_ClearMasterBuffer  (void);
   void i2c_ClearSlaveBuffers  (void);
   
#endif
