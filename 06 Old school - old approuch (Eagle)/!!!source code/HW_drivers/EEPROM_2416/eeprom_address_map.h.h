#ifndef __EEPROM_ADDRESS_MAP
   #define __EEPROM_ADDRESS_MAP
#ednif
//0x00
#define RadioFirstRun                     0x00                                             // if it is first radio run that takes default vars values
#define sizeof_RadioFirstRun              1

//0x01
#define TDA7418VolumeArray                RadioFirstRun+sizeof_RadioFirstRun
#define sizeof_TDA7418VolumeArray         1

//0x02
#define TDA7418TrebleArray                TDA7418VolumeArray+sizeof_TDA7418VolumeArray
#define sizeof_TDA7418TrebleArray         1

//0x03
#define TDA7418MiddleArray                TDA7418TrebleArray+sizeof_TDA7418TrebleArray
#define sizeof_TDA7418MiddleArray         1

//0x03
#define TDA7418BasseArray                 TDA7418MiddleArray+sizeof_TDA7418MiddleArray
#define sizeof_TDA7418BasseArray          1

//0X04
#define TDA7418LeftRightArray             TDA7418BasseArray +sizeof_TDA7418BasseArray
#define sizeof_TDA7418LeftRightArray       1

//0x05
#define TDA7418FrontRearArray             TDA7418LeftRightArray+sizeof_TDA7418LeftRightArray
#define sizeof_TDA7418FrontRearArray      1

//0x06
#define TDA7418LowHighBoost               TDA7418FrontRearArray +sizeof_TDA7418FrontRearArray
#define sizeof_TDA7418LowHighBoost        1
