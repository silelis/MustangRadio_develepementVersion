#include <18F4520.h>
#device ADC=10


#fuses HS,NOWDT,NOPROTECT,NOLVP

#use delay(crystal=20MHz, fast_start)

//#use delay(clock=20MHz,oscillator=20MHz)

#use rs232(UART1, baud=57600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=Hrdw)
#use rs232(baud=38400,parity=N,xmit=PIN_b7,rcv=PIN_b6,bits=8,stream=Dbg)

