#include <i2c_HMI_STEPPER_MOTOR.h>

void STEPPER_MOTOR_send_percentage(float percents)
{
   //float a = percents;
   //_HMI_i2cWritePrepare();
   //ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   i2c_ClearMasterBuffer();
   
   *(i2c_MasterBuffer) = i2c_slave_address;
   i2c_MasterBuffer[1] = command_stepper_motor_percen;
   memcpy (i2c_MasterBuffer+1+command_address+command_len, &percents, STP_MOTTOR_percentage_len);
   I2CWriteLength = 6; //i2cEngine write information
   I2CReadLength = 0;  //i2cEngine read information
   i2cEngine(normal__I2C);        //write or read data  depend of I2CWriteLength and I2CReadLength
   delay_ms(2 * HMI_software_delay_ms);         //additional wait to execute command, this command takes time
}

void STEPPER_MOTOR_send_absolut(int32 min_range, int32 max_range, int32 goto_value)
{
   //ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   i2c_ClearMasterBuffer();
   *(i2c_MasterBuffer) = i2c_slave_address;
   i2c_MasterBuffer[1] = command_stepper_motor_abs_val;
   memcpy (i2c_MasterBuffer+1+command_address+command_len, &min_range, STP_MOTTOR_min_range_len);
   memcpy (i2c_MasterBuffer+1+command_address+command_len+STP_MOTTOR_min_range_len, &max_range, STP_MOTTOR_max_range_len);
   memcpy (i2c_MasterBuffer+1+command_address+command_len+STP_MOTTOR_min_range_len+STP_MOTTOR_max_range_len, &goto_value, STP_MOTTOR_goto_len);   
   I2CWriteLength = 15; //i2cEngine write information
   I2CReadLength = 0;  //i2cEngine read information
   i2cEngine(normal__I2C);        //write or read data  depend of I2CWriteLength and I2CReadLength
   //delay_ms(2 * HMI_software_delay_ms); //additional wait to execute command, this command takes time
}
