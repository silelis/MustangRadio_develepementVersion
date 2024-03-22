#include <i2c_HMI_BACKLIGHT.h>

void set_backlight(int8 value)
{
   dbg_printf("Ta linijka powinna byc wykasowana\r\n");
   dbg_printf("Set backlight %d",value);
   //dbg_return_carriage();
   _HMI_i2cWritePrepare();
   //*(MasterBuffer) = i2c_slave_address;
   i2c_MasterBuffer[1] = command_set_backlight;
   i2c_MasterBuffer[2] = value;
   _HMI_i2cWriteExecute(i2c_HMI_BACKLIGHT_w_command_len);

   //delay_ms(30);
   /*
   I2CWriteLength = 3; //i2cEngine write information
   I2CReadLength = 0;  //i2cEngine read information
   i2cEngine(normal__I2C);        //write or read data  depend of I2CWriteLength and I2CReadLength
   */
}
