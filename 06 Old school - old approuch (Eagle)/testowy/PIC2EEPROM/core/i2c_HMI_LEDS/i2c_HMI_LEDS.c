#include <i2c_HMI_LEDS.h>



void _HMI_i2cWritePrepare (void)
{
/* Prepare Transmission buffer to transmission*/
   //ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   *(MasterBuffer) = i2c_slave_address;
}

void _HMI_i2cWriteExecute(void)
{
/* I2Cengine write parameters setup */
   I2CWriteLength = 5; //i2cEngine write information
   I2CReadLength = 0;  //i2cEngine read information
   i2cEngine(FALSE);        //write or read data  depend of I2CWriteLength and I2CReadLength
}

void set_all_pack_colour(int8 colour_1)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_set_all_packs;
   //MasterBuffer[2] = 0x0;
   MasterBuffer[3] = colour_1;
   //MasterBuffer[4] = 0x0;
   _HMI_i2cWriteExecute();
}

void set_x_pack_colour(int8 pack_no, int8 colour_1)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_set_x_pack;
   MasterBuffer[2] = pack_no;
   MasterBuffer[3] = colour_1;
   //MasterBuffer[4] = 0x0;
   _HMI_i2cWriteExecute();
}

void set_blink_all_packs(int8 colour_1, int8 colour_2)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_blink_all_packs;
   //MasterBuffer[2] = pack_no;
   MasterBuffer[3] = colour_1;
   MasterBuffer[4] = colour_2;
   _HMI_i2cWriteExecute();
}

void set_blink_x_pack(int8 pack_no, int8 colour_1, int8 colour_2)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_blink_x_pack;
   MasterBuffer[2] = pack_no;
   MasterBuffer[3] = colour_1;
   MasterBuffer[4] = colour_2;
   _HMI_i2cWriteExecute();
}




void clrl_all_packs(void)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_clrl_all_packs;
   //MasterBuffer[2] = pack_no;
   //MasterBuffer[3] = colour_1;
   //MasterBuffer[4] = colour_2;
   _HMI_i2cWriteExecute();
}

void clrl_x_pack(int8 pack_no)
{
   _HMI_i2cWritePrepare();
   MasterBuffer[1] = command_clrl_x_pack;
   MasterBuffer[2] = pack_no;
   //MasterBuffer[3] = colour_1;
   //MasterBuffer[4] = colour_2;
   _HMI_i2cWriteExecute();
}


/*void set_all_leds(unsigned int8 on_off)
{  
   ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   ClearBuffers(&SlaveBuffer,I2C_BUFSIZE);
   MasterBuffer[0]=i2c_slave_address;
   MasterBuffer[1]=command_set_all_leds;
   MasterBuffer[2]=on_off;
   I2CWriteLength = 3;
   I2CReadLength = 0;
   i2cEngine(FALSE);
}

void set_leds_pack(unsigned int8 x_pack, unsigned int8 on_off)
{
   ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
   ClearBuffers(&SlaveBuffer,I2C_BUFSIZE);
   MasterBuffer[0]=i2c_slave_address;
   MasterBuffer[1]=command_set_x_pack;
   MasterBuffer[2]=x_pack;
   MasterBuffer[3]=on_off;
   I2CWriteLength = 4;
   I2CReadLength = 0;
   i2cEngine(FALSE);
}




void set_one_led(unsigned int8 led_no, unsigned int8 on_off)
{
  ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
  ClearBuffers(&SlaveBuffer,I2C_BUFSIZE);
  MasterBuffer[0]=i2c_slave_address;
  MasterBuffer[1]=command_set_led;
  MasterBuffer[2]=led_no;
  MasterBuffer[3]=on_off;
  I2CWriteLength = 4;
  I2CReadLength = 0;
  i2cEngine(FALSE);
}*/



