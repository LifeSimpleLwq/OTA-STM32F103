#include "BSP_24l01.h"
#include <string.h>

u8  OTA_RX[OTA_RX_LEN] __attribute__ ((at(0X20001000)));	//接收缓冲,最大OTA_RX_LEN个字节,起始地址为0X20001000. 
u8  SPI_RX_BUF[SPI_REC_LEN];
u8  SPI_TX_BUF[SPI_REC_LEN];

u8 TX_ADDRESS[TX_ADR_WIDTH]; //发送地址
u8 RX_ADDRESS[RX_ADR_WIDTH];

/**
  * @brief  NRF24L01接收数据
  * @retval 0 没有数据 or 1 接收到数据
  */
u8 NRF24L01_Rx(u16 x)
{	  		    		  
	u8 i = 0;
	u16 Index;
	Index = x *32;
	
	if(NRF24L01_RxPacket(SPI_RX_BUF) == 0)	//一旦接收到信息,则显示出来.
	{
		for (i = 0; i<32; i++)
			OTA_RX[Index + i] = SPI_RX_BUF[i];	// 保存到SRAM
		return 1; 
	}
	return 0;
}

/**
  * @brief  NRF24L01发送固件长度
  * @retval 0,发送成功 or 1,发送失败
  */
u8 NRF24L01_TxLenth(u8 * x)
{  
		u8 i = 0;

		for (i = 0; i<32; i++)
			SPI_TX_BUF[i] = x[i];
		
		if(NRF24L01_TxPacket(SPI_TX_BUF) == TX_OK)	
		{
			printf("发送长度成功！\r\n"); 
			return 0;
		}
		else	
		{
			return 1;
		}
}

/**
  * @brief  NRF24L01发送数据
  * @retval 0,发送成功 or 1,发送失败
  */
u8 NRF24L01_Tx(u16 x)
{  
	u8 i = 0;
	
	SPI_TX_BUF[0] = x % 256;
	SPI_TX_BUF[1] = x / 256;	
	
	for (i = 0; i<Single_DataSize; i++)
	{
		SPI_TX_BUF[3 + i] = OTA_RX[x *Single_DataSize + i];
	}

	if(NRF24L01_TxPacket(SPI_TX_BUF) == TX_OK)
	{
		printf("发送中 %3d %3d\r\n",SPI_TX_BUF[4],SPI_TX_BUF[1]*256+SPI_TX_BUF[0]);  // 调试使用
		return 0;
	}else
	{										   	
		printf("%d ",SPI_TX_BUF[0]);
		return 1;
	}
}

/**
  * @brief  启动NRF24L01发送一次数据
  * @param  txbuf:待发送数据首地址
  * @retval 发送完成状况
  */
u8 NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta;

	NRF24L01_CE(RESET);
	NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  TX_PLOAD_WIDTH:32个字节
 	NRF24L01_CE(SET);//启动发送	  

	while(NRF24L01_IRQ!=0);//等待发送完成

	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值	   
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	
	if(sta&MAX_TX)//达到最大重发次数
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);	//清除TX FIFO寄存器 
		return MAX_TX; 
	}
	
	if(sta&TX_OK)//发送完成
	{
		return TX_OK;
	}
	
	return 0xff;//其他原因发送失败
}

/**
  * @brief  启动NRF24L01发送一次数据
  * @param  txbuf:待发送数据首地址
  * @retval 0，接收完成；其他，错误代码
  */
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;	
	
	//SPI2_SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值    	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志

	if(sta&RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(FLUSH_RX,0xff);//清除RX FIFO寄存器 
		return 0; 
	}	   
	return 1;//没收到任何数据
}					 


/**
  * @brief  初始化24L01的IO口
  * @retval NONE
  */
void NRF24L01_Init(void)
{
	HAL_GPIO_WritePin(NRF_IRQ_GPIO_Port,NRF_IRQ_Pin,GPIO_PIN_SET);	// NRF_IRQ_Pin上拉

	while(NRF24L01_Check())
	{
		printf("NRF24L01 Error\r\n");
		HAL_Delay(200);
	}
	printf("NRF24L01 Init OK\r\n");
}

/**
  * @brief  检测24L01是否存在
  * @param  txbuf:待发送数据首地址
  * @retval 0，成功 or 1，失败
  */
u8 NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址
		
	for(i=0;i<5;i++) if(buf[i]!=0XA5) break;	 							   
	if(i!=5) return 1;	//检测24L01错误	
	return 0;		 //检测到24L01

}

/**
  * @brief  在指定位置写指定长度的数据
  * @param  reg:寄存器(位置)
	* @param	*pBuf:数据指针
	* @param	len:数据长度
  * @retval 此次读到的状态寄存器值
  */
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u16 len)
{	 
		u8 status,u8_ctr;	
	
		NRF24L01_CSN(RESET);          //使能SPI传输
  	status = SPI2_ReadWriteByte(reg);		//发送寄存器值(位置),并读取状态值
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++) SPI2_ReadWriteByte(*pBuf++); //写入数据	 
		NRF24L01_CSN(SET);       //关闭SPI传输
		
		return status;          //返回读到的状态值
}			

/**
  * @brief  在指定位置读出指定长度的数据
  * @param  reg:寄存器(位置)
	* @param	*pBuf:数据指针
	* @param	len:数据长度
  * @retval 此次读到的状态寄存器值 
  */
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{	 
	u8 status,u8_ctr;	    
	
	NRF24L01_CSN (RESET);           //使能SPI传输
	status=SPI2_ReadWriteByte(reg);	//发送寄存器值(位置),并读取状态值   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++) pBuf[u8_ctr]=SPI2_ReadWriteByte(0XFF);	//读出数据
	NRF24L01_CSN (SET);       //关闭SPI传输
	
	return status;        //返回读到的状态值
}

/**
  * @brief  SPI写寄存器
  * @param  reg:指定寄存器地址
	* @param	value:写入的值
  * @retval 返回状态值
  */
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
		u8 status;	
	
   	NRF24L01_CSN(RESET);                 //使能SPI传输
  	status = SPI2_ReadWriteByte(reg);	//发送寄存器号 
  	SPI2_ReadWriteByte(value);     	 //写入寄存器的值
  	NRF24L01_CSN(SET);                 //禁止SPI传输	  
	
  	return(status);       			//返回状态值
}

/**
  * @brief  读取SPI寄存器值
  * @param  reg:要读的寄存器
  * @retval 返回状态值
  */
u8 NRF24L01_Read_Reg(u8 reg)
{
		u8 reg_val;	    
	
		NRF24L01_CSN(RESET);          //使能SPI传输	
  	SPI2_ReadWriteByte(reg);   //发送寄存器号
  	reg_val=SPI2_ReadWriteByte(0XFF);//读取寄存器内容
  	NRF24L01_CSN(SET);          //禁止SPI传输		    
  
		return(reg_val);           //返回状态值
}	

/**
  * @brief  函数初始化NRF24L01到RX模式
  * @param  设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR,
						当CE变高后,即进入RX模式,并可以接收数据了		
  * @retval NONE
  */
void NRF24L01_RX_Mode(void)
{
		NRF24L01_CE(RESET);	  
  	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
	  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);    //使能通道0的自动应答    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //使能通道0的接收地址  	 
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);	     //设置RF通信频率		  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH); //选择通道0的有效数据宽度 	    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f); //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f); //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
  	NRF24L01_CE(SET); //CE为高,进入接收模式 
		HAL_Delay(1);
}		

/**
  * @brief  函数初始化NRF24L01到TX模式
  * @param  设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR、PWR_UP,CRC使能
						当CE变高后,即进入TX模式,并可以发送数据了
						CE为高大于10us,则启动发送.
  * @retval NONE
  */ 
void NRF24L01_TX_Mode(void)
{														 
	NRF24L01_CE(RESET);	    
  
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址 
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  
	
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);     //使能通道0的自动应答    
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //使能通道0的接收地址  
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x1a); //设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);       //设置RF通道为40
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,发射1模式,开启所有中断
	
	NRF24L01_CE(SET);	//CE为高,10us后启动发送
	HAL_Delay(1);
}







 






