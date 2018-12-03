#ifndef __ANDROID_H__
#define __ANDROID_H__

#include "main.h"

//带ACK是 单片机应答 安卓
//没有ACK是 安卓应答 单片机
#define ANDROID_COMMUN_PROT_FH	  0x24
#define ANDROID_COMMUN_PROT_FT    0xFF


#define SHREDDER_GET_STATE	     	(unsigned int)(0x5905)
#define SHREDDER_ACK_GET_STATE   	(unsigned int)(0x4905)	
	
#define SHREDDER_LOCK							(unsigned int)(0x5907)
	
// OTA cmd
#define OTA_VERSION_GET						(unsigned int)(0x5951)
	
#define OTA_VERSION_UPDATE				(unsigned int)(0x5952)
	
#define	OTA_PACK_SIZE							(unsigned int)(0x5953)

#define	OTA_PACK_RX								(unsigned int)(0x5954)
	

extern uint8 Usart2SendData[10];
extern uint8 Usart2RxData[12];
extern uint8 fUsart2Rx;

uint8 Android_Uart_Send_Data(uint8*dat, uint16 cmd, uint16 datnum);
void Android_Thread_Init(void);
void Check_Cmd_Android(void);

#endif







