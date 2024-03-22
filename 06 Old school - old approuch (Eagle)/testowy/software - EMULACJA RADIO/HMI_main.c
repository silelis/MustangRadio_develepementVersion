#include </HMI_MCU_common_includes/HMI_i2c_slave_address/i2c_slave_address.h>
#ifndef __CONFIG_FILE__
   #include<HMI_main_SLAVE_CONFIG_FILE.h>
#endif

//hardware I2C SLAVE pins - can be changed
   #define  i2c_SDA_pin       PIN_C4
   #define  i2c_SCL_pin       PIN_C3
//hardware I2C SLAVE pins - can be changed

//hardware LEDS definition - can be changed
   #define LED1_GREEN   PIN_A0
   #define LED1_RED     PIN_A1   
   #define LED1_BLUE    PIN_A2

   #define LED2_GREEN   PIN_A3
   #define LED2_RED     PIN_A5
   #define LED2_BLUE    PIN_E0
   
   #define LED3_GREEN   PIN_E1
   #define LED3_RED     PIN_E2
   #define LED3_BLUE    PIN_C0
//hardware LEDS definition - can be changed

//Active antena selsctor - can be changed
   #define  IS_FM_ANT_ACTIVE             PIN_B0
   #define  IS_DAB_ANT_ACTIVE            PIN_B1
   #define  IS_FM_DAB_CONVERTER_ACTIVE   PIN_B2
//Active antena selsctor - can be changed

//Steper motor pinout - can be changed
   #define  A1  PIN_C5
   #define  A2  PIN_C6
   #define  A3  PIN_C7
   #define  A4  PIN_D4
   #define  use_EN_pins
   #define  EN_12 PIN_D2
   #define  EN_34 EN_12
   
   #define one_fase_control    //chose one of one_fase_control  OR two_fase_control OR halfstep_control
   // #define two_fase_control      //chose one of one_fase_control  OR two_fase_control OR halfstep_control
   //#define halfstep_control    //chose one of one_fase_control  OR two_fase_control OR halfstep_control
   
   #define steper_motor_left_end                PIN_D5
   #define steper_motor_right_end               PIN_D6 
   #define steper_motor_sensor_enable_pin       PIN_D7   
   
//Steper motor pinout - can be changed
#ifndef __RADIO_DEFAULTS_PARAMETERS__
   #include </HMI_MCU_common_includes/MCU_radio_default_parameters/radio_default_parameters.h>
#endif

#include <HMI_main.h>


#ifndef __Actice_antena_select__
   #include </HMI_include/HW_drivers/Active_antena_selector/active_antena_selector.c>
#endif

#ifndef _I2C_ENGINE_SLAVE_
   #include </HMI_include/core/i2c_slave/i2c_slave.c>
#endif

#ifndef _led_module_
   #include </HMI_include/modules/led_module/led_module.c>
#endif

#ifndef  _stepper_motor_module_
   #include </HMI_include/modules/stepper_motor_module/module_stepper_motor.c>
#endif

#ifndef __silelis_math__
      #include </HMI_MCU_common_includes/SileliS_math/silelis_math.c>
#endif


extern unsigned int8   Master_2_Slave_Buffer[I2C_SLAVE_BUFSIZE];
extern unsigned int8   Slave_2_Master_Buffer[I2C_SLAVE_BUFSIZE];
extern unsigned int    i2c_buffer_counter;




int1 i2c_data_to_PROCEED;
#INT_SSP
void  SSP_isr(void) 
{  
   i2c_slave_interrupt ();
   i2c_data_to_PROCEED=TRUE;
}

/*#INT_TIMER1
void timer()
{

dbg_printf("tim");
clear_interrupt(int_timer1);
}*/


#define stepper_motor_delay_time_beetwen_steps 2 //ms

void main()
{  

   #if Backlight_MAX_VAL!=1
      pwm_set_duty_percent(0);
   #endif
   
   
/*  {
   int8 tris_value;
   tris_value = get_tris_A();
   tris_value ^= 1 << 0;
   tris_value ^= 1 << 1;
   tris_value ^= 1 << 2;
   tris_value ^= 1 << 5;
   set_tris_A(tris_value);
   tris_value = get_tris_B();
   tris_value ^= 1 << 1;
   set_tris_B(tris_value);
   tris_value = get_tris_C();
   tris_value ^= 1 << 0;
   tris_value ^= 1 << 1;
   tris_value ^= 1 << 2;
   tris_value ^= 1 << 5;
   tris_value ^= 1 << 6;
   tris_value ^= 1 << 7;
   set_tris_C(tris_value);
   tris_value = get_tris_D();
   tris_value ^= 1 << 2;
   tris_value ^= 1 << 4;
   tris_value ^= 1 << 7;
   set_tris_D(tris_value);
   tris_value = get_tris_E();
   tris_value ^= 1 << 0;
   tris_value ^= 1 << 1;
   tris_value ^= 1 << 2;
   set_tris_E(tris_value);
   }*/
   
   int8 slave_buffer_colour_1;
   int8 slave_buffer_colour_2; 
  
   int1 blink_STATEMENT = FALSE;
   int1 blikn_1_pack_STATEMENT = FALSE;
   int1 blikn_2_pack_STATEMENT = FALSE;
   int1 blikn_3_pack_STATEMENT = FALSE;
   
   int8 pack_1_blink_colour_1;
   int8 pack_1_blink_colour_2;
   int8 pack_2_blink_colour_1;
   int8 pack_2_blink_colour_2;
   int8 pack_3_blink_colour_1;
   int8 pack_3_blink_colour_2;
   
   int8 pack_colour;
   int8 pack_number;
   
   #if Backlight_MAX_VAL!=1
      int1 backlight_max_blink_STATEMENT=FALSE;    //jeœli wartoœæ backlight osi¹gnie max to za³¹cza mrugniêcie
      int8 backlight_max_STATEMENT_blink_count=0;  //iloœæ wykonanych mrugniêæ, jeœli zosta³a osi¹gniêta maksymalna wartoœæ backlight
      #define backlight_blink_blink_timeout     4  //ile czasu ma trwaæ zmiana natê¿enie backlight przy max wartoœci
      #define backlight_blink_count_max_value    5  // musi byæ nieparzyste
   #endif
   
   int1  recalculate_percentage_to_steps_STATEMENT = FALSE;   
   int1  stepper_motor_need_to_drive_STATEMENT=FALSE;
   float stepper_motor_percents_goto =0;        //zmienna przechowuje informacjê otrzyman¹ z mcu na ile % powinna ustawiæ wskazowke w [%  W FORMACIE XXX.YYY]
   unsigned int32  stepper_motor_steps_goto = 0;   //procenty usz¹ byæ przeliczone na kroki, gdy¿ ka¿dy silnik krokowy mo¿e byæ inny

   extern unsigned int32  stepper_motor_position; //zmienna przechowuje informacje na temat aktualnego ustawienia wskazowki w [ krokach od lewej], wartoœæ 0 jest nadawana gdy wskazowka osiagnie podczas kalibracji lewy zderzak
   extern unsigned int32  stepper_motor_position_max;    //po kalibracji zmienna przechowuje informacje na temat maksymalnej ilosci krokow na wskazowce w krokach od lewej], wartosc nadawana jest gdy wskazowka osi¹gnie prawy zderzak podczas kalibracji
   extern int8  steper_motor_calibration_state;   

   stepper_motor_phase = 1;
   stepper_motor_position = 0;
   stepper_motor_position_max = 0;
   steper_motor_calibration_state = stepper_motor_calibrated_NONE;
   
   dbg_printf(slave_fw_version __DATE__ " " __TIME__);
// zerowanie wartoœci silnika krokowego

   delay_ms(250);
   //module_stepper_motor_enable_EN();
   int1 steper_motor_sensor_power_STATEMENT;
   steper_motor_sensor_power_STATEMENT=steper_motor_sensor_enable();
   for (int i=1;i<=50;i++)
   {
      module_steper_motor_step_right();
      delay_ms(2);
   
   }
   module_steper_motor_stop();
   steper_motor_sensor_disable() ;
   steper_motor_sensor_power_STATEMENT=steper_motor_sensor_disable();
   delay_ms(500);

   int8 backlight_current_value;


   //dbg_return_carriage();
   set_i2c_interrupt_ready();
   
   enable_interrupts(INT_SSP);
   enable_interrupts(GLOBAL);
   
   set_all_pack_colour(colour_WHITE);
   
   i2c_data_to_PROCEED=FALSE;
   dbg_printf("Start HMI");
   //dbg_return_carriage();
   while(TRUE)
   {
   if (SSP1STAT_STOP_SEEN==1 && i2c_buffer_counter==1)
      {
      set_i2c_interrupt_ready();    //i2c_engine() test condition only - NO DATA WAS SENT
      
      //i2c_data_to_PROCEED=TRUE;
      
      
      }
   else if (SSP1STAT_STOP_SEEN==1&& i2c_buffer_counter>1)
      {
      memcpy (Master_2_Slave_Buffer, Master_2_Slave_Buffer+1,I2C_SLAVE_BUFSIZE-1); //first char is address
      dbg_printf("c:0x%X ", Master_2_Slave_Buffer[0]);
      switch(Master_2_Slave_Buffer[command_address])
         {
         case command_clrl_all_packs:
            //reset blinking conditions for pack//
            blink_STATEMENT = blikn_1_pack_STATEMENT = blikn_2_pack_STATEMENT = blikn_3_pack_STATEMENT = FALSE;
            //reset blinking conditions for pack//
            clear_all_pack();
            dbg_printf("clear all packs");
            //dbg_return_carriage();
            break;
         case command_clrl_x_pack:
            memcpy (&pack_number, Master_2_Slave_Buffer+pack_no_addres,pack_no_len);
            if (blink_STATEMENT == TRUE)
            {
               if (pack_number==pack_1)
               {
                  blikn_1_pack_STATEMENT = FALSE;
               }
               else if (pack_number==pack_2)
               {
                  blikn_2_pack_STATEMENT = FALSE;
               }
               else if (pack_number==pack_3)
               {
                  blikn_3_pack_STATEMENT = FALSE;
               }
               
               if ((blikn_1_pack_STATEMENT == FALSE) && (blikn_2_pack_STATEMENT == FALSE) &&  (blikn_3_pack_STATEMENT == FALSE))
               {
                  blink_STATEMENT= FALSE;
               }
            }
             //reset blinking conditions for pack//            
            clear_x_pack(pack_number);
            dbg_printf("clear %u pack.", pack_number);
            //dbg_return_carriage();
            break;
         case command_set_all_packs:
            memcpy (&slave_buffer_colour_1, Master_2_Slave_Buffer+colour_1_addres,colour_no_len);
            pack_colour=slave_buffer_colour_1;
            //reset blinking conditions for pack//
            blink_STATEMENT = blikn_1_pack_STATEMENT = blikn_2_pack_STATEMENT = blikn_3_pack_STATEMENT = FALSE;
            //reset blinking conditions for pack//
            set_all_pack_colour(pack_colour);
            dbg_printf("Set all packs colour %u", pack_colour);
            //dbg_return_carriage();
            break;
         case command_set_x_pack:
            memcpy (&pack_number, Master_2_Slave_Buffer+pack_no_addres,pack_no_len);
            memcpy (&slave_buffer_colour_1, Master_2_Slave_Buffer+colour_1_addres,colour_no_len);
            pack_colour = slave_buffer_colour_1;
            if (blink_STATEMENT == TRUE)
               {
               switch (pack_number){
                  case pack_1:
                     blikn_1_pack_STATEMENT = FALSE;
                     break;
                  case pack_2:
                     blikn_2_pack_STATEMENT = FALSE;
                     break;
                  case pack_3:
                     blikn_3_pack_STATEMENT = FALSE;
                     break;
               }
               if ((blikn_1_pack_STATEMENT == FALSE) && (blikn_2_pack_STATEMENT == FALSE) &&  (blikn_3_pack_STATEMENT == FALSE))
                  blink_STATEMENT= FALSE;
               }
             //reset blinking conditions for pack//
            set_x_pack_colour(pack_number, pack_colour);
            dbg_printf("Set pack %u colour %u", pack_number, pack_colour);
            //dbg_return_carriage();
            break;
         case command_blink_all_packs:
            //if (blink_STATEMENT == FALSE)
               {
               //LED_PACK_blink_timer = led_blink_timeout-1;
               set_ticks(LED_blink_timer,(led_blink_time_4_timer-1));
               blink_display_color_no1=FALSE;
               }
            memcpy (&slave_buffer_colour_1, Master_2_Slave_Buffer+colour_1_addres,colour_no_len);
            memcpy (&slave_buffer_colour_2, Master_2_Slave_Buffer+colour_2_addres,colour_no_len);    
            pack_1_blink_colour_1 = pack_2_blink_colour_1 = pack_3_blink_colour_1 = slave_buffer_colour_1;
            pack_1_blink_colour_2 = pack_2_blink_colour_2 = pack_3_blink_colour_2 = slave_buffer_colour_2;
            blink_STATEMENT = blikn_1_pack_STATEMENT = blikn_2_pack_STATEMENT = blikn_3_pack_STATEMENT = TRUE;
            dbg_printf("Blink all packs colour: %u <-> %u", pack_1_blink_colour_1, pack_1_blink_colour_2);
            //dbg_return_carriage();
            break;
         case command_blink_x_pack:
            //if (blink_STATEMENT == FALSE)
               {
               //LED_PACK_blink_timer = led_blink_timeout-1;
               set_ticks(LED_blink_timer,(led_blink_time_4_timer-1));
               blink_display_color_no1=FALSE;
               }
            blink_STATEMENT = TRUE;
            memcpy (&pack_number, Master_2_Slave_Buffer+pack_no_addres,pack_no_len);
            memcpy (&slave_buffer_colour_1, Master_2_Slave_Buffer+colour_1_addres,colour_no_len);
            memcpy (&slave_buffer_colour_2, Master_2_Slave_Buffer+colour_2_addres,colour_no_len);             
            
            switch (pack_number){
            case pack_1:
               pack_1_blink_colour_1 = slave_buffer_colour_1;
               pack_1_blink_colour_2 = slave_buffer_colour_2;            
               blikn_1_pack_STATEMENT = TRUE;
               break;
               
            case pack_2:
               pack_2_blink_colour_1 = slave_buffer_colour_1;
               pack_2_blink_colour_2 = slave_buffer_colour_2;
               blikn_2_pack_STATEMENT = TRUE;
               break;               
               
            case pack_3:
               pack_3_blink_colour_1 = slave_buffer_colour_1;
               pack_3_blink_colour_2 = slave_buffer_colour_2;            
               blikn_3_pack_STATEMENT = TRUE;              
               break;
            }
            dbg_printf("Blink %u pack colour: %u <-> %u", pack_number, pack_1_blink_colour_1, pack_1_blink_colour_2);
            //dbg_return_carriage();
            break;
         case command_set_backlight:
            memcpy(&backlight_current_value,Master_2_Slave_Buffer+SBL_Val_address,SBL_Val_address_len);
            #if Backlight_MAX_VAL!=1
               pwm_set_duty_percent(backlight_current_value*100);
      /*           #ifndef __RADIO_DEFAULTS_PARAMETERS__
                     #include </HMI_MCU_common_includes/MCU_radio_default_parameters/radio_default_parameters.h>
                  #endif*/
                  if ((backlight_current_value == Backlight_MAX_VAL)&&(backlight_max_STATEMENT_blink_count==0))
                  {
                     backlight_max_blink_STATEMENT=True;
                     set_ticks(Backlight_blink_timer,(backlight_blink_blink_timeout-1));
                  }
                  else if ((backlight_current_value != Backlight_MAX_VAL) && (backlight_max_blink_STATEMENT==True))
                  {
                     backlight_max_blink_STATEMENT=False;
                     backlight_max_STATEMENT_blink_count=0;
                  }
                dbg_printf("Set backlight %u0%%", backlight_current_value);
               //dbg_return_carriage();
                  
             #else
               output_bit( BACKLIGHT_pin, backlight_current_value);
               dbg_printf("Set backlight %u", backlight_current_value);
               //dbg_return_carriage();
             #endif
            break;
         case command_stepper_motor_percen: 
            memcpy (&stepper_motor_percents_goto, Master_2_Slave_Buffer+STP_MOTTOR_percentage_addres,STP_MOTTOR_percentage_len);
            //stepper_morot_goto__PERCENT_STATEMENT = TRUE;
            recalculate_percentage_to_steps_STATEMENT=TRUE;
            dbg_printf("Motor goto %.3f%%", stepper_motor_percents_goto);
            //dbg_return_carriage();
            break;
         case command_stepper_motor_abs_val:
            {
            int32 STP_MOTTOR_min_range_temp_value;// =*& Master_2_Slave_Buffer+STP_MOTTOR_min_range_address;
            int32 STP_MOTTOR_max_range_temp_value; // =*& Master_2_Slave_Buffer+STP_MOTTOR_max_range_address;
            int32 STP_MOTTOR_goto_temp_value;// =*& Master_2_Slave_Buffer+STP_MOTTOR_goto_address;
            memcpy (&STP_MOTTOR_min_range_temp_value, Master_2_Slave_Buffer+STP_MOTTOR_min_range_address,STP_MOTTOR_min_range_len);
            memcpy (&STP_MOTTOR_max_range_temp_value, Master_2_Slave_Buffer+STP_MOTTOR_max_range_address ,STP_MOTTOR_max_range_len);
            memcpy (&STP_MOTTOR_goto_temp_value, Master_2_Slave_Buffer+STP_MOTTOR_goto_address, STP_MOTTOR_goto_len);
            stepper_motor_percents_goto =(float)((double)(STP_MOTTOR_goto_temp_value-STP_MOTTOR_min_range_temp_value)/(STP_MOTTOR_max_range_temp_value-STP_MOTTOR_min_range_temp_value))*100;
            }
            recalculate_percentage_to_steps_STATEMENT=TRUE;
            dbg_printf("Motor goto %.3f%%", stepper_motor_percents_goto);
            //dbg_return_carriage();        
            break;
         //case command_stepper_motor_rel_val:
         //   //todo
         //   //i2c_buffer_counter = set_i2c_interrupt_ready();        //przejœcie motora trwa d³u¿ej niæ komunikacja i mo¿e w ka¿dej chwili siê zmieniæ zadany parametr
         //   //stepper_morot_goto_STATEMENT = TRUE;
         //recalculate_percentage_to_steps=TRUE;
         //   break;
         case command_stepper_motor_calibrate:
            recalculate_percentage_to_steps_STATEMENT=FALSE;
            steper_motor_calibration_state = stepper_motor_calibrated_NONE;
            dbg_printf("CALIBRATE");
            //dbg_return_carriage();
            break;
         case i2c_slave_address:      
            //dbg_return_carriage();
            dbg_printf("HMI was bussy and do not took i2c data!!! ");
            //dbg_return_carriage();
            dbg_printf("Change mcu i2c engine 'HMI_software_delay_ms' parameter");
            //dbg_return_carriage();
            break;
         default:
            dbg_printf("Not known");
            //dbg_return_carriage();
            break;
         }
         set_i2c_interrupt_ready();
         
         i2c_data_to_PROCEED=FALSE;
      }

/*-----------------------------------------------------------*/
      /* LEDS BLINKING handling */
/*-----------------------------------------------------------*/
      if ((blink_STATEMENT == TRUE))//&&(SSP1STAT==0))  // lub ((blink_STATEMENT == TRUE)&&(i2c_buffer_counter==0))  //((blink_STATEMENT == TRUE)&&(SSP1STAT==0))
         {
         run_led_blink_timer();
         if (blikn_1_pack_STATEMENT == TRUE)
            {
            set_x_pack_blink(pack_1, pack_1_blink_colour_1, pack_1_blink_colour_2);
            }
         if (blikn_2_pack_STATEMENT == TRUE)
            {
            set_x_pack_blink(pack_2, pack_2_blink_colour_1, pack_2_blink_colour_2);
            }            
         if (blikn_3_pack_STATEMENT == TRUE)
            {
            set_x_pack_blink(pack_3, pack_3_blink_colour_1, pack_3_blink_colour_2);    
            } 
         blink_time_2_switch_color=false;
         }
/*-----------------------------------------------------------*/
      /* LEDS BLINKING handling */
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
      /* Backlight max value blink */
/*-----------------------------------------------------------*/
   #if Backlight_MAX_VAL!=1
   if ((backlight_max_blink_STATEMENT==TRUE)&&(get_ticks(Backlight_blink_timer)>3))
   {
      set_ticks(Backlight_blink_timer,0);
      if ((backlight_max_STATEMENT_blink_count%2)==0)
         {
            pwm_set_duty_percent(backlight_current_value*100);
            dbg_printf("Backlight max 100");
            //dbg_return_carriage();
         }
         else
         {
            pwm_set_duty_percent((backlight_current_value*100)/4);
            dbg_printf("Backlight max 25");
            //dbg_return_carriage();
         }
      backlight_max_STATEMENT_blink_count=backlight_max_STATEMENT_blink_count+1;
      
      if (backlight_max_STATEMENT_blink_count==backlight_blink_count_max_value)
      {
         backlight_max_blink_STATEMENT=FALSE;
         backlight_max_STATEMENT_blink_count=0;
         dbg_printf("Backlight max exit");
         //dbg_return_carriage();
      }
   }
   #endif
/*-----------------------------------------------------------*/
      /* Backlight max value blink */
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/     
      /* STEPPER MOTOR HANDLING */
/*-----------------------------------------------------------*/    
      

      {


         //przeliczenie wartoœci % na goto
        if ((recalculate_percentage_to_steps_STATEMENT == TRUE) && (steper_motor_calibration_state == stepper_motor_calibrated)&&(SSP1STAT_STOP_SEEN==0))
         {
            i2c_slave_not_ready();
            //disable_interrupts(GLOBAL);
            recalculate_percentage_to_steps_STATEMENT=FALSE;
            stepper_motor_need_to_drive_STATEMENT = TRUE;
            stepper_motor_steps_goto = round_closest(stepper_motor_position_max * stepper_motor_percents_goto, 100);
            dbg_printf("New goto: %Lu",stepper_motor_steps_goto);
            //dbg_return_carriage();
           i2c_slave_ready();
         }
      
      
        //przeliczenie wartoœci % na goto
        //obs³uga silnika krokowego - kalibracja i wskazówki 
         if  (get_ticks(Steper_motor_timer)>(stepper_motor_delay_time_beetwen_steps*1.2))
         {    
           if ((steper_motor_calibration_state!=stepper_motor_calibrated)||(stepper_motor_need_to_drive_STATEMENT==TRUE)&&(SSP1STAT_STOP_SEEN==0))
           {
           
           i2c_slave_not_ready();
           
           if (steper_motor_sensor_power_STATEMENT==FALSE)
           {
               steper_motor_sensor_power_STATEMENT=steper_motor_sensor_enable();
           }

           if (steper_motor_calibration_state!=stepper_motor_calibrated)
           {
              switch (steper_motor_calibration_state)
              {
                  case stepper_motor_calibrated_NONE:
                        module_steper_motor_step_left();
                        //if (input(steper_motor_left_end)==1)
                        if (input(steper_motor_left_end)==steper_motor_LIMIT_SWITCH_reached)
                        {
                           steper_motor_calibration_state =stepper_motor_calibrated_left;
                           stepper_motor_position=0;
                        }
                        break;
                   case stepper_motor_calibrated_left:
                        module_steper_motor_step_right();
                        //if (input(steper_motor_right_end)==1)
                        if (input(steper_motor_right_end)==steper_motor_LIMIT_SWITCH_reached)
                        {
                           steper_motor_calibration_state =stepper_motor_calibrated_right;
                           stepper_motor_position_max=stepper_motor_position;
                        }
                        break;
                  case stepper_motor_calibrated_right:
                        module_steper_motor_stop();
                        steper_motor_calibration_state =stepper_motor_calibrated;
                        //module_stepper_motor_disable_EN();
                        steper_motor_sensor_power_STATEMENT=steper_motor_sensor_disable();
                        dbg_printf("Calibration finished with 0 <--> %Lu steps",stepper_motor_position_max);
                        //dbg_return_carriage();
              }
           }
           // kalibracja - KONIEC
           
           //obs³uga wskazówki
           else if (stepper_motor_need_to_drive_STATEMENT==TRUE)
           {
               if (stepper_motor_steps_goto==stepper_motor_position)
               {
                  stepper_motor_need_to_drive_STATEMENT=FALSE;
                  module_steper_motor_stop();
                  steper_motor_sensor_power_STATEMENT=steper_motor_sensor_disable();
                  dbg_printf("Carriage stopped at %Lu step.",stepper_motor_steps_goto);
                  //dbg_return_carriage();
               }
               else if(stepper_motor_steps_goto<stepper_motor_position)
                  module_steper_motor_step_left();
               else if (stepper_motor_steps_goto>stepper_motor_position)
                  module_steper_motor_step_right();
           }
           //obs³uga wskazówki 
           //dbg_printf("%Lu", stepper_motor_position);
           //dbg_return_carriage();
           i2c_slave_ready();
           }
         }

      }
      
/*-----------------------------------------------------------*/
      /* STEPPER MOTOR HANDLING */
/*-----------------------------------------------------------*/ 
   
if ((i2c_data_to_PROCEED==FALSE)&&(stepper_motor_need_to_drive_STATEMENT==FALSE)&&(steper_motor_calibration_state ==stepper_motor_calibrated)&&(blink_STATEMENT == FALSE))
{
   dbg_printf("GtS.\r\n");
   //set_timer1(0);
   //enable_interrupts(INT_TIMER1);
   sleep();
}
   
/*   #if Backlight_MAX_VAL!=1
      if ((stepper_motor_need_to_drive_STATEMENT==FALSE)&&(steper_motor_calibration_state ==stepper_motor_calibrated)&&(backlight_max_blink_STATEMENT==FALSE)&&(blink_STATEMENT == FALSE)&&(SSP1STAT == 0))
   #else
      if ((stepper_motor_need_to_drive_STATEMENT==FALSE)&&(steper_motor_calibration_state ==stepper_motor_calibrated)&&(blink_STATEMENT == FALSE)&&(SSP1STAT == 0))
   #endif
   {
      dbg_printf("Going to sleep mode");
      //dbg_return_carriage();
      sleep();
   }*/
   
   
   }
}
