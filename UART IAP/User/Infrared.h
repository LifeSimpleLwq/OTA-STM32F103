#ifndef __INFRARED_H__
#define	__INFRARED_H__

#include "main.h"

// INF_IN
#define INFRARE_STATE   	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)  	

#define NO_PAPER_COUNTER_MAX    		1000	// NO_PAPER_COUNTER_MAX * 10ms

#define CLEAR_NO_PAPER_COUTER				{gNoPaperCounter = 0;}		// 清除入口纸检测计数器
	

extern uint16 gNoPaperCounter;

void Check_Infrared_Every_State(void);
void Infrared_GPIO_Init(void);

#endif






