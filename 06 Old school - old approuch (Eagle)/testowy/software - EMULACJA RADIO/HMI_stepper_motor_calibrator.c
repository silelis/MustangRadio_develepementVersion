#include </HMI_MCU_common_includes/HMI_i2c_slave_address/i2c_slave_address.h>
#ifndef __CONFIG_FILE__
   #include<HMI_main_SLAVE_CONFIG_FILE.h>
#endif

//Steper motor pinout - can be changed
   #define  A1  PIN_C5
   #define  A2  PIN_C6
   #define  A3  PIN_C7
   #define  A4  PIN_D4
   #define  use_EN_pins
   #define  EN_12 PIN_D2
   #define  EN_34 EN_12
   
   #define one_fase_control    //chose one of one_fase_control  OR two_fase_control OR halfstep_control
   
  //#define two_fase_control      //chose one of one_fase_control  OR two_fase_control OR halfstep_control
   //#define halfstep_control    //chose one of one_fase_control  OR two_fase_control OR halfstep_control
   
   #define steper_motor_left_end                PIN_D5
   #define steper_motor_right_end               PIN_D6 
   #define steper_motor_sensor_enable_pin       PIN_D7
   
   //#define  steper_motor_sensor_enable()        output_bit(steper_motor_sensor_enable_pin,1)
   //#define  steper_motor_sensor_disable()       output_bit(steper_motor_sensor_enable_pin,0)
   
//Steper motor pinout - can be changed

#include <HMI_stepper_motor_calibrator.h>

#ifndef  _stepper_motor_module_
   #include </HMI_include/modules/stepper_motor_module/module_stepper_motor.c>
#endif

#ifndef __silelis_math__
      #include </HMI_MCU_common_includes/SileliS_math/silelis_math.c>
#endif

void main()
{  
   #define stepper_motor_delay_time_beetwen_steps_init_speed 1
   unsigned int stepper_motor_delay_time_beetwen_steps = stepper_motor_delay_time_beetwen_steps_init_speed; //5;  //MO¯E I 6
   unsigned int correct_calibration_counter = 0;
   //#ifndef _STEPPER_MOTOR_ON_INTERRUPTS_
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
   //#endif
  
   dbg_printf(slave_fw_version __DATE__ " " __TIME__);
// zerowanie wartoœci silnika krokowego
   
   delay_ms(250);
   //module_stepper_motor_enable_EN();
   steper_motor_sensor_enable();
   for (int i=1;i<=150;i++)
   {
      module_steper_motor_step_right();
      delay_ms(2);
   
   }
   module_steper_motor_stop();
   steper_motor_sensor_disable() ;
   //module_stepper_motor_disable_EN();
   delay_ms(500);

   //dbg_return_carriage();
   enable_interrupts(GLOBAL);
   
   
   //steper_motor_sensor_enable();
   
   while(TRUE)
   {
/*-----------------------------------------------------------*/     
      /* STEPPER MOTOR HANDLING */
/*-----------------------------------------------------------*/    
      
      {
        if ((recalculate_percentage_to_steps_STATEMENT == TRUE) && (steper_motor_calibration_state == stepper_motor_calibrated))
         {
         
            //steper_motor_sensor_enable();
            //delay_ms(1);
            recalculate_percentage_to_steps_STATEMENT=FALSE;
            stepper_motor_need_to_drive_STATEMENT = TRUE;
            stepper_motor_steps_goto = round_closest(stepper_motor_position_max * stepper_motor_percents_goto, 100);
            dbg_printf("New goto: %Lu",stepper_motor_steps_goto);
            //dbg_return_carriage();
         }
      
        //przeliczenie wartoœci % na goto
        //obs³uga silnika krokowego - kalibracja i wskazówki 
         if  (get_ticks(Steper_motor_timer)>(stepper_motor_delay_time_beetwen_steps*1.2))
         {    
           if ((steper_motor_calibration_state!=stepper_motor_calibrated)||(stepper_motor_need_to_drive_STATEMENT==TRUE))
           {
          
           // kalibracja - POCZ¥TEK
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
                           //stepper_motor_position_max = 10;
                           //stepper_motor_position = 0;
                           if (stepper_motor_position_max==stepper_motor_position)
                           {
                              //stepper_motor_delay_time_beetwen_steps = stepper_motor_delay_time_beetwen_steps;
                              correct_calibration_counter = correct_calibration_counter +1;
                           }
                           else
                           {
                              //stepper_motor_delay_time_beetwen_steps = stepper_motor_delay_time_beetwen_steps+1;
                              correct_calibration_counter = 0;
                              dbg_printf("Calibration NOT CORRECT last max %Lu current max %Lu speed %u",stepper_motor_position_max, stepper_motor_position, stepper_motor_delay_time_beetwen_steps);
                              //dbg_return_carriage();
                           }
                           stepper_motor_position_max=stepper_motor_position;
                        }
                        break;
                  case stepper_motor_calibrated_right:
                        module_steper_motor_stop();
                        #define max_calibration_try   10
                        if (correct_calibration_counter == max_calibration_try)
                        {
                           steper_motor_sensor_disable();
                           steper_motor_calibration_state =stepper_motor_calibrated;
                           dbg_printf("Calibration finished with 0 <--> %Lu, with speed %u ms",stepper_motor_position_max, stepper_motor_delay_time_beetwen_steps);
                           //dbg_return_carriage();
                        }
                        else if (correct_calibration_counter ==0)
                        {
                           steper_motor_calibration_state =stepper_motor_calibrated_NONE;

                           //if ((correct_calibration_counter ==0) && (stepper_motor_delay_time_beetwen_steps==stepper_motor_delay_time_beetwen_steps_init_speed))
                           //{
                              dbg_printf("Calibration NOT finished, last speed %u, new speed %u", stepper_motor_delay_time_beetwen_steps, (stepper_motor_delay_time_beetwen_steps));
                              stepper_motor_delay_time_beetwen_steps = stepper_motor_delay_time_beetwen_steps+1;
                           //}
                           //else
                           //{
                              dbg_printf("Calibration NOT finished, last speed %u, new speed %u", stepper_motor_delay_time_beetwen_steps, (stepper_motor_delay_time_beetwen_steps+1));
                              //stepper_motor_delay_time_beetwen_steps = stepper_motor_delay_time_beetwen_steps+1;    
                           /*}*/
                           //dbg_return_carriage();
                        }
                        else if ((correct_calibration_counter >=1)&&(correct_calibration_counter !=max_calibration_try))
                        {
                           steper_motor_calibration_state =stepper_motor_calibrated_NONE;
                           dbg_printf("Seem to be ok, Last max pos 0<-->%Lu, speed %u remained trys %u", stepper_motor_position_max, stepper_motor_delay_time_beetwen_steps, (max_calibration_try-correct_calibration_counter));
                           //dbg_return_carriage();
                        
                        }
              }
           }
           }
         }

      }
   }
}
