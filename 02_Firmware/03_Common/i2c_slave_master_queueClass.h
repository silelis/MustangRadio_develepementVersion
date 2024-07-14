#pragma once

#include "hwConfig.h"

#ifdef  TOOLCHAIN_ENVIRONMENT == __stm32__


#elif TOOLCHAIN_ENVIRONMENT == __esp32__


#else
#error "TOOLCHAIN_ENVIRONMENT which is unknown!!!!"
#endif
