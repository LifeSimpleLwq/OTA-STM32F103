#include "android.h"

static uint8 Usart2RxDataBuf[12] = {0};

uint8 fUsart2Rx = 0;
uint8 Usart2RxData[12] = {0};
uint8 Usart2SendData[10] = {0};

/**
  * @brief   检验安卓屏下行的数据帧头，帧尾是否正确
  * @param  
  * @retval  1 数据包正确		0 数据包错误
  */
static uint8 Android_Check_Rx_Data(uint8 *pstr)
{
	uint8 i = 0;
	uint8 len1 = 0;
	uint8 len2 = 0;
	uint16 len = 0;
	
	if(*pstr != ANDROID_COMMUN_PROT_FH)
	{
		return 0;
	}
	
	pstr++;
	pstr++;
	pstr++;
	len1 = *pstr;
	pstr++;
	len2 = *pstr;

	len = len1 * 256 + len2;

	for(i = 0; i < len; i++)
	{
		pstr++;
	}

	pstr++;

	if(*pstr != ANDROID_COMMUN_PROT_FT)
	{
		return 0;
	}

	return 1;
}

/**
  * @brief   处理安卓屏下行的信息
  * @param   void
  * @retval  1 有处理数据		0 无处理数据
  */
void Check_Cmd_Android(void)
{
	uint16 wordcmd = 0;
	
	//串口2
	if (fUsart2Rx == TRUE)
	{			
		memset(&Usart2RxDataBuf[0], 0, 12);
		memcpy(&Usart2RxDataBuf[0], &Usart2RxData[0], 12);
		memset(&Usart2RxData[0], 0, 12);
		memset(&Usart2SendData[0], 0, 12);
		
		if (Android_Check_Rx_Data(&Usart2RxDataBuf[0]) == 1)
		{
			//提取关键字
			wordcmd = Usart2RxDataBuf[1] * 256 + Usart2RxDataBuf[2];
		
			switch (wordcmd)
			{
				case SHREDDER_GET_STATE:	// 获取状态	
							Usart2SendData[0] = 0;
				
							Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_GET_STATE, 1);
							printf("GET SHREDDER_GET_STATE  gGetWorkState = 0x0\r\n");
					break;
				
				case SHREDDER_LOCK:				// 电子锁开关				
							CLEAR_LOCK;
							printf("GET SHREDDER_LOCK...\r\n");
					break;
				
				case OTA_VERSION_GET:			// 获取版本			
							Usart2SendData[0] = 0;
							Usart2SendData[1] = 0;
							Usart2SendData[2] = 0;
	
							Android_Uart_Send_Data(&Usart2SendData[0], OTA_VERSION_GET, 3);
							printf("GET OTA_VERSION_GET...\r\n");	
					break; 
				
				case OTA_VERSION_UPDATE:	// 确定升级
				
							Android_Uart_Send_Data(&Usart2SendData[0], OTA_VERSION_UPDATE, 0);
							printf("GET OTA_VERSION_UPDATE...\r\n");	
					break; 
					
				case OTA_PACK_SIZE:				// APP SIZE						
							if ((Usart2RxDataBuf[0] == ~Usart2RxDataBuf[2]) && (Usart2RxDataBuf[1] == ~Usart2RxDataBuf[3]))
							{
								Android_Uart_Send_Data(&Usart2SendData[0], OTA_VERSION_UPDATE, 4);
								
								OTA.APPSize  = Usart2RxDataBuf[0] * 256 + Usart2RxDataBuf[1];
								OTA.RunStatu = OTA_RXD_UART;
								OTA.APPCount = 0;
								
								printf("代码长度:%dBytes\r\n",OTA.APPSize);
								printf("Get	size ok ...\r\n");
							}
							else 
							{
								Usart2SendData[0] = 0;
								Usart2SendData[1] = 0;
								Usart2SendData[2] = 0;
								Usart2SendData[3] = 0;
								Android_Uart_Send_Data(&Usart2SendData[0], OTA_VERSION_UPDATE, 4);
								
								OTA.APPSize  = 0;
								OTA.RunStatu = OTA_WAIT;
								printf("Get	size err ...\r\n");
							}							
					break;
				
				default:
								printf("Get android cmd..\r\n");
					break;
			}
		}
		
		fUsart2Rx = FALSE;
	}
}


/**
  * @brief   上行数据到安卓屏
  * @param   dat: 内容
						 cmd: 下行命令
						 datnum: 内容长度	
  * @retval  void
  */
uint8 Android_Uart_Send_Data(uint8 *dat, uint16 cmd, uint16 datnum)
{
	uint8 datbuf[32] = {0};
	uint8 i = 0;

	datbuf[0] = ANDROID_COMMUN_PROT_FH;
	datbuf[1] = (uint8)(cmd >> 8);
	datbuf[2] = (uint8)cmd;

	datbuf[3] = (uint8)(datnum >> 8);
	datbuf[4] = (uint8)datnum;

	for(i = 0; i < datnum; i++)
	{
		datbuf[i + 5] = *dat;
		dat++;
	}

	datbuf[5 + datnum] = ANDROID_COMMUN_PROT_FT;

	for(i = 0; i < (datnum + 6); i++)
	{
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
		USART_SendData(USART2, datbuf[i]);
	}
	
	return 1;
}
















