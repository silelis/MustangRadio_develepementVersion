#include </HMI_include/modules/led_module/led_module.h>


int16 pin_BLUE_no;
int16 pin_GREEN_no;
int16 pin_RED_no;

//unsigned int16 LED_PACK_blink_timer;
int1 blink_display_color_no1=TRUE;
int1 blink_time_2_switch_color;

#define led_blink_timeout 12000
#define led_blink_time_4_timer   229

void run_led_blink_timer(void)
{

if (get_ticks(LED_blink_timer)>=led_blink_time_4_timer)
{
   blink_time_2_switch_color=true;
   blink_display_color_no1=!blink_display_color_no1;
   set_ticks(LED_blink_timer,0);
   }
}

void choose_LED_PAD(int8 pack_number)
{
   switch ( pack_number){
   case pack_1:
      pin_BLUE_no =  LED1_BLUE;
      pin_GREEN_no = LED1_GREEN;
      pin_RED_no =   LED1_RED;
      break;
   case pack_2:
      pin_BLUE_no =  LED2_BLUE;
      pin_GREEN_no = LED2_GREEN;
      pin_RED_no =   LED2_RED;      
      break;
   case pack_3:
      pin_BLUE_no =  LED3_BLUE;
      pin_GREEN_no = LED3_GREEN;
      pin_RED_no =   LED3_RED;      
      break;
   }
}

void led_ADD(int8 _set_colour)
{
      if ((_set_colour & colour_BLUE)!=0)
         output_high(pin_BLUE_no);
      if ((_set_colour & colour_GREEN)!=0)    
         output_high(pin_GREEN_no);
      if ((_set_colour & colour_RED)!=0)
         output_high(pin_RED_no);
}

void led_DELETE(int8 _uset_colour)
{
      if ((_uset_colour & colour_BLUE)!=0)
         output_low(pin_BLUE_no);
      if ((_uset_colour & colour_GREEN)!=0)    
         output_low(pin_GREEN_no);
      if ((_uset_colour & colour_RED)!=0)
         output_low(pin_RED_no);
}

void set_x_pack_colour(int8 pack_no, int8 colour)
{
   choose_LED_PAD(pack_no);
   if ((colour & unset_mask) != 0)
      led_DELETE(colour);
   else
      {
      led_DELETE(colour_BLACK);
      led_ADD(colour);
      }
}

void set_all_pack_colour(int8 colour)
{
   set_x_pack_colour(pack_1, colour);
   set_x_pack_colour(pack_2, colour);
   set_x_pack_colour(pack_3, colour);
}

void set_x_pack_blink(int8 pack_no, int8 colour1, int8 colour2)
{
   //choose_LED_PAD(pack_no);
   if (blink_time_2_switch_color==true)
      {
      if (blink_display_color_no1 ==TRUE)
         set_x_pack_colour(pack_no,colour1);
      else
         set_x_pack_colour(pack_no,colour2);
      }
}

void set_all_pack_blink(int8 colour1, int8 colour2)
{
set_x_pack_blink(pack_1, colour1, colour2);
set_x_pack_blink(pack_2, colour1, colour2);
set_x_pack_blink(pack_3, colour1, colour2);
}

void clear_x_pack(int8 pack_no)
{
   choose_LED_PAD(pack_no);
   led_DELETE(colour_BLACK);
}

void clear_all_pack(void)
{
   clear_x_pack(pack_1);
   clear_x_pack(pack_2);
   clear_x_pack(pack_3);
}
