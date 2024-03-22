#ifndef _TRANSMISSION_BUFFERS_
   #define _TRANSMISSION_BUFFERS_
   
   //#define mcu_Transmission_BUFSIZE         65
   //#define mcu_Reception_BUFSIZE           255 //257 by³o 257 z jakiegoœ powodu, ale UART dzia³a mi z 255 //poza tym wszêdzie s¹ tak powyliczane pêtke (na int8)
   

    //const unsigned int16 mcu_Transmission_BUFSIZE=16;
    #define mcu_Transmission_BUFSIZE 16
    //const unsigned int16 mcu_Reception_BUFSIZE=32; //257 by³o 257 z jakiegoœ powodu, ale UART dzia³a mi z 255 //poza tym wszêdzie s¹ tak powyliczane pêtke (na int8)
    #define mcu_Reception_BUFSIZE 32
    unsigned int   comunication_MasterBuffer[mcu_Transmission_BUFSIZE ];
    unsigned int   comunication_SlaveBuffer[mcu_Reception_BUFSIZE];
   
   
   void ClearBuffers (unsigned int, short int);
   
   #ifdef _I2C_ENGINE_
      //i2c buffer size (MAX):
      // device:                    TEA5767        TDA7418       HMI      EEPROM       au6850
      //- write (I2CWriteLength):      6              3           15      4+2=6         5???
      //- read  (I2CReadLength):       5              0           1        4           32
      
     
      #define  i2c_MasterBuffer                    comunication_MasterBuffer
      #define  i2c_SlaveBuffer                     comunication_SlaveBuffer
      #define  i2c_mcu_Transmission_BUFSIZE        mcu_Transmission_BUFSIZE
      #define  i2c_mcu_Reception_BUFSIZE           mcu_Reception_BUFSIZE
      
      
      #define i2c_ClearMasterBuffer()     ClearBuffers(&i2c_MasterBuffer,i2c_mcu_Transmission_BUFSIZE)
      #define i2c_ClearSlaveBuffers()     ClearBuffers(&i2c_SlaveBuffer,i2c_mcu_Reception_BUFSIZE)
      //void i2c_ClearMasterBuffer  (void);
      //void i2c_ClearSlaveBuffers  (void);
   #endif

   #ifdef _UART_ENGINE_
       //UART buffer size (MAX):
      // device:                    OVC3860        MonkeyBoard          au6820
      //- write (I2CWriteLength):      
      //- read  (I2CReadLength):                                           19
   
      #define  UART_MasterBuffer                    comunication_MasterBuffer
      #define  UART_SlaveBuffer                     comunication_SlaveBuffer
      #define  UART_mcu_Transmission_BUFSIZE        mcu_Transmission_BUFSIZE
      #define  UART_mcu_Reception_BUFSIZE           mcu_Reception_BUFSIZE   
   
      #define UART_ClearMasterBuffer()     ClearBuffers(&UART_MasterBuffer,UART_mcu_Transmission_BUFSIZE)
      #define UART_ClearSlaveBuffers()     ClearBuffers(&UART_SlaveBuffer,UART_mcu_Reception_BUFSIZE)
      
   #endif
   

   
#endif
