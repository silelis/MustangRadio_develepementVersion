#define _I2C_SCL  PIN_C3
#define _I2C_SDA  PIN_C4
//#include <STDLIB.H>

#include <main.h>

//#include </HW_drivers/tea5767.h>
//#include </core/I2C/I2C_Engine.c>

//#ifdef ACTIVE_ANTENA 
   #include </HW_drivers/AAntena/ActiveAntena.c>
//#endif

//volatile unsigned int8   MasterBuffer[I2C_BUFSIZE];
//volatile unsigned int8   SlaveBuffer[I2C_BUFSIZE];


//float FMfreqency;


//#ifndef _I2C_ENGINE_
   #include </core/I2C_ENGINE/I2C_Engine.c>
//#endif

//extern unsigned int   MasterBuffer[mcu_Transmission_BUFSIZE];
//extern unsigned int   SlaveBuffer[mcu_Reception_BUFSIZE];

#include </HW_drivers/TEA5767/TEA5767.c>

#include </HW_drivers/au7860_mp3/au7860_mp3.c>

//volatile unsigned int8  tea5767_config[5]={0,0,0,0,0};

//extern volatile unsigned int8  tea5767_config[5];
//extern volatile int1 TEA5767_last_i2c_action_WRITE;

#include </HW_drivers/TDA7418/TDA7418.c>
//#include </core/PIC_EEPROM/pic_eeprom.c>

/*void _TDA7418_i2cWritePrepare (void)
{
ClearBuffers(&MasterBuffer,I2C_BUFSIZE);
*(MasterBuffer) = TDA7418_ADDRESS_Write;
}

void _TDA7418_i2cWriteExecute(void)
{
   I2CWriteLength = 3; //i2cEngine write information
   I2CReadLength = 0;  //i2cEngine read information
   i2cEngine();
}*/

//unsigned int buffer =0;

/*
#INT_RDA
void INT_RDA_handler()
{

dbg_printf("%c", fgetc(Hardware__comm_port));
}*/

void main()
{

//StartupVoltageStabilisation;
//unsigned int32 FMfreqency;



//MasterBuffer[0]=0x10;

//delay_ms(2000);
//dbg_printf("START");
//disable_interrupts(GLOBAL);
enable_interrupts(INT_RDA);
enable_interrupts(GLOBAL);




/*   int8  i2c_ACK=1;
   unsigned int i2c_tryout_error = 0;
   while(i2c_ACK!=0&&i2c_tryout_error <100)
   {i2c_ACK = __i2cDeviceStatus();
   i2c_tryout_error = i2c_tryout_error +1;
   }
   */
   
//i2cEngine(FALSE);

//dbg_printf("222");
//delay_ms(1000);
//  AAntenna_ON();
//delay_ms(1000);
/*   delay_ms(1000);
   output_high(pin_b4);
   delay_ms(1000);
   fprintf(LOG_PORT,"tata");
   fprintf(LOG_PORT,"mama");*/

   TDA7418__SelectInput(TDA7418_INPUT_SE1, TDA7418_INPUT_GAIN_0dB, TDA7418_DIFFIN_MODE_DIFFERENTIAL);
   TDA7418__Loudness (AUDIO_Loudness_attenuation[0], TDA7418_SHAPE_LOW_HIGH_BOOST);  
   TDA7418__Volume(AUDIO_volume_attenuation[35]);
   TDA7418__Treble(AUDIO_Treble_Middle_Bass[16]);
   TDA7418__Middle(AUDIO_Treble_Middle_Bass[16]);
   TDA7418__Bass(AUDIO_Treble_Middle_Bass[16]);
   TDA7418__Middle_Bass_Fc_Select();
   TDA7418__LeftFront(AUDIO_Treble_Middle_Bass[16]);
   TDA7418__LeftRear(AUDIO_Treble_Middle_Bass[16]);
   TDA7418__RightRear(AUDIO_Treble_Middle_Bass[16]);
   TDA7418__RightFront(AUDIO_Treble_Middle_Bass[16]);
   TDA7418__Subwoofer(AUDIO_Treble_Middle_Bass[0]);     //SW mute
   TDA7418_Soft_Mut_Step_Time();
   TDA7418_Audio_Processor_Operating_Mode();

tea5767_init(100200000);   //(100200000);
delay_ms(5000);
tea5767_StandBy(1);
tea5767SendData (&tea5767_config);

//TDA7418__SelectInput(TDA7418_INPUT_PD_SE4, TDA7418_INPUT_GAIN_0dB, TDA7418_DIFFIN_MODE_DIFFERENTIAL);
/*delay_ms(1000);
fprintf(LOG_PORT,"dekay");
delay_ms(1000);
output_low(pin_b4);
delay_ms(1000);
fprintf(LOG_PORT,"low");*/


while(1)
{
//fprintf(LOG_PORT,"low");
}
//dbg_printf("1111 ");

//*MasterBuffer=au_header;


}


