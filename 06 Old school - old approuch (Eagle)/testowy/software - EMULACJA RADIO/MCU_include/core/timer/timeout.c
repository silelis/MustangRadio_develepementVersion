//#ifndef __timeout__
//#define __timeout__
#include<timeout.h>

//   /*Timer interrupt function*/
//   #ifndef __timer_inetrrupt__
//      #define __timer_inetrrupt__
//      #INT_TIMER1
//      void  TIMER1_isr(void)
//      {
//         disable_interrupts(INT_EXT);
//         timer_overflow_counter ++;
//         timeout_occured = set_timer(timeout_time_in_seconds);
//         enable_interrupts(INT_EXT);
//      }
//   #endif
//   /*Timer interrupt function*/


void init_timeout(void)
    {
    timer_overflow_counter = 0;
    timeout_occured = 0;
    setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);      //104 ms overflow
    }
#define overflow_time 104

void clear_timer(void)
    {
    disable_interrupts(INT_TIMER1);
    timer_overflow_counter = 0;
    //timeout_occured = 0;
    }

int1 set_timer(unsigned int time_seconds)
    {

    if (timer_overflow_counter > ((unsigned int) (time_seconds*1000/overflow_time)))
        {
        timeout_occured = 1;
        return 1;
        }
    else
        {
        return 0;
        }

    }

int1 check_timeout(void)
    {
    if (timeout_occured == 1)
        {
        return 1;
        }
    else
        {
        return 0;
        }
    }
//#endif


