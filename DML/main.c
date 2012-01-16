/*

DIOS MIOS Lite - Gamecube SD loader for Nintendo Wii

Copyright (C) 2010-2011  crediar

*/
#include "string.h"
#include "global.h"
#include "ipc.h"
#include "alloc.h"
#include "ff.h"
#include "diskio.h"
#include "dol.h"
#include "GCPad.h"
#include "HW.h"
#include "Patches.h"
#include "Config.h"
#include "Card.h"
#include "sdhc.h"
#include "DVD.h"
#include "Drive.h"

#ifdef TRIFORCE
#include "tri.h"
#else
#include "dip.h"
#endif

char __aeabi_unwind_cpp_pr0[0];

void Syscall( u32 a, u32 b, u32 c, u32 d )
{
	//dbgprintf("Syscall,%d,%d,%d,%d\n", a, b, c, d);
	return;
}
void SWI( u32 a, u32 b )
{
	//dbgprintf("SWI:%X,%X\n", a, b );
	return;
}
void PrefetchAbort( void )
{
	EXIControl(1);
	dbgprintf("PrefetchAbort\n");
	while(1);
}
void DataAbort( u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h )
{
	EXIControl(1);
	dbgprintf("DataAbort: %x, %x, %x, %x, %x, %x, %x, %x\n",a,b,c,d,e,f,g,h);
	Shutdown();
}
void IRQHandler( void )
{
	u32 IRQs = read32(HW_ARMIRQFLAG) /*& read32(HW_ARMIRQMASK)*/;
	
	if( IRQs & IRQ_GPIO1 )	// Starlet GPIO IRQ
	{
		if( read32(HW_GPIO_INTFLAG) & (1) )
		{
			set32( HW_EXICTRL, 1 );

			int i;
			for( i = 0; i < 0x20; i+=4 )
				dbgprintf("0x%08X:0x%08X\t0x%08X\n", i, read32( CARD_BASE + i ), read32( CARD_SHADOW + i ) );
			dbgprintf("\n");
			for( i = 0; i < 0x30; i+=4 )
				dbgprintf("0x%08X:0x%08X\t0x%08X\n", i, read32( 0x00002F00 + i ), read32( 0x00002F30 + i ) );
			dbgprintf("\n");
			
			for( i = 0; i < 0x30; i+=4 )
				dbgprintf("0x%08X:0x%08X\t0x%08X\n", 0x0D806000 + i, read32( 0x0D806000 + i ), read32( 0x0D006000 + i ) );
						
			dbgprintf("DVD:Error:%08X\n", DVDLowGetError() );

			udelay(10000);

			set32( HW_GPIO_ENABLE, GPIO_POWER );
			set32( HW_GPIO_OUT, GPIO_POWER );

			while(1);
		}
	}

	return;
}
void FIQHandler( void )
{
	//dbgprintf("FIQHandler\n");
	return;
}
void DebugPoke( u8 Value )
{
	clear32( 0xD8000E0, 0xFF0000 );
	set32( 0xD8000E0, Value<<16 );
}
u32 fail;
FIL Log;


void check_MIOS_patches()
{
	bool remove_MIOS_patches = false;
	switch(read32(0) >> 8)
	{
		case 0x475A4C:
			dbgprintf("Warning: MIOS Patch for Wind Waker(GZL) will be skipped\n");
			dbgprintf("The game will freeze when opening the mini map in dungeons\n");
			remove_MIOS_patches = true;
		break;
		
		case 0x475352:
			dbgprintf("Warning: MIOS Patch for Smuggler's Run: Warzones(GSR) will be skipped\n");
			remove_MIOS_patches = true;
		break;
		
		case 0x505A4C:
			dbgprintf("Warning: MIOS Patch for Zelda: Collector's Edition(PZL) will be skipped\n");
			remove_MIOS_patches = true;
		break;

		default:
		break;
	}
	
	switch(read32(0))
	{
		case 0x47543350:
			dbgprintf("Warning: MIOS Patch for Tony Hawk's Pro Skater 3(GT3P) will be skipped\n");
			remove_MIOS_patches = true;
		break;
		
		case 0x47543346:
			dbgprintf("Warning: MIOS Patch for Tony Hawk's Pro Skater 3(GT3F) will be skipped\n");
			remove_MIOS_patches = true;
		break;

		case 0x47543344:
			dbgprintf("Warning: MIOS Patch for Tony Hawk's Pro Skater 3(GT3D) will be skipped\n");
			remove_MIOS_patches = true;
		break;
		
		case 0x474E4845:
			dbgprintf("MIOS blacklist for Action Replay(GNHE) will be ignored\n");
			remove_MIOS_patches = true;
		break;
		
		case 0x4743364A:
			dbgprintf("Warning: MIOS Patch for Pokémon Colosseum(GC6J) will be skipped\n");
			remove_MIOS_patches = true;
		break;
		
		case 0x47433645:
			dbgprintf("Warning: MIOS Patch for Pokémon Colosseum(GC6E) will be skipped\n");
			remove_MIOS_patches = true;
		break;
		
		case 0x47433650:
			dbgprintf("Warning: MIOS Patch for Pokémon Colosseum(GC6P) will be skipped\n");
			remove_MIOS_patches = true;
		break;
		
		case 0x47504F4A:
			dbgprintf("Warning: MIOS Patch for Phantasy Star Online Episode I & II ? (GPOJ) will be skipped\n");
			remove_MIOS_patches = true;
		break;
		
		default:
		break;
	}
	
	if (remove_MIOS_patches)
	{
		const u32 old_table[18] = {
			0x475A4C00, 0x47535200, 0x505A4C00, 0x47543350,
			0x00000000, 0x47543346, 0x00000000, 0x47543344, 
			0x00000000, 0x474E4845, 0x00000000, 0x4743364A, 
			0x00000000, 0x47433645, 0x00000000, 0x47433650, 
			0x00000000, 0x47504F4A
		};

		const u32 new_table[18] = {
			0x585A4C00, 0x58535200, 0x585A4C00, 0x58543350,
			0x00000000, 0x58543346, 0x00000000, 0x58543344, 
			0x00000000, 0x584E4845, 0x00000000, 0x5843364A, 
			0x00000000, 0x58433645, 0x00000000, 0x58433650, 
			0x00000000, 0x58504F4A
		};
		
		bool patched = false;
		u32 i;
		
		for (i=0x01300000; i<0x01380000; i+=4)		// Estimate 512KB as max MIOS size 
		{
			if (!memcmp((void *)i, (void *)old_table, sizeof(old_table))) 
			{
				memcpy((void *)i, (void *)new_table, sizeof(new_table));
				patched = true;
				dbgprintf("MIOS IPL patched to not patch this game\n");
				break;
			}
		}
		if (!patched)
		{
			dbgprintf("MIOS IPL patch failed\n");
		}
	}
}




int main( int argc, char *argv[] )
{
	bool boot_retail_disc = false;
	udelay(800);

/*	The BC replacement code is not required when executing the actual BC
	PPCReset();
	clear32( HW_RESETS, 0x48000 );
	clear32( 0xD800184, 0x438E );

	ChangeClock();

	DRAMInit(1,0);

	set32( HW_RESETS, 0x48000 );
	set32( 0xD800184, 0x438E );

	UNKInit( 1, 1 );
*/	
	set32( 0xD800038, IRQ_RESET|IRQ_GPIO1 );
	set32( 0xD80003C, IRQ_RESET|IRQ_GPIO1 );
	udelay(200);

	u32 SP[2];
	GetRevision( SP+1, SP );
	if( SP[1] == 0 )
	{
		write32( HW_MEMIRR, 0x67 );
	} else
	{
		write32( HW_MEMIRR, 0x27 );
	}

	MIOSInit();
  
#ifdef DEBUG
	dbgprintf("$IOSVersion: DIOS-MIOS Lite INTERNAL: " __DATE__ " " __TIME__ " 64M DEBUG$\n");
	//dbgprintf("This is an INTERNAL version and may not be copied or re-distributed without prior written consent!\n");
#else
	//dbgprintf("DIOS-MIOS Lite PoC-Version by crediar\n");
	//dbgprintf("Built: " __DATE__ " " __TIME__ "\n");
	//dbgprintf("It is not allowed to resell, rehost, redistribute or include this file in any packages!\n");
	dbgprintf("DIOS-MIOS Lite\n");
	dbgprintf("Built: " __DATE__ " " __TIME__ "\n");
	dbgprintf("This software is licensed under GPLv3, for more details visit: http://code.google.com/p/dios-mios-lite-source-project\n");
#endif
	
	dbgprintf("CPU Ver:%d.%d\n", SP[1], SP[0] );
	
	dbgprintf("MEMInitLow()...\n");
	MEMInitLow();

	//EHCIInit();
	//dbgprintf("EHCIInit()\n");
	
	udelay(8000);
	
	HeapInit();
	//dbgprintf("HeapInit()\n");

	FATFS fatfs;
	s32 fres=0;
		
	sdhc_init();

	//dbgprintf("f_mount()");
	fres = f_mount(0, &fatfs );
	//dbgprintf(":%d\n", fres );
	
	//DVDReadConfig();

	//dbgprintf("DIP:DI-Config: Region:%d Slot:%02d Games:%02d\n", DICfg->Region, DICfg->SlotID, DICfg->Gamecount );

	set32( HW_EXICTRL, 1 );

	SRAM_Init();

	LowReadDiscID((void*)0);

	u32 DVDError = DVDLowGetError();
	if (DVDError)
	{
		dbgprintf("DVD:Error:%08X\n", DVDError );
	}

	if ( (DVDError >> 24 ) == 0x01 )
	{
		dbgprintf("DIP:No disc in drive, can't continue!\n");
		Shutdown();
	} else if( (DVDError >> 24 ) == 0x02 || (DVDError >> 24 ) == 0x05  )
	{
		DVDLowReset();
		LowReadDiscID(0);
		DVDEnableAudioStreaming( read32(8) >> 24 );
		dbgprintf("DVD:Error:%08X\n",  DVDLowGetError() );
	}
	
	if (DVDSelectGame() < 0)
	{
		boot_retail_disc = true;
	}

	check_MIOS_patches();

	CardInit();
		
	clear32( HW_EXICTRL, 0 );

	DIInit();
	
	if (!boot_retail_disc)
	{
		PatchGCIPL();
	}

#if defined(debugprintf) && !defined(debugprintfSD)
	dbgprintf("Switching exi control to ppc. DML's debug output will be logged to sd:/dm.log\n");
#ifdef fwrite_patch
	dbgprintf("The games' fwrite output will still be logged via usb gecko\n");
#endif
#endif

	EXIControl(0);

	write32( 0x1860, 0xdeadbeef );	// Clear OSReport area
	write32( 0x30F8, 0 );			// Tell PPC side to start

	ahb_flush_to( AHB_PPC );

	dbgprintf("\n\nDML main loop start\n");

	if (boot_retail_disc)
	{
		while (1);
	} else
	{
		while (1)
		{
			ahb_flush_from( AHB_STARLET );	//flush to arm
			
			if( read32(0x1860) != 0xdeadbeef )
			{
				if( read32(0x1860) != 0 )
				{
					//dbgprintf("%08X\n", P2C(read32(0x1860)) );
					dbgprintf(	(char*)(P2C(read32(0x1860))),
								(char*)(P2C(read32(0x1864))),
								(char*)(P2C(read32(0x1868))),
								(char*)(P2C(read32(0x186C))),
								(char*)(P2C(read32(0x1870))),
								(char*)(P2C(read32(0x1864)))
							);
				}

				write32(0x1860, 0xdeadbeef);
			}

			DIUpdateRegisters();
			//CARDUpdateRegisters();
			ahb_flush_to( AHB_PPC );	//flush to ppc
		}
	}
}
