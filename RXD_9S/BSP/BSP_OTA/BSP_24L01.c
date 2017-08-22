#include "BSP_24l01.h"

u8  OTA_RX[OTA_RX_LEN] __attribute__ ((at(0X20001000)));	//接收缓冲,最大OTA_RX_LEN个字节,起始地址为0X20001000.  单次最大接收6KB  0X20001000
u8  SPI_RX_BUF[SPI_REC_LEN];
u16 Rxd_NUM = 0;

u8 TX_ADDRESS[TX_ADR_WIDTH]; //发送地址
u8 RX_ADDRESS[RX_ADR_WIDTH];


/**
  * @brief  NRF24L01读取固件长度
  * @retval 0 没有数据 or 1 接收到数据
  */
u8 NRF24L01_RxLenth(void)
{	  		    		 
	if(NRF24L01_RxPacket(SPI_RX_BUF) == 0)	
	{			
		AppLenth = SPI_RX_BUF[1]*256 + SPI_RX_BUF[0];		// 计算数据长度
		if (AppLenth%32 != 0) Rxd_NUM = AppLenth/32 + 1;		// 计算NRF发送次数
		else  Rxd_NUM = AppLenth/32;
		printf("固件长度:%dBytes\r\n",AppLenth);
		
		return 1; 
	}
	return 0;
}

/**
  * @brief  NRF24L01接收固件
  * @retval 1 没有数据 or 0 接收到数据
  */
u8 NRF24L01_Rx(u16 x)
{	  		    		  
	u8 i = 0;
	u16 Index;
	Index = x * 28;
	
	if(NRF24L01_RxPacket(SPI_RX_BUF) == 0)	//一旦接收到信息,则显示出来.
	{	
		for (i = 0; i< 28; i++)
		{
			OTA_RX[Index + i] = SPI_RX_BUF[i+3];	// 保存到SRAM
		}
	
		//printf("接收到 %3d %3d \r\n",SPI_RX_BUF[16],SPI_RX_BUF[0]+SPI_RX_BUF[1]*256);  // 调试使用
		return 0; 
	}
	 
	return 1;
}

/**
  * @brief  启动NRF24L01接收一次数据
  * @param  txbuf:待发送数据首地址
  * @retval 0，接收完成；其他，错误代码
  */
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;	

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
	
	printf("NRF24L01 Mode: Receive\r\n");	
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
	
//	SPI2_SetSpeed(SPI_BaudRatePrescaler_4); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   	 
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址
		
	for(i=0;i<5;i++) if(buf[i]!=0XA5) break;	 							   
	if(i!=5) return 1;//检测24L01错误	
	return 0;		 //检测到24L01

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
		printf("NRF24L01 Error");
		HAL_Delay(200);
	}
	printf("NRF24L01 Init OK\r\n");
}





