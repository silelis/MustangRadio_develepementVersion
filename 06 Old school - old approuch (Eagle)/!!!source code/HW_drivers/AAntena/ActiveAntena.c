#include </ActiveAntena.h>


// TODO: w sterowaniu radio zrobi� mo�liwo�c wyboru CZY ma by� SUPPORT ACTIVE ANTENA - chodzi o to, �e czasem radio dzia�a lepiej z AA, a czaem bez

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
