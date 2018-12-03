#include "body.h"

uint8 fBody = 0;		// 1：有手   0 无手

/**
	* @brief  检测是否有手深入送纸口, 有即停止电机
  * @param  void  
  * @retval void
  */
static void Check_Body_In(void)
{
	static uint8 BodyCount = 0, HaveCount = 0;
	static uint8 DelayTime = 0;			// 人体感应器封锁时间
	
	/***
	*		设计逻辑：考虑到电路板老化可能存在输出不是稳定的高电平
	*							10次读取中，若存在5次高电平，则认为传感器有输出高电平
	*/
	if (BODY_STATE)		// 有人
	{
		HaveCount++;
	}	

	if (++BodyCount >= 10)
	{
		if (HaveCount <= 2) 			// 没有人
		{		
			if (++DelayTime >= 15)		// 将封锁时间缩小为1s, 也保证变频器启动与停止有1.5s间隙
			{
				if (  (fBody & BODY_HAVE) )  // (gMcuWorkStep == MCU_TIME_CUT_DOWN) &&
				{
					fState |= SHREDDER_CUT; 	// 进入碎纸状态				
				}
				CLEAR_BODY_HAVE;
				fBody &= ~BODY_HAVE;
				DelayTime = 15;						
			}
		}
		else if (HaveCount >= 5) 	// 有人
		{				
			SET_BODY_HAVE;
					
			DelayTime = 0;	
			Motor_Stop();								
			
			if ((fBody & BODY_HAVE) == 0)
			{
				fBody |= BODY_HAVE;
				
				if ((fBody & INF_HAVE) == 0)			fState |= TIME_CUT_DOWN;  // 上传倒计时	
			}						
		}
		
		HaveCount = 0;
		BodyCount = 0;
	}
}

/**
	* @brief  检测是否有手深入送纸口, 有即停止电机
  * @param  void  
  * @retval void
  */
void Check_Body_State(void)
{
	switch (gMcuWorkStep)
	{
		case MCU_GET_CUT_PAPER_CMD:		// 点击“开始碎纸”
		case MCU_CUT_PAPER:						// 等待“碎纸完成” 
		case MCU_TIME_CUT_DOWN:				// 上传倒计时		
							Check_Body_In();	
				break;
		
		case MCU_GET_CUT_DOWM_CMD:		// 收到“碎纸完成” -> 电机延时	
		case MCU_SYS_OPEN:						// 扫描二维码
		case MCU_GET_WEIGHT_TO_SEND: 	// 称重 
		case MCU_RUN_PROCESS_OK:    	// 完成整一个过程
		case MCU_SYS_CLOSE:						// 机器关闭中
				break;
		
		default:
					
				break;
	}
}

/**
  * @brief  人体感应输入 -- PA12
  * @param  void  
  * @retval void
  */
void Body_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}




