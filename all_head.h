
#ifndef __all_head_H
#define __all_head_H

#include "cs32l010_hal.h"
#include <stdbool.h>

#define __R_ volatile const
#define ___W volatile
#define __RW volatile

typedef  uint16_t u16;
typedef  uint32_t u32;
typedef  uint8_t u8;

//#include "CJ_Config.h"
#include "cs32l010_it.h"
#include "main.h"

#include "SW3516H.h"
#include "SW3516H_User.h"
#include "CCC_i2c_COPY_22.h"
#include "CCC_i2c.h"

#if DEBUG_UART
#include "CJ_UART.h"
#endif

#endif
