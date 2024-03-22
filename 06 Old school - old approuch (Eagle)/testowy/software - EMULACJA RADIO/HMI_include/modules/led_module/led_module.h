#ifndef  _led_module_
   #define _led_module_
   
   #ifndef _i2c_Slave_buffer_addresses_
      #include </HMI_include/core/i2c_Slave_buffer_addresses/i2c_Slave_buffer_addresses.h>
   #endif

void run_led_blink_timer(void);

void choose_LED_PAD(int8);
void led_ADD(int8);
void led_DELETE(int8);
void set_x_pack_colour(int8, int8);
void set_all_pack_colour(int8);
void set_x_pack_blink(int8, int8, int8);
void set_all_pack_blink(int8, int8);
void clear_x_pack(int8);
void clear_all_pack(void);
   
#endif
