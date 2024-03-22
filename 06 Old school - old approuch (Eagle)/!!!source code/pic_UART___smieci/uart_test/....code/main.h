#if device=="PIC18F4520"
   #FUSES nomclr, brownout   //TODO: Wykasowaæ w ostatecznej wersji programu dla gotowej p³yty MCU
#endif

#device ADC=10
#use delay(crystal=20000000)



   #use i2c(Master,Slow,sda=_I2C_SDA,scl=_I2C_SCL)
   
 
#if defined(DEBUG_ENABLED)
   //#use rs232(ICD,stream=ICD_DBG,DISABLE_INTS)
   #use rs232(baud=38400,parity=N,xmit=PIN_B7,bits=8,stream=LOG_PORT, DISABLE_INTS)
   #define dbg_printf(fmt,...)   fprintf(LOG_PORT,fmt,__VA_ARGS__)
   #define dbg_return_carriage() fprintf_return_carriage()
#elif defined(_USE_LOGPORT_)
   #use rs232(baud=38400,parity=N,xmit=PIN_B7,bits=8,stream=LOG_PORT)//, DISABLE_INTS)
   #define dbg_printf(fmt,...)   fprintf(LOG_PORT,fmt,__VA_ARGS__)//;  delay_ms(500)
   #define dbg_return_carriage() fprintf_return_carriage()
#else
   #define dbg_printf(fmt,...)   //
   #define dbg_return_carriage()
#endif


/*void dbg_printf(char *array[])
{
    fprintf(LOG_PORT,array[1]);
}*/

void fprintf_return_carriage(void)
{
   fprintf(LOG_PORT,"\r\n");
   //fprintf(LOG_PORT,"%c%c", 0x0a,0x0d);
}
