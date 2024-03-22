#include </ActiveAntena.h>


// TODO: w sterowaniu radio zrobiæ mo¿liwoœc wyboru CZY ma byæ SUPPORT ACTIVE ANTENA - chodzi o to, ¿e czasem radio dzia³a lepiej z AA, a czaem bez

//#ifdef ACTIVE_ANTENA 
   void AAntenna_ON(void)
   {
      output_high(ACTIVE_ANTENA);
      delay_ms(500);
   }
 
   void AAntenna_OFF(void)
   {
      output_low(ACTIVE_ANTENA);
      delay_ms(500);
   }
//#endif
