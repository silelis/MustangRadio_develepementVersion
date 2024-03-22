//#include <16F877.h>
#include <16F77.h>
#device ADC=10
#use delay(crystal=20000000)



//#if device=="PIC16F877"
   #FUSES BROWNOUT   //TODO: Wykasowaä w ostatecznej wersji programu dla gotowej p-yty MCU
   #FUSES PROTECT
   //#FUSES WDT
//#endif

#USE STANDARD_IO (A)
#USE STANDARD_IO (B)
#USE STANDARD_IO (C)
#USE STANDARD_IO (D)
#USE STANDARD_IO (E)

#use i2c(Slave,Fast,sda=i2c_SDA_pin,scl=i2c_SCL_pin,force_hw,address=i2c_slave_address)

//#ifndef _STEPPER_MOTOR_ON_INTERRUPTS_
   #use timer(timer=0, tick=1 ms, bits=16, NOISR, stream=Steper_motor_timer, define=TICKS_PER_SECOND_stepper)
   #warning STEPPER MOTOR tick per second: TICKS_PER_SECOND_stepper
//#endif
#use timer(timer=1, tick=750 ms, bits=16, NOISR, stream=LED_blink_timer, define=TICKS_PER_SECOND_led)

//#use timer(timer=1, tick=750 us, bits=8, NOISR, stream=LED_blink_timer, define=TICKS_PER_SECOND_led)
#warning LED blink tick per second: TICKS_PER_SECOND_led

#if Backlight_MAX_VAL!=1
   #use timer(timer=2, tick=100 ms,bits=8, NOISR, stream=Backlight_blink_timer, define=TICKS_PER_SECOND_backlight)
   #warning BACKLIGHT blink tick per second: TICKS_PER_SECOND_backlight
#endif

#if defined(DEBUG_ENABLED)
   //#use rs232(ICD,stream=ICD_DBG,DISABLE_INTS)
   #use rs232(baud=38400,parity=N,xmit=PIN_B7,rcv=PIN_B6,bits=8,stream=LOG_PORT, ICD) //, DISABLE_INTS)
   #define dbg_return_carriage() fprintf_return_carriage()
   #define dbg_printf(fmt,...)   fprintf(LOG_PORT,fmt,__VA_ARGS__); dbg_return_carriage()
   
#elif defined(_USE_LOGPORT_)
   #use rs232(baud=38400,parity=N,xmit=PIN_B7,rcv=PIN_B6,bits=8,stream=LOG_PORT)//, DISABLE_INTS)
   #define dbg_return_carriage() fprintf_return_carriage()
   #define dbg_printf(fmt,...)   fprintf(LOG_PORT,fmt, __VA_ARGS__); dbg_return_carriage()//;  fprintf_return_carriage()//return_carrier//;  delay_ms(500)
#else
   #define dbg_printf(fmt,...)
   #define dbg_return_carriage()
#endif




#use FIXED_IO( A_outputs=PIN_A0, PIN_A1, PIN_A2,PIN_A5)
#use FIXED_IO( B_outputs=PIN_B3)
//#use FIXED_IO( C_outputs=PIN_C0, PIN_C5, PIN_C6, PIN_C7)
#use FIXED_IO( C_outputs=PIN_C0,PIN_C1,PIN_C2, PIN_C5, PIN_C6, PIN_C7)
#use FIXED_IO( D_outputs=PIN_D2, PIN_D4, PIN_D7 )
#use FIXED_IO( E_outputs=PIN_E0, PIN_E1, PIN_E2)

//#use PWM(CCP2, FREQUENCY=1kHz, DUTY=100)//, STREAM=backlight)

#if Backlight_MAX_VAL!=1
   #use pwm(CCP2,TIMER=2,FREQUENCY=10kHz)//, STREAM=backlight)
   #warning USE PWM backlight
#else
   #define BACKLIGHT_pin   PIN_C2
#endif

void fprintf_return_carriage(void)
{
   fprintf(LOG_PORT,"\r\n");
}


