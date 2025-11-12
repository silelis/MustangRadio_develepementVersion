#ifndef INC_SILELIS_CODE_KEYBOARDTOFUNCTION_BUTTONDEFINITIONS_H_
#define INC_SILELIS_CODE_KEYBOARDTOFUNCTION_BUTTONDEFINITIONS_H_

#define EXECUTALBE_BUTTONS_ARRAY_SIZE 18

#define POWER_ON_OFF_SHORT_PRESS					{'b', 0x7e}
#define POWER_ON_OFF_LONG_PRESS						{'B', 0xfe}
#define POWER_ON_OFF_LONG_RELEASED					{'b', 0xfe}
#define VOL_INCREMENT								{'v', 0x04}
#define VOL_DECREMENT								{'v', 0xfc}

#define EQUALIZER_SHORT_PRESS						{'b', 0x3f}
#define EQUALIZER_LONG_PRESS						{'B', 0xbf}
#define EQUALIZER_LONG_RELEASED						{'b', 0xbf}		//aka. RETURN_FROM_EQUALIZER
#define EQUALIZER_INCREMENT							{'e', 0x04)
#define EQUALIZER_DECREMENT							('e', 0xfc}
#define RETURN_FROM_EQUALIZER						EQUALIZER_LONG_RELEASED

#define BUT1_SHORT_PRESS							{'b', 0x7d}
#define BUT1_LONG_PRESS								{'B', 0xfd}
#define BUT1_LONG_RELEASED							{'b', 0xfd}

#define BUT2_SHORT_PRESS							{'b', 0x7b}
#define BUT2_LONG_PRESS								{'B', 0xfb}
#define BUT2_LONG_RELEASED							{'b', 0xfb}

#define BUT3_SHORT_PRESS							{'b', 0x77}
#define BUT3_LONG_PRESS								{'B', 0xf7}
#define BUT3_LONG_RELEASED							{'b', 0xf7}

#define BUT4_SHORT_PRESS							{'b', 0x6f}
#define BUT4_LONG_PRESS								{'B', 0xef}
#define BUT4_LONG_RELEASED							{'b', 0xef}

#define BUT5_SHORT_PRESS							{'b', 0x5f}
#define BUT5_LONG_PRESS								{'B', 0xdf}
#define BUT5_LONG_RELEASED							{'b', 0xdf}

//#define BUT1_BUT3_SHORT_PRESS						{'b', 0b01110101}
//#define BUT1_BUT3_LONG_PRESS						{'B', 0xf5}
#define BUT1_BUT3_LONG_RELEASED						{'b', 0xf5}		//aka. TUNE_LEFT_TIP_BOARDER
#define TUNE_LEFT_TIP_BOARDER						BUT1_BUT3_LONG_RELEASED

//#define BUT3_BUT5_SHORT_PRESS						{'b', 0b01010111}
//#define BUT3_BUT5_LONG_PRESS						{'B', 0xd7}
#define BUT3_BUT5_LONG_RELEASED						{'b', 0xd7}		//aka. TUNE_RIGHT_TIP_BOARDER
#define TUNE_RIGHT_TIP_BOARDER						BUT3_BUT5_LONG_RELEASED

//#define BUT2_BUT3_BUT4_EQUALIZER_SHORT_PRESS			{'b', 0b0010 0011}
//#define BUT2_BUT3_BUT4_EQUALIZER_LONG_PRESS			{'B', 0xa3}
#define BUT2_BUT3_BUT4_EQUALIZER_LONG_RELEASED		{'b', 0xa3}		//aka. RESET_TO_DEFAULT
#define RESET_TO_DEFAULT							BUT2_BUT3_BUT4_EQUALIZER_LONG_RELEASED

#endif /* INC_SILELIS_CODE_KEYBOARDTOFUNCTION_BUTTONDEFINITIONS_H_ */
