#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#include "main.h"

/*
 *	FLASH驱动使用简易说明
 *	1，根据MCU更改 STM32_FLASH_SIZE 大小，若不更改可能导致写入失败
 *	2，调用 Flash_Text进行测试驱动能否读写数据。
 */
#define STM32_FLASH_SIZE 		128			// 所选STM32的FLASH容量大小(单位为K)
																	// STM32F105RB闪存只有128k，实际有256k

// ----------------------------- addr ---------------------------------
#define OTA_ADDR						FLASH_BASE + 1024 * 15			// OTA检测标志位
#define DATA_ADDR				 		FLASH_BASE + 1024 * 120			// 注意Flash大小
#define OTA_FLAG_ADDR				FLASH_BASE + 1024 * 122			// OTA检测标志位

// ----------------------------- function --------------------------------
void BSP_FLASHWrite(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite);

void BSP_FLASHRead(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t NumToRead);
uint16_t BSP_FLASHReadHalfWord(uint32_t faddr);

#endif

