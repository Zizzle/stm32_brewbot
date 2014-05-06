#include <stdio.h>
#include "FreeRTOS.h"
#include "stm32f10x.h"

#define  FLASH_ADR   0x0807F0F0 // 4K from the end

uint8_t *flash_read()
{
	return ((uint8_t *) FLASH_ADR);
}

void flash_write(uint8_t *data, int dataLenBytes)
{
    uint32_t FlashData;

    FLASH_Unlock();
	FLASH_ErasePage(FLASH_ADR);
	FLASH_Lock();

	uint16_t *wordData = (uint16_t *)data;
	for (int ii =0 ; ii < (dataLenBytes / 2) + 1; ii++)
	{
		FLASH_Unlock();
	    //printf("\r\nflash 0x%x\r\n", FLASH_ADR + (ii * 2));
		FLASH_ProgramHalfWord(FLASH_ADR + (ii * 2), wordData[ii]);
		FLASH_Lock();
	}
}
