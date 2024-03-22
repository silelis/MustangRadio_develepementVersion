#include <16F77.h>
#device ADC=16
#use delay(crystal=20000000)



#use timer(timer=2, tick=1 ms,bits=8, NOISR, stream=Backlight_blink_timer, define=TICKS_PER_SECOND_backlight)
#use pwm(CCP2,TIMER=2,FREQUENCY=10kHz)

