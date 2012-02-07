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
void DIInit( void )
{
	memset32( (void*)DI_BASE, 0xdeadbeef, 0x30 );
	memset32( (void*)(DI_SHADOW), 0, 0x30 );

	write32( DI_SCONFIG, 0xFF );
	write32( DI_SCOVER, 0 );

	f_lseek( &GameFile, 0 );
	f_read( &GameFile, (void*)0, 0x20, &read );
}
u32 DIUpdateRegisters( void )
{	
	u32 read,i;
	static u32 PatchState = 0;
	static u32 DOLReadSize= 0;

	if( read32(DI_CONTROL) != 0xdeadbeef )
	{
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
				case 0xA7:
				case 0xA9:
					//dbgprintf("DIP:Async!\n");
				case 0xA8:
				{					
					u32 Buffer	= P2C(read32(DI_SDMA_ADR));
					u32 Length	= read32(DI_SDMA_LEN);
					u32 Offset	= read32(DI_SCMD_1) << 2;

				//	dbgprintf("DIP:DVDRead( 0x%08x, 0x%08x, 0x%08x )\n", Offset, Length, Buffer|0x80000000  );
					
					//	udelay(250);
						
					if( GameFile.fptr != Offset )
					if( f_lseek( &GameFile, Offset ) != FR_OK )
					{
						EXIControl(1);
						dbgprintf("DIP:Failed to seek to 0x%08x\n", Offset );
						while(1);
					}
					if( f_read( &GameFile, (char*)Buffer, Length, &read ) != FR_OK )
					{
						EXIControl(1);
						dbgprintf("DIP:Failed to read from 0x%08x to 0x%08X\n", Offset, Buffer );
						while(1);
					}
					//if( ((read+31)&(~31)) != Length )
					//{
					//	dbgprintf("DIP:DVDLowRead Offset:%08X Size:%08d Dst:%08X\n", Offset, Length, Buffer  );
					//	dbgprintf("DIP:Failed to read %d bytes, only got %d\n", Length, read );
					//	break;
					//}

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
					EXIControl(1);
					dbgprintf("DIP:Unknown CMD:%08X %08X %08X %08X %08X %08X\n", read32(DI_SCMD_0), read32(DI_SCMD_1), read32(DI_SCMD_2), read32(DI_SIMM), read32(DI_SDMA_ADR), read32(DI_SDMA_LEN) );
					while(1);
				} break;
			}
#ifdef ACTIVITYLED
			clear32( HW_GPIO_OUT, 1<<5 );
#endif
			return 1;
		} else {
			;//dbgprintf("DIP:DI_CONTROL:%08X:%08X\n", read32(DI_CONTROL), read32(DI_CONTROL) );
		}
	}

	return 0;
}
