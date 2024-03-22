#ifndef __timeout__
   #define __timeout__


   #define timeout_time_in_seconds 10
   
   unsigned int timer_overflow_counter;
   int1 timeout_occured;
   
   void init_timeout(void);                              //should be executed at the begining
   #define run_timer() enable_interrupts(INT_TIMER1)     //starts timer - enabling the interrupts   
   void clear_timer(void);                               //stops timer - disabling the interrupts clear variables
   int1 set_timer(unsigned int);                         //set timer for X seconds - return 1 if time passed
   int1 check_timeout(void);                             //check if timeout occured (timeout_occured == 1) if YES return "1" and set timeout_occured to 0
 
#endif



