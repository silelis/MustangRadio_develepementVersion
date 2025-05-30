//#include <16F877.h>
//#include <16F1939.h>
#include <18F4520.h>
#define device getenv("DEVICE")


#ifndef __CONFIG_FILE__
   #include<MASTER_CONFIG_FILE.h>
#endif

#include <main.h>

signed int8  use_active_antena_val;

signed int8    Backlight_value;
   
#ifndef __keyboard__
   #include</hardware/keyboard/kyeboard.c>
#endif

#ifndef _i2c_HMISTEPPER_MOTOR_
   #include </hardware/I2C_HMI/i2c_HMI_STEPPER_MOTOR/i2c_HMI_STEPPER_MOTOR.c> 
#endif

#ifndef __timeout__
   #include</core/timer/timeout.c>
#endif

 #ifdef _USE_BACKLIGHT_
   #ifndef _i2c_HMI_BACKLIGHT_
      #include </hardware/I2C_HMI/i2c_HMI_BACKLIGHT/i2c_HMI_BACKLIGHT.c>      
   #endif
#endif   

#ifndef __menu_module_function__
   #include</modules/menu_module/menu_module.c>
#endif

#ifndef __EEPROM_MODULE_
   #include </modules/eeprom_module/eeprom_module.c>
#endif   

#ifndef __i2c_read_active_antena_setup__
   #include </hardware/I2C_HMI/i2c_read_active_antena_setpu/i2c_read_active_antena_setpu.c>
#endif

extern unsigned int   MasterBuffer[I2C_BUFSIZE];
extern unsigned int   SlaveBuffer[I2C_BUFSIZE];
#ifndef _I2C_ENGINE_
   #include </core/I2C_Engine/I2C_Engine.c>
#endif


//-----------------------------------------------------------//
//---------------- INTERRUPTS CONFIGURATION ---------------- //
//-----------------------------------------------------------// 
/*Obs�uga przerwania klawiatury*/

#ifndef __keyboard_interrupt__
#INT_EXT
void  EXT_isr(void)
    {
    disable_interrupts(INT_TIMER1);    //wy��cza przerwanie timera, bardzo wa�ne �eby zadzia���a funkcja odsprz�glania drgaj�cych styk�w
    read_keyboard_value();
    enable_interrupts(INT_TIMER1);    //w��cza przerwanie timera
    }
#endif
/*Obs�uga przerwania klawiatury*/

/*Obs�uga timera licz�cego timeout w menu programu g��wnego*/
#ifndef __timer_inetrrupt__
#define __timer_inetrrupt__
#INT_TIMER1
void  TIMER1_isr(void)
    {
    disable_interrupts(INT_EXT);
    timer_overflow_counter ++;
    timeout_occured = set_timer(timeout_time_in_seconds);
    enable_interrupts(INT_EXT);
    }
#endif
/*Obs�uga timera licz�cego timeout w menu programu g��wnego*/
//-----------------------------------------------------------//
//---------------- INTERRUPTS CONFIGURATION ---------------- //
//-----------------------------------------------------------//


#opt 11     //default: 9
#zero_ram
void main()
    {  
    
    delay_ms(1500);
    dbg_printf(master_fw_version __DATE__ " " __TIME__);
    dbg_return_carriage();
    //Brown-out routine //
    
  #if getenv("SFR_VALID:RCON")
      //#warning   RCON getenv("SFR:RCON")
      #byte      RCON = getenv("SFR:RCON")
      //#warning   BOR getenv("SFR:RCON").0
      #bit       BOR = RCON.0
      BOR = 1;
      brownout_enable(True);
  #else
      #warning   "This PIC do not have RCON register (BOR not possible) or if PIC have PCON register there is BOD"
  #endif
  
  
  
    
/*   #if device=="PIC18F4520"
      #warning   RCON getenv("SFR:RCON")
      #byte      RCON = getenv("SFR:RCON")
      #warning   BOR getenv("SFR:RCON").0
      #bit       BOR = RCON.0
      BOR = 1;
      brownout_enable(True);
    #endif*/
    //Brown-out routine //
    
    // czyszczenie EEPromu - w ostatecznej wresji programu mo�esz z tego zrezygnowa�, a mo�e lepiej nie a na obudowie umie�ci� przycisk RESET EEPROM
    
    int8 active_antena_power_setup = get_active_antena_power_setup();
    dbg_printf("Antena power setup:%x", active_antena_power_setup);
    dbg_return_carriage();

   if (input(button1)==1 && input(button2)==1 && input(button3)==1 && input(button4)==1)
       {
         eeprom_erease();
       }

    //zmienne do obs�ugi menu radia   
    int8  main_menu    = MENU_MODULE_PLAY_menu;       // przechowuje informacj� na temat aktualnego menu w kt�rym si� znajdujesz, na samym pocz�tku program startuje w menu obs�ugi urz�dze� graj�cych
    int8  back_to_menu = main_menu;
    int1  device_already_started   = TRUE;
    signed int   equaliser_submenu;
    //zmienne do obs�ugi menu radia
    
    // Zmienne trzymaj�ce parametry radio zapisywane w EEPROM
    signed int     _radio_1st_run;
    signed int     audio_devices_playing_submenu; //RADIO_Input_value int //zmienna w menu radia 
    signed int32   FM_freuency_value; //FM_91000000 hz
    signed int32   AM_freuency_value; //AM_522000 hz

    
    
    signed int8    EQU_VOL_value;
    signed int8    EQU_Treble_value;
    signed int8    EQU_Middle_value;
    signed int8    EQU_Bass_value;
    signed int8    EQU_L_H_Boost_value;
    signed int8    EQU_Left_Right_value;
    signed int8    EQU_Front_Rear_value;
    
    

    
    //TODO: AM FREQ
    //TODO: DAB_freuency_value

    
   //TODO:    #define DAB_freuency_DEFAULT_VAL
   //TODO:     #define Backlight_DEFAULT_VAL    
    // Zmienne trzymaj�ce parametry radio zapisywane w EEPROM

//-----------------------------------------------------------//
//------------ READ EEPROM RADIO CONFIGURATION ------------- //
//-----------------------------------------------------------//  
       {
       EEPROM_MODULE_READ_radio_1st_run();         //czyta warto�� _radio_1st_run z EEPROM

       if (_radio_1st_run!=1)
          {
          dbg_printf("Radio 1st run.Set default vals");
          dbg_return_carriage();
   
//ustawia pierwsze skonfigurowane w RADIO wej�cie audio
          for(int i=0; i<MENU_MODULE_audio_devices_list_len; i++)
              {
              if (MENU_MODULE_audio_devices_list[i] !=MENU_MODULE_NOTSUPPORTED_input)
                  {
                  audio_devices_playing_submenu = i;
                  dbg_printf("Audio dev:%x",MENU_MODULE_audio_devices_list[i]);
                  dbg_return_carriage();
                  break;
                  }
              }
            EEPROM_MODULE_WRITE_audio_input();
//ustawia pierwsze skonfigurowane w RADIO wej�cie audio
 
         use_active_antena_val=active_antena_MAX_VAL;
         EEPROM_MODULE_WRITE_active_antena();
         
         FM_freuency_value = FM_freuency_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_fm_freqency();
         
         EQU_VOL_value = EQU_VOL_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_equ_vol();
          
         EQU_Treble_value=EQU_Treble_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_equ_treble();
         
         EQU_Middle_value=EQU_Middle_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_equ_middle();
         
         EQU_Bass_value=EQU_Bass_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_equ_bass();
         
         EQU_L_H_Boost_value=EQU_L_H_Boost_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_equ_l_h_boost();
         
         EQU_Left_Right_value=EQU_Left_Right_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_equ_left_right();
         
         EQU_Front_Rear_value=EQU_Front_Rear_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_equ_front_rear();
     
         AM_freuency_value=AM_freuency_DEFAULT_VAL;
         EEPROM_MODULE_WRITE_am_freqency();
         
         #ifdef _USE_BACKLIGHT_
            Backlight_value=Backlight_DEFAULT_VAL;
            EEPROM_MODULE_WRITE_hmi_backlight();
         #endif
         
         dbg_printf("Nie pisze do eeprom DAB+ default freq");
         dbg_return_carriage();
         
         dbg_printf("Nie pisze w eeprom zerowej granej mp3");
         dbg_return_carriage();        
         
         //TODO: ustawienie i pisanie do EEPROM tych wartto�ci: DAB_freuency_value,  Backlight_value
         //TODO:DAB_freuency_value

          _radio_1st_run = 1;
          EEPROM_MODULE_WRITE_radio_1st_run();        //musi by� na sam koniec, na wypadej jak by w trakcie pierwszego uruchomienia w radio by� power off
          }
       else 
          {
          dbg_printf("Read EEprom");
          dbg_return_carriage();
         
          dbg_printf("R.input");
          dbg_return_carriage();
          EEPROM_MODULE_READ_audio_input();    
          EEPROM_MODULE_CHECK_audio_input();
          //EEPROM_MODULE_R_C_audio_input();             
         
          dbg_printf("R.aa");
          dbg_return_carriage();
          EEPROM_MODULE_READ_active_antena();
          EEPROM_MODULE_CHECK_active_antena();
          //EEPROM_MODULE_R_C_active_antena();
            
          dbg_printf("R.fm_freq");   
          dbg_return_carriage();
          EEPROM_MODULE_READ_fm_freqency();
          EEPROM_MODULE_CHECK_fm_freqency(); 
          //EEPROM_MODULE_R_C_fm_freqency();         
          
          dbg_printf("R.vol"); 
          dbg_return_carriage();
          EEPROM_MODULE_READ_equ_vol();
          EEPROM_MODULE_CHECK_equ_vol();
          //EEPROM_MODULE_R_C_equ_vol(); 

          dbg_printf("R.tre"); 
          dbg_return_carriage();
          EEPROM_MODULE_READ_equ_treble();
          EEPROM_MODULE_CHECK_equ_treble();
          //EEPROM_MODULE_R_C_equ_treble();

          dbg_printf("R.mid"); 
          dbg_return_carriage();
          EEPROM_MODULE_READ_equ_middle();
          EEPROM_MODULE_CHECK_equ_middle();
          //EEPROM_MODULE_R_C_equ_middle();

          dbg_printf("R.bass"); 
          dbg_return_carriage();
          EEPROM_MODULE_READ_equ_bass();
          EEPROM_MODULE_CHECK_equ_bass();
          //EEPROM_MODULE_R_C_equ_bass();

          dbg_printf("R.boost"); 
          dbg_return_carriage();
          EEPROM_MODULE_READ_equ_l_h_boost();
          EEPROM_MODULE_CHECK_equ_l_h_boost();
          //EEPROM_MODULE_R_C_equ_l_h_boost();

          dbg_printf("R.l-r");
          dbg_return_carriage();
          EEPROM_MODULE_READ_equ_left_right();
          EEPROM_MODULE_CHECK_equ_left_right();
          //EEPROM_MODULE_R_C_equ_left_right();

          dbg_printf("R.f-r");
          dbg_return_carriage();
          EEPROM_MODULE_READ_equ_front_rear();
          EEPROM_MODULE_CHECK_equ_front_rear();
          //EEPROM_MODULE_R_C_equ_equ_front_rear();
          
          dbg_printf("R.am freq");
          dbg_return_carriage();
          EEPROM_MODULE_READ_am_freqency(); 
          EEPROM_MODULE_CHECK_am_freqency();
          
          #ifdef _USE_BACKLIGHT_
             dbg_printf("R.BCK_light");
             dbg_return_carriage();
             EEPROM_MODULE_READ_hmi_backlight();
             EEPROM_MODULE_CHECK_hmi_backlight();
          #endif
          
         dbg_printf("Nie czytam z eeprom DAB+ default freq");
         dbg_return_carriage();
         
         dbg_printf("Nie czytam z eeprom ostatnio granej mp3");
         dbg_return_carriage();  
          
          //TODO: Sprawdzenie poprawno�ci odczytu w/w warto�ci          //TODO: zrobi� define z min, max i default waro�ciami w zale�no�ci od konfigu w config_file.h???
          //TODO: czytanie DAB freq
          //TODO: Sprawdzenie poprawno�ci odczytu w/w warto�ci          //TODO: zrobi� define z min, max i default waro�ciami w zale�no�ci od konfigu w config_file.h???
          //TODO: czytanie BACKLIGHT
          //TODO: Sprawdzenie poprawno�ci odczytu w/w warto�ci          //TODO: zrobi� define z min, max i default waro�ciami w zale�no�ci od konfigu w config_file.h???
          }
       }
//-----------------------------------------------------------//
//------------ READ EEPROM RADIO CONFIGURATION ------------- //
//-----------------------------------------------------------//  



//-----------------------------------------------------------//
//------- INIT HARDWARE: KB, TIMER, INTERRUPT, ETC --------- //
//-----------------------------------------------------------//
    //ext_int_edge(1, L_TO_H);
    //enable_interrupts(INT_EXT);
    init_keyboard();
    init_timeout();
    clrl_all_packs();
    enable_interrupts(GLOBAL);
    
    
  //TODO INIT equalizera i wzmacniacza
  //TODO INIT backlight
  //TODO: RERSTE WSZYSTKICH URZ�DZE� AUDIO
  
//-----------------------------------------------------------//
//------- INIT HARDWARE: KB, TIMER, INTERRUPT, ETC --------- //
//-----------------------------------------------------------//    
    
    
    
    
//-----------------------------------------------------------//
//--------------- RADIO MENU LOOP BEGINNING ---------------- //
//-----------------------------------------------------------//   
   #ifdef _USE_BACKLIGHT_
      //set_1st_time_backlight
      set_backlight(Backlight_value);
      delay_ms(30);
      //set_1st_time_backlight
   #endif
  //TODO: NIe wiedziec czemu w tym miejscu (w symulatorze proteus) jak byla komenta STEPPER_MOTOR_send_percentage to wywala�o b��dn� komenh� i2C_HMI :/
   
   
    while(true)          //Main while loop
        {
   
//-----------------------------------------------------------//
//----------------- VOLUME BUTTON HANDLING ----------------- //
//-----------------------------------------------------------//
        if ((button_value_pattern==encoder1_right_pattern) || (button_value_pattern==encoder1_left_pattern))
            {
            int8 last_main_menu = main_menu;
            int1 is_VOLUME_loop_timeout;
            main_menu = MENU_MODULE_VOLUME_menu;
            back_to_menu = MENU_MODULE_NO_NEED_to_back;
            
            dbg_printf( "Vol menu");
            dbg_return_carriage();

            run_timer();
            // !!!!!!!!!!!!!! variabnles used only to save data !!!!!!!!!!!!!!
            signed int8 EQU_VOL_temp = EQU_VOL_value;
            // !!!!!!!!!!!!!! variabnles used only to save data !!!!!!!!!!!!!!
            do
                {
                if ((button_value_pattern==encoder1_right_pattern))
                    //Volume UP routine
                    {
                    dbg_printf( "Vol+");
                    dbg_return_carriage();
                    EQU_VOL_value=EQU_VOL_value+1;
                    
                    //TODO: set vol    
                    
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();
                    }
                else if (button_value_pattern==encoder1_left_pattern)
                    //Volume DOWN routine
                    {
                    dbg_printf( "Vol-");
                    dbg_return_carriage();
                    EQU_VOL_value=EQU_VOL_value-1;
                    
                    //TODO: set vol
                    
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();
                    }
                else if (button_value_pattern==0)
                    //No key presses WAIT untill timeout
                    {
                    MENU_MODULE_reset_loop_conditions_BUTTON_KEYBOARD();
                    }
                else
                    //Any other key presses end Volume loop
                    {
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON();
                    }
                is_VOLUME_loop_timeout = check_timeout();
                }
            while(button_loop_breakout&&!is_VOLUME_loop_timeout);
            clear_timer();    //MUSI BY� poniewa� z p�tli mo�na wyj�c w przypadku timeoutu i naci�ni�cia innego klawisza ni� obs�ugiwany
            back_to_menu =last_main_menu;
            
            //SAVE VOL if required
            if (EQU_VOL_temp!=EQU_VOL_value)
               {
               dbg_printf("SAVE volume");
               dbg_return_carriage();
               //EEPROM_MODULE_CHECK_TEMP_equ_vol();   //t� funkcj� da� bym w innym module od rtazu ze sprawdzeniem ifa
               EEPROM_MODULE_WRITE_equ_vol();
               }
            //SAVE VOL if required
            }
//-----------------------------------------------------------//
//----------------- VOLUME BUTTON HANDLING END-------------- //
//-----------------------------------------------------------//



//--------------------------------------------------------------//
//----------------- AUDIO INPUT MODE HANDLING ----------------- //
//--------------------------------------------------------------//
        else if (button_value_pattern==button1_pattern ||                                //Naci�ni�to przycisk 1
                 (back_to_menu == MENU_MODULE_PLAY_menu&&button_value_pattern!=button2_pattern) ||    //Wyj�cie z innefo menu i nie naci�ni�to przyciskou ekuqlizera
                 (main_menu == MENU_MODULE_PLAY_menu&&button_value_pattern==encoder2_right_pattern) ||
                 (main_menu == MENU_MODULE_PLAY_menu&&button_value_pattern==encoder2_left_pattern) ||
                 (main_menu == MENU_MODULE_PLAY_menu&&button_value_pattern==button3_pattern) ||
                 (main_menu == MENU_MODULE_PLAY_menu&&button_value_pattern==button4_pattern) ||
                 device_already_started ==TRUE)
            {
            
            MENU_MODULE_submenu_clear_LED(main_menu); //reset EQUALISER LED PACK
            
            main_menu = MENU_MODULE_PLAY_menu;
            back_to_menu = MENU_MODULE_NO_NEED_to_back;

            dbg_printf("Player source switch");
            dbg_return_carriage();

            int1 is_AUDIO_INPUT_loop_timeout;
            run_timer();
            do
                {
                if (button_value_pattern==button1_pattern||device_already_started==TRUE)         //Prze��czenie sprz�tu audio
                    {
                    /*timer_overflow_counter =0;
                    set_button_loop_breakout();
                    clear_keyboard_value();*/
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();
                    dbg_printf("Switch input");
                    dbg_return_carriage();

                    clear_keyboard_value();
                    if (device_already_started==TRUE)
                        {
                        device_already_started=FALSE;
                        audio_devices_playing_submenu=audio_devices_playing_submenu;
                        }
                    else
                        {
                        audio_devices_playing_submenu=audio_devices_playing_submenu+1;
                        }

                    if (audio_devices_playing_submenu>MENU_MODULE_audio_devices_list_len)
                        {
                        audio_devices_playing_submenu = 0;
                        }

                    dbg_printf( "Dev. %u init", audio_devices_playing_submenu);
                    dbg_return_carriage();

                    device_already_started = MENU_MODULE_audio_device_init(&audio_devices_playing_submenu);

                    }
                else if (button_value_pattern==encoder2_right_pattern)
                    {

                    /*timer_overflow_counter =0;
                    set_button_loop_breakout();
                    clear_keyboard_value();*/
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();
                    //dbg_printf("Step UP ");

                    MENU_MODULE_audio_device_step_action(audio_devices_playing_submenu,MENU_MODULE_audio_device_stepUP);

                    }
                else if (button_value_pattern==encoder2_left_pattern)
                    {



                    /*timer_overflow_counter =0;
                    set_button_loop_breakout();
                    clear_keyboard_value();*/
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();
                    MENU_MODULE_audio_device_step_action(audio_devices_playing_submenu,MENU_MODULE_audio_device_stepDOWN);

                    }
                else if (button_value_pattern==button3_pattern)
                    {

                    /*timer_overflow_counter =0;
                    set_button_loop_breakout();
                    clear_keyboard_value();*/
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();

                    //dbg_printf("autoTuneDOWN ");
                    MENU_MODULE_audio_device_step_action(audio_devices_playing_submenu,MENU_MODULE_audio_device_auto_tuneDOWN);

                    }
                else if (button_value_pattern==button4_pattern)
                    {


                    /*timer_overflow_counter =0;
                    set_button_loop_breakout();
                    clear_keyboard_value();*/
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();

                    //dbg_printf("autoTuneUP ");

                    MENU_MODULE_audio_device_step_action(audio_devices_playing_submenu,MENU_MODULE_audio_device_auto_tuneUP);

                    }
                else if (button_value_pattern==0)
                    //No key presses WAIT untill timeout
                    {
                    //timer_overflow_counter =0;
                    /*set_button_loop_breakout();
                    clear_keyboard_value();          //Je�li po klawiszu nieczytanym ma wyj�� i zrobi� akcj� to zahaszhowa�*/
                    MENU_MODULE_reset_loop_conditions_BUTTON_KEYBOARD();
                    //dbg_printf("Play blank ");
                    }
                else
                    //Any other key presses end AUDIO INPUT loop
                    {
                    /*timer_overflow_counter =0;       //key pressed so it should count from 0
                    reset_button_loop_breakout();    //not supported key pressed so button_loop_breakout = 0 to exit loop*/
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON();
                    }
                is_AUDIO_INPUT_loop_timeout = check_timeout();
                }
            while(button_loop_breakout&&!is_AUDIO_INPUT_loop_timeout);
            //reset_button_loop_breakout();
            clear_timer();    //MUSI BY� poniewa� z p�tli mo�na wyj�c w przypadku timeoutu i naci�ni�cia innego klawisza ni� obs�ugiwany

            //dbg_printf("SAVE AUDIO INPUT params:");
            //dbg_return_carriage();
            //dbg_printf("s.AUDIO INPUT input");
           // dbg_return_carriage();
            //dbg_printf("s.AUDIO INPUT freq FM");
           // dbg_return_carriage();
           // dbg_printf("s.AUDIO INPUT freq AM");
            //dbg_return_carriage();
            //dbg_printf("s.AUDIO INPUT freq DAB");
            //dbg_return_carriage();
            //TODO: zapis audio input
            //TODO: zapis AM freq
            //TODO: zapis FM freq
            //TODO: zapis DAB freq
            }
//--------------------------------------------------------------//
//----------------- AUDIO INPUT MODE HANDLING END-------------- //
//--------------------------------------------------------------//



//--------------------------------------------------------------//
//----------------- EQUALISER BUTTON HANDLING ----------------- //
//--------------------------------------------------------------//
        else if   (button_value_pattern==button2_pattern||
                   (back_to_menu == MENU_MODULE_EQUALIZER_menu&&button_value_pattern!=button1_pattern)||
                   (main_menu == MENU_MODULE_EQUALIZER_menu&&button_value_pattern==encoder2_right_pattern) ||
                   (main_menu == MENU_MODULE_EQUALIZER_menu&&button_value_pattern==encoder2_left_pattern) ||
                   (main_menu == MENU_MODULE_EQUALIZER_menu&&button_value_pattern==button3_pattern) ||
                   (main_menu == MENU_MODULE_EQUALIZER_menu&&button_value_pattern==button4_pattern))
            {

            int8 last_main_menu = main_menu;
            int1 is_EQUALISERT_loop_timeout;
            int1 _1st_EQUALISER_loop_execution= TRUE;

            main_menu = MENU_MODULE_EQUALIZER_menu;
            back_to_menu = MENU_MODULE_NO_NEED_to_back;
            dbg_printf("Equaliser menu");
            dbg_return_carriage();

            run_timer();
            
            // !!!!!!!!!!!!!! variabnles used only to save data !!!!!!!!!!!!!! 
            signed int8 Backlight_temp=Backlight_value;
            signed int8 use_active_antena_val_temp=use_active_antena_val;
            
            // !!!!!!!!!!!!!! variabnles used only to save data !!!!!!!!!!!!!! 
            
            do
                {
                if (button_value_pattern==button2_pattern)
                    {
//#ifdef _USE_ACTIVE_ANTENA_
                    int1 is_ACTIVE_ANTENA_MENU_REQUIRED;
//#endif
                    if (_1st_EQUALISER_loop_execution == TRUE && last_main_menu != MENU_MODULE_VOLUME_menu)  //Je�li by� w MENU PLAY lub powr�t z MENU VOLUME
                        {
//#ifdef _USE_ACTIVE_ANTENA_
                        if (MENU_MODULE_equaliser_menu_list[0]!=MENU_MODULE_EQUALISER_MENU_ERROR &&
                                (MENU_MODULE_audio_devices_list[audio_devices_playing_submenu]== MENU_MODULE_TEA5767_input ||
                                 MENU_MODULE_audio_devices_list[audio_devices_playing_submenu]== MENU_MODULE_MONKEYBOARD_FM_input ||
                                 MENU_MODULE_audio_devices_list[audio_devices_playing_submenu]== MENU_MODULE_MONKEYBOARD_AM_input ||
                                 MENU_MODULE_audio_devices_list[audio_devices_playing_submenu]==MENU_MODULE_MONKEYBOARD_DAB_input))
                            {
                            equaliser_submenu=0;                //je��i zdefiniowano ACTIVE ANTENA i jest odtwarzane RADO FM lub DAB
                            is_ACTIVE_ANTENA_MENU_REQUIRED = TRUE;
                            }
                        else
//#endif
                            {
                            equaliser_submenu=1;
//#ifdef _USE_ACTIVE_ANTENA_
                            is_ACTIVE_ANTENA_MENU_REQUIRED = FALSE;
//#endif
                            }
                        }
                    else
                        {
                        equaliser_submenu=equaliser_submenu+1;
                        //if (equaliser_submenu>MENU_MODULE_equaliser_menu_list_len)
                        if (equaliser_submenu>MENU_MODULE_equaliser_menu_list_len|| (equaliser_submenu==MENU_MODULE_EQUALISER_MENU_Backlight&&MENU_MODULE_equaliser_menu_list[MENU_MODULE_EQUALISER_MENU_Backlight]==MENU_MODULE_EQUALISER_MENU_ERROR)) // czy equaliser submenu jest wi�ksze od listy urz�dze� lub nie ma zarz�dzania pod�wietleniem
                            {
//#ifdef _USE_ACTIVE_ANTENA_
                            if (is_ACTIVE_ANTENA_MENU_REQUIRED == TRUE)
                                {
                                equaliser_submenu=0;
                                }
                            else
//#endif
                                {
                                    equaliser_submenu=1;
                                }

                                }
                        }
                    _1st_EQUALISER_loop_execution = FALSE;
                    dbg_printf("EQ submenu %u", equaliser_submenu);
                    dbg_return_carriage();

                   MENU_MODULE_submenu_set_LED(equaliser_submenu);

                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();
                    }
                else if (button_value_pattern==encoder2_right_pattern || button_value_pattern==button4_pattern)
                    {

                    MENU_MODULE_submenu_action(MENU_MODULE_equaliser_menu_list[equaliser_submenu], MENU_MODULE_EQUALISER_direct_UP); //EQUALISER ACTION UP
                    //MENU_MODULE_submenu_action_UP(MENU_MODULE_equaliser_menu_list[equaliser_submenu]); //EQUALISER ACTION UP

                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();
                    }
                else if (button_value_pattern==encoder2_left_pattern || button_value_pattern==button3_pattern)
                    {

                    MENU_MODULE_submenu_action(MENU_MODULE_equaliser_menu_list[equaliser_submenu], MENU_MODULE_EQUALISER_direct_DOWN); //EQUALISER ACTION DOWN
                    //MENU_MODULE_submenu_action_DOWN(MENU_MODULE_equaliser_menu_list[equaliser_submenu]); //EQUALISER ACTION DOWN

                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON_KEYBOARD();
                    }
                else if (button_value_pattern==0)
                    //No key presses WAIT untill timeout
                    {
                    //timer_overflow_counter =0;
                    /*set_button_loop_breakout();
                    clear_keyboard_value();          //Je�li po klawiszu nieczytanym ma wyj�� i zrobi� akcj� to zahaszhowa�*/
                    MENU_MODULE_reset_loop_conditions_BUTTON_KEYBOARD();
                    //dbg_printf("EQ blank ");
                    //dbg_printf("");
                    }
                else
                    //Any other key presses end AUDIO INPUT loop
                    {
                    /*timer_overflow_counter =0;       //key pressed so it should count from 0
                    reset_button_loop_breakout();    //not supported key pressed so button_loop_breakout = 0 to exit loop*/
                    MENU_MODULE_reset_loop_conditions_TIMER_BUTTON();
                    }
                is_EQUALISERT_loop_timeout = check_timeout();
                }
            while(button_loop_breakout&&!is_EQUALISERT_loop_timeout);
            clear_timer();    //MUSI BY� poniewa� z p�tli mo�na wyj�c w przypadku timeoutu i naci�ni�cia innego klawisza ni� obs�ugiwany
            
            
            
            dbg_printf("SAVE EQUALISER:");
            dbg_return_carriage();
            
            if (Backlight_temp!=Backlight_value)
            {
               dbg_printf("SAVE BCK_light");
               dbg_return_carriage();
               EEPROM_MODULE_WRITE_hmi_backlight();
            }
            if (use_active_antena_val_temp!=use_active_antena_val)
            {
               dbg_printf("SAVE AAntena ");
               dbg_return_carriage();
               EEPROM_MODULE_WRITE_active_antena();
            }
            
            //What to save
            //EQUALISER_MENU_Treble           1
            //EQUALISER_MENU_Middle           2
            //EQUALISER_MENU_Bass             3
            //EQUALISER_MENU_Low_HighBoost    4
            //EQUALISER_MENU_Left_Right       5
            //EQUALISER_MENU_Front_Rear       6
            //enable_interrupts(GLOBAL);
            //delay_ms(500);
            //#endif
            back_to_menu = MENU_MODULE_PLAY_menu;


            }
//-----------------------------------------------------------------//
//----------------- EQUALISER BUTTON HANDLING END----------------- //
//---------------------------------------------------------+--------//




//---------------------------------------------------------------------//
//-------------UNSUPPORTED BUTTONS WHILE MAIN LOOP-------------------- //
//---------------------------------------------------------------------//
        else if (button_value_pattern)
            {
            int8 last_main_menu = main_menu;
            main_menu = MENU_MODULE_UNSUPPORTED_menu;
            dbg_printf("Unsupported button patern: %x", button_value_pattern);
            dbg_return_carriage();
            clear_keyboard_value();
            back_to_menu =last_main_menu;
            }
//---------------------------------------------------------------------//
//-------------UNSUPPORTED BUTTONS WHILE MAIN LOOP END_--------------- //
//---------------------------------------------------------------------//
        }
//-----------------------------------------------------------//
//------------------ RADIO MENU LOOP END ------------------- //
//-----------------------------------------------------------//        
        
    }
