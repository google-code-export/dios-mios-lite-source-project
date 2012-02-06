#include "dip.h"

u32	StreamBufferSize= 54*1024;
u32 Streaming		= 0;
u32	StreamOffset	= 0;
u32 StreamDiscOffset= 0;
s32 StreamSize		= 0;
u32 StreamRAMOffset	= 0;
u32 StreamTimer		= 0;
u32 StreamStopEnd	= 0;
u32 GameRun			= 0;
u32 DOLMaxOff		= 0;
u32 DOLSize			= 0;
u32 DOLOffset		= 0;

FIL GameFile;
u32 read;

u32 DiscRead=0;

extern u32 fail;

#ifdef CODES
FIL KenobiGCFile;
FIL CodesFile;
u32 HandlerFlag;
u32 CodesFlag;
void DIReadKenobiGC( void )
{
	if( HandlerFlag == HANDLER_OK || HandlerFlag == HANDLER_DONE )
	{
		//u32 testhandler;
		f_lseek( &KenobiGCFile, 0 );
		f_read( &KenobiGCFile, (void*)0x1800, KenobiGCFile.fsize, &read );
		memcpy((void *)0x1800, (void*)0x0, 6);
		//f_close( &KenobiGCFile );
		HandlerFlag = HANDLER_DONE;
	}
}
void DIReadCodes( void )
{
	if( CodesFlag == CODES_OK || CodesFlag == CODES_DONE )
	{
		//u32 testcodes;
		u32 maxcodesfile = 0x650;
		f_lseek( &CodesFile, 0 );		
		if( CodesFile.fsize >= maxcodesfile )
		{
			f_close( &CodesFile );
			Shutdown();
		}
		f_read( &CodesFile, (void*)0x27B0, CodesFile.fsize, &read );
		//f_close( &CodesFile );
		CodesFlag = CODES_DONE;
	}	
}
#endif
void DIInit( void )
{
	memset32( (void*)DI_BASE, 0xdeadbeef, 0x30 );
	memset32( (void*)(DI_SHADOW), 0, 0x30 );

	write32( DI_SCONFIG, 0xFF );
	write32( DI_SCOVER, 4 );

	f_lseek( &GameFile, 0 );
	f_read( &GameFile, (void*)0, 0x20, &read );
}
void DIUpdateRegisters( void )
{	
	u32 read,i;
	static u32 PatchState = 0;
	static u32 DOLReadSize= 0;
	//static FEntry *FSTable;
	//static u32 FSTEntries = 0;
	//static char *FSTNames;
	
	if( read32(DI_CONTROL) != 0xdeadbeef )
	{
		if( read32( DI_CONTROL ) & (~3) )
		{	
			//EXIControl(1);
			dbgprintf("DIP:Bogus write to DI_CONTROL:%08X\n", read32( DI_CONTROL ) );			
			Shutdown();
			
			memset32( (void*)DI_BASE, 0xdeadbeef, 0x30 );
			memset32( (void*)(DI_SHADOW), 0, 0x30 );

			return;			
		}

		write32( DI_SCONTROL, read32(DI_CONTROL) & 3 );
		
		clear32( DI_SSTATUS, 0x14 );

		write32( DI_CONTROL, 0xdeadbeef );
			
		if( read32(DI_SCONTROL) & 1 )
		{
#ifdef ACTIVITYLED
			set32( HW_GPIO_OUT, 1<<5 );
#endif
			if( read32(DI_CMD_0) != 0xdeadbeef )
			{
				write32( DI_SCMD_0, read32(DI_CMD_0) );
				write32( DI_CMD_0, 0xdeadbeef );
			}
						
			if( read32(DI_CMD_1) != 0xdeadbeef ) 
			{
				write32( DI_SCMD_1, read32(DI_CMD_1) );
				write32( DI_CMD_1, 0xdeadbeef );
			}
						
			if( read32(DI_CMD_2) != 0xdeadbeef )
			{
				write32( DI_SCMD_2, read32(DI_CMD_2) );
				write32( DI_CMD_2, 0xdeadbeef );
			}
						
			if( read32(DI_DMA_ADR) != 0xdeadbeef )
			{
				write32( DI_SDMA_ADR, read32(DI_DMA_ADR) );
				write32( DI_DMA_ADR, 0xdeadbeef );
			}

			if( read32(DI_DMA_LEN) != 0xdeadbeef )
			{
				write32( DI_SDMA_LEN, read32(DI_DMA_LEN) );
				write32( DI_DMA_LEN, 0xdeadbeef );
			}

			if( read32(DI_IMM) != 0xdeadbeef )
			{
				write32( DI_SIMM, read32(DI_IMM) );
				write32( DI_IMM, 0xdeadbeef );
			}
							
			switch( read32(DI_SCMD_0) >> 24 )
			{
				case 0xA9:
					//dbgprintf("DIP:Async!\n");
				case 0xA8:
				{
					u32 Buffer	= P2C(read32(DI_SDMA_ADR));
					u32 Length	= read32(DI_SDMA_LEN);
					u32 Offset	= read32(DI_SCMD_1) << 2;

					//dbgprintf("DIP:DVDRead( 0x%08x, 0x%08x, 0x%08x )\n", Offset, Length, Buffer|0x80000000  );

					if( GameFile.fptr != Offset )
					if( f_lseek( &GameFile, Offset ) != FR_OK )
					{
						//EXIControl(1);
						dbgprintf("DIP:Failed to seek to 0x%08x\n", Offset );
						Shutdown();
					}
					if( f_read( &GameFile, (char*)Buffer, Length, &read ) != FR_OK )
					{
						//EXIControl(1);
						dbgprintf("DIP:Failed to read from 0x%08x to 0x%08X\n", Offset, Buffer );
						Shutdown();
					}
					//if( ((read+31)&(~31)) != Length )
					//{
					//	dbgprintf("DIP:DVDLowRead Offset:%08X Size:%08d Dst:%08X\n", Offset, Length, Buffer  );
					//	dbgprintf("DIP:Failed to read %d bytes, only got %d\n", Length, read );
					//	break;
					//}
					
					/* The MIOS IPL is patched to not use its patcher on Wind Waker, so this shouldn't be required
					//Zelda hack
					if( (read32(0) >> 8 ) == 0x475A4C )
					{
						write32( 0, 0x475B4C00 | (read32(0) & 0xFF) );
					}*/

					/* This should not be required anymore, because DML now detects when the main.dol is loaded
					//Hack for Metroid since it reads the FST before the main.dol
					if( Offset == 0x28ebd340 )
						DoPatches( (char*)(0x3100), DOLMaxOff, 0x80000000, 1 );
						*/
						

/*					//GC-IPL has loaded the main.dol to RAM, now we can patch it
					if( Buffer > 0x01700000 && Length != 0x2000 && PatchState == 0 )
					{
						FSTable = (FEntry*)Buffer;
						dbgprintf("DIP:FST Offset:%p\n", FSTable );
						
						FSTEntries= read32( Buffer + 0x08 );
						FSTNames= (char*)((u32)FSTable + FSTEntries * 0x0C );
						
						dbgprintf("DIP:FST Entries:%d\n", FSTEntries );
						dbgprintf("DIP:FST NameOff:%p\n", FSTNames );

						DoPatches( (char*)(0x3100), DOLMaxOff, 0x80000000, 1 );
#ifdef CODES
						DIReadKenobiGC();
						DIReadCodes();
#endif
						switch( read32( 0 ) & 0xFF )
						{
							default:
							case 'E':
							case 'J':
							{
								write32( 0xCC, 0 );
							} break;
							case 'P':
							{
								if( SRAM_GetVideoMode() & GCVideoModePROG )
									write32( 0xCC, 0 );
								else
									write32( 0xCC, 5 );
							} break;
						}
							
						dbgprintf("Video:%08x\n", *(u32*)0xCC );

						PatchState = 1;

					// Game is loading a new loader!
					} else
					*/

					if( (u32)Buffer == 0x01300000 )
					{
						DoPatchesLoader( (char*)(0x01300000), Length );
					}

					if( PatchState == 0 )
					{
						if( Length == 0x100 )
						{
							if( read32( (u32)Buffer ) == 0x100 )
							{
								//quickly calc the size
								DOLSize = sizeof(dolhdr);
								dolhdr *dol = (dolhdr*)Buffer;
						
								for( i=0; i < 7; ++i )
									DOLSize += dol->sizeText[i];
								for( i=0; i < 11; ++i )
									DOLSize += dol->sizeData[i];
						
								DOLReadSize = sizeof(dolhdr);

								DOLMaxOff=0;
								for( i=0; i < 7; ++i )
								{
									if( dol->addressText[i] == 0 )
										continue;

									if( DOLMaxOff < dol->addressText[i] + dol->sizeText[i] )
										DOLMaxOff = dol->addressText[i] + dol->sizeText[i];
								}

								DOLMaxOff -= 0x80003100; 

								dbgprintf("DIP:DOLSize:%d DOLMaxOff:0x%08X\n", DOLSize, DOLMaxOff );

								PatchState = 1;
							}
						} else if( read32(Buffer) == 0x7F454C46 )
						{
							dbgprintf("DIP:Game is loading an .elf %u\n", Length );
						}

					} else if ( PatchState == 1 )
					{
						DOLReadSize += Length;
						//dbgprintf("DIP:DOLSize:%d DOLReadSize:%d\n", DOLSize, DOLReadSize );
						if( DOLReadSize == DOLSize )
						{
							DoPatches( (char*)(0x3100), DOLMaxOff, 0x80000000, 0 );
#ifdef CODES
							DIReadKenobiGC();
							DIReadCodes();
#endif
							//Setting 0xCC is ignored by the PPC side, use SRAM to set the videomode.

							PatchState = 0;
						}
					}
										
					write32( DI_SDMA_ADR, P2C(read32(DI_SDMA_ADR) + read32(DI_SDMA_LEN)) );
					write32( DI_SDMA_LEN, 0 );
					
					while( read32(DI_SCONTROL) & 1 )
						clear32( DI_SCONTROL, 1 );

					set32( DI_SSTATUS, 0x3A );

					write32( 0x0d80000C, (1<<0) | (1<<4) );
					write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
					write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
					set32( 0x0d80000C, (1<<1) | (1<<2) );

				} break;
				default:
				{
					//EXIControl(1);
					dbgprintf("DIP:Unknown CMD:%08X %08X %08X %08X %08X %08X\n", read32(DI_SCMD_0), read32(DI_SCMD_1), read32(DI_SCMD_2), read32(DI_SIMM), read32(DI_SDMA_ADR), read32(DI_SDMA_LEN) );
					Shutdown();
				} break;
			}
#ifdef ACTIVITYLED
			clear32( HW_GPIO_OUT, 1<<5 );
#endif
		} else {
			;//dbgprintf("DIP:DI_CONTROL:%08X:%08X\n", read32(DI_CONTROL), read32(DI_CONTROL) );
		}
	}

}
