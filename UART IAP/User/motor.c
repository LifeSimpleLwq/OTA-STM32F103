#include "motor.h"

static uint8 Usart1RxDataBuf[4] = {0};

uint8 Usart1RxData[4] = {0};
uint8 fUsart1Rx = 0;
uint8 fMotorPoAndNegEnd = 0;//0--没有堵转了  1--堵转了
uint8 fMotorLock = 0;				//最终确认堵转  0--没有堵转 1--堵转了
uint8 DcMotorCloseFlag = 0xff;

//---------------------- 变频器 ----------------------------
/**
  * @brief   Motor_Check_Rx_Data
  * @param  
  * @retval  1--通信成功 无故障  2--通信成功有故障   0--通信不成功	格式不对
  */
static uint8 Motor_Check_Rx_Data(uint8 *pstr)
{
	//根据不同帧头 判断
	if(pstr[0] == MOTOR_COMMUN_PROT_FH)
	{
		return 1;
	}
	return 0;
}

/**
  * @brief   接收数据检测
  * @param   void
  * @retval  0 通讯失败	 1 正常  2 堵转
  */
static uint8 Check_Cmd_Motor(void)
{
	uint8 ireturn = 0;
	
	if (fUsart1Rx == TRUE)
	{
		fUsart1Rx = FALSE;
		
		memset(&Usart1RxDataBuf[0], 0, 4);
		memcpy(&Usart1RxDataBuf[0], &Usart1RxData[0], 4);
		memset(&Usart1RxData[0], 0, 4);
		
		ireturn = Motor_Check_Rx_Data(&Usart1RxDataBuf[0]);

		if(ireturn == 1)//电流读取
		{
	
		}	
		return (uint8)(ireturn);
	}
	
	if (fMotorPoAndNegEnd)	// 堵转
	{
		return 2;
	}
	return 0;
}

/**
  * @brief   变频器串口通讯 发送数据
  * @param   
  * @retval  void
  */
static void Motor_Uart_Send_Data(uint8 cmd)
{
	uint8 datbuf[4] = {0};
	uint8 i = 0;

	datbuf[0] = MOTOR_COMMUN_PROT_FH;
	datbuf[1] = cmd;
	datbuf[2] = (uint8)(~MOTOR_COMMUN_PROT_FH);

	datbuf[3] = (uint8)(~cmd);

	for(i = 0; i < 4; i++)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, datbuf[i]);
	}
}

/**
  * @brief   发送数据
  * @param   cmd ： 动作指令
  * @retval  0--通信失败 
									fState   |= MOTOR_ERR;	电机超时通信	
									gGetWorkState |= WORKSTATE_ERR;			处于故障状态
						 1--成功	
						 2--堵转	
  */
uint8 Check_Motor_Step(uint8 cmd)
{
	uint8 buff = 0;
	uint8 counter = 0;
	uint8 errcount = 0;

#if	!MOTOR_SWITCH
	return 1;	// debug
#else 	
	Motor_Uart_Send_Data(cmd);
	counter = 0;
	errcount = 0;
	
	//超时应答
	while (1)
	{	
		rt_thread_delay(500); 
		
		buff = Check_Cmd_Motor();

		if (buff == 1)
		{
			fState &= ~MOTOR_ERR;
			
			return 1;
		}

		if (buff == 2)	//堵转
		{
			printf("Motor lock %x ...\r\n",cmd);
			return 2;
		}
		
		counter++;	
		if (counter >= 2)
		{		
			errcount++;		
			Motor_Uart_Send_Data(cmd);
			counter = 0;
		}
			
		if (errcount >= 5)
		{
			//电机超时通信
			fState |= MOTOR_ERR;
			//处于故障状态
			SET_WORK_STATE_ERR;
			printf("电机通讯超时...\r\n");
			return 0;
		}
	}
#endif
}

/**
  * @brief   停止电机
  * @retval  0--通信失败 
									fState   |= MOTOR_ERR;	电机超时通信	
									gGetWorkState |= WORKSTATE_ERR;			处于故障状态
						 1--成功						
  */
uint8 Motor_Stop(void)
{
	uint8 count = 0;
	uint8 buf = 0;
	
	buf = Check_Motor_Step(MOTOR_CMD_STOP);
	
	while (buf != 1)
	{
		if (buf == 0) return 0;	// 通讯故障
		
		rt_thread_delay(500);		
		fMotorPoAndNegEnd = 0;
		
		if (count++ >= 1)	Motor_Uart_Send_Data(MOTOR_CMD_RESET);
		if (count >= 10)	return 0;
		
		buf = Check_Motor_Step(MOTOR_CMD_STOP);
	}
	
	return 1;
}

/**
  * @brief   关机时，自动复位
  * @retval  0--通信失败 
									fState   |= MOTOR_ERR;	电机超时通信	
									gGetWorkState |= WORKSTATE_ERR;			处于故障状态
						 1--成功						
  */
uint8 Motor_Stop_Reset(void)
{
	uint8 count = 0;
	uint8 buf = 0;
	
	buf = Check_Motor_Step(MOTOR_CMD_STOP);
	
	while (buf != 1)
	{
		if (buf == 0) return 0;	// 通讯故障
		
		rt_thread_delay(500);		
		fMotorPoAndNegEnd = 0;		
		
		if (count++ >= 1)	Motor_Uart_Send_Data(MOTOR_CMD_RESET);
		
		if (count >= 10)	return 0;
		
		buf = Check_Motor_Step(MOTOR_CMD_STOP);
	}
	
	if (Usart1RxDataBuf[1] & 0x40) 	Motor_Uart_Send_Data(MOTOR_CMD_CHANGE_DIR);
	
	return 1;
}

//-------------------------- 堵转处理 ---------------------------------
/**
  * @brief   堵转时，反转5s，再正转，再堵转，则反转5s后停止
  * @param   void
  * @retval  1--堵转  
									gGetWorkState |= WORKSTATE_ERR;
						 0--不堵转	
  */
static uint8 Check_Motor_CurErr_State(void)
{
	uint8 gMotorCurErrCounter = 0; //堵转时候正反转次数
	
	fMotorPoAndNegEnd = 0;
	
	//发送纸张过多 导致电机堵住
	Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_MOTOR_LOCK, 0);
	
	while(1)
	{	
		if (gMotorCurErrCounter < MOTOR_CUR_ERR_MAX)	// 
		{		
			gMotorCurErrCounter++;	
			
			// 反转5s  正转5s
			Motor_Stop();
			
			fMotorPoAndNegEnd = 0;			
			Check_Motor_Step(MOTOR_CMD_CHANGE_DIR);
			rt_thread_delay(300);		
			Check_Motor_Step(MOTOR_CMD_GO);	
			
			if (gMotorCurErrCounter == 1 )	
			{	
				printf("1 反转5s\r\n");
				rt_thread_delay(RT_TICK_PER_SECOND * 5);	// 5s		
			}
			else 
			{	
				printf("1 正转5s\r\n");
				rt_thread_delay(RT_TICK_PER_SECOND * 5);	//2s		
				fUsart1Rx = FALSE;
			}
		}
		else //正反转过后 一段时间 如果仍然接收到 错误代码 即是堵转
		{
//			if (( fUsart1Rx != TRUE ) && ( fMotorPoAndNegEnd != 1))
//			{
//				Motor_Uart_Send_Data(MOTOR_CMD_STOP);			
//				rt_thread_delay(300);
//				printf("1...fUsart1Rx = %d ,fMotorPoAndNegEnd = %d\r\n",fUsart1Rx,fMotorPoAndNegEnd);
//			}
//			
//			if (( fUsart1Rx != TRUE ) && ( fMotorPoAndNegEnd != 1))
//			{
//				Motor_Uart_Send_Data(MOTOR_CMD_STOP);			
//				rt_thread_delay(300);
//				printf("2....fUsart1Rx = %d ,fMotorPoAndNegEnd = %d\r\n",fUsart1Rx,fMotorPoAndNegEnd);
//			}
			printf("fUsart1Rx = %d ,fMotorPoAndNegEnd = %d\r\n",fUsart1Rx,fMotorPoAndNegEnd);
			if (fMotorPoAndNegEnd)	
			{			
				Motor_Stop();

				Check_Motor_Step(MOTOR_CMD_CHANGE_DIR);
				rt_thread_delay(300);
				Check_Motor_Step(MOTOR_CMD_GO);
				printf("2 反转5s ...\r\n");
				rt_thread_delay(RT_TICK_PER_SECOND * 5);	// 反转5s
			
				Motor_Stop();
				rt_thread_delay(300);
				Check_Motor_Step(MOTOR_CMD_CHANGE_DIR);		
				
				SET_WORK_STATE_ERR;
				fMotorPoAndNegEnd = 1;
			}
			else 
			{
				Check_Motor_Step(MOTOR_CMD_GO);	
			}
			
			printf("fUsart1Rx = %d ,fMotorPoAndNegEnd = %d\r\n",fUsart1Rx,fMotorPoAndNegEnd);
			return fMotorPoAndNegEnd;											
		}
	}
}

/**
  * @brief   检查电机堵转
  * @param   void
  * @retval  1--堵转  0--不堵转
  */
uint8 Check_Motor_Lock_State(void)
{
	//已经判断过变频器堵转了
	if(fMotorLock == 1)
	{
		Motor_Stop();
		SET_WORK_STATE_ERR;
		return 1;
	}
	
	//没有判断过堵转问题 进入判断堵转问题 电机会正反转一次
	if(Check_Motor_CurErr_State() == 1)
	{
		fMotorLock = 1;
		return 1;
	}
	else
	{
		return 0;
	}	
}

/**
  * @brief   串口通讯接收数据时数据校验函数   (USART1_IRQHandler中调用)
  * @param   void
  * @retval  1--通信成功 而且变频器有故障 
  */
uint8 Motor_Check_ErrCode(uint8*pstr)
{
	uint8 buf[4];
	//格式分析
	buf[0] = *pstr;
	pstr++;
	buf[1] = *pstr;
	pstr++;
	buf[2] = *pstr;
	pstr++;
	buf[3] = *pstr;
	
	if(buf[0] == MOTOR_ERR_FH)
	{
		if(buf[0] != (uint8)(~buf[2]))
		{
			return 0;
		}
		if(buf[1] != (uint8)(~buf[3]))
		{
			return 0;
		}
		return 1;
	}
	return 0;
}

//------------------------入纸口电机-------------------------------------
uint8 MotorCloseCount = 0;		// 电机关闭计数，重复关闭两次则不再关闭入纸口
/**
  * @brief   入纸口直流电机检测  10ms
  * @param   void
  * @retval  void
  */
void Check_Motor_Limit_State(void)
{
	static uint8 ForwardCounter = 0;
	static uint8 BackCounter = 0;
	static uint16 CurErrCounter = 0;
	
	//过流 电机往上抬升  
	if (gCurAdcErr == TRUE)
	{	
		if (DC_MOTOR_FORWARD_OK == 0)
		{
			CurErrCounter++;
			if(CurErrCounter >= 3)	// 30ms
			{
				DC_MOTOR_CLOSE;
				DcMotorCloseFlag = 0xff;
			}	
			if (CurErrCounter >= 500)	// 5s 后，重新关闭入纸口
			{
				MotorCloseCount++;
				gCurAdcErr = FALSE;
				printf("\r\n two close motor\r\n");
				
				if (MotorCloseCount >= MOTOR_CLOSE_COUNT_MAX)		// 尝试2次关闭入纸口，关闭失败则不关闭
				{
					DcMotorCloseFlag = 0xff;
				}
				else 
				{
					PAPER_CLOSE;						// 入纸口关闭				
				}											
			}
		}
		else
		{
			CurErrCounter = 0;
		}
		return;//而且优先级高	
	}
	
	//开机检测是否到位
	if ((gGetWorkState & WORK_STAT_BIT) != WORKSTATE_CLOSE)
	{
		if (DC_MOTOR_FORWARD_OK == 0)
		{
			ForwardCounter++;
			if (ForwardCounter >= 3)	// 30ms
			{		
				DC_MOTOR_CLOSE;
				ForwardCounter = 200;	
				DcMotorCloseFlag = 0xff;
			}
		}
		else
		{
			ForwardCounter = 0;
		}
	}
	
	//关机检测是否到位
	if ((gGetWorkState & WORK_STAT_BIT) == WORKSTATE_CLOSE)
	{
		if (DC_MOTOR_BACK_OK == 0)
		{
			BackCounter++;
			if (BackCounter >= 3)	// 30ms
			{
				DC_MOTOR_CLOSE;
				BackCounter = 200;
				DcMotorCloseFlag = 0xff;
			}
		}
		else
		{
			BackCounter = 0;	
		}
	}	
}

/**
  * @brief   PWM输出初始化 ,MT_INT使能脚		
  * @param   arr：自动重装值
						 psc：时钟预分频数
  * @retval  void
  */
static void TIM3_PWM_Init(uint16 arr,uint16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3部分重映射  TIM3_CH2->PB5    
 
   //设置该引脚为复用输出功能,输出TIM3 CH2的PWM脉冲波形	GPIOB.5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
 
   //初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM3 Channel2 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; 			//输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2

	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
 
	TIM_Cmd(TIM3, DISABLE);  //失能TIM3
	//TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}

/**
  * @brief   直流电机IO口
	* @note 	 PB6,PB9检测入口纸门位置
						 PC8,PC9 控制电机正反转
						 PB5 电机启动使能脚
  * @param   void						
  * @retval  void
  */
void Motor_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	//HX1 -- PC9
	//HX2 -- PC8  电机正反转
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	//MT_ENA 电机使能端
	TIM3_PWM_Init(99,719);	// PWM频率：72Mhz/720/100 = 1Khz	
	
	DC_MOTOR_CLOSE;				//关闭直流电机	  
	
	//V1.0.0   PB3,PB4检测入口纸门位置
	//V2.0.0   PB6,PB9检测入口纸门位置
#if (VERSION == 1)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
#else 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9;
#endif
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
} 








