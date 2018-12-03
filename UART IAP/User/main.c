#include "Main.h"

/**
  * @brief  外设初始化
  * @retval None
  */
static void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	USART2_Config(9600);	//平板			9600	
	USART3_Config(115200);//信息打印	115200
	
	Led_GPIO_Init();
	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}

/**
  * @brief  Show_Version
  * @retval None
  */
static void Show_Version(void)
{
	printf("\r\n\r\n");
	printf(" \\           /\r\n");
	printf("- Paperbanker - Shredder System\r\n");
	printf(" /           \\  Bootloader Version %d.%d.%d \r\n", 
						VERSION, SUBVERSION, REVISION);
	printf("                Copyright by paperbanker team\r\n\r\n");
}

/**
  * @brief  main
  * @retval None
  */
int main(void)
{		
	uint8 tim = 0;
	
	Hardware_Init();

	Show_Version();
	
	BSP_Flash_Init();

	while (1)
	{	
		BSP_Delay_us(10000);	// 10ms
		
		if (tim++ >= 50)	 		// 500ms
		{
			tim = 0; 

			check_Firmware();	 // 检查固件是否接收完毕
			Receive_Firmware();
			
			Led_Run_Show();
		}			
	}
}

static const uint8 fac_us = 72; // 滴答定时器的时钟频率，滴答定时器时钟为: HCLK/8
/**
  * @brief  延时n*us
  * @param  nus: 延时时间，单位us 
  * @retval None
  */
void BSP_Delay_us(uint32 nus)
{
	uint32 temp;	    
	
	SysTick->LOAD = nus * fac_us; 						//时间加载	  		 
	SysTick->VAL = 0x00;        							//清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//开始倒数	
  
	do
	{
		temp = SysTick->CTRL;
	}while((temp & 0x01) && !(temp & (1 << 16)));		//等待时间到达 
  
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;			//关闭计数器
	SysTick->VAL = 0X00;      											//清空计数器	 
}



