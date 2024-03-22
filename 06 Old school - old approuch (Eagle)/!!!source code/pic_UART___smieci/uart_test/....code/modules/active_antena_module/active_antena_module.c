#include </active_antena_module.h>

//#ifdef _USE_ACTIVE_ANTENA_ 

   void ACTIVE_ANTENA_MODULE_active_antena_ON(void)
   {    
      set_x_pack_colour(pack_3, colour_RED);
      AAntenna_ON();
   }
   
   void ACTIVE_ANTENA_MODULE_active_antena_OFF(void)
   {
      set_x_pack_colour(pack_3, colour_BLACK);
      AAntenna_OFF();
   }
//#endif

