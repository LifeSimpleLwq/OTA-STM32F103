#include "BSP_OTA.h"

uint8 OTA_RxBuf[OTA_RX_LEN] __attribute__ ((at(0X20001000)));	//接收缓冲,最大OTA_RX_LEN个字节,起始地址为0X20001000. 

struct OTA_struct OTA;

uint16 Usart_Rx_Cnt, oldCount, AppLenth;

iapfun jump2app; 


/**
  * @brief  跳转到应用程序段
  * @param  appxaddr:用户代码起始地址.
  * @retval None	  
  */
static void Iap_Load_App(uint32 Addr)
{
	if(((*(vu32*)Addr) & 0x2FFE0000) == 0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app = (iapfun)*(vu32*)(Addr + 4);		//用户代码区第二个字为程序开始地址(复位地址)					
		__set_MSP(*(vu32*) Addr);  	//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP. 
	}
}	

/**
  * @brief  保存、执行固件
  * @retval 0  接收数据非固件
  */
static uint8 Update_Firmware(uint32 addr)
{
		if(((*(vu32*)(addr + 4)) & 0xFF000000) == 0x08000000)//判断是否为0X08XXXXXX.																			
		{	 
			printf("开始执行FLASH用户代码!!\r\n");
			Iap_Load_App(addr);			//执行FLASH APP代码	
		}
		
		printf("非FLASH应用程序,无法执行!\r\n");

		OTA.RunStatu = OTA_WAIT;	
		Usart_Rx_Cnt = 0;
		oldCount = 0;	
		AppLenth = 0;
		
		return 0;
}

/**
  * @brief  APP写入FLASH
  * @param  appxaddr:应用程序的起始地址
  * @param 	appbuf:  应用程序CODE.
  * @param	appsize: 应用程序大小(字节).
  * @retval None	  
  */
static void IAP_Write_APP(uint32 App_Addr, uint8 *App_Buf, uint32 App_Size)
{
	uint32 t;
	uint16 buf[512];
	uint16 i = 0;
	uint8  *dfu = App_Buf;
	
	printf("Static write flash...\r\n");
	printf("write addr = 0x%x\r\n",App_Addr);
	for(t = 0; t < App_Size; t += 2)
	{			
		buf[i]  = (uint16)dfu[1] << 8;
		buf[i] += (uint16)dfu[0];	  
		
		dfu += 2;		//偏移2个字节
		i++;
		
		if(i == 512)
		{
			i = 0;
			BSP_FLASHWrite(App_Addr, buf, 512);
			App_Addr += 1024; //偏移2048  32 = 4*8.所以要乘以4.
		}
	}
	
	if(i) BSP_FLASHWrite(App_Addr, buf, i);//将最后的一些内容字节写进去. 
	App_Addr += i*2;
	
	printf("Write flash OK...\r\n");
}


/**
  * @brief  接收固件,并写入到flash
  * @param  void 
  * @retval NONE
  */
void Receive_Firmware(void)
{
	if (OTA.RunStatu == OTA_RXD_UART_OK)
	{
		IAP_Write_APP(OTA_ADDR, OTA_RxBuf, AppLenth);
		Update_Firmware(OTA_ADDR);
	}
}

/**
  * @brief  check_Firmware
	* @note   检查固件是否接收完毕
	* @param  void 
  * @retval NONE
  */
void check_Firmware(void)
{
	static uint16 oldCount = 0;
	if (Usart_Rx_Cnt)
	{
		if (Usart_Rx_Cnt == oldCount)
		{
			AppLenth = Usart_Rx_Cnt;
			Usart_Rx_Cnt = 0;
			oldCount = 0;

			printf("用户程序接收完成!\r\n");
			printf("代码长度:%dBytes\r\n",AppLenth);
			OTA.RunStatu = OTA_RXD_UART_OK;
		}
		else
			oldCount = Usart_Rx_Cnt;
	}
}

/**
  *	@brief  下载勾选擦除下载部分，重复下载两次，即可测试flash写入是否成功
	* @note 	上电时，读取flash数据
  * @retval None
  */
void BSP_Flash_Init(void)
{
	uint16 flag = 0;
	
//	Run_Bootloader();
	
	flag = BSP_FLASHReadHalfWord(OTA_FLAG_ADDR);
	
	switch (flag)
	{
		case FLAG_FIRMWARE:	
						printf("\r\n");
						printf("----- Have Firmware -----\r\n");
						printf("----- Run Firmware! -----\r\n");
						printf("\r\n");
						Iap_Load_App(OTA_ADDR);
					break;
		
		case FLAG_NO_FIRMWARE:
						printf("\r\n");
						printf("----- One Download -----\r\n");
						printf("----- No Firmware! -----\r\n");
						printf("\r\n");
					break;
		
		case FLAG_RX_FIRMWARE:
						printf("\r\n");
						printf("----- Update Firmware -----\r\n");
						printf("\r\n");
						
						flag = FLAG_NO_FIRMWARE;
						BSP_FLASHWrite(OTA_FLAG_ADDR, &flag, 1);
		
						Android_Uart_Send_Data(&Usart2SendData[0], OTA_VERSION_UPDATE, 0);
					break;
		
		default:
						flag = FLAG_NO_FIRMWARE;
						BSP_FLASHWrite(OTA_FLAG_ADDR, &flag, 1);
						printf("\r\n");
						printf("----- flash error, init -----\r\n");
						printf("\r\n");
					break;
	}
}


