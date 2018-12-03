#ifndef __ADC_H__
#define	__ADC_H__

#include "main.h"

#define ADC1_DR_Address    			((u32)0x40012400+0x4c)
#define CUR_ADC_VALUE_MIN   		(uint16)900 



extern uint16 MotorDelay;
extern uint8 gCurAdcErr; 

void ADC_Config(void);
void ADC_Thread_Init(void);

#endif


