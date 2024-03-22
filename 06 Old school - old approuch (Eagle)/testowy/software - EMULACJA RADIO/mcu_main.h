//#if device=="PIC18F4520"
   #FUSES nomclr   //TODO: Wykasowaæ w ostatecznej wersji programu dla gotowej p³yty MCU
   #FUSES BROWNOUT
   #FUSES PROTECT
//#endif

#device ADC=10
#use delay(crystal=20000000)

#use i2c(Master,Slow,sda=_I2C_SDA,scl=_I2C_SCL)

//#use rs232(baud=38400 ,parity=N,xmit=PIN_C6,RCV=PIN_C7, bits=8,stream=comm_port)//, DISABLE_INTS)
//#use rs232(UART1,baud=38400 ,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=Hardware__comm_port)
#use rs232(UART1,baud=38400 ,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=Hardware__comm_port, ERRORS)

#if defined(DEBUG_ENABLED)
   //#use rs232(ICD,stream=ICD_DBG,DISABLE_INTS)
   #use rs232(baud=38400,parity=N,xmit=PIN_B7,bits=8,stream=LOG_PORT, DISABLE_INTS)
   #define dbg_return_carriage() fprintf_return_carriage()
   #define dbg_printf(fmt,...)   fprintf(LOG_PORT,fmt,__VA_ARGS__); dbg_return_carriage()

#elif defined(_USE_LOGPORT_)
   #use rs232(baud=38400,parity=N,xmit=PIN_B7,bits=8,stream=LOG_PORT)//, DISABLE_INTS)
   //#use rs232(baud=38400,parity=N,xmit=PIN_B7,bits=8,stream=LOG_PORT)//, DISABLE_INTS)
   #define dbg_return_carriage() fprintf_return_carriage()
   #define dbg_printf(fmt,...)   fprintf(LOG_PORT,fmt,__VA_ARGS__); dbg_return_carriage()
#else
   #define dbg_printf(fmt,...)   //
   #define dbg_return_carriage() //
#endif




#use standard_io(A)
#use standard_io(B)
#use standard_io(C)
#use standard_io(D)
#use standard_io(E)

//#use FIXED_IO( B_outputs=PIN_B0,PIN_B2,PIN_B3,PIN_B4,PIN_B5)
//#use FIXED_IO( C_outputs=PIN_C0,PIN_C1,PIN_C2)   
//#use FIXED_IO( D_outputs=PIN_D0,PIN_D2,PIN_D7) 

   void fprintf_return_carriage(void)
   {
      fprintf(LOG_PORT,"\r\n");
      //fprintf(LOG_PORT,"%c%c", 0x0a,0x0d);
   }

