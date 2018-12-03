#ifndef __MAIN_H__
#define	__MAIN_H__

#include "stm32f10x_it.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h" 

#include "misc.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

typedef unsigned          	char uint8;
typedef unsigned short    	int  uint16;
typedef unsigned           	int  uint32;

#include "usart.h"
#include "android.h"
#include "led.h"
#include "main.h"
#include "watchdog.h"

#include "BSP_flash.h"
#include "BSP_OTA.h"

// ------------------- Bootloader version -----------------
#define VERSION							1
#define SUBVERSION					0
#define REVISION						0

#define OTA_ANDROID
//#define OTA_UART

void BSP_Delay_us(uint32 nus);

#endif





