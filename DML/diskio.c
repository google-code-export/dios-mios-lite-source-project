#include "diskio.h"
#include "string.h"
#include "sdmmc.h"
#include "memory.h"

DSTATUS disk_initialize( BYTE drv )
{
	if (sdmmc_check_card() == SDMMC_NO_CARD)
		return STA_NOINIT;

	sdmmc_ack_card();
	return disk_status(drv);
}

DSTATUS disk_status( BYTE drv )
{
	(void)drv;
	if (sdmmc_check_card() == SDMMC_INSERTED)
		return 0;
	else
		return STA_NODISK;
}

DRESULT disk_read( BYTE drv, BYTE *buff, DWORD sector, BYTE count )
{
	u8 *buffer = (u8*)buff;
	//dbgprintf("sdmmc_read( %d, %d, %p, %p)\n", sector, count, buff, buffer );

	if( (u32)buff & 0xF0000000 )
	{
		buffer = (u8*)0x1000;
		u32 i=0;
		u32 Blocks = 3;
		while(1)
		{
			if( (count-i) < Blocks )
				Blocks = (count-i);

			sdmmc_read( sector + i, Blocks, buffer );		
			memcpy( buff + i * 512, buffer, Blocks * 512 );

			i+=Blocks;

			if( i >= count )
				break;
		}
	} else {
		sdmmc_read( sector, count, buffer );
		dc_flushrange( buffer, count*512 );
		ahb_flush_from( AHB_SDHC );
	}

	return RES_OK;
}
// Write Sector(s)
DRESULT disk_write( BYTE drv, const BYTE *buff, DWORD sector, BYTE count )
{
	u8 *buffer = (u8*)buff;

	if( (u32)buff & 0xF0000000 )
	{
		buffer = (u8*)0x1000;
		u32 i;
		for( i=0; i < count; ++i )
		{
			memcpy( buffer, (void*)buff + i * 512, 512 );	
			sdmmc_write( sector + i, 1, buffer );
		}
	} else {

		ahb_flush_to( AHB_SDHC );
		sdmmc_write( sector, count, buffer );

	}

	return RES_OK;
}
