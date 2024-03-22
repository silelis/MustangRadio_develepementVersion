#include <main.h>


int16 timer_counter =0;

#INT_TIMER0
void  TIMER0_isr(void) 
{
      timer_counter = timer_counter +1;
      
      if (timer_counter> 15)
      {
        disable_interrupts(INT_TIMER0);
        //timer_counter=0;
        output_high(pin_a0);
        delay_ms(250);
        output_low(pin_a0);
        timer_counter=0;
        enable_interrupts(INT_TIMER0);
      }
}

void main()
{
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1|RTCC_8_bit);      //51,2 us overflow
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_2|RTCC_8_bit);      //102 us overflow
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_4|RTCC_8_bit);      //204 us overflow
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_8|RTCC_8_bit);      //409 us overflow
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_16|RTCC_8_bit);      //819 us overflow
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_32|RTCC_8_bit);      //1,6 ms overflow
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_64|RTCC_8_bit);      //3,2 ms overflow
  setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128|RTCC_8_bit);      //6,5 ms overflow
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_256|RTCC_8_bit);      //13,1 ms overflow



   enable_interrupts(INT_TIMER0);
   enable_interrupts(GLOBAL);

   while(TRUE)
   {
      //TODO: User Code
      output_low(pin_a0);
   }

}
