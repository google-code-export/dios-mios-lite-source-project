/*

SNEEK - SD-NAND/ES + DI emulation kit for Nintendo Wii

Copyright (C) 2009-2010  crediar

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include "string.h"
#include "global.h"
#include "ipc.h"
#include "alloc.h"
#include "ff.h"
#include "dip.h"
#include "diskio.h"
#include "dol.h"
#include "GCPad.h"
#include "HW.h"
#include "ehci_types.h"
#include "ehci.h"
#include "DSP.h"
#include "Patches.h"

extern DIConfig *DICfg;

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
	dbgprintf("PrefetchAbort\n");
	while(1);
	return;
}
void DataAbort( u32 a, u32 b, u32 c, u32 d, u32 e, u32 f )
{
	dbgprintf("DataAbort: %x, %x, %x, %x, %x, %x\n",a,b,c,d,e,f);
	return;
	Shutdown();
}
void IRQHandler( void )
{
	u32 IRQs = read32(HW_ARMIRQFLAG) & read32(HW_ARMIRQMASK);
	
	if( IRQs & IRQ_GPIO1 )	// Starlet GPIO IRQ
	{
		if( read32(HW_GPIO_INTFLAG) & (1) )
		{
			//int i;
			//for( i = 0; i < 0x40; i+=4 )
			//	dbgprintf("0x%08X:0x%08X\t0x%08X\n", DSP_BASE-0x2480 + i, read32( DSP_BASE + i ), read32( DSP_SBASE + i ) );
			//dbgprintf("\n");
			int i;
			for( i = 0; i < 0x30; i+=4 )
				dbgprintf("0x%08X:0x%08X\t0x%08X\n", i, read32( DI_BASE + i ), read32( DI_SHADOW + i ) );
			dbgprintf("\n");
			set32( HW_EXICTRL, 1 );
			
			for( i = 0; i < 0x30; i+=4 )
				dbgprintf("0x%08X:0x%08X\n", 0x0D806000 + i, read32( 0x0D806000 + i ) );
						
			dbgprintf("DVD:Error:%08X\n", DVDLowGetError() );

			udelay(4000);

			set32( HW_GPIO_ENABLE, GPIO_POWER );
			set32( HW_GPIO_OUT, GPIO_POWER );
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
int main( int argc, char *argv[] )
{
	udelay(800);
	
	PPCReset();
	clear32( HW_RESETS, 0x48000 );
	clear32( 0xD800184, 0x438E );
	
	ChangeClock();

	DRAMInit(1,0);

	set32( HW_RESETS, 0x48000 );
	set32( 0xD800184, 0x438E );

	UNKInit( 1, 1 );

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

	//MIOSInit();

#ifdef DEBUG
	dbgprintf("$IOSVersion: DIOS-MIOS: " __DATE__ " " __TIME__ " 64M DEBUG$\n");
#else
	dbgprintf("$IOSVersion: DIOS-MIOS: " __DATE__ " " __TIME__ " 64M Release$\n");
#endif
	
	dbgprintf("CPU Ver:%d.%d\n", SP[1], SP[0] );
	
	dbgprintf("MEMInitLow()...\n");
	MEMInitLow();
	
	EHCIInit();
	dbgprintf("EHCIInit()\n");
	
	udelay(8000);
	
	HeapInit();
	dbgprintf("HeapInit()\n");

	FATFS fatfs;
	s32 fres=0;
	
	dbgprintf("f_mount()");
	fres = f_mount(0, &fatfs );
	dbgprintf(":%d\n", fres );

	DVDReadConfig();

	dbgprintf("DIP:DI-Config: Region:%d Slot:%02d Games:%02d\n", DICfg->Region, DICfg->SlotID, DICfg->Gamecount );

	if( DVDSelectGame( DICfg->SlotID ) != DI_SUCCESS )
	{
		dbgprintf("Failed to select game:%d\n", DICfg->SlotID );
		Shutdown();
	}

	set32( HW_EXICTRL, 1 );

	LowReadDiscID(0);
	dbgprintf("DVD:Error:%08X\n", DVDLowGetError() );
	
	clear32( HW_EXICTRL, 0 );

	DIInit();
	DSPInit();
	
	PatchGCIPL();
	EXIControl(0);

	write32( 0x1860, 0xdeadbeef );	// Clear OSReport area
	write32( 0x30F8, 0 );			// Tell PPC side to start

	ahb_flush_to( AHB_PPC );

	while (1)
	{
		ahb_flush_from( AHB_STARLET );	//flush to arm

		//if( ((*(vu32*)0x0d800010 - Timmer)* 19 / 10) >= 10*1000*1000 )
		//{
		//	dbgprintf("DI_SSTATUS :%08x\n", *(vu32*)DI_SSTATUS );
		//	dbgprintf("DI_SCOVER  :%08x\n", *(vu32*)DI_SCOVER );
		//	dbgprintf("DI_SCONTROL:%08x\n", *(vu32*)DI_SCONTROL );
		//	dbgprintf("0x0d800000 :%08x\n", *(vu32*)0x0d800000 );

		//	 Timmer = *(vu32*)0x0d800010;
		//}
		
		//if( Streaming )
		//{
		//	if( (*(vu32*)0x0d800010 * 19 / 10) - StreamTimer >= 5000000 )
		//	{
		//		dbgprintf(".");
		//		StreamOffset += StreamBufferSize;
		//		if( StreamOffset >= StreamSize )
		//		{
		//			StreamOffset = StreamSize;
		//			Streaming = 0;
		//		}
		//		StreamTimer = *(vu32*)0x0d800010 * 19 / 10;
		//	}
		//}
		//
		if( read32(0x1860) != 0xdeadbeef )
		{
			if( read32(0x1860) != 0 )
			{
				//dbgprintf("%08X\n", (*(vu32*)0x1860) & 0x7FFFFFFF );
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
		//DSPUpdateRegisters();

		//sync_before_read( (void*)DI_BASE, 0x100 );
		ahb_flush_to( AHB_PPC );	//flush to ppc
	}
}
