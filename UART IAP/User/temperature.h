#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include "main.h"

#ifndef   _ENABLE_TEMP
#define 	_ENABLE_TEMP
#endif 

#define 	TEMPERATURE_ADDR_W			0x92
#define 	TEMPERATURE_ADDR_R			0x93

#define 	CONFIG_ADDR							0x01
#define 	TEMP_ADDR								0x00
#define 	OVER_ADDR								0x03


//IO·½ÏòÉèÖÃ
#define 	TEMP_SDA_IN()  					{ GPIOB->CRL &= 0XFFFFFFF0; \
																		GPIOB->CRL |= (uint32)8 << 0; }
#define 	TEMP_SDA_OUT() 					{ GPIOB->CRL &= 0XFFFFFFF0; \
																		GPIOB->CRL |= (uint32)3 << 0; }

#define 	TEMP_SDA_RESET					GPIO_ResetBits(GPIOB,GPIO_Pin_0)
#define 	TEMP_SDA_SET						GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define		TEMP_SCL_RESET					GPIO_ResetBits(GPIOB,GPIO_Pin_1)
#define		TEMP_SCL_SET						GPIO_SetBits(GPIOB,GPIO_Pin_1)
																	
#define 	TEMP_READ_SDA						GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)

void Temperature_Init(void);
void Get_Temperature(void);
																		
#endif 










