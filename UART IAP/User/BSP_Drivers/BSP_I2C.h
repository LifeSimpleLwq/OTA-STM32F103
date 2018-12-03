#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "main.h"

#define delay_us(n)		BSP_Delay_us(n);

#ifdef _ENABLE_TEMP
//IO方向设置
#define SDA_IN()  		TEMP_SDA_IN()
#define SDA_OUT()		 	TEMP_SDA_OUT()

#define SDA_RESET			TEMP_SDA_RESET    		
#define SDA_SET    		TEMP_SDA_SET
#define SCL_RESET    	TEMP_SCL_RESET	
#define SCL_SET 			TEMP_SCL_SET

#define READ_SDA			TEMP_READ_SDA
#else 

#endif

//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);								//发送IIC开始信号
void IIC_Stop(void);	  						//发送IIC停止信号
void IIC_Send_Byte(uint8 txd);			//IIC发送一个字节
void IIC_Ack(void);									//IIC发送ACK信号
void IIC_NAck(void);								//IIC不发送ACK信号


uint8 IIC_Read_Byte(uint8 ack);			//IIC读取一个字节
uint8 IIC_Wait_Ack(void); 					//IIC等待ACK信号

void IIC_Write_One_Byte(uint8 daddr,uint8 addr,uint8 data);
uint8 IIC_Read_One_Byte(uint8 daddr,uint8 addr);	

#endif
















