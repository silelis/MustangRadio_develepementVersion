#include <16F877.h>
#device ADC=10
#use delay(crystal=20000000)
#use rs232(baud=57600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=PORT1)//,brgh10k)

