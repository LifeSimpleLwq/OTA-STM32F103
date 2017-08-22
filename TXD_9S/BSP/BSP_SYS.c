#include "BSP_SYS.H"


//uint8_t NRF_ID[5] = {0xB5,0x08,0x7B,0x02,0x1b};	//674696531B
//uint8_t NRF_ID[5] = {0x28,0x33,0xbc,0x62,0x1b};
uint8_t NRF_ID[5] = {0x67,0x46,0x96,0x53,0x1b};

static uint8_t SoleAddr[5];


/**
  * @brief  读取ID
  * @param  None
  * @note   从flash读取一个组ID并保存到SoleAddr
  * @retval None
  */
static void SetChipID(void)
{
	uint32_t ChipUniqueID[4];

	ChipUniqueID[0] = *(__IO uint32_t *)(0x1FFFF7F0);
	ChipUniqueID[1] = *(__IO uint32_t *)(0x1FFFF7EC);
	ChipUniqueID[2] = *(__IO uint32_t *)(0x1FFFF7E8);

	ChipUniqueID[3] = (ChipUniqueID[0] ^ ChipUniqueID[1]) ^ ChipUniqueID[2];

	SoleAddr[0] = (uint8_t)((ChipUniqueID[3] >> 24) & 0x000000FF);
	SoleAddr[1] = (uint8_t)((ChipUniqueID[3] >> 16) & 0x000000FF);
	SoleAddr[2] = (uint8_t)((ChipUniqueID[3] >> 8) & 0x000000FF);
	SoleAddr[3] = (uint8_t)(ChipUniqueID[3]& 0x000000FF);
	SoleAddr[4] = SoleAddrEP;

	printf ("\r\n设备ID号: %02X%02X%02X%02X%02X\r\n\r\n", SoleAddr[0]
																											, SoleAddr[1]
																											, SoleAddr[2]
																											, SoleAddr[3]
																											, SoleAddr[4]);
}

/**
  * @brief  读取ID
  * @param  None
  * @retval None
  */
void BSP_SYSInit(void)
{
	SetChipID();	
	SetNRFID();
}


/**
  * @brief  SetNRFID
	* @note		设置NRF模块ID号
  * @retval None	  
  */
void SetNRFID(void)
{
	u8 i;
	
	for(i = 0; i < 5; i++)
	{
		RX_ADDRESS[i] = TX_ADDRESS[i] = NRF_ID[i];
	}
}



