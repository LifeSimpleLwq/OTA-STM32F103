#include "BSP_flash.h"

#define STM_SECTOR_SIZE 		1024 // 字节
#define Page_SIZE 	STM_SECTOR_SIZE		// FLASH一页存储空间大小
#define Page_Num		Page_SIZE/2			// 以2个字节为单位，注意，此处与写入的数据宽度有关

// ----------------------------------- Read flash -----------------------------
/**
  * @brief  读取指定地址的字
  * @param  faddr :读取的地址, 该地址必须是 4 的倍数					  
  * @retval 读取的内容
  */
uint16_t BSP_FLASHReadHalfWord(uint32_t faddr)
{
	return *(__IO uint16_t*)faddr; 
}

/**
  * @brief  从指定地址开始读出指定长度的数据
  * @param  ReadAddr: 起始地址
  * @param  pBuffer: 读出数据的缓冲区
  * @param  NumToWrite: 读出数据的大小(单位: 半字)
  * @retval None
  */
void BSP_FLASHRead(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t NumToRead)   	
{
	uint16_t i;

	for(i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = BSP_FLASHReadHalfWord(ReadAddr);		// 读取 2 个字节	
		ReadAddr += 2;																	// 偏移 2 个字节	
	}
}

// ---------------------------------- Write flash --------------------------
/**
  * @brief  不检查的写入, 单位：半字
  * @param  WriteAddr: 起始地址
  * @param  pBuffer: 写入数据的缓冲区
  * @param  NumToWrite: 写入数据的大小(单位: 字)
  * @retval None
  */
static void BSP_FLASHWrite_NoCheck(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite)   
{
	uint16_t i;

	for(i = 0; i < NumToWrite; i++)
	{
		FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
		WriteAddr += 2;														// 地址增加4.	
	}  
}

/**
  * @brief  往 Flash 写数据(一页写入1024字节)
  * @param  WriteAddr: 起始地址
  * @param  pBuffer: 写入数据的缓冲区  
  * @param  NumToWrite: 写入数据的大小(单位: 字)  
  * @retval None	  
  */
void BSP_FLASHWrite(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite)	
{	
	uint32_t secpos;		// 扇区地址
	
	uint32_t offaddr;
	
	uint16_t Numsize;	

	uint16_t secoff;		// 扇区内偏移地址(16位计算)
	
	uint16_t secremain;	// 扇区内剩余地址(16位计算)
	
 	uint16_t i;  
	
	uint16_t BSP_FLASHBUF[Page_Num];	
	

	if(WriteAddr < FLASH_BASE || (WriteAddr >= (FLASH_BASE + STM_SECTOR_SIZE * STM32_FLASH_SIZE)))
		return;		// 非法地址  
	
	Numsize = NumToWrite;  // 写入字节 
	
	// 计算写入的长度  
	offaddr = WriteAddr - FLASH_BASE;  	// 实际偏移地址. 
	
	secpos = offaddr / Page_SIZE;    					// 扇区地址 	
	
	secoff = (offaddr % Page_SIZE) / 2;   		// 在扇区内的偏移( 2个字节为基本单位. )	
	
	secremain = Page_Num - secoff;		  			// 扇区剩余空间大小
	

	if(Numsize <= secremain)	secremain = Numsize;  // 不大于该扇区范围
	
	FLASH_Unlock();		// 解锁
	
	while(1) 
	{
		// 读出整个扇区的内容
		BSP_FLASHRead (secpos * Page_SIZE + FLASH_BASE, BSP_FLASHBUF, Page_Num);				

		for (i = 0; i < secremain; i++)		// 校验数据	
		{
			if (BSP_FLASHBUF[secoff + i] != 0xFFFF)
				break;		// 需要擦除			
		}

		if (i < secremain)		// 需要擦除			
		{
			FLASH_ErasePage(secpos * Page_SIZE + FLASH_BASE); 	// 擦除这个扇区
			printf("Erase\r\n");
			
			for (i = 0; i < secremain; i++)
			{
				BSP_FLASHBUF[i + secoff] = pBuffer[i]; 
			}
			
			BSP_FLASHWrite_NoCheck(secpos * Page_SIZE + FLASH_BASE, BSP_FLASHBUF, Page_Num);  // 写入整个扇区 	
		}
		else
		{
			BSP_FLASHWrite_NoCheck(WriteAddr, pBuffer, secremain);
		}

		// 写已经擦除了的,直接写入扇区剩余区间.
		if(Numsize == secremain)
			break;		// 写入结束了
		else				// 写入未结束		
		{
			secpos++;  		// 扇区地址增1  
			
			secoff = 0;  	// 偏移位置为0 	 
			
			pBuffer += secremain;					// 指针偏移
			
			WriteAddr += secremain * 2;		// 写地址偏移	   
			
			Numsize -= secremain;					// 字节(16位)数递减
			
			if(Numsize > (Page_Num))
				secremain = Page_Num;				// 下一个扇区还是写不完	
			else
				secremain = Numsize;					// 下一个扇区可以写完了	
		}	 
	}
	
	FLASH_Lock();	// 上锁
}


// ---------------------------- Init ----------------------------------





