#include <i2c_HMI_LEDS.h>

/*#ifndef _i2c_HMI_main_
   #include </hardware/I2C_HMI/i2c_HMI_main.c> 
#endif*/


//void _HMI_i2cWritePrepare (void)
//{
/* Prepare Transmission buffer to transmission*/
   //ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
//   *(MasterBuffer) = i2c_slave_address;
//}

//void _HMI_i2cWriteExecute(void)
//{
///* I2Cengine write parameters setup */
//   I2CWriteLength = 5; //i2cEngine write information
//   I2CReadLength = 0;  //i2cEngine read information
//   i2cEngine(normal__I2C);        //write or read data  depend of I2CWriteLength and I2CReadLength
//}





void set_all_pack_colour(int8 colour_1)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_set_all_packs;
   //MasterBuffer[2] = 0x0;
   MasterBuffer[3] = colour_1;
   //MasterBuffer[4] = 0x0;
   _HMI_i2cWriteExecute(i2c_HMI_LEDS_w_command_len);
}

void set_x_pack_colour(int8 pack_no, int8 colour_1)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_set_x_pack;
   MasterBuffer[2] = pack_no;
   MasterBuffer[3] = colour_1;
   //MasterBuffer[4] = 0x0;
   _HMI_i2cWriteExecute(i2c_HMI_LEDS_w_command_len);
}

void set_blink_all_packs(int8 colour_1, int8 colour_2)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_blink_all_packs;
   //MasterBuffer[2] = pack_no;
   MasterBuffer[3] = colour_1;
   MasterBuffer[4] = colour_2;
   _HMI_i2cWriteExecute(i2c_HMI_LEDS_w_command_len);
}

void set_blink_x_pack(int8 pack_no, int8 colour_1, int8 colour_2)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_blink_x_pack;
   MasterBuffer[2] = pack_no;
   MasterBuffer[3] = colour_1;
   MasterBuffer[4] = colour_2;
   _HMI_i2cWriteExecute(i2c_HMI_LEDS_w_command_len);
}




void clrl_all_packs(void)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_clrl_all_packs;
   //MasterBuffer[2] = pack_no;
   //MasterBuffer[3] = colour_1;
   //MasterBuffer[4] = colour_2;
   _HMI_i2cWriteExecute(i2c_HMI_LEDS_w_command_len);
}

void clrl_x_pack(int8 pack_no)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_clrl_x_pack;
   MasterBuffer[2] = pack_no;
   //MasterBuffer[3] = colour_1;
   //MasterBuffer[4] = colour_2;
   _HMI_i2cWriteExecute(i2c_HMI_LEDS_w_command_len);
}



