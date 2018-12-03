//称重功能优化：
//现象：回收桶纸有水分，一段时间后挥发，会影响下一个用户称重数值。
//优化：开机时，读取一次单次重量，即清除挥发的数值。

//现象：回收桶拿掉后，电子秤会是负数。
//优化：值为负数时，上报重量为0
#include "weigher.h"

const uint8 AsiiData[13] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', '+', '-'};
static uint8 Usart4RxDataBuf[9] = {0};
static int8 gOverWeightCounter = 0;//重量消抖

uint8 Usart4RxData[9] = {0};
uint8 fUsart4Rx = 0;

uint8 gSendWeightClass = 0xff;//0--上传单次重量 1--上传总共重量
uint8 fSendWeight = FALSE;
//是否已经成功发送 回收桶已满 的信息

uint16 gAllWeightData = 0;
uint16 gOneWeightData = 0;

/**
  * @brief  串口通讯，发送数据
  * @param  
  * @retval None
  */
static void Weigher_Uart_Send_Data(uint8 dat)
{
	while (USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
	USART_SendData(UART4, dat);
}

/**
  * @brief  重量数据转换
  * @param  
  * @retval None
  */
static float Asii_To_Num(uint8 *asii, uint8 len)
{
	int32 buf = 0;
	uint8 i = 0, j = 0;
	uint8 dat = 0;
	
	for(i = 0; i < len; i++)
	{
		dat = (uint8)(*asii);
		
		for(j = 0; j < 13; j++)
		{
			if(dat == AsiiData[j])
			{
				if(j < 10)
				{
					buf	+= j;
					buf *= 10;
				}
				else if(j == 12) //负数
				{
						return (float)0;
				}

				break;
			}
		}

		asii++;
	}
	
	return (float)(buf * 1.0 / 1000);	
}

/**
  * @brief  获取总重量 or 单次重量
  * @param  cmd ：ALL_WEIGHT_CMD，INCREASE_WEIGHT_CMD
  * @retval 0 没有处理数据，1  处理数据
  */
static uint8 Check_Cmd_Weigher(uint8 cmd)
{
	float buf = 0;

	//串口4 电子称
	if(fUsart4Rx == TRUE)
	{
		fUsart4Rx = FALSE;
		
		memset(&Usart4RxDataBuf[0], 0, sizeof(Usart4RxDataBuf));
		memcpy(&Usart4RxDataBuf[0], &Usart4RxData[0], sizeof(Usart4RxData));
		memset(&Usart4RxData[0], 0, sizeof(Usart4RxData));

		buf = Asii_To_Num(&Usart4RxDataBuf[0], 8);

		switch(cmd)
		{
			case ALL_WEIGHT_CMD://重总量
					gAllWeightData = (int16)(buf * 1000);
//     		 	printf("Get all weight = %d\r\n", gAllWeightData);

					//连续判断超过最大值
					if (gAllWeightData >= ALL_WEIGHT_FULL)
					{
						gOverWeightCounter++;
						if(gOverWeightCounter >= 2)
						{
							gOverWeightCounter = 3;
						}
					}
					else if (gAllWeightData < ALL_WEIGHT_NO_FULL)
					{
						//没有超过最大值
						gOverWeightCounter--;
						if(gOverWeightCounter <= 0)
						{
							gOverWeightCounter = 0;
						}
					}
				break;

			case INCREASE_WEIGHT_CMD:	//单次数据
					gOneWeightData = (int16)(buf * 1000);
//					printf("Get one buf = %d\r\n", gOneWeightData);
				break;

			default:
				break;
		}		
		return 1;
	}
	return 0;
}

/**
  * @brief  读取一次单次重量
  * @param 	void
  * @retval 0--通信失败 1--成功
  */
static uint8 Get_Weight_One(void)
{
	uint8 counter = 0;
	uint8 errcount = 0;
	
#if !WEIGHER_SWITCH		
	return 1;
#else 	
	
	//单次
	Weigher_Uart_Send_Data(INCREASE_WEIGHT_CMD);
	counter = 0;
	errcount = 0;
	
	while(1)
	{	
		rt_thread_delay(100);		
		
		if (Check_Cmd_Weigher(INCREASE_WEIGHT_CMD) == 1)
		{
			break;
		}

		counter++;
		if(counter >= 5)			// 500ms
		{
			errcount++;
			Weigher_Uart_Send_Data(INCREASE_WEIGHT_CMD);
			counter = 0;
		}

		if(errcount >= 4)			// 500*4 = 2s
		{
			printf("电子秤通讯超时...\r\n");
			return 0;                            //退出该循环
		}
	}
	return 1;
	
#endif	
}

/**
  * @brief  读取一次总重量
  * @param 	void
  * @retval 0--通信失败 1--成功
  */
static uint8 Get_Weight_All(void)
{
	uint8 counter = 0;
	uint8 errcount = 0;
	
#if !WEIGHER_SWITCH		
	return 1;
#else 			
	
	Weigher_Uart_Send_Data(ALL_WEIGHT_CMD);
	counter = 0;
	errcount = 0;
	
	while(1)
	{
		rt_thread_delay(100);		
		
		if (Check_Cmd_Weigher(ALL_WEIGHT_CMD) == 1)
		{
			break;
		}

		counter++;
		if (counter >= 4)		// 等待400ms
		{
			errcount++;
			Weigher_Uart_Send_Data(ALL_WEIGHT_CMD);
			counter = 0;
		}

		if (errcount >= 4)		// 400 * 4  = 1.6s
		{	
			printf("电子秤通讯超时...\r\n");
			return 0;                            //退出该循环
		}
	}
	
	return 1;
	
#endif	
}

/**
  * @brief  判断是不是需要清空垃圾桶
	* @note   垃圾箱状态：fState：BIN_FULL
						未满：	gOverWeightCounter <= 0
						已满 未上传数据：	gAckBinFull = FALSE	gOverWeightCounter >= 2
						已满 上传数据：		gAckBinFull = TRUE
  * @retval void
  */
static void Check_Bin_State(void)
{
	static uint8 clearweight = 0;

	//已清空或者没满
	if (gOverWeightCounter <= 0)
	{
		gGetWorkState &= ~WORKSTATE_BIN_FULL;	
		gAckBinFull = FALSE;
		//清空
		if (clearweight == 1)
		{
			clearweight = 0;
		}
		return;
	}

	//已满 上传成功
	if (gAckBinFull == TRUE)
	{
		clearweight = 1;
		return;
	}

	//已满 未上传成功
	if ((gOverWeightCounter >= 2) && (gAckBinFull == FALSE))
	{
		gGetWorkState |= WORKSTATE_BIN_FULL;
		//if(fMcuOpen == TRUE)	//开机
			fState |= BIN_FULL;
	}
}

/**
  * @brief  读取总重量  and 单次重量
	* @note   
  * @retval void
  */
static uint8 Get_All_One_Weight(void)
{			
		if (!Get_Weight_One())	
		{
			fState |= WEIGHER_ERR;			//仍是不通讯成功 报错误
			printf("Weigher Error... gStateFlag = %x\r\n",fState);
			return 0;
		}
		else 
		{
			Get_Weight_All();
			printf("gOneWeightData = %d,gAllWeightData = %d\r\n\n",gOneWeightData,gAllWeightData);
		}	
		return 1;
}

/**
  * @brief  当安卓不请求读取重量，MCU定时读取总重量（定时1s 读取总重量 ）
  * @param 	void
  * @retval void
  */
static void Time_Get_Weigth(void)
{
		static uint8 WeightCount = 0,ErrCount = 0;
			
		if (Get_Weight_All())
		{
//				fState &= ~WEIGHER_ERR;
//			printf("gAllWeightData = %d\r\n",gAllWeightData);
		}
		else 
		{
				ErrCount++;
//				fState |= WEIGHER_ERR;			//仍是不通讯成功 报错误
		}
		
		WeightCount++;
		if (WeightCount >= 10)		// 10次读重，2次读取不到重量则认为线路老化，通讯失败
		{			
			if (ErrCount >= 2)	fState |= WEIGHER_ERR;			
			else fState &= ~WEIGHER_ERR;		
			
			ErrCount = 0;
			WeightCount = 0;
		}	
}

/**
  * @brief  工作完成，获取重量
  * @param 	void
  * @retval void
  */
static void Get_weight_work_OK(void)
{
	uint8 buf;
	
	//读取电子称数据
	buf = fState & GET_WEIGHT_AND_SEND;
	if(buf == GET_WEIGHT_AND_SEND)
	{
		rt_thread_delay(3000);
		if(Get_All_One_Weight() == 1)
		{
			SET_WORK_STATE_WORK;
			printf("读取重量完毕..\r\n");

			//进入下一步 上传总重量
			fSendWeight = TRUE;
			gSendWeightClass = 0;
		}
		else  //存在err
		{
			SET_WORK_STATE_ERR;					// 电子秤通讯错误				
			printf("EROR: Get Weight err...\r\n");
		}
		
		fState &= ~GET_WEIGHT_AND_SEND;
		return;
	}
}

static struct rt_thread Weigth_Thread;
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t Weigth_stack[512];

/**
  * @brief  Weigth_Thread_entry
  * @param 	void
  * @retval void
  */
void Weigth_Thread_entry(void *parameter)
{
	uint8 tim1s = 0;
	uint8 fStartRead;
	
	while(1)
	{
			rt_thread_delay(RT_TICK_PER_SECOND/100);	// 10ms调用一次
 		
			Get_weight_work_OK();
		
			if (fMcuOpen == TRUE)		
			{
				if (fStartRead)					// 扫码开机，未点击“开始碎纸”前读一次重量
				{
					fStartRead --;
					Get_All_One_Weight();
				}
			}
			else 
				fStartRead = 1;
			
			if (++tim1s >= 200 && !fMcuOpen)	// 2s 读取一次总重量
			{
				tim1s = 0;			
				Time_Get_Weigth();		
				Check_Bin_State();							//判断是否需要清空垃圾桶	
			}			
	}
}

/**
  * @brief  Weigth_Thread_Init:	优先级22 时间片20
  * @param 	void
  * @retval void
  */
void Weigth_Thread_Init(void)
{
		rt_err_t err_t;
		
		err_t = rt_thread_init(&Weigth_Thread,
													"Work",
													&Weigth_Thread_entry,
													RT_NULL,
													&Weigth_stack[0],
													sizeof(Weigth_stack),
													22,
													20);
													
		if (err_t == RT_EOK)
		{
				rt_thread_startup(&Weigth_Thread);			
				printf("Weigth_Thread start success...\r\n\r\n");
		}	
}


