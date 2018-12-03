#ifndef __WORK_H__
#define	__WORK_H__

#include "main.h"

#define SEND_STATE_TIM		200

#define CLEAR_F_STATE			{	fState &= ~SHREDDER_CUT;\
														fState &= ~SHREDDER_CUT_OK;\
														fState &= ~GET_WEIGHT_AND_SEND;\
														fState &= ~TIME_CUT_DOWN;}

void Work_Thread_Init(void);
void Para_Init(void);
														
#endif

