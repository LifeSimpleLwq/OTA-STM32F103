#include "BSP_OTA.h"
#include "BSP_Main.h"


#define SendNum_len			10		

u16 sendNum[SendNum_len];
u32 iapbuf[512];  
iapfun jump2app; 
u16 AppLenth,oldCount,USART_RX_CNT = 0;
u8 seng_num[3];
extern 	struct OTA_Run ota;

/**
  * @brief  check_Firmware
	* @note   检查固件是否接收完毕
	* @param  void 
  * @retval NONE
  */
void check_Firmware(void)
{
		if(USART_RX_CNT)
		{
			if(oldCount == USART_RX_CNT)	//新周期内,没有收到任何数据,认为本次数据接收完成.
			{
				AppLenth = USART_RX_CNT;
				oldCount=0;
				USART_RX_CNT=0;
				printf("用户程序接收完成!\r\n");
				printf("代码长度:%dBytes\r\n",AppLenth);
				
				seng_num[0] = AppLenth%256;
				seng_num[1] = AppLenth/256;	
				Count_SendNum();		// 计算发送次数	
				ota.Run_Statu = 0x00;	
			}
			else 
				oldCount=USART_RX_CNT;			
		}	
}

/**
  * @brief  Group_SendFirmware
	* @note   群组更新固件
  * @param  void
  * @retval NONE
  */
void Group_SendFirmware(void)
{
		u8 i = 0,y = 0;
	
		printf("Group_SendFirmware\r\n");
	
		if (!seng_num[0] && !seng_num[1]) printf("没有固件，请传输固件！\r\n");
		else 
		{
			while( y < ota.ID_Num )		// 群组更新固件
			{
				Group_UpdateID(y++);		// 更新接收端ID
				
				i = 0;
				while	(NRF24L01_TxLenth(seng_num) && i< 10) // 发送固件长度，并检查RX端是否存在，1s内不存在，即自动退出此本发送	
				{
					HAL_Delay(100);
					printf("第%d次发送长度失败！\r\n",++i); 
				}  	
				
				if (i < 10)	Send_Firmware();		// 发送固件
				else {printf("ID:%x%x%x%x%x  更新失败，检查不到接收端！\r\n",RX_ADDRESS[0],RX_ADDRESS[1],RX_ADDRESS[2],RX_ADDRESS[3],RX_ADDRESS[4]);}	
			}
		}
		//seng_num[0] = seng_num[1] = 0;
}

/**
  * @brief  Group_UpdateID
	* @note   更新接收端ID
  * @param  tmp，ID号
  * @retval None	   
  */
void Group_UpdateID(u8 tmp)
{
	u8 i;
	for(i = 0; i < 5; i++)
	{
		RX_ADDRESS[i] = TX_ADDRESS[i] = ota.ID_Buf[i + tmp*5];	// 赋值地址
	}
	
	NRF24L01_TX_Mode();	// 初始化地址
}

/**
  * @brief  Send_Firmware
	* @note   发送固件
  * @param  void
  * @retval 1,发送成功 or 0XFF	RXD没有应答,结束本次发送
  */
u8 Send_Firmware(void)
{
	u8 z;
	u16 y,x,index;
	x = y = z = 0;
	
	HAL_Delay(3);
	
	while(sendNum[z])
	{	 
		for ( y = 0; y < sendNum[z]; y++ )
		{
			HAL_Delay(4);
			index = z*sendNum[0] + y;
			
			while( NRF24L01_Tx( index ) && x < 1000)	// 发送固件,1s内没有收到应答位结束发送
			{
				HAL_Delay(2);
				printf("更新中断  %d\r\n",index); 
				x++;
				if (x >= 950) { printf("ID：%d%d%d%d%d 更新中断\r\n",RX_ADDRESS[0],RX_ADDRESS[1],RX_ADDRESS[2],RX_ADDRESS[3],RX_ADDRESS[4]); return 0xFF; }		// 超时检测
			}
			//HAL_Delay(2);
			x = 0;
	  }
		
		HAL_Delay(840);  // 等待写入flash
		++z;
	}
	
	printf("发送结束");
	
	return 1;
}

/**
  * @brief  Count_SendNum
	* @note   计算发送次数,根据RX端接收器SIZE设计
  * @param  size:单次最大发送量
  * @retval None	  
  */
void Count_SendNum(void)
{
		u8 i = 0;
	
		while (AppLenth)
		{
			if (AppLenth>=Single_SendSize)	
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
		printf("\r\n");
}

/**
  * @brief  OTA_RxMode
	* @note   初始化NRF为接收端
  * @param  void
  * @retval None	  
  */
void OTA_RxMode(void)
{
		NRF24L01_Init();
		NRF24L01_RX_Mode();
		printf("NRF24L01 Mode: Receive\r\n");	
}

/**
  * @brief  OTA_TxMode
	* @note   初始化NRF为发送端
  * @param  void
  * @retval None	  
  */
void OTA_TxMode(void)
{
		NRF24L01_Init();
		NRF24L01_TX_Mode();
		printf("NRF24L01 Mode: Sends\r\n");	
}

/**
  * @brief  iap_write_appbin
	* @note   APP写入FLASH
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
		dfu+=4;				//偏移4个字节
		iapbuf[i++]=temp;	 	
		if(i==512)
		{
			i=0;
			BSP_FLASHWrite(fwaddr,iapbuf,512);
			fwaddr+=2048; 		//偏移2048  32=4*8.所以要乘以4.
		}
	}
	if(i)BSP_FLASHWrite(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}

/**
  * @brief  iap_load_app
	* @note   跳转到应用程序段
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





