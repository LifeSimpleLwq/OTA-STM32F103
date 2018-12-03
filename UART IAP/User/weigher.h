#ifndef __WEIGHER_H__
#define	__WEIGHER_H__

#include "main.h"

//电子称
//(uint8)(0x41) //'A'
#define ALL_WEIGHT_CMD        (uint8)(0x41)

//(uint8)(0x57) //'W'
#define INCREASE_WEIGHT_CMD    (uint8)(0x57)

#define ALL_WEIGHT_FULL  		(float)(30000.0)//(float)(80000.0)
#define ALL_WEIGHT_NO_FULL  (float)(28000.0)

extern uint8 Usart4RxData[9];
extern uint8 fUsart4Rx;

extern uint8 gSendWeightClass;//0--上传单次重量 1--上传总共重量
extern uint8 fSendWeight;

extern uint16 gAllWeightData;
extern uint16 gOneWeightData;

void Weigth_Thread_Init(void);

#endif








