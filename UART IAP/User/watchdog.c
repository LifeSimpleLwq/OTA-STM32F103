#include "watchdog.h"

/*
设置IWDG_PR和IWDG_RLR的初值。
我们计算一下看门狗的喂狗时间（看门狗溢出时间）计算公式：
Tout=((4*2^prer)*rlr)/40
其中Tout就是看门狗溢出时间（单位ms），
prer是看门狗时钟预分频值(IWDG_PR值)，
范围为0~7，rlr位看门狗重载值(IWDG_RLR)。
比如我们设置prer为4, 
rlr的值为625，我们就可以计算得到       
Tout=64*625/40=1000ms，这样，看门狗的溢出时间就是1S，只要在这一秒钟  
内，有一次写入0XAAAA到IWDG_KR，就不会导致看门狗复位。
*/
void IWDG_Init(uint8 prer,uint32 rlr)
{
	IWDG->KR=0X5555;          
	IWDG->PR=prer;        
	IWDG->RLR=rlr;           
	IWDG->KR=0XAAAA;     
	IWDG->KR=0XCCCC;     
}

void IWDG_Feed(void)
{
    IWDG->KR=0XAAAA;                                  
}



