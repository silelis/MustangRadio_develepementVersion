#ifndef __l293d__
   #define __l293d__
   #ifndef A1
      #define A1  PIN_C5
   #endif
   #ifndef A2
      #define A2  PIN_C6
   #endif
   #ifndef A3
      #define A3  PIN_C7
   #endif
   #ifndef A4
      #define A4  PIN_D4
   #endif
   #define L293_OUTPUT_lag_time       1     //800 ns co daje 1 us
   #ifdef use_EN_pins
      #define  L293_EN_lag_time_ON    1     //800 ns co daje 1 us
      #define  L293_EN_lag_time_OFF   1     //800 ns co daje 1 us
      #ifndef  EN_12
         #define  EN_12 PIN_D2       // define in Your program
      #endif
      #ifndef  EN_34
         #define  EN_34 PIN_Y       // define in Your program
      #endif
   #endif
   
   #define  ON    1
   #define  OFF   0
   #define  LEFT  0
   #define  RIGHT 1
   #define  STOP  2
   
   
   #define  stop_phase  0
   #define  first_phase 1
   
   #ifdef   one_fase_control
      //#define  last_phase 5
      //const int8 stepper_motor_pase [last_phase][4] = //Wave or one-fase control
      #define  max_phase 4
      const int8 stepper_motor_pase [max_phase+1][4] = //Wave or one-fase control
      {
         //A1, A2, A3, A4
         0,0,0,0,    //stop condition
         1,0,0,0,
         0,0,1,0,
         0,1,0,0,
         0,0,0,1
      };
   //#endif
   
   #ELIF defined   two_fase_control
      //#define  last_phase  5
      //const int8 stepper_motor_pase [last_phase][4] = //Fullstep or two-fase control
      #define  max_phase  4
      const int8 stepper_motor_pase [max_phase+1][4] = //Fullstep or two-fase control      
      
      {
         //A1, A2, A3, A4
         0,0,0,0,    //stop condition
         1,0,1,0,
         0,1,1,0,
         0,1,0,1,
         1,0,0,1
      };
   //#endif
   
   #ELIF defined   halfstep_control
      //#define last_phase 9
      //const int8 stepper_motor_pase [last_phase][4] = //Halfstep or two-faze + two-faze control
      
      #define max_phase 8
      const int8 stepper_motor_pase [max_phase+1][4] = //Halfstep or two-faze + two-faze control      
      
      {
         //A1, A2, A3, A4
         0,0,0,0,    //stop condition
         1,0,0,0,
         1,0,1,0,
         0,0,1,0,
         0,1,1,0,
         0,1,0,0,
         0,1,0,1,
         0,0,0,1,
         1,0,0,1
      };
   #ELSE
      #ERROR "PLEASE DEFINE STEPPER MOTOR CONTROL METHOD: one_fase_control or two_fase_control or halfstep_control"
   #endif


void  enable_EN_12(void);
void  enable_EN_34(void);
void  disable_EN_12(void);
void  disable_EN_34(void);

int8 STEPPER_motor(int8, int1);
void DC_motor_Bidirectional(int8, int1);
void DC_motor_Unidirectional(int1, int16);


#endif
