#include<menu_module.h>

/*int8 check_boarders(signed int8 current_value, signed int8 min_value, signed int8 max_value)
{
if (current_value<min_value)
   return min_value;
else if (current_value>max_value)
   return max_value;
else
   return current_value;
}*/

void MENU_MODULE_reset_all_devices(int which_device)
{
//#ifdef _USE_ACTIVE_ANTENA_

   //if (MENU_MODULE_audio_devices_list[which_device]==MENU_MODULE_VMUSIC3_input ||MENU_MODULE_audio_devices_list[which_device]==MENU_MODULE_VMUSIC3_RND_input||MENU_MODULE_audio_devices_list[which_device]==MENU_MODULE_AUX_input||MENU_MODULE_audio_devices_list[which_device]==MENU_MODULE_BLUETOOTH_input)     //LACK OF SPACE YOU CAN DELATE THIS 1%
   
/*   if (MENU_MODULE_audio_devices_list[which_device]!=MENU_MODULE_TEA5767_input ||
   MENU_MODULE_audio_devices_list[which_device]!=MENU_MODULE_MONKEYBOARD_FM_input ||
   MENU_MODULE_audio_devices_list[which_device]!=MENU_MODULE_MONKEYBOARD_DAB_input ||
   MENU_MODULE_audio_devices_list[which_device]!=MENU_MODULE_MONKEYBOARD_AM_input
   )     //LACK OF SPACE YOU CAN DELATE THIS 1%*/
   
   
   ACTIVE_ANTENA_MODULE_active_antena_OFF();
//#endif

#ifdef _USE_TEA5767_
   //TODO: RESET TEA5767
#endif

#ifdef _USE_MNONKEYBOARD_DAB_
   //TODO: RESET MonkeyBoard
#endif

#ifdef _USE_VMUSIC3_
   //TODO: RESET VMusic
#endif

#ifdef _USE_BLUETOOTH_
   //TODO: RESET VMusic
#endif

#ifdef _USE_AUX_
   //TODO: RESET AUX
#endif

//TODO: Reset konvertera FM na FM/ DAB+ jeœli bêdzie jego w³aczanie sterowane PICem
}

void MENU_MODULE_check_variable_boarders(signed int8 *current_value, signed int8 min_value, signed int8 max_value)
{
if (*current_value<=min_value)
   *current_value = min_value;
else if (*current_value>=max_value)
   *current_value = max_value;
}





void MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD(void)
    {
    timer_overflow_counter =0;
    set_button_loop_breakout();
    clear_keyboard_value();
    
    delay_ms(1);
    }

void MENU_MODULE_reset_loop_conditions_BUTTON_KEYBOARD(void)
    {
    set_button_loop_breakout();
    clear_keyboard_value();          //Jeï¿½li po klawiszu nieczytanym ma wyjï¿½ï¿½ i zrobiï¿½ akcjï¿½ to zahaszhowaï¿½
    
    delay_ms(1);
    }

void MENU_MODULE_reset_loop_conditions_TIMER_BUTTON(void)
    {
    timer_overflow_counter =0;       //key pressed so it should count from 0
    reset_button_loop_breakout();    //not supported key pressed so button_loop_breakout = 0 to exit loop
    
    delay_ms(1);
    }

int1 MENU_MODULE_audio_device_init(int *which_device)
    {
    int1 is_dev_already_starter = FALSE;
    const char comunicate[] = "Init device ";
    switch (MENU_MODULE_audio_devices_list[*which_device])
        {
#ifdef _USE_TEA5767_
        case MENU_MODULE_TEA5767_input:
        
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            MENU_MODULE_reset_all_devices(*which_device);
            //dbg_printf("%s%u",comunicate, *which_device);
            dbg_printf("%s%u",comunicate, *which_device);
            dbg_return_carriage();
            set_x_pack_colour(pack_1, colour_RED);
//            #ifdef _USE_ACTIVE_ANTENA_
               if (use_active_antena_val==1)
               {
                  ACTIVE_ANTENA_MODULE_active_antena_ON();
               }
  //          #endif
            //TODO: w³¹czyæ radio ³acznie z prze³aczeniem wejœcia equalizera i ustawieniem gain wejœcia

            
            
            break;
#endif

#ifdef _USE_MNONKEYBOARD_DAB_
        case MENU_MODULE_MONKEYBOARD_FM_input:
        
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            MENU_MODULE_reset_all_devices(*which_device);
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            dbg_printf("%s%u",comunicate, *which_device);
            dbg_return_carriage();
            set_x_pack_colour(pack_1, colour_RED);
            //TODO: Przed za³¹czeniem anteny prze³¹czyæ przekaŸnik steruj¹cy wejœciem anteny FM i DAB+
 //           #ifdef _USE_ACTIVE_ANTENA_
               if (use_active_antena_val==1)
               {
                  ACTIVE_ANTENA_MODULE_active_antena_ON();
               }
   //         #endif
            //TODO: w³¹czyæ radio ³acznie z prze³aczeniem wejœcia equalizera i ustawieniem gain wejœcia
           
            break;
        case MENU_MODULE_MONKEYBOARD_DAB_input:
        
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            MENU_MODULE_reset_all_devices(*which_device);
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            dbg_printf("%s%u",comunicate, *which_device);
            dbg_return_carriage();
            set_x_pack_colour(pack_1, colour_GREEN);
            //TODO: Przed za³¹czeniem anteny prze³¹czyæ przekaŸnik steruj¹cy wejœciem anteny FM i DAB+
 //           #ifdef _USE_ACTIVE_ANTENA_
               if (use_active_antena_val==1)
               {
                  ACTIVE_ANTENA_MODULE_active_antena_ON();
               }
  //          #endif
            //TODO: w³¹czyæ radio ³acznie z prze³aczeniem wejœcia equalizera i ustawieniem gain wejœcia
            
            break;
            
        case MENU_MODULE_MONKEYBOARD_AM_input:            
            
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            MENU_MODULE_reset_all_devices(*which_device);
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            dbg_printf("%s%u",comunicate, *which_device);
            dbg_return_carriage();
            set_x_pack_colour(pack_1, colour_SKYBLUE);
            //TODO: Przed za³¹czeniem anteny prze³¹czyæ przekaŸnik steruj¹cy wejœciem anteny FM i DAB+
  //          #ifdef _USE_ACTIVE_ANTENA_
               if (use_active_antena_val==1)
               {
                  ACTIVE_ANTENA_MODULE_active_antena_ON();
               }
    //        #endif
            //TODO: w³¹czyæ radio ³acznie z prze³aczeniem wejœcia equalizera i ustawieniem gain wejœcia
            
            break;
            
#endif

#ifdef _USE_VMUSIC3_
        case MENU_MODULE_VMUSIC3_input:
        
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            MENU_MODULE_reset_all_devices(*which_device);
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            dbg_printf("%s%u",comunicate, *which_device);
            dbg_return_carriage();
            //set_x_pack_colour(pack_1, colour_BLUE);
            set_x_pack_colour(pack_1, colour_WHITE);
            
            //TODO: w³¹czyæ urz¹dzenie ³acznie z prze³aczeniem wejœcia equalizera i ustawieniem gain wejœcia
            
            
            
            break;
        case MENU_MODULE_VMUSIC3_RND_input:
        
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            MENU_MODULE_reset_all_devices(*which_device);
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            dbg_printf("%s%u",comunicate, *which_device);
            dbg_return_carriage();
            set_x_pack_colour(pack_1, colour_YELLOW);
            //TODO: w³¹czyæ urz¹dzenie ³acznie z prze³aczeniem wejœcia equalizera i ustawieniem gain wejœcia
            
            
            break;
#endif

#ifdef _USE_BLUETOOTH_
        case MENU_MODULE_BLUETOOTH_input:
        
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            MENU_MODULE_reset_all_devices(*which_device);
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            dbg_printf("%s%u",comunicate, *which_device);
            dbg_return_carriage();
            
            
            //set_x_pack_colour(pack_1, colour_WHITE);
            set_x_pack_colour(pack_1, colour_BLUE);
            
            //TODO: w³¹czyæ urz¹dzenie ³acznie z prze³aczeniem wejœcia equalizera i ustawieniem gain wejœcia        
        

#endif

#ifdef _USE_AUX_
        case MENU_MODULE_AUX_input :
        
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            MENU_MODULE_reset_all_devices(*which_device);
            //TODO: wy³¹czyæ wszystkie urz¹dzenia ³acznie z active antena, a po set kolor w³aczyæ w³aœciwe
            dbg_printf("%s%u",comunicate, *which_device);
            dbg_return_carriage();
            set_x_pack_colour(pack_1, colour_PURPLE);
            //TODO: w³¹czyæ urz¹dzenie ³acznie z prze³aczeniem wejœcia equalizera i ustawieniem gain wejœcia
            
            
            break;
#endif
        case MENU_MODULE_NOTSUPPORTED_input :

            dbg_printf("Dev %u not suppotred", *which_device);
            dbg_return_carriage();

            is_dev_already_starter = TRUE;   //fake 1st run - potrzebne aby przeskoczyï¿½ sprzet nie obsï¿½ugiwany przez firmwarer
            *which_device=*which_device+1;
            break;
        }
    return is_dev_already_starter;
    }

void MENU_MODULE_audio_device_step_action(int which_device, int direction_UP_DOWN)
    {
    char direction[13];
    switch (direction_UP_DOWN)
        {
        case MENU_MODULE_audio_device_stepUP:
            strcpy(direction,"stepUP");
            break;
        case MENU_MODULE_audio_device_stepDOWN:
            strcpy(direction,"stepDOWN");
            break;
        case MENU_MODULE_audio_device_auto_tuneUP:
            strcpy(direction,"autoTuneUP");
            break;
        case MENU_MODULE_audio_device_auto_tuneDOWN:
            strcpy(direction,"autoTuneDOWN");
        }
    switch (MENU_MODULE_audio_devices_list[which_device])
        {
#ifdef _USE_TEA5767_
        case MENU_MODULE_TEA5767_input:
            dbg_printf("TEA5767 %s",direction);
            dbg_return_carriage();
            break;
#endif
#ifdef _USE_MNONKEYBOARD_DAB_
        case MENU_MODULE_MONKEYBOARD_FM_input:
            dbg_printf("MonkeyBoard FM %s",direction);
            dbg_return_carriage();
            break;
        case MENU_MODULE_MONKEYBOARD_DAB_input:
            dbg_printf("MonkeyBoard DAB+ %s",direction);
            dbg_return_carriage();
            break;

        case MENU_MODULE_MONKEYBOARD_AM_input:
            dbg_printf("MonkeyBoard AM %s",direction);
            dbg_return_carriage();
            break;
            
            
#endif
#ifdef _USE_VMUSIC3_
        case MENU_MODULE_VMUSIC3_input:
            dbg_printf("VMusic %s",direction);
            dbg_return_carriage();
            break;
        case MENU_MODULE_VMUSIC3_RND_input:
            dbg_printf("VMusic RDN %s",direction);
            dbg_return_carriage();
            break;
#endif

#ifdef _USE_BLUETOOTH_
        case MENU_MODULE_BLUETOOTH_input:
            dbg_printf("BLUETOOTH %s",direction);
            dbg_return_carriage();
            break;
#endif

#ifdef _USE_AUX_
        case MENU_MODULE_AUX_input :
            dbg_printf("AUX %s",direction);
            dbg_return_carriage();
            break;
#endif
        case MENU_MODULE_NOTSUPPORTED_input :
            dbg_printf("Dev not suppotred %s",direction);
            dbg_return_carriage();
            
            break;
        }
    }

void MENU_MODULE_submenu_action(int which_submenu, int1 direction_UP_DOWN)
    {
//void MENU_MODULE_submenu_action(unsigned int which_submenu, int1 direction_UP_DOWN){
    char direction[5];
    if (direction_UP_DOWN == MENU_MODULE_EQUALISER_direct_UP)
        {
        strcpy(direction,"UP");
        }
    else
        {
        strcpy(direction,"DOWN");
        }

    switch (which_submenu)
        {
//#ifdef _USE_ACTIVE_ANTENA_
        case MENU_MODULE_EQUALISER_MENU_ACTIVE_ANTENA:
            dbg_printf("Active Antena:%s", direction);
            dbg_return_carriage();
            switch(direction_UP_DOWN)
            {
            case MENU_MODULE_EQUALISER_direct_DOWN:
               use_active_antena_val=use_active_antena_val-1;
               ACTIVE_ANTENA_MODULE_active_antena_OFF();
               break;
            case MENU_MODULE_EQUALISER_direct_UP:
               use_active_antena_val=use_active_antena_val+1;
               ACTIVE_ANTENA_MODULE_active_antena_ON();
               break;
            }   
            MENU_MODULE_check_variable_boarders(&use_active_antena_val,active_antena_MIN_VAL, active_antena_MAX_VAL);
            
            break;
//#endif
        case MENU_MODULE_EQUALISER_MENU_Treble:

            dbg_printf("Treble:%s", direction);
            dbg_return_carriage();
            break;
        case MENU_MODULE_EQUALISER_MENU_Middle:

            dbg_printf("Middle:%s", direction);
            dbg_return_carriage();

            break;
        case MENU_MODULE_EQUALISER_MENU_Bass:

            dbg_printf("Bass:%s", direction);
            dbg_return_carriage();

            break;
        case MENU_MODULE_EQUALISER_MENU_Low_HighBoost:

            dbg_printf("L_H_Boost:%s", direction);
            dbg_return_carriage();

            break;
        case MENU_MODULE_EQUALISER_MENU_Left_Right:

            dbg_printf("Left-Right:%s", direction);
            dbg_return_carriage();

            break;
        case MENU_MODULE_EQUALISER_MENU_Front_Rear:

            dbg_printf("Front-Rear:%s", direction);
            dbg_return_carriage();

            break;
#ifdef _USE_BACKLIGHT_
         case MENU_MODULE_EQUALISER_MENU_Backlight:
            dbg_printf("Backlight:%s", direction);
            dbg_return_carriage();
            switch(direction_UP_DOWN)
            {
            case MENU_MODULE_EQUALISER_direct_DOWN:
               Backlight_value=Backlight_value-1;
               break;
            case MENU_MODULE_EQUALISER_direct_UP:
               Backlight_value=Backlight_value+1;
               break;
            }      
            //Backlight_value=check_boarders(Backlight_value, Backlight_MIN_VAL, Backlight_MAX_VAL);
            
            MENU_MODULE_check_variable_boarders(&Backlight_value, Backlight_MIN_VAL, Backlight_MAX_VAL);
            
            set_backlight(Backlight_value);
            break;           
#endif        
        case MENU_MODULE_EQUALISER_MENU_ERROR:
            dbg_printf("EQ !supp.:%s", direction);
            dbg_return_carriage();
            break;
        }
    }

void MENU_MODULE_submenu_set_LED(int which_submenu)
{
    switch (which_submenu)
         {
         case MENU_MODULE_EQUALISER_MENU_ACTIVE_ANTENA:
            set_x_pack_colour(pack_2, colour_RED);
            break;
         case MENU_MODULE_EQUALISER_MENU_Treble:
            set_x_pack_colour(pack_2, colour_GREEN);
            break;
         case MENU_MODULE_EQUALISER_MENU_Middle:
            set_x_pack_colour(pack_2, colour_BLUE);
            break;
         case MENU_MODULE_EQUALISER_MENU_Bass:
            set_x_pack_colour(pack_2, colour_YELLOW);
            break;
         case MENU_MODULE_EQUALISER_MENU_Low_HighBoost:
            set_x_pack_colour(pack_2, colour_PURPLE);
            break;
         case MENU_MODULE_EQUALISER_MENU_Left_Right:
            set_blink_x_pack(pack_2, colour_SKYBLUE, colour_WHITE);
            //set_x_pack_colour(pack_2, colour_SKYBLUE);
            break;
         case MENU_MODULE_EQUALISER_MENU_Front_Rear:
            set_blink_x_pack(pack_2, colour_BLACK, colour_SKYBLUE);
            break;
         #ifdef _USE_BACKLIGHT_
         case MENU_MODULE_EQUALISER_MENU_Backlight:
            set_x_pack_colour(pack_2, colour_WHITE);
            break;
         #endif
         }
}

void MENU_MODULE_submenu_clear_LED(int _main_menu)
{
   //if (_main_menu==MENU_MODULE_EQUALIZER_menu)
   
   if (_main_menu!=MENU_MODULE_PLAY_menu)
      {
      clrl_x_pack(pack_2);
      }
}
