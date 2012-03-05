#include "DVD.h"

DVDConfig *DICfg = (DVDConfig *)NULL;
u32 read;

extern FIL GameFile;
extern u32 DOLMaxOff;
extern u32 DOLOffset;
#ifdef SPEEDTEST
void SpeedTest( void )
{
	double timerA,timerB,timerC,timerD,now;
	u32 resA,resB,resC,resD;
	u32 i,read,rand;

	dbgprintf("DIP:Starting speed test of SD card\n");
	
	timerA = read32(HW_TIMER);
	for( i=0; i < 15*1024*1024; i+=32*1024 )
	{
		f_read( &GameFile, (void*)0x800000, 32*1024, &read );
	}
	now = *(vu32*)0x0d800010;
	resA = (u32)((now-timerB) * 128.0f / 243000000.0f);

	timerB = read32(HW_TIMER);
	for( i=0; i < 15*1024*1024; i+=64*1024 )
	{
		f_read( &GameFile, (void*)0x800000, 64*1024, &read );
	}
	now = *(vu32*)0x0d800010;
	resB = (u32)((now-timerB) * 128.0f / 243000000.0f);
	
	timerC = read32(HW_TIMER);
	for( i=0; i < 15*1024*1024; i+=32*1024 )
	{
		rand = (read32(HW_TIMER) & 0x3ff) << 17;

		f_lseek( &GameFile, rand );
		f_read( &GameFile, (void*)0x800000, 32*1024, &read );
	}
	now = *(vu32*)0x0d800010;
	resC = (u32)((now-timerC) * 128.0f / 243000000.0f);

	timerD = read32(HW_TIMER);
	for( i=0; i < 15*1024*1024; i+=64*1024 )
	{
		rand = (read32(HW_TIMER) & 0x3ff) << 17;

		f_lseek( &GameFile, rand );
		f_read( &GameFile, (void*)0x800000, 64*1024, &read );
	}
	now = *(vu32*)0x0d800010;
	resD = (u32)((now-timerD) * 128.0f / 243000000.0f);

	EXIControl(1);
	
	dbgprintf("15MB (32KB reads) Speed:%ukB/s\n", 15*1024*1024 / resA / 2 / 1024 );
	dbgprintf("15MB (64KB reads) Speed:%ukB/s\n", 15*1024*1024 / resB / 2 / 1024 );
	dbgprintf("15MB (32KB random reads) Speed:%ukB/s\n", 15*1024*1024 / resC / 2 / 1024 );
	dbgprintf("15MB (64KB random reads) Speed:%ukB/s\n", 15*1024*1024 / resD / 2/ 1024 );
	
	EXIControl(0);
	
	udelay(10000);
	Shutdown();
}
#endif
s32 DVDSelectGame( void )
{
	FIL BootInfo;

	char *str = (char *)malloca( 0x400, 32 );
	sprintf( str, "/games/boot.bin" );

	switch( f_open( &BootInfo, str, FA_READ ) )
	{
		case FR_OK:
		{
			char *Path = (char*)malloca( BootInfo.fsize, 32 );

			f_read( &BootInfo, Path, BootInfo.fsize, &read );
			f_close( &BootInfo );
			
			f_unlink(str);          // Delete the boot.bin, so retail discs can be loaded via the disc channel

			if( ConfigGetConfig(DML_CFG_GAME_PATH) )
				sprintf( str, "%s", ConfigGetGamePath() );
			else
				sprintf( str, "/games/%s/game.iso", Path );

			free( Path );
		} break;
		default:
		{
			dbgprintf("DIP:Couldn't open /games/boot.bin!\n");
			return -1;
		} break;
	}

	s32 fres = f_open( &GameFile, str, FA_READ );
	if( fres != FR_OK )
	{
		dbgprintf("Failed to open:\"%s\" fres:%d\n", str, fres );
		return -2;
	}
	
	f_lseek( &GameFile, 0 );
	f_read( &GameFile, (void*)0, 0x20, &read );

	f_lseek( &GameFile, 0 );
	f_read( &GameFile, str, 0x400, &read );
	
	dbgprintf("DIP:Loading game %.6s: %s\n", str, (char *)(str+0x20));

	f_lseek( &GameFile, 0x420 );
	f_read( &GameFile, str, 0x40, &read );

#ifdef SPEEDTEST
	SpeedTest();
#endif

	GC_SRAM *sram = SRAM_Unlock();

	dbgprintf("DIP:Region:%u\n", *(u32*)(str+0x38) );
	dbgprintf("SRAM:Mode:%u(%u) EURGB60:%u Prog:%u\n", sram->Flags&3, read32(0xCC), !!(sram->BootMode&0x40), !!(sram->Flags&0x80) );
			
	switch( ConfigGetVideMode() & 0xFFFF0000 )
	{
		case DML_VID_FORCE:
		{
			if( ConfigGetVideMode() & DML_VID_FORCE_PAL50 )
				SRAM_SetVideoMode( GCVideoModeNone );

			if( ConfigGetVideMode() & DML_VID_FORCE_PAL60 )
				SRAM_SetVideoMode( GCVideoModePAL60 );

			if( ConfigGetVideMode() & DML_VID_FORCE_NTSC )
				SRAM_SetVideoMode( GCVideoModeNTSC );

			if( ConfigGetVideMode() & DML_VID_FORCE_PROG )
				SRAM_SetVideoMode( GCVideoModePROG );

		} break;
		case DML_VID_NONE:
		{
		} break;
		case DML_VID_DML_AUTO:
		default:
		{
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
		} break;
	}

	SRAM_Flush();

	dbgprintf("SRAM:Mode:%u(%u) EURGB60:%u Prog:%u\n", sram->Flags&3, read32(0xCC), !!(sram->BootMode&0x40), !!(sram->Flags&0x80) );
	
	free( str );

	return DI_SUCCESS;
}
