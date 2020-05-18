#ifndef __MEW_FLASH__ 
#define __MEW_FLASH__

#include "mew.h"

#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/crc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned int mew_otp_storage_init(void);

#endif
