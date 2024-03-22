#include <i2c_HMI_main.h>

#define i2c_HMI_LEDS_w_command_len           5
#define i2c_HMI_BACKLIGHT_w_command_len      3
#define i2c_HMI_STEPPER_MOTOR_w_command_len  0

void _HMI_i2cWritePrepare (void)
{
/* Prepare Transmission buffer to transmission*/
   //ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   *(MasterBuffer) = i2c_slave_address;
}

void _HMI_i2cWriteExecute(int8 _I2CWriteLength)
{
/* I2Cengine write parameters setup */
   I2CWriteLength = _I2CWriteLength; //i2cEngine write information
   I2CReadLength = 0;  //i2cEngine read information
   i2cEngine(normal__I2C);        //write or read data  depend of I2CWriteLength and I2CReadLength
}


/*void _HMI_i2cReadPrepare (void)
{
   *(MasterBuffer) = i2c_slave_address+1;
}

void _HMI_i2cReadExecute(void)
{
// I2Cengine write parameters setup
   I2CWriteLength = 1; //i2cEngine write information
   I2CReadLength = 2;  //i2cEngine read information
   i2cEngine(normal__I2C);        //write or read data  depend of I2CWriteLength and I2CReadLength
}*/

