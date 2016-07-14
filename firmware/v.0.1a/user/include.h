

#ifndef INCLUDE_H_
#define INCLUDE_H_

//typedef enum {FALSE = 0, TRUE = !FALSE} bool;

/* DATA TYPE ------------------------------------------------------------------*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity         */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity         */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity         */
typedef signed   short INT16S;                   /* Signed   16 bit quantity         */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity         */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity         */
typedef float          FP32;                     /* Single precision floating point  */
typedef double         FP64;                     /* Double precision floating point  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "usb_type.h"
#include "Timer3.h"


/*#define RCC_APB2Periph_ALLGPIO      (RCC_APB2Periph_GPIOA \
                                     | RCC_APB2Periph_GPIOB \
                                     | RCC_APB2Periph_GPIOC \
                                     | RCC_APB2Periph_GPIOD \
					 | RCC_APB2Periph_GPIOE )*/

#endif
