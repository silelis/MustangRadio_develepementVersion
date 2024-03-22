#include <l293d.h>

int8 STEPPER_motor(int8 present_phase, int8 LEFT_RIGHT_STOP)
{   
   int8 output_order[4];  
   switch (LEFT_RIGHT_STOP)
   {
      case LEFT:
         present_phase = present_phase -1;
         if (present_phase<first_phase)
         {
            present_phase = max_phase;
         }
         {
            for(int8 i=0; i<4;i++)
            {
               output_order[i]=stepper_motor_pase[present_phase][i];
            }
         }
         
         break;
      case RIGHT:
         present_phase = present_phase +1;
         if (present_phase>max_phase)
         {
            present_phase = first_phase;
         }
         {
            for(int8 i=0; i<4;i++)
            {
               output_order[i]=stepper_motor_pase[present_phase][i];
            }           
         }
         break;
      case STOP:     //nie dzia³a za dobrze na symulatorze
         {
            output_order[0]=0;
            output_order[1]=0;
            output_order[2]=0;
            output_order[3]=0; 
         }
   }
   output_bit (A1, output_order[0]);
   output_bit (A2, output_order[1]);
   output_bit (A3, output_order[2]);
   output_bit (A4, output_order[3]);
   delay_us(L293_OUTPUT_lag_time);
   return present_phase;
}


void DC_motor_Bidirectional(int8 LEFT_RIGHT_STOP, int1 morot_number)
{
   int16 motor_pin_1;
   int16 motor_pin_2;
   if (morot_number ==0)
   {
      motor_pin_1 =A1;
      motor_pin_2 =A2;
   }
   else
   {
      motor_pin_1 =A3;
      motor_pin_2 =A4;
   
   }
   switch(LEFT_RIGHT_STOP)
   {
      case LEFT:
         output_high(motor_pin_1);
         output_low(motor_pin_2);
         break;
      case RIGHT:
         output_low(motor_pin_1);
         output_high(motor_pin_2);      
         break;
      case STOP:
         output_low(motor_pin_1);
         output_low(motor_pin_2);    
   }
}

void DC_motor_Unidirectional(int1 ON_OFF, int16 motor_pin)
{
if (ON_OFF==1)
   output_high(motor_pin);
else
   output_low(motor_pin);
}

void  enable_EN_12(void)
{
   #ifdef use_EN_pins
      //delay_ms(L293_EN_lag_time_OFF);
      delay_us(L293_EN_lag_time_OFF);
      output_high(EN_12);
      //delay_us(L293_EN_lag_time_ON);
      delay_us(L293_EN_lag_time_ON);
   #else
      ;
   #endif
}   

void  enable_EN_34(void)
{
   #ifdef use_EN_pins
      //delay_ms(L293_EN_lag_time_OFF);
      delay_us(L293_EN_lag_time_OFF);
      output_high(EN_34);
      //delay_ms(L293_EN_lag_time_ON);
      delay_us(L293_EN_lag_time_ON);   
   #else
      ;
   #endif
}   
   
void  disable_EN_12(void)
{
   #ifdef use_EN_pins
      //delay_ms(L293_EN_lag_time_OFF);
      delay_us(L293_EN_lag_time_OFF);
      output_low(EN_12);
      //delay_ms(L293_EN_lag_time_OFF);
      delay_us(L293_EN_lag_time_OFF);
   #else
      ;
   #endif
}      
   
void  disable_EN_34(void)
{
   #ifdef use_EN_pins
     // delay_ms(L293_EN_lag_time_OFF);
      delay_us(L293_EN_lag_time_OFF);
      output_low(EN_12);
      //delay_ms(L293_EN_lag_time_OFF);
      delay_us(L293_EN_lag_time_OFF);
      
   #else
      ;
   #endif
}      

