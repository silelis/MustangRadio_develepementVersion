#ifndef __keyboard__
   #define __keyboard__
   
   #define keyboar                     input_a()
   #define button3_button4             (input_e()&0b00000011)
   //#define button4   PIN_E1
   
   #define encoder1_right_pattern      0b00000001
   #define encoder1_left_pattern       0b00000010
   #define encoder2_right_pattern      0b00000100
   #define encoder2_left_pattern       0b00001000
   #define button1_pattern             0b00010000
   #define button2_pattern             0b00100000
   #define button3_pattern             0b01000000
   #define button4_pattern             0b10000000
   #define button1_long_pattern                             //in case of Volume roto encoder breakdown (encoder1_right_pattern breakdown)
   #define button2_long_pattern                             //in case of Volume roto encoder breakdown (encoder1_left_pattern breakdown) 
   #define button3_long_pattern                             //in case of Equaliser roto encoder breakdown (encoder2_right_pattern breakdown)
   #define button4_long_pattern                             //in case of Equaliser roto encoder breakdown (encoder2_left_pattern breakdown) 
   
   #define debounce_time               2                   //ms
   #define deboundec_input             keyboard_interrupt      //PIN_B0 //Keyboard interrupt pin
   void debounce_inputs(int16);
   
   void init_keyboard(void);
   void read_keyboard_value(void);
   void clear_keyboard_value(void);
   
   
   void set_button_loop_breakout(void);
   void reset_button_loop_breakout(void);

#endif
