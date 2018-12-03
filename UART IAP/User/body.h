#ifndef __BODY_H__
#define __BODY_H__

#include "main.h"

// BODY_IN	检测到人时，高电平，否则
#define BODY_STATE   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12)

#define 		BODY_HAVE				0x01
#define 		INF_HAVE				0x02

extern uint8 fBody;

void Check_Body_State(void);
void Body_GPIO_Init(void);

#endif 

