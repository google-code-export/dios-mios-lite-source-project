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
u32 DOLMinOff		= 0;
u32 DOLMaxOff		= 0;
u32 DOLSize			= 0;
u32 DOLOffset		= 0;
s32 ELFNumberOfSections = 0;


FIL GameFile;
u32 read;
u32 DiscRead=0;

char *getfilenamebyoffset(u32 offset)
{
	u32 fst_offset = read32(0x38) & ~0x80000000;
	
	u32 i;
	for (i = fst_offset + 12; i < 0x01800000; i+=12)
	{
		if (read8(i) == 0 && offset >= read32(i + 4) && offset < read32(i + 4) + read32(i + 8))
		{
			return (char *)(fst_offset + read32(fst_offset+8)*12 + (read32(i) & 0x00ffffff));
		}
	}
	return NULL;
}


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
			if( ConfigGetConfig(DML_CFG_ACTIVITY_LED) )
				set32( HW_GPIO_OUT, 1<<5 );

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
					u32 Length	= read32(DI_SCMD_2);
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

								DOLMinOff=0x81800000;
								DOLMaxOff=0;
								
								for( i=0; i < 7; ++i )
								{
									if( dol->addressText[i] == 0 )
										continue;

									if( DOLMinOff > dol->addressText[i])
										DOLMinOff = dol->addressText[i];

									if( DOLMaxOff < dol->addressText[i] + dol->sizeText[i] )
										DOLMaxOff = dol->addressText[i] + dol->sizeText[i];
								}
								DOLMinOff -= 0x80000000;
								DOLMaxOff -= 0x80000000;								

								dbgprintf("DIP:DOLSize:%d DOLMinOff:0x%08X DOLMaxOff:0x%08X\n", DOLSize, DOLMinOff, DOLMaxOff );

								PatchState = 1;
							}
						} else if( read32(Buffer) == 0x7F454C46 )
						{
							if (getfilenamebyoffset(Offset) != NULL)
							{
								dbgprintf("DIP:The Game is loading %s\n", getfilenamebyoffset(Offset));
							} else
							{
								dbgprintf("DIP:The Game is loading some .elf that is not in the fst...\n");
							}
							for (i = ((*(u32 *)0x00000038) & ~0x80000000) + 16; i < 0x01800000; i+=12)	// Search the fst for the dvd offset of the .elf file
							{
								if (*(u32 *)i == Offset)
								{
									DOLSize = *(u32 *)(i+4);
									DOLReadSize = Length;

									if( DOLReadSize == DOLSize )	// The .elf is read completely already
									{
										dbgprintf("DIP:The .elf is read completely, file size: %u bytes\n", DOLSize);
										DoPatches( (char*)(Buffer), Length, 0x80000000 );
									} else							// a part of the .elf is read
									{
										PatchState = 2;
										DOLMinOff=Buffer;
										DOLMaxOff=Buffer+Length;
										if (Length <= 4096) // The .elf header is read
										{
											ELFNumberOfSections = read16(Buffer+0x2c) -2;	// Assume that 2 sections are .bss and .sbss which are not read
											dbgprintf("DIP:The .elf header is read(%u bytes), .elf file size: %u bytes, number of sections to load: %u\n", Length, DOLSize, ELFNumberOfSections);
										} else				// The .elf is read into a buffer
										{
											ELFNumberOfSections = -1;						// Make sure that ELFNumberOfSections == 0 does not become true
											dbgprintf("DIP:The .elf is read into a buffer, read progress: %u/%u bytes\n", Length, DOLSize);
										}
									}
									break;
								}
							}
						}

					} else if ( PatchState != 0 )
					{
						DOLReadSize += Length;
						
						if (PatchState == 2)
						{
							ELFNumberOfSections--;
							
							// DOLMinOff and DOLMaxOff are optimised when loading .dol files
							if (DOLMinOff > Buffer)
								DOLMinOff = Buffer;
								
							if (DOLMaxOff < Buffer+Length)
								DOLMaxOff = Buffer+Length;
							
							if (ELFNumberOfSections < 0)
							{
								dbgprintf("DIP:.elf read progress: %u/%u bytes\n", DOLReadSize, DOLSize);
							} else
							{
								dbgprintf("DIP:.elf read progress: %u/%u bytes, sections left: %u\n", DOLReadSize, DOLSize, ELFNumberOfSections);
							}
						}

						//dbgprintf("DIP:DOLSize:%d DOLReadSize:%d\n", DOLSize, DOLReadSize );
						if( DOLReadSize >= DOLSize || (PatchState == 2 && ELFNumberOfSections == 0))
						{
							DoPatches( (char*)(DOLMinOff), DOLMaxOff-DOLMinOff, 0x80000000 );
							PatchState = 0;
						}
					}
										
					write32( DI_SDMA_LEN, 0 );
					
					while( read32(DI_SCONTROL) & 1 )
						clear32( DI_SCONTROL, 1 );

					set32( DI_SSTATUS, 0x3A );

					if( ConfigGetConfig(DML_CFG_NODISC) )
					{
						write32( 0x0d80000C, (1<<0) | (1<<4) );
						write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
						write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
						set32( 0x0d80000C, (1<<2) );

					} else {
						
						if( (read32(DI_SCMD_0) >> 24) == 0xA7 )
						{
							write32( 0x0d80000C, (1<<0) | (1<<4) );
							write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
							write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
							set32( 0x0d80000C, (1<<2) );		
						}
					}
									
					
				} break;
				default:
				{
					EXIControl(1);
					dbgprintf("DIP:Unknown CMD:%08X %08X %08X %08X %08X %08X\n", read32(DI_SCMD_0), read32(DI_SCMD_1), read32(DI_SCMD_2), read32(DI_SIMM), read32(DI_SDMA_ADR), read32(DI_SDMA_LEN) );
					while(1);
				} break;
			}

			if( ConfigGetConfig(DML_CFG_ACTIVITY_LED) )
				clear32( HW_GPIO_OUT, 1<<5 );

			return 1;
		} else {
			;//dbgprintf("DIP:DI_CONTROL:%08X:%08X\n", read32(DI_CONTROL), read32(DI_CONTROL) );
		}
	}

	return 0;
}
