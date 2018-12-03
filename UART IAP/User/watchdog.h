#ifndef __WATCHDOG_H__
#define	__WATCHDOG_H__

#include "main.h"

void IWDG_Init(uint8 prer,uint32 rlr);
void IWDG_Feed(void);

#endif

