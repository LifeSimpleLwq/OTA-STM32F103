#include "BSP_Main.h"

struct OTA_Run ota;

/**
  * @brief  主函数
  * @param  void
  * @retval None	  
  */
void BSP_Main(void)
{
		u8 i;
		
		my_mem_init(SRAMIN);
	  ota.Run_Statu = 0x00;  
		BSP_SYSInit();
		OTA_TxMode();
		
		while(1)
		{
			if (i++ >= 100)	{i = 0;check_Firmware();}			 // 检查固件是否接收完毕
			
			switch (ota.Run_Statu)
			{
				case 0x00:HAL_UART_Receive_IT(&huart1,(uint8_t*)&ota.RxBuf,1);break;		// 等待执行指令
				case 0xff:HAL_UART_Receive_IT(&huart1,(uint8_t*)&ota.RxBuf,1);break; 		// 接收固件
				case 0xfe:GET_IDHandle();	ota.Run_Statu = 0x00;break;										// 获取从机ID
				case 0xfd:Group_SendFirmware();ota.Run_Statu = 0x00;break;							// 升级固件	
				default :printf("输入指令错误！请重新输入!\r\n");ota.Run_Statu = 0x00;break;
			}	
		}
		
}

/**
  * @brief  UART中断服务函数 
  * @param  huart: 串口
  * @retval None	  
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{ 
		switch (ota.Run_Statu)
		{
			case 0xff:OTA_RX[USART_RX_CNT++] = ota.RxBuf;break;		// 等待执行指令
			case 0x00:ota.Run_Statu = ota.RxBuf;
								if(ota.RxBuf == 0xff) printf("请发送固件\r\n");
								break; 						// 接收固件
			default :ota.Run_Statu = 0x00;break;
		}	
		if (ota.Run_Statu == 0xff)	
		if (ota.Run_Statu == 0x00)	ota.Run_Statu = ota.RxBuf;
}

/**
  * @brief  串口获取从机ID
  * @param  void
  * @retval None	  
  */
void GET_IDHandle(void)
{
		u8 i,x;
		
		printf("请输入ID个数及ID号码：\r\n");
		HAL_UART_Receive(&huart1,(uint8_t*)&ota.ID_Num,1,0xffff);		// 获取ID个数
		HAL_UART_Receive(&huart1,(uint8_t*)&ota.ID_Buf,ota.ID_Num * 5,0xffff);		// 读取ID
				
		for(x = 0; x < ota.ID_Num;x++)
		{
			printf("\r\n第%d个ID为：",x+1);
			for(i = 0; i < 5; i++) 
				printf("%x ",ota.ID_Buf[x*5+i]);
		}
		printf("接收ID完毕!\r\n");
}

























