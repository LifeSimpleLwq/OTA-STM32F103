#include "BSP_OTA.h"
				
#define BSP_Write_Addr 0x08003C00	

#define SendNum_len			10
u16 sendNum[SendNum_len];

u32 iapbuf[512];  
iapfun jump2app; 
u16 AppLenth,oldCount,USART_RX_CNT = 0;
u16 OTAFLAG;

/**
  * @brief  保存、执行固件
  * @retval 0  接收数据非固件
  */
u8 Update_Firmware(void)
{
		if(((*(vu32*)(BSP_Write_Addr+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.																			
		{	 
			printf("开始执行FLASH用户代码!!\r\n");
			iap_load_app(BSP_Write_Addr);			//执行FLASH APP代码	
		}else 
		{
			printf("非FLASH应用程序,无法执行!\r\n");
		}	
		 
		return 0;
}

/**
  * @brief  Count_SendNum
	* @note   计算接收次数,根据固件SIZE设计
  * @param  size:单次最大发送量
  * @retval None	  
  */
void Count_ReceiveNum(void)
{
		u8 i = 0;
		
		while (AppLenth)
		{
			if (AppLenth >= Single_SendSize)	
			{
				sendNum[i] = Single_SendSize/Single_DataSize;	// 计算NRF发送次数
				AppLenth -= Single_SendSize;
			}				
			else 
			{	
				if (AppLenth%Single_DataSize != 0) sendNum[i] = AppLenth/Single_DataSize + 1;		// 计算NRF发送次数
				else  sendNum[i] = AppLenth/Single_DataSize;
				AppLenth = 0;
			}
			printf("第%d次接收次数为%d\r\n",i+1,sendNum[i]);
			i++;		
		}
}

/**
  * @brief  Receive_Firmware
	* @note:	接收固件,并写入到flash
  * @param  
  * @retval 1 更新成功 or 0 更新失败  
  */
u8 Receive_Firmware(void)
{
		u16 i = 0;
		u16 x = 0;
		USART_RX_CNT = 0;
		u32 Addr =	0x08003C00; 
		
	
		while (sendNum[i])
		{
			while(USART_RX_CNT <= (sendNum[i]-1))
			{	
					x = 0;
					//HAL_Delay(1);
					while( NRF24L01_Rx(USART_RX_CNT) && x < 1000 )
					{
						HAL_Delay(1); //printf("1");
						if(++x >= 950) 
						{
							printf("1s内检测不到数据，取消升级\r\n");
							Change_OTAFlag(0xffff);
							return 0;
						}
					}
				//	printf(" %3d %3d %3d\r\n",SPI_RX_BUF[0] + SPI_RX_BUF[1]*256,USART_RX_CNT + i*256,SPI_RX_BUF[4]); 		// 调试使用
					if (SPI_RX_BUF[0] + SPI_RX_BUF[1]*256 == i*256 + USART_RX_CNT)	USART_RX_CNT++; 					
			}
			USART_RX_CNT = 0;		// 初始化SRAM接收器，准备下一轮接收	

			if(!i)
			{
				if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)  //判断是否为0X08XXXXXX. 
				{	 
					iap_write_appbin(Addr,OTA_RX,sendNum[i]*Single_DataSize);		//更新FLASH代码   
					printf("\t写入flash成功！\r\n");	
				}
				else 
				{
					printf("\t非固件，不能写入flash！\r\n");	
				}
			}		
			else  			
			{
				iap_write_appbin(Addr,OTA_RX,sendNum[i]*Single_DataSize);		//更新FLASH代码   
				printf("\thello,写入flash成功！\r\n");				
			}
			printf("\t%d %x\r\n",i,Addr);
			Addr += sendNum[i]*Single_DataSize;		// 地址偏移   
			
			i++;
		}
		
		Change_OTAFlag(0xd0d0);
		Update_Firmware();		// 更新固件
		return 1;
}

/**
  * @brief  OTA_RxMode
  * @param  void
  * @retval None	  
  */
void OTA_RxMode(void)
{
		NRF24L01_Init();
		NRF24L01_RX_Mode();
}

/**
  * @brief  APP写入FLASH
  * @param  appxaddr:应用程序的起始地址
  * @param 	appbuf:  应用程序CODE.
  * @param	appsize: 应用程序大小(字节).
  * @retval None	  
  */
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;

	for(t=0;t<appsize;t+=4)
	{			
		temp=(u32)dfu[3]<<24;
		temp+=(u32)dfu[2]<<16;
		temp+=(u32)dfu[1]<<8;
		temp+=(u32)dfu[0];	  
		dfu+=4;		//偏移4个字节
		iapbuf[i++]=temp;	 	
		if(i==512)
		{
			i=0;
			BSP_FLASHWrite(fwaddr,iapbuf,512);
			fwaddr+=2048; //偏移2048  32=4*8.所以要乘以4.
		}
	}
	if(i)BSP_FLASHWrite(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}

/**
  * @brief  跳转到应用程序段
  * @param  appxaddr:用户代码起始地址.
  * @retval None	  
  */
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)					
		__set_MSP(*(vu32*) appxaddr);  	//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP. 
	}
}	
  
/**
  * @brief  跳转到应用程序段
  * @param  appxaddr:用户代码起始地址.
  * @retval None	  
  */
void check_RunStatu(void)
{	
		OTAFLAG = BSP_FLASHReadWord(OTA_Sign);	// 读取运行状态标志位
		
		printf("OTAFLAG =  %x\r\n",OTAFLAG);
		switch (OTAFLAG)
		{
			case 0xFFFF:	printf("\r\n没有固件，等待接收固件\r\n");break;
			case 0xd0d0:	printf("\r\n执行固件，无需升级！\r\n");
										Update_Firmware();
										OTAFLAG = 0xffff;	// 检查不到固件
										break;
			case 0xd1d1:	printf("\r\n升级固件，等待数据包\r\n");break;
			default : OTAFLAG = 0xffff;	Change_OTAFlag(OTAFLAG);break;
		}
}

/**
  * @brief  更改OTA标志位	
  * @param  tmp： FFFF 没有固件   D0D0 执行固件
  * @retval None	  
  */
void Change_OTAFlag(u16 tmp)
{
		OTAFLAG = tmp;
		BSP_FLASHWriteWord(OTA_Sign,OTAFLAG);  // 		
}


