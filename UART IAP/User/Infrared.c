#include "Infrared.h"

uint16 gNoPaperCounter = 0;					//上传倒计时

/**
  * @brief  上传倒计时 只要检测到有纸 
	*	@note   立马刷新工作状态, 去消倒计时状态
  * @param  void  
  * @retval void
  */
static void Check_Infrared_Cut_Dowm(void)
{
	if (fBody & BODY_HAVE)	return;			// 有手
	
	if (INFRARE_STATE == 0)	// 有纸
	{		
		fBody &= ~INF_HAVE;
		
		fState &= ~TIME_CUT_DOWN;	// 清除倒计时
		fState |= SHREDDER_CUT; 	// 进入碎纸状态
		
		gMcuWorkStep = MCU_CUT_PAPER;
		
		SET_WORK_STATE_WORK;			// 进入工作状态		
		CLEAR_NO_PAPER_COUTER;		// 刷新 碎纸状态检测纸的参数	
	}
}

/**
  * @brief  未上传倒计时，检测是否有纸
	* @note		有纸立马刷新工作状态
  * @param  void  
  * @retval void
  */
static void Check_Infrared_State(void)
{
	uint8 buf = 0;

	// 此过程发生电机堵转 也不再判断是否有纸进入
	buf = fState & MOTOR_CUR_ERR;
	if (buf == MOTOR_CUR_ERR)
	{
		return;
	}

	// 一旦进入上报倒计时 不再判断是否有纸进入 直到倒计时完成
	buf = fState & TIME_CUT_DOWN;
	if (buf == TIME_CUT_DOWN)
	{
		return;
	}

	//有纸
	if (INFRARE_STATE == 0) 
	{
		CLEAR_NO_PAPER_COUTER;
		fState &= ~TIME_CUT_DOWN;
	}
	else
	{
		gNoPaperCounter++;
		if (gNoPaperCounter >= NO_PAPER_COUNTER_MAX)
		{
			gNoPaperCounter = NO_PAPER_COUNTER_MAX;
			SET_WORK_STATE_FREE;									// 空闲状态
			
			gMcuWorkStep = MCU_TIME_CUT_DOWN;			// MCU处于上传倒计时状态		
			
			if (fBody == 0)			// 检测到有手，不上传无纸通知
			{			
				fState |= TIME_CUT_DOWN;  						// 上传倒计时							
			}	
			fBody |= INF_HAVE;		
		}
	}
}

/**
  * @brief  入纸口检测	
  * @param  void  
  * @retval void
  */
void Check_Infrared_Every_State(void)
{
	switch (gMcuWorkStep)
	{
		case MCU_CUT_PAPER:					//等待“碎纸完成” 
					Check_Infrared_State();
			break;

		case MCU_TIME_CUT_DOWN:			//上传倒计时
					Check_Infrared_Cut_Dowm();					
			break;
		
		case MCU_SYS_OPEN:					//扫描二维码				 
		case MCU_GET_CUT_PAPER_CMD:	//点击“开始碎纸”
		case MCU_GET_CUT_DOWM_CMD:	//收到“碎纸完成” -> 电机延时
			
		default:
					fBody &= ~INF_HAVE;
					CLEAR_NO_PAPER_COUTER;
			break;
	}
}

/**
  * @brief  红外-电源 PA11  红外电平检测 PA8
  * @param  void  
  * @retval void
  */
void Infrared_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//红外 电源
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_11);	
	
	//红外电平检测
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}





