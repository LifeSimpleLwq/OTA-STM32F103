#include "BSP_I2C.h"

/**
  * @brief  产生IIC起始信号
  * @param  void  
  * @retval void
  */
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	SDA_SET;	  	  
	SCL_SET;
	delay_us(4);
 	SDA_RESET;		//START:when CLK is high,DATA change form high to low 
	delay_us(4);	
	SCL_RESET;		//钳住I2C总线，准备发送或接收数据 
}	  

/**
  * @brief  产生IIC停止信号
  * @param  void  
  * @retval void
  */
void IIC_Stop(void)
{
	SDA_OUT();	//sda线输出
	SCL_RESET;
	SDA_RESET;	//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	SCL_SET; 
	SDA_SET;		//发送I2C总线结束信号
	delay_us(4);							   	
}

/**
  * @brief  等待应答信号到来
  * @param  void  
  * @retval 1，接收应答失败
			0，接收应答成功
  */      
uint8 IIC_Wait_Ack(void)
{
	uint8 ucErrTime = 0;
	SDA_IN();      //SDA设置为输入  
	
	SDA_SET;
	delay_us(1);	   
	SCL_SET;
	delay_us(1);	
		
	while (READ_SDA)
	{
		ucErrTime++;
		delay_us(1);
		
		if (ucErrTime > 25)
		{
			IIC_Stop();
			return 1;
		}
	}
	
	SCL_RESET;		//时钟输出0 	   
	return 0;  
} 

/**
  * @brief  产生ACK应答
  * @param  void  
  * @retval void
  */
void IIC_Ack(void)
{
	SCL_RESET;
	SDA_OUT();
	SDA_RESET;
	delay_us(2);
	SCL_SET;
	delay_us(2);
	SCL_RESET;
}

/**
  * @brief 	不产生ACK应答		
  * @param  void  
  * @retval void
  */   
void IIC_NAck(void)
{
	SCL_RESET;
	SDA_OUT();
	SDA_SET;
	delay_us(2);
	SCL_SET;
	delay_us(2);
	SCL_RESET;
}					

/**
  * @brief 	IIC发送一个字节,返回从机有无应答  (高位先发)
  * @param  void  
  * @retval 1，有应答
						0，无应答			
  */	  
void IIC_Send_Byte(uint8 txd)
{                        
	uint8 t;   
	
	SDA_OUT(); 	    
	SCL_RESET;	//拉低时钟开始数据传输
	
	for (t = 0; t < 8; t++)
	{              
		if (( txd & 0x80) >> 7)
			SDA_SET;
		else
			SDA_RESET;
		
		txd <<= 1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		SCL_SET;
		delay_us(2); 
		SCL_RESET;	
		delay_us(2);
	}	 
} 

/**
  * @brief 	读1个字节，ack=1时，发送ACK，ack=0，发送nACK   		
  * @param  void  
  * @retval void
  */	    
uint8 IIC_Read_Byte(uint8 ack)
{
	uint8 i,receive = 0;
	
	SDA_IN();//SDA设置为输入
	
  for (i = 0; i < 8; i++ )
	{
    SCL_RESET; 
    delay_us(2);
		SCL_SET;
    receive <<= 1;
		
    if (READ_SDA)	receive++; 
		
		delay_us(1); 
  }					
	
	if (!ack)
			IIC_NAck();//发送nACK
	else
			IIC_Ack(); //发送ACK   
	
	return receive;
}









