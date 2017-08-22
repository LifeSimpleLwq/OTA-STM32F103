#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#include "main.h"
#include "stm32f1xx_hal.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//用户根据自己的需要设置
#define STM32_FLASH_SIZE 64
//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 1
//使能FLASH写入(0，不是能;1，使能)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000
//STM32 FLASH的起始地址
//FLASH解锁键值

extern uint32_t BSP_FLASHWrite_Addr;

uint32_t BSP_FLASHGetWriteAddr(uint8_t Len);
void BSP_FLASHRead(uint32_t ReadAddr,uint32_t *pBuffer,uint16_t NumToRead);
void BSP_FLASHWrite(uint32_t WriteAddr,uint32_t *pBuffer,uint16_t NumToWrite);
	
#endif

