#ifndef __BSP_MAIN_H
#define __BSP_MAIN_H

#include "BSP_OTA.h"
#include "BSP_24l01.h"

#include "BSP_flash.h"
#include "BSP_SYS.H"
#include "BSP_Malloc.h"

#include "usart.h"
extern 		u8 Rx[20];

struct OTA_Run
{
	u8 Run_Statu ;	// ff 接收固件  aa 接收ID  bb升级固件  00 闲置中
	u8 RxBuf;				// 固件接收缓冲
	u8 ID_Num;			// 
	u8 ID_Buf[50];
};

void BSP_Main(void);
void GET_IDHandle(void);

#endif



