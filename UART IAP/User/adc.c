#include "adc.h"

#define CUR_ADC_MAX  10

#if (VERSION == 1)
#define 	ADC_ADD_VALUE		50			
#else 
#define 	ADC_ADD_VALUE		30
#endif 

static uint16 CurAdcValuebuf[CUR_ADC_MAX]={0,0,0,0,0,0};
static uint16 gCurAdcValue,CUR_ADC_VALUE_MAX = 1800;
static uint16 sum = 0;
static uint16	AdcMax = 0,AdcMin = 0xffff;
static uint8 gCurAdcCounter = 0;

uint16 MotorDelay = 0;	// 入纸口电机启动瞬间延时
uint16 ADC_ConvertedValue[2] = {0,0};
uint8 gCurAdcErr = FALSE;   // TRUE 入纸口直流电机过流

/**
  * @brief  10ms调用一次，ADC_Thread_entry 中调用
  * @param  void  
  * @retval void
  */
static void Get_Cur_Adc_Value(void)
{
	CurAdcValuebuf[gCurAdcCounter] = ADC_ConvertedValue[0];
	
	if(CurAdcValuebuf[gCurAdcCounter] != 0)
	{	
		sum += CurAdcValuebuf[gCurAdcCounter];
		
		// Get ADC Max && Min value
		if (AdcMax < CurAdcValuebuf[gCurAdcCounter]) AdcMax = CurAdcValuebuf[gCurAdcCounter];
		if (AdcMin > CurAdcValuebuf[gCurAdcCounter]) AdcMin = CurAdcValuebuf[gCurAdcCounter];
		
		gCurAdcCounter++;
		if(gCurAdcCounter >= CUR_ADC_MAX)
		{	
			sum -= AdcMin;
			sum -= AdcMax;
			gCurAdcValue = sum / (CUR_ADC_MAX - 2);		
			//printf("gCurAdcValue = %d  %d  %d\r\n", gCurAdcValue,CUR_ADC_VALUE_MAX,gCurAdcErr);
			
			AdcMax = sum = 0;
			AdcMin = 0xFFFF;
			gCurAdcCounter = 0;
			
			if((gCurAdcValue >= CUR_ADC_VALUE_MAX) || (gCurAdcValue <= CUR_ADC_VALUE_MIN))
			{
				printf("过流过流  ");
				printf("gCurAdcValue = %d  %d  %d\r\n", gCurAdcValue,CUR_ADC_VALUE_MAX,gCurAdcErr);
				
				//过流 电机往上抬升
				gCurAdcErr = TRUE;
				DcMotorCloseFlag = 0;	 // 电机上抬，关闭读取电流					
				
				DC_MOTOR_CLOSE;
				DC_MOTOR_OPEN;
				MOTOR_ONE_GO_FORWARD;				
			} 
		}  						
	}
}


/**
  * @brief  10ms
  * @param  void  
  * @retval void
  */
void ADC_Thread_entry(void *parameter)
{
	while(1)
	{
		rt_thread_delay(RT_TICK_PER_SECOND/100);
		
		switch(DcMotorCloseFlag)
		{
			case 1:		// 入纸口正在关闭
						if (MotorDelay >= 50)
						{	
							Get_Cur_Adc_Value();//检测电流		
							MotorDelay = 50;	
//							printf("gCurAdcValue = %d  %d  %d\r\n", gCurAdcValue,CUR_ADC_VALUE_MAX,gCurAdcErr);							
						}
//						if (MotorDelay == 0) printf("gCurAdcValue = %d  %d \r\n", gCurAdcValue,CUR_ADC_VALUE_MAX);
						break;		
						
			case 0xff:		// 入纸口无动作
						Get_Cur_Adc_Value();
						if (gCurAdcValue > 200)
							CUR_ADC_VALUE_MAX = gCurAdcValue + ADC_ADD_VALUE;			
			 			break;
			
			case 0:			// 入纸口正在打开
			default:		
						break;
		}
		// 电机限位开关检测
		Check_Motor_Limit_State();
		
		// 入纸口红外检测
		Check_Infrared_Every_State();
		
#if (VERSION == 1) 		
		fBody &= ~BODY_HAVE;
#else 		
		// 人体感应检测
		Check_Body_State();		
#endif 		
	}
}

static struct rt_thread ADC_Thread;
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t ADC_stack[256];

/**
  * @brief  ADC_Thread_Init: 优先级15 时间片20
  * @param  void  
  * @retval void
  */
void ADC_Thread_Init(void)
{
		rt_err_t err_t;
		
		err_t = rt_thread_init(&ADC_Thread,
													"Work",
													&ADC_Thread_entry,
													RT_NULL,
													&ADC_stack[0],
													sizeof(ADC_stack),
													15,
													20);
													
		if (err_t == RT_EOK)
		{
				rt_thread_startup(&ADC_Thread);
				printf("ADC_Thread start success...\r\n");
		}	
}

/**
  * @brief  ADC_GPIO_Init:	PA7
  * @param  void  
  * @retval void
  */
static void ADC_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		  //模拟输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
}

/**
  * @brief  单次采样时间 1/9Mhz * (55.5+12.5) = 7.5us
  * @param  void  
  * @retval void
  */
static void ADC_Mode_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	/* DMA channel1 configuration */
	DMA_DeInit(DMA1_Channel1);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	 			//ADC地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue;	//内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//外设作为数据源
	DMA_InitStructure.DMA_BufferSize = 1;							    //本次传送2个数据
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	   //外设地址关闭自增功能
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  				//内存地址自增功能
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//16位，即半个字
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//16位，即半个字
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;										//循环传输
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	/* Enable DMA channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/* ADC1 configuration */	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;			//独立ADC模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE ; 	 				//禁止扫描模式，扫描模式用于多通道采集
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;			//开启连续转换模式，即不停地进行ADC转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//不使用外部触发转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 	//采集数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	 								//要转换的通道数目2
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/*配置ADC时钟，为PCLK2的8分频，即9MHz*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
	/*配置ADC1的通道  55.5个采样周期 */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7 , 1, ADC_SampleTime_55Cycles5);	//单次采样时间 1/9Mhz * (55.5+12.5) = 7.5us

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
	
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
	
	/*复位校准寄存器 */   
	ADC_ResetCalibration(ADC1);
	/*等待校准寄存器复位完成 */
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	/* ADC校准 */
	ADC_StartCalibration(ADC1);
	/* 等待校准完成*/
	while(ADC_GetCalibrationStatus(ADC1));
	
	/* 由于没有采用外部触发，所以使用软件触发ADC转换 */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


/**
  * @brief  ADC_Config
  * @param  void  
  * @retval void
  */
void ADC_Config(void)
{
	ADC_GPIO_Init();
	ADC_Mode_Init();
}

