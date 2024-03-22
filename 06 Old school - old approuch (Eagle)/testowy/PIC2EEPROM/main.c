#include <main.h>

#define I2C_BUFSIZE         64
//#include </core/I2C/I2C_Engine.c>

#ifndef __EEPROM_2416_
   //#define __EEPROM_2416_
   #include </core/EEPROM_2416/EEPROM_2416.c>
#endif



//#define EEPROM_WRITE_DELAY 10

extern volatile unsigned int   MasterBuffer[I2C_BUFSIZE];
extern volatile unsigned int   SlaveBuffer[I2C_BUFSIZE];

/*volatile unsigned int8   MasterBuffer[I2C_BUFSIZE];
volatile unsigned int8   SlaveBuffer[I2C_BUFSIZE];*/

#ifndef _i2c_HMI_LEDS_
   #include </core/i2c_HMI_LEDS/i2c_HMI_LEDS.c>
#endif


//#define READ_24LC16 TRUE
//#define _24LC16_WRITE_DELAY   10

void main()
{  
   //init_i2cEngine();
   //Example blinking LED program
   //dbg_printf("t");
   
   //i2cInit();
   //i2cEngine();
  // double a=-152.221;
  // double b=152.0;
  // double c = (double)a+b;
   /*dbg_printf("%3.5f ",c);
   write_ext_eeprom(0x0,&a,sizeof(a));
   read_ext_eeprom(0x0,&b,sizeof(b));
   //b=read_ext_eeprom(0x0);
   //dbg_printf("tata ");
   dbg_printf("%3.5f ",a);
   dbg_printf("%3.5f",b);*/
   
   /*if (b==a)
      {output_high(PIN_C1);}*/
    
   delay_ms(2000);
   
   dbg_printf("1st ");   
   
   set_blink_x_pack(pack_1, colour_RED, colour_WHITE);
   
   delay_ms(2500);
   set_blink_x_pack(pack_2, colour_RED, colour_PURPLE);
   set_x_pack_colour(pack_1, colour_RED);
   /*delay_ms(2000);
   clrl_x_pack(pack_1);
   delay_ms(2000);
   clrl_all_packs();*/
   
/*   set_all_leds(led_ON);
   delay_ms(2000);
   set_all_leds(led_OFF);

   set_one_led(LED1_BLUE_no, led_ON);
   set_one_led(LED1_GREEN_no, led_ON);
   set_one_led(LED1_RED_no, led_ON);
   
   set_one_led(LED2_BLUE_no, led_ON);      
   set_one_led(LED2_GREEN_no, led_ON);      
   set_one_led(LED2_RED_no, led_ON);      
   
   
   set_one_led(LED3_BLUE_no, led_ON);
   set_one_led(LED3_GREEN_no, led_ON);      
   set_one_led(LED3_RED_no, led_ON);
 


   set_one_led(LED1_BLUE_no, led_OFF);
   set_one_led(LED1_GREEN_no, led_OFF);
   set_one_led(LED1_RED_no, led_OFF);
   
   set_one_led(LED2_BLUE_no, led_OFF);      
   set_one_led(LED2_GREEN_no, led_OFF);      
   set_one_led(LED2_RED_no, led_OFF);      
   
   set_one_led(LED3_BLUE_no, led_OFF);
   set_one_led(LED3_GREEN_no, led_OFF);      
   set_one_led(LED3_RED_no, led_OFF);
 
 set_leds_pack(set_1st_pack, led_ON);
  set_leds_pack(set_2nd_pack, led_ON);
   set_leds_pack(set_3rd_pack, led_ON);
 
 delay_ms(500);
 set_leds_pack(set_1st_pack, led_OFF);
  set_leds_pack(set_2nd_pack, led_OFF);
   set_leds_pack(set_3rd_pack, led_OFF);*/
   while(true)
   {
      output_low(LED);
      delay_ms(DELAY);
      output_high(LED);
      delay_ms(DELAY);
   }

}
