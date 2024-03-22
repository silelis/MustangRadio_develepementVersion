#include <16F877.h>
#device ADC=10
#use delay(crystal=20000000)
//#use rs232(baud=9600,parity=N,xmit=None,rcv=None,bits=8,stream=PORT1)

#define _I2C_SDA  PIN_D1
#define _I2C_SCL  PIN_D0

#use i2c(Master,Fast,sda=_I2C_SDA,scl=_I2C_SCL)

#define LED PIN_C0
#define DELAY 1000


#define _USE_LOGPORT_

#if defined(DEBUG_ENABLED)
   #use rs232(ICD,stream=ICD_DBG,DISABLE_INTS)
   #define dbg_printf(fmt,...)   fprintf(ICD_DBG,fmt,__VA_ARGS__)
#elif defined(_USE_LOGPORT_)
   #use rs232(baud=9600,parity=N,xmit=PIN_B7,bits=8,stream=LOG_PORT, DISABLE_INTS)
   #define dbg_printf(fmt,...)   fprintf(LOG_PORT,fmt,__VA_ARGS__);  delay_ms(500)
#else
   #define dbg_printf(fmt,...)
#endif
