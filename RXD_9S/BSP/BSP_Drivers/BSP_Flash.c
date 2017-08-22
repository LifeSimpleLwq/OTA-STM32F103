#include "BSP_flash.h"

#define STM_SECTOR_SIZE 1024 // 字节
#define Page_SIZE 	STM_SECTOR_SIZE		// FLASH一页存储空间大小
#define Page_Num		Page_SIZE/4			// 以4个字节为单位，注意，此处与写入的数据宽度有关

uint32_t BSP_FLASHBUF[STM_SECTOR_SIZE/4];

/**
  * @brief  指定位置写入半字（16位）
  * @param  WriteAddr :	写入地址
  * @param  pBuffer ：写入内容	
  * @retval NONE
  */
void BSP_FLASHWriteWord(uint32_t WriteAddr,uint16_t pBuffer)
{
		uint32_t buff_u32;
		uint16_t buff_u16;
		
		buff_u16 = BSP_FLASHReadWord(WriteAddr);
	
		buff_u32 = buff_u16 << 16;
		buff_u32 += pBuffer;
	
		BSP_FLASHWrite(WriteAddr,&buff_u32,1);
}

/**
  * @brief  读取指定地址的字
  * @param  faddr :读取的地址
					该地址必须是 4 的倍数
  * @retval 读取的内容
  */
uint32_t BSP_FLASHReadWord(uint32_t faddr)
{
	return *(__IO uint32_t*)faddr; 
}

/**
  * @brief  不检查的写入
  * @param  WriteAddr: 起始地址
  * @param  pBuffer: 写入数据的缓冲区
  * @param  NumToWrite: 写入数据的大小(单位: 字)
  * @retval None
  */
void BSP_FLASHWrite_NoCheck(uint32_t WriteAddr,uint32_t *pBuffer,uint16_t NumToWrite)   
{
	uint16_t i;
	
	for(i=0;i<NumToWrite;i++)
	{
		HAL_FLASH_Program(TYPEPROGRAM_WORD, WriteAddr, pBuffer[i]);
		WriteAddr+=4;
		// 地址增加4.
	}  
}

/**
  * @brief  擦除一页
  * @param  Page_Address :要擦除的页地址
  * @retval None
  */
void FLASH_ErasePage(uint32_t Page_Address)
{
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef f;

	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = Page_Address;
	f.NbPages = 1;

	HAL_FLASHEx_Erase(&f, &PageError);
}

/**
  * @brief  往 Flash 写数据(一页写入256字节)
  * @param  WriteAddr: 起始地址
  * @param  pBuffer: 写入数据的缓冲区  
  * @param  NumToWrite: 写入数据的大小(单位: 字)  
  * @retval None	  
  */
void BSP_FLASHWrite(uint32_t WriteAddr,uint32_t *pBuffer,uint16_t NumToWrite)	
{
	uint32_t secpos;
	// 扇区地址
	uint16_t secoff;
	// 扇区内偏移地址(32位计算)
	uint16_t secremain;
	// 扇区内剩余地址(32位计算)
 	uint16_t i;    
	uint32_t offaddr;
	uint32_t Numsize;

	if(WriteAddr < STM32_FLASH_BASE||(WriteAddr >= (STM32_FLASH_BASE+STM_SECTOR_SIZE*STM32_FLASH_SIZE)))
		return;
	// 非法地址  

	HAL_FLASH_Unlock();
	// 解锁

	Numsize = NumToWrite;  // 写入字节 
	// 计算写入的长度  
	offaddr = WriteAddr - STM32_FLASH_BASE;  
	// 实际偏移地址. 
	secpos = offaddr/(Page_SIZE);    
	// 扇区地址  0~63 for STM32F103C8
	secoff = (offaddr%(Page_SIZE))/4;   	
	// 在扇区内的偏移( 4个字节为基本单位. )
	secremain = Page_Num - secoff;		  
	// 扇区剩余空间大小

	if(Numsize <= secremain)	secremain = Numsize;  // 不大于该扇区范围
	
	while(1) 
	{
		BSP_FLASHRead (secpos*Page_SIZE+STM32_FLASH_BASE, BSP_FLASHBUF, Page_Num);		
		// 读出整个扇区的内容

		for (i = 0; i < secremain; i++)
		// 校验数据
		{
			if (BSP_FLASHBUF[secoff+i] != 0x00000000)
				break;
			// 需要擦除
		}

		if (i<secremain)
			// 需要擦除
		{
			FLASH_ErasePage (secpos*Page_SIZE+STM32_FLASH_BASE); 
			FLASH_WaitForLastOperation (500);
			// 擦除这个扇区
			for (i=0;i<secremain;i++)
			// 复制
			{
				BSP_FLASHBUF[i+secoff] = pBuffer[i];
			}
			BSP_FLASHWrite_NoCheck (secpos*Page_SIZE+STM32_FLASH_BASE, BSP_FLASHBUF, Page_Num);   
			// 写入整个扇区
			FLASH_WaitForLastOperation (500);
		}
		else
		{
			BSP_FLASHWrite_NoCheck (WriteAddr, pBuffer, secremain);
			FLASH_WaitForLastOperation (500);
		}

		// 写已经擦除了的,直接写入扇区剩余区间.
		if(Numsize == secremain)
			break;
		// 写入结束了
		else
			// 写入未结束
		{
			secpos++;  //  20
			// 扇区地址增1
			secoff = 0;  
			// 偏移位置为0 	 
			pBuffer += secremain;
			// 指针偏移
			WriteAddr += (secremain*4);
			// 写地址偏移	   
			Numsize -= secremain;
			// 字节(16位)数递减
			if(Numsize > (Page_Num))
				secremain = Page_Num;
			// 下一个扇区还是写不完
			else
				secremain = Numsize;
			// 下一个扇区可以写完了
		}	 
	}
	HAL_FLASH_Lock();
	// 上锁
}

/**
  * @brief  从指定地址开始读出指定长度的数据
  * @param  ReadAddr: 起始地址
  * @param  pBuffer: 读出数据的缓冲区
  * @param  NumToWrite: 读出数据的大小(单位: 字)
  * @retval None
  */
void BSP_ReadByte(uint32_t ReadAddr,uint8_t *pBuffer)
{
	uint32_t tmp;

	BSP_FLASHRead(ReadAddr,&tmp,1);
	*pBuffer = (uint8_t)tmp;
}

/**
  * @brief  从指定地址开始读出指定长度的数据
  * @param  ReadAddr: 起始地址
  * @param  pBuffer: 读出数据的缓冲区
  * @param  NumToWrite: 读出数据的大小(单位: 字)
  * @retval None
  */
void BSP_FLASHRead(uint32_t ReadAddr,uint32_t *pBuffer,uint16_t NumToRead)   	
{
	uint16_t i;

	for(i=0; i<NumToRead; i++)
	{
		pBuffer[i] = BSP_FLASHReadWord(ReadAddr);
		// 读取 4 个字节
		ReadAddr += 4;
		// 偏移 4 个字节
	}
}






