#include "temperature.h"

/**
  * @brief 	Temperature_Config
  * @param  void  
  * @retval void
  */
static void Temperature_Config(void)
{
		// Config register
		IIC_Start();
		IIC_Send_Byte(TEMPERATURE_ADDR_W);
		IIC_Wait_Ack();
		IIC_Send_Byte(CONFIG_ADDR);
		IIC_Wait_Ack();
		IIC_Send_Byte(0);
		IIC_Wait_Ack();
		IIC_Stop();	
}

/**
  * @brief  Get_Temperature
  * @param  void  
  * @retval void
  */
void Get_Temperature(void)
{
		uint8	temp[2] = {0,0};
	
		IIC_Start();
		IIC_Send_Byte(TEMPERATURE_ADDR_W);
		IIC_Wait_Ack();
		IIC_Send_Byte(TEMP_ADDR);
		IIC_Wait_Ack();
		
		TEMP_SDA_OUT();
		TEMP_SDA_SET;
		BSP_Delay_us(1);
		TEMP_SCL_SET;
		BSP_Delay_us(1);
		TEMP_SDA_RESET;
		BSP_Delay_us(1);
		TEMP_SCL_RESET;
	
		IIC_Send_Byte(TEMPERATURE_ADDR_R);
		IIC_Wait_Ack();
		temp[0] = IIC_Read_Byte(1);
		temp[1] = IIC_Read_Byte(0);
		IIC_Stop();
		printf("%d.%d\r\n",temp[0],(temp[1]>>5)*125);		
}

/**
  * @brief  EE_SDA -- PB0  EE_SCL -- PB1
	* @note 	LM57A上电后需要等待1min，读取温度才稳定
  * @param  void  
  * @retval void
  */
void Temperature_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	TEMP_SDA_SET;
	TEMP_SCL_SET;	
	
	Temperature_Config();
}





