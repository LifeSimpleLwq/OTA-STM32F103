
Project: OTA BootLoader Rx
Ver: OTA_BootLoader_Rx  V1.0.0
描述：从机通过24L01模块接收固件进行升级，单片机运行Bootloader时，LED灯闪烁

从机接收固件方式：NRF

接收端添加固件长度，自动检测固件大小
添加分片接收，修改Single_SendSize即可修改单次接收大小

添加bootloader运行状态检查
0xD0D0	直接跳转到已有的固件
0xFFFF	没有固件，等待接收固件
0xD1D1	升级固件，等待数据包

/**********硬件资源***************

MCU: STM32F103C8T6
SRAM:20KB 	FLASH:64KB
无线模块：NRF24L01

**********************************/

/**********程序设计****************
1，接收固件占用SRAM:	5KB（SRAM）
2，bootloader占用： 	8KB (FLASH) 预留：	20KB（FLASH）
3, 写入1KB数据到flash需要250ms

**********************************/

/**********bug调试修复************
bug1：设置UART为115200，串口检测43000才能读取
	  设置HSE时钟为8MHz，问题解决

bug2：2个NRF24L01通讯出现掉包。
	  由于电源为电脑电源，导致NRF工作不稳定 
**********************************/

/************更新历史*************
Ver: OTA_BootLoader_Rx  V1.1.0


**********************************/
G:\project\OTA\APP


/**************************生成bin文件指令********************************

1，SCB->VTOR = FLASH_BASE | 0x3C00; /* Vector Table Relocation in Internal FLASH. */

2，E:\Keil_v5\ARM\ARMCC\bin\fromelf.exe   --bin -o  ..\OBJ\RTC.bin ..\OBJ\RTC.axf

********************************************************************/

