#include "BSP_SYS.H"

static uint8_t SoleAddr[5];

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

	printf ("\r\n…Ë±∏ID∫≈: %02X%02X%02X%02X%02X\r\n\r\n", SoleAddr[0]
																											, SoleAddr[1]
																											, SoleAddr[2]
																											, SoleAddr[3]
																											, SoleAddr[4]);
}

void BSP_SYSInit(void)
{
	SetChipID();	
	SetNRFID();
}

void SetNRFID(void)
{
	u8 i;
	
	for(i = 0; i < 5; i++)
	{
		RX_ADDRESS[i] = TX_ADDRESS[i] = SoleAddr[i];
	}
}


