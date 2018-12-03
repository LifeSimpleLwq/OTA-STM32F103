#ifndef __BSP_OTA_H_
#define __BSP_OTA_H_

#include "main.h"



#define OTA_RX_LEN 								40 * 1024

// -------------- OTA_Run_Statu --------------
#define OTA_RXD_UART							0xFF
#define OTA_RXD_UART_OK						0xFE
#define OTA_WAIT									0X00

#define FLAG_NO_FIRMWARE					0xffff		
#define FLAG_RX_FIRMWARE					0x5555
#define FLAG_FIRMWARE							0x6666


struct OTA_struct
{
	uint8 RunStatu;
	
	uint16 APPSize;
	
	uint16 APPCount;
	
	uint16 oldCount;		
};

extern struct OTA_struct OTA;

extern uint16 Usart_Rx_Cnt;

extern uint8 OTA_RxBuf[OTA_RX_LEN];

typedef  void (*iapfun)(void);				//定义一个函数类型的参数.

void Iap_Load_App(uint32 Addr);
void check_Firmware(void);
void Receive_Firmware(void);


void BSP_Flash_Init(void);

#endif 


