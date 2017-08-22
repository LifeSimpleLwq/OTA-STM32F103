#ifndef __BSP_MALLOC_H
#define __BSP_MALLOC_H

#include "SYS.h"

//定义一个内存池
#define SRAMIN	 0		//内部内存池


#define SRAMBANK 	2	//定义支持的SRAM块数.	

//mem1内存参数设定.mem1完全处于内部SRAM里面.
#define MEM1_BLOCK_SIZE			32  	  						//内存块大小为32字节
#define MEM1_MAX_SIZE			10*1024  						//最大管理内存 40K
#define MEM1_ALLOC_TABLE_SIZE	MEM1_MAX_SIZE/MEM1_BLOCK_SIZE 	//内存表大小

//mem2内存参数设定.mem2的内存池处于外部SRAM里面
#define MEM2_BLOCK_SIZE			32  	  						//内存块大小为32字节
#define MEM2_MAX_SIZE			960 *1024  						//最大管理内存960K
#define MEM2_ALLOC_TABLE_SIZE	MEM2_MAX_SIZE/MEM2_BLOCK_SIZE 	//内存表大小 

//内存管理控制器
struct _m_mallco_dev
{
	void (*init)(u8);							//初始化
	u8 (*perused)(u8);		  	    //内存使用率
	u8 	*membase[SRAMBANK];				//内存池 管理SRAMBANK个区域的内存
	u16 *memmap[SRAMBANK]; 				//内存管理状态表
	u8  memrdy[SRAMBANK]; 				//内存管理是否就绪
};
extern struct _m_mallco_dev mallco_dev;	 //在mallco.c里面定义


void my_mem_init(u8 memx);				//内存管理初始化函数(外/内部调用)
u8 my_mem_perused(u8 memx);				//获得内存使用率(外/内部调用) 
u32 my_mem_malloc(u8 memx,u32 size);	//内存分配(内部调用)
u8 my_mem_free(u8 memx,u32 offset);		//内存释放(内部调用)
////////////////////////////////////////////////////////////////////////////////
//用户调用函数
void myfree(u8 memx,void *ptr);  			//内存释放(外部调用)
void *mymalloc(u8 memx,u32 size);			//内存分配(外部调用)


#endif


