#ifndef  _stepper_motor_module_
   #define  _stepper_motor_module_
   
   #ifndef __l293d__
      #include </HMI_include/HW_drivers/L293/L293D.c>
   #endif
   
   #ifndef steper_motor_left_end
      #define steper_motor_left_end                PIN_D6
   #endif
   #ifndef  steper_motor_right_end
      #define steper_motor_right_end               PIN_D5
   #endif
  
   #ifndef steper_motor_sensor_enable_pin
      #define steper_motor_sensor_enable_pin       PIN_D7
   #endif
   
   #define stepper_motor_calibrated_NONE        0b00000000
   #define stepper_motor_calibrated_left        0b00000001
   #define stepper_motor_calibrated_right       0b00000010
   #define stepper_motor_calibrated             0b00000011
   
//   #define L293_state_EN_disabled               0b00000000
//   #define L293_state_EN_enabled                0b00000001

   #define steper_motor_LIMIT_SWITCH_reached       0    //for switch 1 for opto 0
   #define steper_motor_LIMIT_SWITCH_NOT_reached   1    //for switch 0 for opto 1
   //int8 L293_driver_state;
   int8 steper_motor_calibration_state;
   int8 stepper_motor_phase;
   unsigned int32 stepper_motor_position;
   unsigned int32 stepper_motor_position_max;
   
   //void module_stepper_motor_disable_EN(void);
   //oid module_stepper_motor_enable_EN(void);
   
   void module_steper_motor_step_left(void);
   void module_steper_motor_step_right(void);
   void module_steper_motor_stop(void);
   int1 steper_motor_sensor_enable(void);
   int1 steper_motor_sensor_disable(void);
   #ifndef _STEPPER_MOTOR_ON_INTERRUPTS_
      void __reset_L293D_conditions__(void);
   #endif
   
   #ifdef _STEPPER_MOTOR_ON_INTERRUPTS_
      #if getenv("SFR_VALID:INTCON")
         #byte INTCON_register = getenv("SFR:INTCON")
         #bit  timer0_interrupts_STATEMENT=INTCON_register.5
         #WARNING "STEPPER MOTOR on TIMER0 interrups"
      
      
      #else
         #ERROR "PIC_INTCON REGISTER SHOULD BE DEFINED in module_stepper_motor.h"
      #endif
   #endif      
   
#endif
