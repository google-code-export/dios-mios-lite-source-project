/*

DIOS MIOS Lite - Gamecube SD loader for Nintendo Wii

Copyright (C) 2010-2012  crediar

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
#include "dip.h"


char __aeabi_unwind_cpp_pr0[0];
u32 dbgprintf_sd_access;

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
	return;
}
void DataAbort( u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h )
{
	u32 val;

	__asm("mov	%0,lr": "=r" (val) );

	*(vu32*)0xD800070 |= 1;
	
	dbgprintf("DataAbort: LR:%08x, %x, %x, %x, %x, %x, %x, %x\n",val,b,c,d,e,f,g,h);
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

			dbgprintf_sd_access = 0; // Writing to sd card here might be the cause for the file system corruptions
			
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

	//Clear IRQ Flags
	write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
	write32( HW_GPIO_INTFLAG, read32(HW_GPIO_INTFLAG) );

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
void SysReset( void )
{
	write32( HW_RESETS, (read32( HW_RESETS ) | 0x20 ) & (~1) );
}
void SysShutdown( void )
{
	set32( HW_GPIO_ENABLE, GPIO_POWER );
	set32( HW_GPIO_OUT, GPIO_POWER );

	while(1);
}

bool LoadDOL( void *DOLOffset );
u32 fail;
FIL Log;

int main( int argc, char *argv[] )
{
	u32 BootGCDisc = 0;
	FATFS fatfs;
	s32 fres = 0;
	dbgprintf_sd_access = 0;

	udelay(800);

#ifndef REALNAND	
	PPCReset();
	clear32( HW_RESETS, 0x48000 );
	clear32( 0xD800184, 0x438E );
	
	ChangeClock();

	DRAMInit(1,0);

	set32( HW_RESETS, 0x48000 );
	set32( 0xD800184, 0x438E );

	UNKInit( 1, 1 );
#endif

	set32( 0xD800038, IRQ_RESET|IRQ_GPIO1 );
	set32( 0xD80003C, IRQ_RESET|IRQ_GPIO1 );
	udelay(200);

	u32 SP[2];
	GetRevision( SP+1, SP );
	if( SP[1] == 0 )
	{
		write32( HW_MEMIRR, 0x67 );
	} else {
		write32( HW_MEMIRR, 0x27 );
	}

	MIOSInit();

	if( strncmp( (char *)0x007FFFE0, "gchomebrew dol", 32 ) == 0)		// Load GC homebrew
	{
		BootGCDisc = 1;

		if( LoadDOL((void *)0x00800000) )								// Copy the .dol and overwrite the ppc entrypoint
		{
			dbgprintf("DIP:Found gamecube homebrew...\n");
		} else {
			dbgprintf("DIP:Gamecube homebrew error...\n");
		}
	}
	
	//dbgprintf("MEMInitLow()...\n");
	MEMInitLow();
	
	//EHCIInit();
	//dbgprintf("EHCIInit()\n");
	
	udelay(8000);
	
	HeapInit();
	//dbgprintf("HeapInit()\n");
			
	sdhc_init();

	//dbgprintf("f_mount()");
	fres = f_mount(0, &fatfs );
	//dbgprintf(":%d\n", fres );
	
	if (fres != FR_OK)
	{
		dbgprintf("Error: Could not mount fatfs, ret: %d\n", fres);
		BootGCDisc = 1;
	} else
	{
		dbgprintf_sd_access = 1;
	}

	dbgprintf("\n\n\n");

#ifdef DEBUG
	dbgprintf("DIOS-MIOS Lite [DEBUG]\n");
#else
	dbgprintf("DIOS-MIOS Lite\n");
#endif
	dbgprintf("Built: " __DATE__ " " __TIME__ "\n");
	dbgprintf("This software is licensed under GPLv3, for more details visit:\nhttp://code.google.com/p/dios-mios-lite-source-project\n");

	//dbgprintf("CPU Ver:%d.%d\n", SP[1], SP[0] );
	
	//DVDReadConfig();

	//dbgprintf("DIP:DI-Config: Region:%d Slot:%02d Games:%02d\n", DICfg->Region, DICfg->SlotID, DICfg->Gamecount );

	set32( HW_EXICTRL, 1 );

	SRAM_Init();

	LowReadDiscID((void*)0);

	u32 DVDError = DVDLowGetError();
	if( DVDError )
	{
		dbgprintf("DVD:Error:%08X\n", DVDError );
	}
	if ( (DVDError >> 24 ) == 0x01 )
	{
		dbgprintf("DIP:No disc in drive, can't continue!\n");
		Shutdown();

	} else if( (DVDError >> 24 ) == 0x02 || (DVDError >> 24 ) == 0x05 )
	{
		DVDLowReset();
		LowReadDiscID(0);
		DVDEnableAudioStreaming( read32(8) >> 24 );

		DVDError = DVDLowGetError();

		if( DVDError )
			dbgprintf("DVD:Error:%08X\n",  DVDLowGetError() );
	} 

	if( !BootGCDisc )
	{
		if( DVDSelectGame() < 0)
			BootGCDisc = 1;		
	}

#ifdef CARDMODE
	CardInit();
#endif

	clear32( HW_EXICTRL, 0 );

	DIInit();
	
	if( !BootGCDisc )
	{
		MIOSCheckPatches();
		DoPatchesIPL();
	}
	
	write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
	write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
	
	set32( HW_PPCIRQMASK, (1<<31) );
	set32( HW_IPC_PPCCTRL, 0x30 );
	
	EXIControl(0);

	write32( 0x1860, 0xdeadbeef );	// Clear OSReport area
	write32( 0x30F8, 0 );			// Tell PPC side to start

	ahb_flush_to( AHB_PPC );

	if( BootGCDisc )
	{
		while(1)
			udelay(100);
	}
	
	while (1)
	{
		ahb_flush_from( AHB_STARLET );	//flush to arm

#ifdef PADHOOK
		if( (((read32(0x12FC) >> 16) & 0x254) == 0x254 ) )
		{
			SysReset();
		}
		if( (((read32(0x12FC) >> 16) & 0x154) == 0x154 ) )
		{
			SysShutdown();
		}
#endif
		
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
#ifdef CARDMODE
		CARDUpdateRegisters();
#endif
		ahb_flush_to( AHB_PPC );	//flush to ppc
	}
}

bool LoadDOL( void *DOLOffset )
{
	u32 i;
	dolhdr *dolfile;

	if(DOLOffset)
	{
		dolfile = (dolhdr *)DOLOffset;

		for (i = 0; i < 7; i++)
		{
			dolfile->addressText[i] &= ~0x80000000;
			if ((!dolfile->sizeText[i]) || (dolfile->addressText[i] < 0x100))
				continue;
			
			dc_invalidaterange((void *) dolfile->addressText[i], dolfile->sizeText[i]);
			memcpy ((void *) dolfile->addressText[i], DOLOffset+dolfile->offsetText[i], dolfile->sizeText[i]);
			dc_flushrange((void *) dolfile->addressText[i], dolfile->sizeText[i]);
		}

		for(i = 0; i < 11; i++)
		{
			dolfile->addressData[i] &= ~0x80000000;
			if ((!dolfile->sizeData[i]) || (dolfile->addressData[i] < 0x100))
				continue;

			dc_invalidaterange((void*) dolfile->addressData[i], dolfile->sizeData[i]);
			memcpy ((void*) dolfile->addressData[i], DOLOffset+dolfile->offsetData[i], dolfile->sizeData[i]);
			dc_flushrange((void*) dolfile->addressData[i], dolfile->sizeData[i]);
		}

		dolfile->addressBSS &= ~0x80000000;
		if (dolfile->addressBSS != 0 && dolfile->sizeBSS != 0)
		{
			// Clear BSS area
			dc_invalidaterange((void *) dolfile->addressBSS, dolfile->sizeBSS);
			memset32((void *) dolfile->addressBSS, 0, dolfile->sizeBSS);
			dc_flushrange((void *) dolfile->addressBSS, dolfile->sizeBSS);
		}

		// Overwrite the ppc entrypoint with the one from the .dol
		*(u32 *) 0x000037fc = dolfile->entrypoint;
		dc_flushrange((void *) 0x00003700, 0x100);

		ic_invalidateall();
		
		return true;
	}
	return false;
}
