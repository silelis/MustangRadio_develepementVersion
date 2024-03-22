#include </eeprom_module.h>

// funkcja sprawdzajaca wartosci odczytane i zwracajaca default jesli b³¹d i piszaca o b³êdzie jeœli by³ b³¹d

// funkcja sprawdzaj¹ca do zapisu ??


void check_read_value_int8(signed int8 *readen_val, signed int8  min_val, signed int8  max_val, signed int8  default_val)
{
   if ((*readen_val<min_val) || (*readen_val>max_val))
   {
   //dbg_printf("EEPROM parameter error.EEPROM is damaged? Adress %x ", address);
   dbg_printf("EEPROM is damaged?");
   dbg_return_carriage();
   *readen_val = default_val;
   delay_ms(2000);
   }
}


void check_read_value_int32(signed int32 *readen_val, signed int32 min_val, signed int32 max_val, signed int32 default_val)
{
   if ((*readen_val<min_val) || (*readen_val>max_val))
   {
   dbg_printf("EEPROM is damaged?");
   dbg_return_carriage();
   *readen_val = default_val;
   delay_ms(2000);
   }
}

void read_and_check_value_int8 (EEPROM_ADDRESS address, void *data, EEPROM_ADDRESS _size_of_data, int8 min_val, int8 max_val, int8 default_val)
{
   //int8 _internal_data;
   read_ext_eeprom(address, *data, _size_of_data);
   //dbg_printf("AAAAA:   %d     ", *data);
   //delay_ms(5000);
   check_read_value_int8(&data, min_val, max_val, default_val);//, address);
   //dbg_printf("AAAAA:   %d     ", *data);
   //delay_ms(5000);
//check_value_int8(data, min_val, max_val, default_val);
}

void read_and_check_value_int32(EEPROM_ADDRESS address, void *data, EEPROM_ADDRESS _size_of_data, int32 min_val, int32 max_val, int32 default_val)
{
   read_ext_eeprom(address, *data, _size_of_data);
   check_read_value_int32(&data, min_val, max_val, default_val);//, address);
}

void check_temp_value_int8(signed int8 *readen_val, signed int8 min_val, signed int8 max_val)
{
if (*readen_val<min_val)
   *readen_val = min_val;
else if  (*readen_val>max_val)
   *readen_val = max_val;
//else
//   *readen_val=*readen_val;
}


#define  EEPROM_MODULE_READ_radio_1st_run();       read_ext_eeprom( RADIO_1st_run_addr, &_radio_1st_run, RADIO_1st_run_len);
#define  EEPROM_MODULE_WRITE_radio_1st_run();      write_ext_eeprom(RADIO_1st_run_addr, &_radio_1st_run, RADIO_1st_run_len);

#define  EEPROM_MODULE_WRITE_audio_input();        write_ext_eeprom(RADIO_Input_addr, &audio_devices_playing_submenu, RADIO_Input_len);
#define  EEPROM_MODULE_READ_audio_input();         read_ext_eeprom( RADIO_Input_addr, &audio_devices_playing_submenu, RADIO_Input_len);
#define  EEPROM_MODULE_CHECK_audio_input();        check_read_value_int8(&audio_devices_playing_submenu, audio_input_MIN_VAL, audio_input_MAX_VAL, audio_input_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_audio_input();          read_and_check_value_int8(RADIO_Input_addr, &audio_devices_playing_submenu, RADIO_Input_len, audio_input_MIN_VAL, audio_input_MAX_VAL, audio_input_DEFAULT_VAL);
                                                   
#define  EEPROM_MODULE_WRITE_active_antena();      write_ext_eeprom(Active_Antena_addr, &use_active_antena_val, Active_Antena_len);
#define  EEPROM_MODULE_READ_active_antena();       read_ext_eeprom( Active_Antena_addr, &use_active_antena_val, Active_Antena_len);
//#define  EEPROM_MODULE_CHECK_active_antena();      check_read_value_int8(&use_active_antena_val, active_antena_MIN_VAL, active_antena_MAX_VAL, #ifdef _USE_ACTIVE_ANTENA_ active_antena_MAX_VAL #else active_antena_MIN_VAL #endif);
#define  EEPROM_MODULE_CHECK_active_antena();      check_read_value_int8(&use_active_antena_val, active_antena_MIN_VAL, active_antena_MAX_VAL, active_antena_MAX_VAL);
//#define  EEPROM_MODULE_R_C_active_antena();        read_and_check_value_int8(RADIO_Input_addr, &audio_devices_playing_submenu, RADIO_Input_len, active_antena_MIN_VAL, active_antena_MAX_VAL, #ifdef _USE_ACTIVE_ANTENA_ active_antena_MAX_VAL #else active_antena_MIN_VAL #endif);
#define  EEPROM_MODULE_R_C_active_antena();        read_and_check_value_int8(RADIO_Input_addr, &audio_devices_playing_submenu, RADIO_Input_len, active_antena_MIN_VAL, active_antena_MAX_VAL, active_antena_MAX_VAL);

#define  EEPROM_MODULE_WRITE_fm_freqency();        write_ext_eeprom(FM_freuency_addr, &FM_freuency_value, FM_freuency_len);
#define  EEPROM_MODULE_READ_fm_freqency();         read_ext_eeprom( FM_freuency_addr, &FM_freuency_value, FM_freuency_len);
#define  EEPROM_MODULE_CHECK_fm_freqency();        check_read_value_int32(&FM_freuency_value, FM_freuency_MIN_VAL, FM_freuency_MAX_VAL, FM_freuency_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_fm_freqency();          read_and_check_value_int32(FM_freuency_addr, &FM_freuency_value, FM_freuency_len, FM_freuency_MIN_VAL, FM_freuency_MAX_VAL, FM_freuency_DEFAULT_VAL);



#define  EEPROM_MODULE_WRITE_am_freqency();        write_ext_eeprom(AM_freuency_addr, &AM_freuency_value, AM_freuency_len);
#define  EEPROM_MODULE_READ_am_freqency();         read_ext_eeprom( AM_freuency_addr, &AM_freuency_value, AM_freuency_len);
#define  EEPROM_MODULE_CHECK_am_freqency();        check_read_value_int32(&AM_freuency_value, AM_freuency_MIN_VAL, AM_freuency_MAX_VAL, AM_freuency_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_am_freqency();          read_and_check_value_int32(AM_freuency_addr, &AM_freuency_value, AM_freuency_len, AM_freuency_MIN_VALL, AM_freuency_MAX_VAL, AM_freuency_DEFAULT_VAL);



#define  EEPROM_MODULE_WRITE_equ_vol();            write_ext_eeprom(EQU_Vol_addr, &EQU_VOL_value, EUQ_Vol_len);
#define  EEPROM_MODULE_READ_equ_vol();             read_ext_eeprom( EQU_Vol_addr, &EQU_VOL_value, EUQ_Vol_len);
#define  EEPROM_MODULE_CHECK_equ_vol();            check_read_value_int8(&EQU_VOL_value, EQU_VOL_MIN_VAL, EQU_VOL_MAX_VAL, EQU_VOL_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_equ_vol();              read_and_check_value_int8(EQU_Vol_addr, &EQU_VOL_value, EUQ_Vol_len, EQU_VOL_MIN_VAL, EQU_VOL_MAX_VAL, EQU_VOL_DEFAULT_VAL);
#define  EEPROM_MODULE_CHECK_TEMP_equ_vol();       check_temp_value_int8(&EQU_VOL_value, EQU_VOL_MIN_VAL, EQU_VOL_MAX_VAL);

#define  EEPROM_MODULE_WRITE_equ_treble();         write_ext_eeprom(EQU_Treble_addr, &EQU_Treble_value, EQU_Treble_len);
#define  EEPROM_MODULE_READ_equ_treble();          read_ext_eeprom( EQU_Treble_addr, &EQU_Treble_value, EQU_Treble_len);
#define  EEPROM_MODULE_CHECK_equ_treble();         check_read_value_int8(&EQU_Treble_value, EQU_Treble_MIN_VAL, EQU_Treble_MAX_VAL, EQU_Treble_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_equ_treble();           read_and_check_value_int8(EQU_Treble_addr, &EQU_Treble_value, EQU_Treble_len, EQU_Treble_MIN_VAL, EQU_Treble_MAX_VAL, EQU_Treble_DEFAULT_VAL);

#define  EEPROM_MODULE_WRITE_equ_middle();         write_ext_eeprom(EQU_Middle_addr, &EQU_Middle_value, EQU_Middle_len);
#define  EEPROM_MODULE_READ_equ_middle();          read_ext_eeprom( EQU_Middle_addr, &EQU_Middle_value, EQU_Middle_len);
#define  EEPROM_MODULE_CHECK_equ_middle();         check_read_value_int8(&EQU_Middle_value, EQU_Middle_MIN_VAL, EQU_Middle_MAX_VAL, EQU_Middle_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_equ_middle();           read_and_check_value_int8(EQU_Middle_addr, &EQU_Middle_value, EQU_Middle_len, EQU_Middle_MIN_VAL, EQU_Middle_MAX_VAL, EQU_Middle_DEFAULT_VAL);

#define  EEPROM_MODULE_WRITE_equ_bass();           write_ext_eeprom(EQU_Bass_addr, &EQU_Bass_value, EQU_Bass_len);
#define  EEPROM_MODULE_READ_equ_bass();            read_ext_eeprom( EQU_Bass_addr, &EQU_Bass_value, EQU_Bass_len);
#define  EEPROM_MODULE_CHECK_equ_bass();           check_read_value_int8(&EQU_Bass_value, EQU_Bass_MIN_VAL, EQU_Bass_MAX_VAL, EQU_Bass_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_equ_bass();             read_and_check_value_int8(EQU_Bass_addr, &EQU_Bass_value, EQU_Bass_len, EQU_Bass_MIN_VAL, EQU_Bass_MAX_VAL, EQU_Bass_DEFAULT_VAL);

#define  EEPROM_MODULE_WRITE_equ_l_h_boost();      write_ext_eeprom(EQU_L_H_Boost_addr, &EQU_L_H_Boost_value, EQU_L_H_Boost_len);
#define  EEPROM_MODULE_READ_equ_l_h_boost();       read_ext_eeprom( EQU_L_H_Boost_addr, &EQU_L_H_Boost_value, EQU_L_H_Boost_len);
#define  EEPROM_MODULE_CHECK_equ_l_h_boost();      check_read_value_int8(&EQU_L_H_Boost_value, EQU_L_H_Boost_MIN_VAL, EQU_L_H_Boost_MAX_VAL, EQU_L_H_Boost_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_equ_l_h_boost();        read_and_check_value_int8(EQU_L_H_Boost_addr, &EQU_L_H_Boost_value, EQU_L_H_Boost_len, EQU_L_H_Boost_MIN_VAL, EQU_L_H_Boost_MAX_VAL, EQU_L_H_Boost_DEFAULT_VAL);

#define  EEPROM_MODULE_WRITE_equ_left_right();     write_ext_eeprom(EQU_Left_Right_addr, &EQU_Left_Right_value, EQU_Left_Right_len);
#define  EEPROM_MODULE_READ_equ_left_right();      read_ext_eeprom( EQU_Left_Right_addr, &EQU_Left_Right_value, EQU_Left_Right_len);
#define  EEPROM_MODULE_CHECK_equ_left_right();     check_read_value_int8(&EQU_Left_Right_value, EQU_Left_Right_MIN_VAL, EQU_Left_Right_MAX_VAL, EQU_Left_Right_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_equ_left_right();       read_and_check_value_int8(EQU_Left_Right_addr, &EQU_Left_Right_value, EQU_Left_Right_len, EQU_Left_Right_MIN_VAL, EQU_Left_Right_MAX_VAL, EQU_Left_Right_DEFAULT_VAL);

#define  EEPROM_MODULE_WRITE_equ_front_rear();     write_ext_eeprom(EQU_Front_Rear_addr, &EQU_Front_Rear_value, EQU_Front_Rear_len);
#define  EEPROM_MODULE_READ_equ_front_rear();      read_ext_eeprom( EQU_Front_Rear_addr, &EQU_Front_Rear_value, EQU_Front_Rear_len);
#define  EEPROM_MODULE_CHECK_equ_front_rear();     check_read_value_int8(&EQU_Front_Rear_value, EQU_Front_Rear_MIN_VAL, EQU_Front_Rear_MAX_VAL, EQU_Front_Rear_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C_equ_equ_front_rear();   read_and_check_value_int8(EQU_Front_Rear_addr, &EQU_Front_Rear_value, EQU_Front_Rear_len, EQU_Front_Rear_MIN_VAL, EQU_Front_Rear_MAX_VAL, EQU_Front_Rear_DEFAULT_VAL);

#define  EEPROM_MODULE_WRITE_hmi_backlight();      write_ext_eeprom(Backlight_addr, &Backlight_value, Backlight_len);
#define  EEPROM_MODULE_READ_hmi_backlight();       read_ext_eeprom( Backlight_addr, &Backlight_value, Backlight_len);
#define  EEPROM_MODULE_CHECK_hmi_backlight();      check_read_value_int8(&Backlight_value, Backlight_MIN_VAL, Backlight_MAX_VAL, Backlight_DEFAULT_VAL);
#define  EEPROM_MODULE_R_C__hmi_backlight();       ead_and_check_value_int8(Backlight_addr, &Backlight_value, Backlight_len, Backlight_MIN_VAL, Backlight_MAX_VAL, Backlight_DEFAULT_VAL);


//#define  EEPROM_MODULE_READ_dab_frequency();
//#define  EEPROM_MODULE_WRITE_dab_frequency();

