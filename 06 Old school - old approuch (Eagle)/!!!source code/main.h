#include <18F4520.h>
#device ADC=10
#use delay(crystal=20000000)



#FUSES brownout //,nomclr 
//#use rs232(baud=9600,parity=N,xmit=None,rcv=None,bits=8,stream=PORT1)
//#use rs232(baud=38400,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8) //,stream=PORT1)
#ifndef _I2C_
   #define _I2C_
   #use i2c(Master,Slow,sda=_I2C_SDA,scl=_I2C_SCL)
   //#use i2c(Master,Fast,sda=_I2C_SDA,scl=_I2C_SCL)
#endif

//#define LED PIN_D1
//#define ACTIVE_ANTENA PIN_C5
//#define DELAY 1000
//#define I2C_BUFSIZE         64
//#define MAX_TIMEOUT         0x00FFFFFF
#define StartupVoltageStabilisation   delay_ms(2000)
//#define BYTE int 
//#define BOOLEAN short int 

//#use rs232(UART1,baud=38400 ,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=Hardware__comm_port)
#use rs232(UART1, baud=57600 ,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8, stream=Hardware__comm_port)//, TRANSMIT_BUFFER=50,NOTXISR)

#byte UART_BUFFOR = getenv("SFR:TXREG")
#warning "UART_BUFFOR" UART_BUFFOR



#define _USE_LOGPORT_


#if defined(DEBUG_ENABLED)
#use rs232(ICD,stream=ICD_DBG,DISABLE_INTS)
#define dbg_return_carriage() fprintf_return_carriage()
#define dbg_printf(fmt,...)   fprintf(ICD_DBG,fmt,__VA_ARGS__); dbg_return_carriage()

#elif defined(_USE_LOGPORT_)
//#use rs232(baud=38400,parity=N,xmit=PIN_B7,bits=8,stream=LOG_PORT, DISABLE_INTS)
#use rs232(baud=38400,parity=N,xmit=PIN_B7,bits=8,stream=LOG_PORT)
#define dbg_return_carriage() fprintf_return_carriage()
#define dbg_printf(fmt,...)   fprintf(LOG_PORT,fmt,__VA_ARGS__); dbg_return_carriage()//;  delay_ms(500)

#else
#define dbg_printf(fmt,...)
#define dbg_return_carriage()
#endif

   void fprintf_return_carriage(void)
   {
      fprintf(LOG_PORT,"\r\n");
      //fprintf(LOG_PORT,"%c%c", 0x0a,0x0d);
   }

