#ifndef __BSP_OTA_H
#define __BSP_OTA_H

#include "sys.h"
#include "BSP_flash.h"
#include "BSP_24l01.h"

#define BSP_FLASHWrite_Addr 0x08003C00  // 预留20KB bootloader
#define Single_DataSize		28
#define Single_SendSize  7*1024		// 单次发送最大长度


extern u8 NRF_Sign; 	// NRF模式标识位：0接收 or 1发射
extern u16 USART_RX_CNT;
extern u16 AppLenth;
typedef  void (*iapfun)(void);				//定义一个函数类型的参数.

void check_Firmware(void);				// 检查固件是否接收完毕
void Group_SendFirmware(void);		// 群组更新固件
void Group_UpdateID(u8 tmp);  		// 更新接收端ID	
void Count_SendNum(void);	// 计算发送次数,根据RX端接收器SIZE设计
u8 Send_Firmware(void);		// 发送固件


void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize);		// APP写入FLASH
void iap_load_app(u32 appxaddr);		// 跳转到应用程序段

void OTA_RxMode(void);		// 初始化NRF为接收端
void OTA_TxMode(void);		// 初始化NRF为发送端

#endif





