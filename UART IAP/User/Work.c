#include "work.h"

static struct rt_thread Work_Thread;
static struct rt_thread LED_Thread;
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t Work_stack[512];
rt_uint8_t LED_stack[256];

/**
  * @brief  机器关闭，清除参数
  * @param  void
  * @retval None
  */
void Para_Init(void)
{	
	//接收到碎纸完成指令 电机延时
	gMotorDelayStopCounter = 0;

	gMcuWorkStep = MCU_SYS_CLOSE;

	fState &= (MOTOR_ERR | WEIGHER_ERR);
	
	SET_WORK_STATE_CLOSE;
	
	fSendWeight = FALSE;
	gSendWeightClass = 0xff;//0--上传单次重量 1--上传总共重量
}														
														
/**
  * @brief  机器运行，机制处理
  * @param  void
  * @retval None
  */
static void Check_State(void)
{
	uint8 buf = 0;
	
	/**************************************************************/
	/*************************异常处理*****************************/
	/**************************************************************/
	//电子称通信不成功
	buf = fState & WEIGHER_ERR;
	if (buf == WEIGHER_ERR)
	{
		if (gAckPcErr == FALSE)
		{
			//处于故障状态
			SET_WORK_STATE_ERR;
			//清除相关工作状态
			CLEAR_F_STATE;

			if (gSendStateCounter >= SEND_STATE_TIM)
			{
				gSendStateCounter = 0;

				// Usart2SendData[0] = 0x01;		// 电子秤通讯失败
				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_FAULT, 0);
				printf("发送电子秤通讯故障..\r\n");
			}
		}
		else 
		{
			switch (gMcuWorkStep)
			{
				case MCU_SYS_CLOSE:
					break;
				
				// 机器开始运行，进入称重
				case MCU_GET_CUT_PAPER_CMD:		//点击“开始碎纸”
				case MCU_CUT_PAPER:						//等待“碎纸完成” 
				case MCU_GET_CUT_DOWM_CMD:		//收到“碎纸完成” -> 电机延时	
				case MCU_GET_WEIGHT_TO_SEND:	//称重 			
				case MCU_TIME_CUT_DOWN:				//上传倒计时
								Motor_Stop();								
				
				// 没有开始运行，直接关机
				case MCU_RUN_PROCESS_OK:			//完成整一个过程			
				case MCU_SYS_OPEN:						//扫描二维码
								gMcuWorkStep = MCU_SYS_CLOSE;
								SET_WORK_STATE_CLOSE;					//停止工作			
								fMcuOpen = FALSE;											
											
								PAPER_CLOSE;						// 入纸口关闭		
								FAN_CLOSE;
								printf("电子秤通讯故障关机\r\n");
					break;
						
				default:
					 printf("ERROR: WEIGHER_ERR...\r\n");
					break;
			}
		}
		return;
	}
	
	// 变频器通讯故障
	buf = fState & MOTOR_ERR;
	if (buf == MOTOR_ERR)
	{
		if (gAckPcErr == FALSE)
		{
			//处于故障状态
			SET_WORK_STATE_ERR;
			//清除相关工作状态
			CLEAR_F_STATE;

			//每隔2S 发送一次ERR
			if (gSendStateCounter >= SEND_STATE_TIM)
			{		
				gSendStateCounter = 0;
				
				// Usart2SendData[0] = 0x02;		// 变频器通讯故障
				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_FAULT, 0);
				printf("发送变频器通讯故障..\r\n");			
			}
		}
		else 
		{
			switch (gMcuWorkStep)
			{
				case MCU_SYS_CLOSE:
					break;
				
				// 机器开始运行，进入称重
				case MCU_GET_CUT_PAPER_CMD:		//点击“开始碎纸”
				case MCU_CUT_PAPER:						//等待“碎纸完成” 			
				case MCU_TIME_CUT_DOWN:				//上传倒计时						
								//Motor_Stop();
								gMotorDelayStopCounter = 0;					// 应等待一段10s再进行称重
								fState |= SHREDDER_CUT_OK;
				
				case MCU_GET_WEIGHT_TO_SEND:	//称重 
				case MCU_GET_CUT_DOWM_CMD:		//收到“碎纸完成” -> 电机延时	
								goto	_SHREDDER_CUT_OK_FUN;							
												
				// 没有开始运行，直接关机
				case MCU_RUN_PROCESS_OK:			//完成整一个过程
				case MCU_SYS_OPEN:						//扫描二维码
								gMcuWorkStep = MCU_SYS_CLOSE;
								SET_WORK_STATE_CLOSE; 	//停止工作			
								fMcuOpen = FALSE;												
											
								PAPER_CLOSE;						// 入纸口关闭				
								FAN_CLOSE;
								printf("变频器通讯故障关机\r\n");
					break;
							
				default:
						printf("ERROR: MOTOR_ERR...\r\n");
					break;
			}
		}
		return;
	}
	
	// 外部变频器错误
	if (gEnternErrFlag == TRUE && fMcuOpen == TRUE)
	{
		if (gAckPcErr == FALSE)
		{
			//处于故障状态
			SET_WORK_STATE_ERR;
			//清除相关工作状态
			CLEAR_F_STATE;
			
			//每隔2S 发送一次ERR
			if (gSendStateCounter >= SEND_STATE_TIM)
			{
				gSendStateCounter = 0;

				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_FAULT, 0);
				printf("发送变频器错误..\r\n");		
			}
		}
		else 
		{
			switch (gMcuWorkStep)
			{
				case MCU_SYS_CLOSE:
					break;
				
				// 机器开始运行，进入称重
				case MCU_GET_CUT_PAPER_CMD:		//点击“开始碎纸”
				case MCU_CUT_PAPER:						//等待“碎纸完成” 		
				case MCU_TIME_CUT_DOWN:				//上传倒计时
								Motor_Stop();		
								gMotorDelayStopCounter = 0;					// 应等待一段10s再进行称重
								fState |= SHREDDER_CUT_OK;
				
				case MCU_GET_WEIGHT_TO_SEND:	//称重 	
				case MCU_GET_CUT_DOWM_CMD:		//收到“碎纸完成” -> 电机延时	
								goto	_SHREDDER_CUT_OK_FUN;								
				
				// 没有开始运行，直接关机
				case MCU_RUN_PROCESS_OK:			//完成整一个过程
				case MCU_SYS_OPEN:						//扫描二维码		
								gMcuWorkStep = MCU_SYS_CLOSE;
								SET_WORK_STATE_CLOSE; //停止工作			
								fMcuOpen = FALSE;												
																
								PAPER_CLOSE;					// 入纸口关闭				
								FAN_CLOSE;
								printf("变频器错误关机");
					break;
							
				default:
						printf("外部变频器错误...\r\n");
					break;
			}
		}
		return;
	}
	
	CLEAR_WORK_STATE_ERR;		// 清除机器故障
	
	// 堵转
	buf = fState & MOTOR_CUR_ERR;
	if (buf == MOTOR_CUR_ERR && fMcuOpen == TRUE)
	{
		printf("堵转...\r\n");
		
		if (Check_Motor_Lock_State() == 1)//仍然是堵转
		{	
			if (gAckPcErr == FALSE)
			{
				// 清除相关工作状态
				CLEAR_F_STATE;

				if (gSendStateCounter >= SEND_STATE_TIM)		// 每隔2S 发送一次			
				{			
					gSendStateCounter = 0;					
					printf("发送故障..\r\n");					
					Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_FAULT, 0);
				}			
			}		
			else 
			{
				gAckPcErr = FALSE;
				fState &= ~MOTOR_CUR_ERR;	
				fState |= SHREDDER_CUT_OK;				
				gMotorDelayStopCounter = MOTOR_DELAY_STOP_MAX;		
			}
		}
		else //不堵转
		{
			fState &= ~MOTOR_CUR_ERR;			
		}
		
		return;
	}
	gAckPcErr = FALSE;
	
	// 纸箱已满
	buf = fState & BIN_FULL;
	if (buf == BIN_FULL)		//处于满纸状态
	{
		if (gAckBinFull == FALSE)
		{
			//清除相关工作状态
			CLEAR_F_STATE;
			
			//定时2s 发送一次
			if (gSendStateCounter >= SEND_STATE_TIM) 
			{
				gSendStateCounter = 0;

				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_BIN_FULL, 0);
				printf("发送回收桶已满..\r\n");
//				Motor_Stop();
			}
		}
		else 
		{
//			fState &= ~BIN_FULL;
//			fState |= SHREDDER_CUT_OK;				
//			gMotorDelayStopCounter = 0;				// 回收桶满，应等待一段10s再进行称重
//			Motor_Stop();
		}		
	}
	
	//
	if (fMcuOpen != TRUE)	
	{
		fMotorPoAndNegEnd = 0;//0--没有堵转了  1--堵转了
		fMotorLock = 0;				//最终确认堵转  0--没有堵转 1--堵转了
		gEnternErrFlag = 0;
		return;	//关机	
	}		
	/**************************************************************/
	/***************************正常情况***************************/
	/**************************************************************/
	// 使能碎纸
	buf = fState & SHREDDER_CUT;		
	if (buf == SHREDDER_CUT)
	{
		fState &= ~SHREDDER_CUT;
		
		if (Check_Motor_Step(MOTOR_CMD_GO) == 1) //进入碎纸状态
		{		
			gMcuWorkStep = MCU_CUT_PAPER;			//表明进入 碎纸状态--接收到碎纸完成 之间的时间段
			printf("使能碎纸..\r\n");
		}
		else  //通信存在err 或者堵转
		{
			printf("使能碎纸失败..\r\n");
		}
		return;
	}
	
_SHREDDER_CUT_OK_FUN:	
	// 碎纸完成
	buf = fState & SHREDDER_CUT_OK;
	if (buf == SHREDDER_CUT_OK)
	{	
		if (gMotorDelayStopCounter >= MOTOR_DELAY_STOP_MAX)	//等待10S 碎纸
		{
			fState &= ~SHREDDER_CUT_OK;
			
			if(Motor_Stop() == 1)
			{
				printf("碎纸完成\r\n");
				
				// 进入称重状态
				fState  |= GET_WEIGHT_AND_SEND;
				gMcuWorkStep = MCU_GET_WEIGHT_TO_SEND;
			}
			else // 或者堵转
			{
				printf("堵转..\r\n");
			}			
		}
		else
		{		
			gMcuWorkStep = MCU_GET_CUT_DOWM_CMD;	//MCU处于电机10S延时中
		}
	}

	// 读取电子称数据
	buf = fState & GET_WEIGHT_AND_SEND;
	if(buf == GET_WEIGHT_AND_SEND)
	{
		return;
	}

	// 使能上传重量
	if (fSendWeight == TRUE)
	{
		SET_WORK_STATE_WORK;
		//每隔1S 发送一次
		if(gSendStateCounter >= 100)
		{
			gSendStateCounter = 0;
			//单次
			if (gSendWeightClass == 0)
			{
				Usart2SendData[0] = (uint8)(gOneWeightData >> 8);
				Usart2SendData[1] = (uint8)gOneWeightData;
				printf("发送单次重量 0x%x, %d\r\n", gOneWeightData, gOneWeightData);
				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_ONE_WEIGHT, 2);
			}
			else if (gSendWeightClass == 1)//总重量
			{
				Usart2SendData[0] = (uint8)(gAllWeightData >> 8);
				Usart2SendData[1] = (uint8)gAllWeightData;
				printf("发送总重量 0x%x, %d\r\n", gAllWeightData, gAllWeightData);
				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_ALL_WEIGHT, 2);
			}
		}
		return;
	}
	
	// 等待一段时后 没有人为确定 碎纸完成 通知上位机为开始倒计时
	// 在CheckCmd() 清除 TIME_CUT_DOWN
	buf = fState & TIME_CUT_DOWN;
	if (buf == TIME_CUT_DOWN)
	{
		//每隔1S 发送一次		
		if (gSendStateCounter >= (SEND_STATE_TIM/2))
		{
			gSendStateCounter = 0;
			Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_COUNT_DOWN, 0);
			printf("倒计时..\r\n");
		}
		return;
	}	
}

/**
  * @brief  Work_Thread_entry
  * @param  void  
  * @retval void
  */
void Work_Thread_entry(void *parameter)
{	
	uint8 time = 0;
	uint16 countTime = 0;
		
	while(1)
	{
		rt_thread_delay(RT_TICK_PER_SECOND/100);	// 10ms调用一次
	
		Check_State();
		
		if (fMcuOpen != TRUE)		//关机
		{					
			Para_Init();//清空参数
			
			if (countTime++ >= 500)	// 5s	停止一下电机
			{
				countTime = 0;
				Motor_Stop_Reset();						
			}
		}
		else 
		{
			if (time++ > 100) { time = 0; Check_Motor_Step(MOTOR_CMD_RESET);}
		}
	}
}


/**
  * @brief  Printf_Thread_entry
  * @param  void  
  * @retval void
  */
void LED_Thread_entry(void *parameter)
{
		uint8 i = 0;
		while(1)
		{
				if(fMcuOpen)
					rt_thread_delay(RT_TICK_PER_SECOND/4);	// 250ms调用一次
				else 
					rt_thread_delay(RT_TICK_PER_SECOND/2);	// 500ms调用一次
				
				IWDG_Feed();	//喂狗			
				Led_Run_Show();	
				
				Lock_Check();
				
				if (++i >= 4)
				{
					i = 0;					
//					printf("fState = %x\r\n", fState);
					printf("INFRARE_STATE = %d\r\n", INFRARE_STATE);		
//					printf("gMcuWorkStep = %d\r\n", gMcuWorkStep);		
				}
		}
}

/**
  * @brief  Work_Thread_Init: 优先级18 时间片20
						LED_Thread_Init : 优先级30 时间片20
  * @param  void  
  * @retval void
  */
void Work_Thread_Init(void)
{
		rt_err_t err_t;
		
		err_t = rt_thread_init(&Work_Thread,
													"Work",
													&Work_Thread_entry,
													RT_NULL,
													&Work_stack[0],
													sizeof(Work_stack),
													18,
													20);
													
		if (err_t == RT_EOK)
		{
				rt_thread_startup(&Work_Thread);
				printf("Work_Thread start success...\r\n");
		}	
		
		err_t =  rt_thread_init(&LED_Thread,
													"LED",
													&LED_Thread_entry,
													RT_NULL,
													&LED_stack[0],
													sizeof(LED_stack),
													30,
													20);	
		if (err_t == RT_EOK)
		{
				rt_thread_startup(&LED_Thread);
				printf("Printf_Thread start success...\r\n");
		}												
}








