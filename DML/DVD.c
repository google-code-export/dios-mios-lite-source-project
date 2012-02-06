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

	dbgprintf("DIP:Region:%d\n", *(u32*)(str+0x38) );
	
	switch( *(u32*)(str+0x38) )
	{
		default:
		case 0:
		case 1:
		{
			switch( SRAM_GetVideoMode() )
			{
				case GCVideoModeNone:
				{
					SRAM_SetVideoMode( GCVideoModePROG );
					dbgprintf("SRAM:Setting PROG flags\n");
				} break;
				case GCVideoModePAL60:
				{
					SRAM_SetVideoMode( GCVideoModeNone );
					dbgprintf("SRAM:Clearing PAL60 flags\n");
				} break;
				case GCVideoModePROG:
				{
					// do nothing
				} break;
				default:
				{
					dbgprintf("SRAM:Invalid Video mode setting:%d\n", SRAM_GetVideoMode() );
				} break;
			}
		} break;
		case 2:
		{
			switch( SRAM_GetVideoMode() )
			{
				case GCVideoModeNone:
				{
					SRAM_SetVideoMode( GCVideoModePROG );
					SRAM_SetVideoMode( GCVideoModePAL60 );
					dbgprintf("SRAM:Setting PROG&PAL60 flags\n");
				} break;
				case GCVideoModePAL60:
				{
					SRAM_SetVideoMode( GCVideoModePROG );
					dbgprintf("SRAM:Setting PROG flags\n");
				} break;
				case GCVideoModePROG:
				{
					// do nothing
				} break;
				default:
				{
					dbgprintf("SRAM:Invalid Video mode setting:%d\n", SRAM_GetVideoMode() );
				} break;
			}
		} break;
	}
	
	SRAM_Flush();

	DOLOffset = *(u32*)str;
	dbgprintf("DIP:DOL Offset:0x%06X\n", DOLOffset );

	f_lseek( &GameFile, DOLOffset );
	f_read( &GameFile, str, 0x100, &read );
			
	dol = (dolhdr*)str;
	
	DOLMaxOff=0;
	for( i=0; i < 7; ++i )
	{
		if( dol->addressText[i] == 0 )
			continue;

		if( DOLMaxOff < dol->addressText[i] + dol->sizeText[i] )
			DOLMaxOff = dol->addressText[i] + dol->sizeText[i];
	}

	DOLMaxOff -= 0x80003100; 
	dbgprintf("DIP:DOL MaxOffset:0x%06X\n", DOLMaxOff );
	
	u32 DOLSize = sizeof(dolhdr);
						
	for( i=0; i < 7; ++i )
		DOLSize += dol->sizeText[i];
	for( i=0; i < 11; ++i )
		DOLSize += dol->sizeData[i];
	
	dbgprintf("DIP:DOL size:0x%06X\n", DOLSize );

	free( str );

	return DI_SUCCESS;
}
