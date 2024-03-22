#include <i2c_HMI_STEPPER_MOTOR.h>

void STEPPER_MOTOR_send_percentage(void)
{
   //_HMI_i2cWritePrepare();
   ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   *(MasterBuffer) = i2c_slave_address;
   MasterBuffer[1] = 0x7;
   //dbg_printf("%g ", *percentage);
   //dbg_printf("%x ", *percentage);
   //memcpy (MasterBuffer+1+command_address+command_len, percentage, STP_MOTTOR_percentage_len);
   I2CWriteLength = 5; //i2cEngine write information
   I2CReadLength = 0;  //i2cEngine read information
   i2cEngine(normal__I2C);        //write or read data  depend of I2CWriteLength and I2CReadLength
}
