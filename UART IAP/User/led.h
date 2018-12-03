#ifndef LED_H
#define	LED_H

#include "main.h"

#define LOCK_TIME_MAX		4+8*2		// 变量1s增加8次，故开锁时间： 8*2S = 16  最大不能超过5s，即40
#define CLEAR_LOCK			{if (Lock_Time >= LOCK_TIME_MAX + 8) Lock_Time = 0;}	


// 风扇pin	P24
#define FAN_OPEN      	GPIO_SetBits(GPIOB,GPIO_Pin_8)  
#define FAN_CLOSE				GPIO_ResetBits(GPIOB,GPIO_Pin_8)

// 电子锁pin	P21
#define LOCK_OPEN      	GPIO_SetBits(GPIOB,GPIO_Pin_4)  
#define LOCK_CLOSE			GPIO_ResetBits(GPIOB,GPIO_Pin_4)

extern uint8 Lock_Time;

void Led_Run_Show(void);
void Led_GPIO_Init(void);

void FAN_GPIO_Init(void);

void LOCK_GPIO_Init(void);
void Lock_Check(void);

#endif

