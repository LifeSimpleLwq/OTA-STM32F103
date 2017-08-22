#ifndef __BSP_SYS_H__
#define __BSP_SYS_H__

#include "BSP_24l01.h"
#include "sys.h"

#define SoleAddrEP 0x1B
// 0x1B ´ú±í Ö²ÎïµÆ 
extern uint8_t NRF_ID[5];

void BSP_SYSInit(void);
void SetNRFID(void);

#endif


