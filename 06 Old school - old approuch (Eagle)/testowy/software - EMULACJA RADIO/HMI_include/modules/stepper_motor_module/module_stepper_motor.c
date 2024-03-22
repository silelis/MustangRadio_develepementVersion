#include <module_stepper_motor.h>

/*void module_stepper_motor_disable_EN(void)
{
   disable_EN_12();     //same pin as EN_34
   //__reset_L293D_conditions__();
   dbg_printf("L293: disabled");
   //dbg_return_carriage();
   //L293_driver_state = L293_state_EN_disabled;
}*/

/*void module_stepper_motor_enable_EN(void)
{
   enable_EN_12();      //same pin as EN_34
   //__reset_L293D_conditions__();
   dbg_printf("L293: enabled");
   //dbg_return_carriage();
   //L293_driver_state = L293_state_EN_enabled;
}*/


void module_steper_motor_step_left(void)
{
   //if (input(steper_motor_left_end)==0)
   
   if (input(steper_motor_left_end)==steper_motor_LIMIT_SWITCH_NOT_reached)
   {
      disable_EN_12();
      stepper_motor_phase = STEPPER_motor(stepper_motor_phase, left);
      enable_EN_12();
//      #ifndef _STEPPER_MOTOR_ON_INTERRUPTS_
         __reset_L293D_conditions__();
//      #endif
      stepper_motor_position = stepper_motor_position-1;
   }
   else
   {
      dbg_printf("STOP:left end reached");
      //dbg_return_carriage();
      module_steper_motor_stop();
   }
}

void module_steper_motor_step_right(void)
{
   //if (input(steper_motor_right_end)==0)
   if (input(steper_motor_right_end)==steper_motor_LIMIT_SWITCH_NOT_reached)
   
   {
      disable_EN_12();
      stepper_motor_phase = STEPPER_motor(stepper_motor_phase, right);
      enable_EN_12();
//      #ifndef _STEPPER_MOTOR_ON_INTERRUPTS_
         __reset_L293D_conditions__();
//      #endif
      stepper_motor_position = stepper_motor_position+1;
   }
   else
   {
      dbg_printf("STOP:right end reached");
      //dbg_return_carriage();
      module_steper_motor_stop();
   }
   
}

void module_steper_motor_stop(void)
{
   disable_EN_12();
   stepper_motor_phase = STEPPER_motor(stepper_motor_phase, stop);
   enable_EN_12();
   delay_ms(1);
//   #ifndef _STEPPER_MOTOR_ON_INTERRUPTS_
      __reset_L293D_conditions__();
//   #endif
   disable_EN_12();
}

//#ifndef _STEPPER_MOTOR_ON_INTERRUPTS_
   void __reset_L293D_conditions__(void)
   {
      set_ticks(Steper_motor_timer,0);
      //motor_command_executed = true;
   }
//#endif

int1 steper_motor_sensor_enable(void)
{
   output_bit(steper_motor_sensor_enable_pin,1);
   delay_ms(1);
   return TRUE;
}


int1 steper_motor_sensor_disable(void)
{
   output_bit(steper_motor_sensor_enable_pin,0);
   delay_ms(1);
   return FALSE;
}

