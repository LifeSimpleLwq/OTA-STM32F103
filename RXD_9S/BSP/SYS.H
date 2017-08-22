#ifndef __SYS_H
#define __SYS_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

#define     __IO    volatile                  /*!< defines 'read / write' permissions   */

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef __IO uint32_t  vu32;

void MSR_MSP(u32 addr);	//ÉèÖÃ¶ÑÕ»µØÖ·

#endif





