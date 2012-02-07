#include "DVD.h"

DVDConfig *DICfg = (DVDConfig *)NULL;
u32 read;

extern FIL GameFile;
extern u32 DOLMaxOff;
extern u32 DOLOffset;

s32 DVDSelectGame( void )
{
	u32 i;
	dolhdr *dol;
	FIL BootInfo;

	char *str = (char *)malloca( 0x100, 32 );
	sprintf( str, "/games/boot.bin" );

	switch( f_open( &BootInfo, str, FA_READ ) )
	{
		case FR_OK:
		{
			char *Path = (char*)malloca( BootInfo.fsize, 32 );

			f_read( &BootInfo, Path, BootInfo.fsize, &read );
			f_close( &BootInfo );

			sprintf( str, "/games/%s/game.iso", Path );

			free( Path );
		} break;
		default:
		{
			//dbgprintf("DIP:Couldn't open /games/boot.bin!\n");
			Shutdown();
		} break;
	}

	s32 fres = f_open( &GameFile, str, FA_READ );
	if( fres != FR_OK )
	{
		//dbgprintf("Failed to open:\"%s\" fres:%d\n", str, fres );
		Shutdown();
	}
	
	f_lseek( &GameFile, 0 );
	f_read( &GameFile, (void*)0, 0x20, &read );

	f_lseek( &GameFile, 0x420 );
	f_read( &GameFile, str, 0x40, &read );

	GC_SRAM *sram = SRAM_Unlock();

	dbgprintf("DIP: Region:%u\n", *(u32*)(str+0x38) );
	dbgprintf("SRAM: Mode:%u(%u) EURGB60:%u Prog:%u\n", sram->Flags&3, read32(0xCC), !!(sram->BootMode&0x40), !!(sram->Flags&0x80) );
			
	switch( *(u32*)(str+0x38) )
	{
		default:
		case 0:		//	JAP
		case 1:		//	USA
		{
			switch( sram->Flags&3 )
			{
				case 0:		// NTSC
				{
					if( !(sram->Flags&0x80) )		// PROG flag
						SRAM_SetVideoMode( GCVideoModePROG );

				} break;
				case 1:		// PAL
				case 2:		// MPAL
				{
					SRAM_SetVideoMode( GCVideoModeNTSC );
					SRAM_SetVideoMode( GCVideoModePROG );

					write32( 0x1312078, 0x60000000 );
					write32( 0x1312070, 0x38000001 );

				} break;
				default:
				{
					dbgprintf("SRAM:Invalid Video mode setting:%d\n", SRAM_GetVideoMode() );
				} break;
			}
		} break;
		case 2:			// EUR
		{
			switch( sram->Flags&3 )
			{
				case 0:
				{
					SRAM_SetVideoMode( GCVideoModePAL60 );
					SRAM_SetVideoMode( GCVideoModePROG );

					write32( 0x1312078, 0x60000000 );
					write32( 0x1312070, 0x38000001 );
				} break;
				case 1:
				case 2:
				{
					if( !(sram->BootMode&0x40) )	// PAL60 flag
					if( !(sram->Flags&0x80) )		// PROG flag
						SRAM_SetVideoMode( GCVideoModePAL60 );

				} break;
				default:
				{
					dbgprintf("SRAM:Invalid Video mode setting:%d\n", SRAM_GetVideoMode() );
				} break;
			}
		} break;
	}

	//SRAM_Flush();

	dbgprintf("SRAM: Mode:%u(%u) EURGB60:%u Prog:%u\n", sram->Flags&3, read32(0xCC), !!(sram->BootMode&0x40), !!(sram->Flags&0x80) );
	
	free( str );

	return DI_SUCCESS;
}
