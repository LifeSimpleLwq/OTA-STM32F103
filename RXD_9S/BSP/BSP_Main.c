#include "BSP_Main.h"

u8 RxBuf;
u8 time = 0;

/**
  * @brief  BSP_Main
  * @param  void
  * @retval None	  
  */
void BSP_Main(void)
{	 
		u16 i = 0;
	
		printf("OTA bootloader V1.3.1\r\n");

		check_RunStatu();				// 检查Bootloader 运行标志位
		BSP_SYSInit();					// 读取ID
		OTA_RxMode();						// 初始化为接收模式
	
		HAL_TIM_Base_Start_IT(&htim3);		// 开启定时器中断
	
		while(1)
		{
			HAL_Delay(1);
			
			if (NRF24L01_RxLenth()) 	// 接收固件长度
			{	
					Count_ReceiveNum();		// 计算接收次数	
					Receive_Firmware();		// 接收固件
			}	
			if (i++ >= 500) {i = 0;HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);}  // 运行指示灯
		}	
}

///**
//  * @brief  UART中断服务函数
//  * @param  huart: 串口
//  * @retval None	  
//  */
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{ 
//	 if(huart->Instance == USART1) 
//	 { 
//			OTA_RX[USART_RX_CNT++] = RxBuf;
//   }
//}

/**
  * @brief  TIM中断服务函数
  * @param  htim：定时器
  * @retval None	  
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	time++;
	printf("\r\n");
	
	if (time >= 5)	
	{
		HAL_TIM_Base_Stop_IT(&htim3);
		check_RunStatu();
		time = 0;
	}
}







