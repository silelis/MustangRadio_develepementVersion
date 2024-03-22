#include <active_antena_selector.h>

int8 get_active_antenas_supply_setup(void)
// Funkcja sprawdz, ustawienie wskazuj¹cych przez u¿ytkownika na anteny, które powinny byæ zasilane
{
   return ((input(IS_FM_ANT_ACTIVE)*IS_FM_ANT_ACTIVE_bit)+
            (input(IS_DAB_ANT_ACTIVE)*IS_DAB_ANT_ACTIVE_bit)+
            (input(IS_FM_DAB_CONVERTER_ACTIVE)*IS_FM_DAB_CONVERTER_bit));
}
