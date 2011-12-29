#include "DVD.h"

DVDConfig *DICfg = (DVDConfig *)NULL;
u32 read;

extern FIL GameFile;
extern u32 DOLMaxOff;
extern u32 DOLOffset;

#ifdef CODES
extern FIL KenobiGCFile;
extern FIL CodesFile;
extern u32 HandlerFlag;
extern u32 CodesFlag;
#endif

void DVDReadConfig( void )
{
	char *str = (char*)malloca( 128, 32 );
	FIL f;

	sprintf( str, "/sneek/diconfig.bin" );
	s32 fres = f_open( &f, str, FA_READ );
	if( fres != FR_OK )
	{
		dbgprintf("Failed to open:\"%s\" fres:%d\n", str, fres );
		Shutdown();
	}
	
	DICfg = (DVDConfig*)malloca( f.fsize, 32 );
	f_read( &f, DICfg, f.fsize, &read );
	f_close( &f );
	free(str);
}
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

#ifdef CODES
			char *strhandler = (char*)malloca( 0x100, 0x20 );
			sprintf( strhandler, "/games/kenobigc.bin" );
			s32 freshandler = f_open( &KenobiGCFile, strhandler, FA_READ );
			if( freshandler != FR_OK )
			{
				dbgprintf("Failed to open:\"%s\" freshandler:%d\n", strhandler, freshandler );
				HandlerFlag = HANDLER_ERR;
			}
			else
			{	
				HandlerFlag = HANDLER_OK;
			}
			free( strhandler );

			char *strcodes = (char *)malloca( 0x100, 0x20);
			sprintf( strcodes, "/games/%s/codes.gct", Path);
			s32 frescodes = f_open( &CodesFile, strcodes, FA_READ);
			if( frescodes != FR_OK )
			{
				dbgprintf("Failed to open:\"%s\" frescodes:%d\n", strcodes, frescodes );
				CodesFlag = CODES_ERR;
			}
			else
			{
				CodesFlag = CODES_OK;
			}
			free( strcodes );			
#endif

			free( Path );
		} break;
		default:
		{
			dbgprintf("DIP:Couldn't open /games/boot.bin!\n");
			Shutdown();
		} break;
	}

	s32 fres = f_open( &GameFile, str, FA_READ );
	if( fres != FR_OK )
	{
		dbgprintf("Failed to open:\"%s\" fres:%d\n", str, fres );
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
					// do nothing
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
					// do nothing
				} break;
				case GCVideoModePAL60:
				{
					// do nothing
				} break;
				case GCVideoModePROG:
				{
					SRAM_SetVideoMode( GCVideoModeNone );
					dbgprintf("SRAM:Clearing PROG flags\n");
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

	free( str );

	return DI_SUCCESS;
}
