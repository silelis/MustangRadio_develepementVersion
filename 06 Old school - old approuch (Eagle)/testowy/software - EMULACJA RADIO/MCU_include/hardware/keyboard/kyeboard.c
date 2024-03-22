#ifndef __keyboard__
//#define __keyboard__
#include<kyeboard.h>


//  /*keyboard interrupt handling*/
//  #ifndef __keyboard_interrupt__
//     #define __keyboard_interrupt__
//      #INT_EXT
//      void  EXT_isr(void)
//      {
//          disable_interrupts(INT_TIMER1);
//          read_keyboard_value();
//          enable_interrupts(INT_TIMER1);
//      }
//   #endif
//  /*keyboard interrupt handling*/



unsigned int8 button_value_pattern;
int1          button_loop_breakout;

void set_button_loop_breakout(void)
    {
    button_loop_breakout = 1;
    }

void reset_button_loop_breakout(void)
    {
    button_loop_breakout = 0;
    }

void init_keyboard(void)
    {
    button_value_pattern =0;
    ext_int_edge(1, L_TO_H);
    enable_interrupts(INT_EXT);
    //reset_button_loop_breakout();
    }

void debounce_inputs(int16 time2debounce)
    {
    //unsigned int16 COUNT_TARGET = (unsigned int16) (time2debounce_us/100)+1;
    unsigned int16 count_low_signal_A=0;
    unsigned int16 count_high_signal_A=0;
    do
        {
        delay_us(100);
        if (input(deboundec_input) == 0)
            {
            count_low_signal_A++;
            count_high_signal_A = 0;
            }
        else
            {
            count_low_signal_A = 0;
            count_high_signal_A++;
            }
        }
    while(!((count_low_signal_A > time2debounce) || (count_high_signal_A > time2debounce)));
    }



void read_keyboard_value(void)
    {
    disable_interrupts(INT_EXT);
    #ifdef _USE_debounce_function_
    if (input(button1)==1 || input(button2)==1 || input(button3)==1 || input(button4)==1)    //sprawdzanie tylko dla przycisków
    {
      debounce_inputs(debounce_time);
    }
    #endif
    button_value_pattern = button3_button4<<6|keyboar;
    }


void clear_keyboard_value(void)
    {
    if (button_value_pattern)
        {
        button_value_pattern =0;
        enable_interrupts(INT_EXT);
        clear_interrupt(INT_EXT_L2H);
        }
    }
//#endif
