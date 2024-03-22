#include </ActiveAntena.h>

   void AAntenna_ON(void)
   {
      output_high(ActiveAntena);
      delay_ms(600);
   }
 
   void AAntenna_OFF(void)
   {
      output_low(ActiveAntena);
      delay_ms(100);
   }

