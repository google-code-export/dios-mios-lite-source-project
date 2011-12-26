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
u32 Patch			= 0;
u32 DOLSize			= 0;
u32 DOLOffset		= 0;

DIConfig *DICfg		= (DIConfig *)NULL;

FIL f;
u32 read;

static char GamePath[64];

u32 DiscRead=0;

void DIInit( void )
{
	memset32( (void*)DI_BASE, 0xdeadbeef, 0x40 );
	memset32( (void*)(DI_SHADOW), 0, 0x40 );

	write32( DI_SCONFIG, 0xFF );
	write32( DI_SCOVER, 4 );

	f_lseek( &f, 0 );
	f_read( &f, (void*)0, 0x20, &read );

}
void DVDReadConfig( void )
{
	char *str = (char*)malloca( 128, 32 );


	sprintf( str, "/sneek/diconfig.bin" );
	s32 fres = f_open( &f, str, FA_READ );
	if( fres != FR_OK )
	{
		dbgprintf("Failed to open:\"%s\" fres:%d\n", str, fres );
		Shutdown();
	}
	
	DICfg = (DIConfig*)malloca( f.fsize, 32 );
	f_read( &f, DICfg, f.fsize, &read );
	f_close( &f );
	free(str);
}
s32 DVDSelectGame( int SlotID )
{
	sprintf( GamePath, "/games/%s/", DICfg->GameInfo[SlotID] );

	free( DICfg );

	dbgprintf("DIP:Set game path to:\"%s\" \n", GamePath );
	
	char *str = (char *)malloca( 0x100, 32 );
	sprintf( str, "%sgame.iso", GamePath );
	s32 fres = f_open( &f, str, FA_READ );
	if( fres != FR_OK )
	{
		dbgprintf("Failed to open:\"%s\" fres:%d\n", str, fres );
		Shutdown();
	}

	f_lseek( &f, 0x420 );
	f_read( &f, str, 32, &read );

	DOLOffset = *(u32*)str;
	dbgprintf("DIP:DOL Offset:%06X\n", DOLOffset );

	f_lseek( &f, DOLOffset );
	f_read( &f, str, 0x100, &read );
			
	u32 i;
	DOLSize=0;
	dolhdr *dol = (dolhdr*)str;

	for( i=0; i < 7; ++i )
	{
		if( dol->addressText[i] == 0 )
			continue;

		if( DOLSize < dol->addressText[i] + dol->sizeText[i] )
			DOLSize = dol->addressText[i] + dol->sizeText[i];
	}

	DOLSize -= 0x80003100; 
	dbgprintf("DIP:DOL MaxOffset:%06X\n", DOLSize );

	free( str );

	return DI_SUCCESS;
}
u32 DVDLowGetError( void )
{
	write32( 0x0D806000, 0x2E );
	write32( 0x0D806008, 0xE0000000 );
	write32( 0x0D806020, 0 );
	write32( 0x0D80601c, 1 );

	while( read32(0x0D80601c) & 1 );
	
	set32( 0x0D806000, (1<<4) );

	return read32( 0x0D806020 );
}
u32 LowReadDiscID( void *data )
{	
	write32( 0x0D806008, 0xA8000040 );
	write32( 0x0D80600C, 0 );
	write32( 0x0D806010, 0x20 );
	write32( 0x0D806018, 0x20 );
	
	write32( 0x0D806014, (u32)data );

	write32( 0x0D806000, 0x3A );
	
	write32( 0x0D80601C, 3 );
	
	while (1)
	{
		if( read32( 0x0D806000 ) & (1<<2) )
		{
			set32( 0x0D806000, (1<<2) );
			return 1;
		}
		if( read32( 0x0D806000 ) & (1<<4) )
		{
			set32( 0x0D806000, (1<<4) );
			return 0;
		}
	}
	
	return 0;
}
void DIUpdateRegisters( void )
{
	////Oh noes something changed!
	//if( read32( DI_STATUS ) != 0xdeadbeef )
	//{
	//	//dbgprintf("DIP:DI_STATUS:%08X:", read32(DI_SSTATUS) );

	//	clear32( DI_SSTATUS, 0x2A );						// clear all set-able flags
	//	set32(	 DI_SSTATUS, read32(DI_STATUS) & 0x2A );	// set flags
	//	clear32( DI_SSTATUS, read32(DI_STATUS) & (~0x2A) );	// clear IRQs

	//	//dbgprintf("%08X:%08X\n", read32(DI_STATUS), read32(DI_SSTATUS) );

	//	while( read32( DI_STATUS ) == 0xdeadbeef )
	//		write32( DI_STATUS, 0xdeadbeef );
	//}
	//if( read32(DI_COVER) != 0xdeadbeef )
	//{
	//	clear32( DI_SCOVER,	2 );							// clear all set-able flags
	//	set32(	 DI_SCOVER, read32(DI_COVER) & 2 );			// set flags
	//	clear32( DI_SCOVER, read32(DI_COVER) & (~2) );		// clear IRQs
	//		
	//	set32(	 DI_SCOVER, 4 );

	//	//dbgprintf("DIP:DI_COVER:%08X:%08X\n", read32(DI_COVER), read32(DI_SCOVER) );
	//	write32( DI_COVER, 0xdeadbeef );
	//}
	//if( read32(DI_CONFIG) != 0xdeadbeef )
	//{
	//	//dbgprintf("DIP:DI_CONFIG:%08X:%08X\n", read32(DI_SCONFIG), read32(DI_CONFIG) );
	//	write32( DI_SCONFIG, 0x000000FF );
	//	write32( DI_CONFIG, 0xdeadbeef );
	//}
	//if( read32(DI_CMD_0) != 0xdeadbeef )
	//{
	//	//dbgprintf("DIP:DI_CMD_0:%08X:%08X\n", read32(DI_SCMD_0), read32(DI_CMD_0) );
	//	write32( DI_SCMD_0, read32(DI_CMD_0) );
	//	write32( DI_CMD_0, 0xdeadbeef );
	//}
	//if( read32(DI_CMD_1) != 0xdeadbeef )
	//{
	//	//dbgprintf("DIP:DI_CMD_1:%08X:%08X\n", read32(DI_SCMD_1), read32(DI_CMD_1) );
	//	write32( DI_SCMD_1, read32(DI_CMD_1) );
	//	write32( DI_CMD_1, 0xdeadbeef );
	//}
	//if( read32(DI_CMD_2) != 0xdeadbeef )
	//{
	//	//dbgprintf("DIP:DI_CMD_2:%08X:%08X\n", read32(DI_SCMD_2), read32(DI_CMD_2) );
	//	write32( DI_SCMD_2, read32(DI_CMD_2) );
	//	write32( DI_CMD_2, 0xdeadbeef );
	//}
	//if( read32(DI_IMM) != 0xdeadbeef )
	//{
	//	//dbgprintf("DIP:DI_IMM:%08X:%08X\n", read32(DI_SIMM), read32(DI_IMM) );
	//	write32( DI_SIMM, read32(DI_IMM) );
	//	write32( DI_IMM, 0xdeadbeef );
	//}
	//if( read32(DI_DMA_LEN) != 0xdeadbeef )
	//{
	//	//dbgprintf("DIP:DI_DMA_LEN:%08X:%08X\n", read32(DI_SDMA_LEN), read32(DI_DMA_LEN) );
	//	write32( DI_SDMA_LEN, read32(DI_DMA_LEN) );
	//	write32( DI_DMA_LEN, 0xdeadbeef );
	//}
	//if( read32(DI_DMA_ADR) != 0xdeadbeef )
	//{
	//	//dbgprintf("DIP:DI_DMA_ADR:%08X:%08X\n", read32(DI_SDMA_ADR), read32(DI_DMA_ADR) );
	//	write32( DI_SDMA_ADR, read32(DI_DMA_ADR) );
	//	write32( DI_DMA_ADR, 0xdeadbeef );
	//}
		
	if( read32(DI_CONTROL) != 0xdeadbeef )
	{
		//dbgprintf("Timer:%d\n", *(vu32*)0x0d800010 - Timmer );

		//dbgprintf("DIP:DI_CONTROL:%08X:%08X\n", read32(DI_SCONTROL), read32(DI_CONTROL) );
		write32( DI_SCONTROL, read32(DI_CONTROL) & 3 );
		write32( DI_CONTROL, 0xdeadbeef );
			
		if( read32(DI_SCONTROL) & 1 )
		{
			set32( HW_GPIO_OUT, 1<<5 );

			ahb_flush_from(0);

			clear32( DI_SSTATUS, 0x14 );
			
			while( read32(DI_CMD_0) == 0xdeadbeef );
			write32( DI_SCMD_0, read32(DI_CMD_0) );
			write32( DI_CMD_0, 0xdeadbeef );
			
			while( read32(DI_CMD_1) == 0xdeadbeef );
			write32( DI_SCMD_1, read32(DI_CMD_1) );
			write32( DI_CMD_1, 0xdeadbeef );
			
			while( read32(DI_CMD_2) == 0xdeadbeef );
			write32( DI_SCMD_2, read32(DI_CMD_2) );
			write32( DI_CMD_2, 0xdeadbeef );
			
			while( read32(DI_DMA_LEN) == 0xdeadbeef );
			write32( DI_SDMA_LEN, read32(DI_DMA_LEN) );
			write32( DI_DMA_LEN, 0xdeadbeef );

			while( read32(DI_DMA_ADR) == 0xdeadbeef );
			write32( DI_SDMA_ADR, read32(DI_DMA_ADR) );
			write32( DI_DMA_ADR, 0xdeadbeef );
			
			//dbgprintf("DIP:Command:%08X\n", read32(DI_SCMD_0) );
			//if( (DI_SCONTROL>>1)&1 )
			//{
			//	dbgprintf("DIP:DMA Transfer: Offset:%08X Size:%08X\n", DI_SDMA_ADR, DI_SDMA_LEN );
			//}
			int i;
			//if( read32(DI_SDMA_LEN) == 0x0004f860 )
			//	for( i=0; i < 0x26; i+=4 )
			//	{
			//		dbgprintf("%02X:%08X:%08X\n", i, read32(DI_BASE+i), read32(DI_SHADOW+i) );
			//	}
				
			switch( read32(DI_SCMD_0) >> 24 )
			{
				case 0x12:
				{
					u32 ptr = P2C(read32(DI_SDMA_ADR));
						
					////write32( ptr + 0x00, 0x00000002 );	// Wii
					////write32( ptr + 0x04, 0x20060526 );
					////write32( ptr + 0x08, 0x41000000 );

					write32( ptr + 0x00, 0x00000000 );		// GC
					write32( ptr + 0x04, 0x20020402 );
					write32( ptr + 0x08, 0x61000000 );

					write32( ptr + 0x0C, 0x00000000 );		// always zero
					write32( ptr + 0x10, 0x00000000 );
					write32( ptr + 0x14, 0x00000000 );
					write32( ptr + 0x18, 0x00000000 );
					write32( ptr + 0x1C, 0x00000000 );

					write32( DI_DMA_ADR, read32(DI_DMA_ADR) + read32(DI_DMA_LEN) );
					write32( DI_DMA_LEN, 0 );
						
					//Set Transfer complete bit if Tranfer IRQ is enabled
					set32( DI_STATUS, (read32(DI_STATUS)<<1) & 0x10 );
					clear32( DI_CONTROL, 1 );
						
					GameRun = 1;
						
					dbgprintf("DIP:DVDLowInquiry()\n");
				} break;
				case 0xE3:
				{
					//Set Transfer complete bit if Tranfer IRQ is enabled
					set32( DI_STATUS, (read32(DI_STATUS)<<1) & 0x10 );
					clear32( DI_CONTROL, 1 );
					dbgprintf("DIP:DVDLowStopMotor()\n");
				} break;
				case 0xE0:
				{
					//Set Transfer complete bit if Tranfer IRQ is enabled
					set32( DI_STATUS, (read32(DI_STATUS)<<1) & 0x10 );
					clear32( DI_CONTROL, 1 );
					dbgprintf("DIP:RequestError()\n");
				} break;
				case 0xE1:
				{
					//for( i=0; i < 0x26; i+=4 )
					//{
					//	dbgprintf("%02X:%08X:%08X\n", i, *(vu32*)(DI_BASE+i), *(vu32*)(DI_HADOW+i) );
					//}

					if( read32(DI_SCMD_1) == 0 && read32(DI_SCMD_2) == 0 )
					{	
						StreamStopEnd	= 1;
						
						//Set Transfer complete bit if Tranfer IRQ is enabled
						set32( DI_STATUS, (read32(DI_STATUS)<<1) & 0x10 );
						clear32( DI_CONTROL, 1 );

						dbgprintf("DIP:DVDLowAudioStreaming( %08X, %08X )\n", read32(DI_SCMD_1), read32(DI_SCMD_2) );

					} else {

						StreamDiscOffset= read32(DI_SCMD_1)<<2;
						StreamSize		= read32(DI_SCMD_2);
						StreamOffset	= 0;
						Streaming		= 1;
						StreamStopEnd	= 0;
						StreamTimer		= *(vu32*)0x0d800010 * 19 / 10;
													
						//Set Transfer complete bit if Tranfer IRQ is enabled
						set32( DI_SSTATUS, (read32(DI_STATUS)<<1) & 0x10 );
						clear32( DI_SCONTROL, 1 );

						dbgprintf("DIP:Streaming %ds of audio...\n", StreamSize / 32 * 28 / 48043 );  
						dbgprintf("DIP:DVDLowAudioStreaming( %08X, %08X )\n", StreamDiscOffset, StreamSize );

					}
				} break;
				case 0xE2:	// DVDLowRequestAudioStatus 
				{
					switch( read32(DI_SCMD_0)<<8 )
					{
						case 0x00000000:	// Streaming?
						{
							write32( DI_IMM, Streaming );
						} break;
						case 0x01000000:	// What is the current address?
						{
							dbgprintf("DIP:StreamInfo:Cur:%08X End:%08X\n", StreamOffset, StreamSize );
							write32( DI_IMM, ((StreamDiscOffset+StreamOffset) >> 2) & (~0x1FFF) );
						} break;
						case 0x02000000:	// disc offset of file
						{
							write32( DI_IMM, StreamDiscOffset>>2 );
						} break;
						case 0x03000000:	// Size of file
						{
							write32( DI_IMM, StreamSize );
						} break;
					}
						
					//Set Transfer complete bit if Tranfer IRQ is enabled
					set32( DI_STATUS, (read32(DI_STATUS)<<1) & 0x10 );
					clear32( DI_SCONTROL, 1 );
					dbgprintf("DIP:DVDLowAudioGetConfig( %d, %08X )\n", (read32(DI_CMD_0)>>16)&0xFF, read32(DI_IMM) );
				} break;
				case 0xAB:
				{
					//if( f_lseek( &f, read32(DI_CMD_1) << 2 ) != FR_OK )
					//{
					//	dbgprintf("DIP:Failed to seek to %08x\n", read32(DI_CMD_1) << 2 );
					//	break;
					//}
						
					//Set Transfer complete bit if Tranfer IRQ is enabled
					set32( DI_STATUS, (read32(DI_STATUS)<<1) & 0x10 );
					clear32( DI_SCONTROL, 1 );
					
					dbgprintf("DIP:DVDLowSeek(0x%08X)\n", read32(DI_CMD_1) << 2 );
				} break;
				case 0xA8:
				{
					u32 Buffer	= P2C(read32(DI_SDMA_ADR));
					u32 Length	= read32(DI_SDMA_LEN);
					u32 Offset	= read32(DI_SCMD_1) << 2;

					if( f_lseek( &f, Offset ) != FR_OK )
					{
						dbgprintf("DIP:Failed to seek to 0x%08x\n", Offset );
						break;
					}
					if( f_read( &f, (char*)Buffer, Length, &read ) != FR_OK )
					{
						dbgprintf("DIP:Failed to read from 0x%08x to 0x%08X\n", Offset, Buffer );
						break;
					}
					//if( ((read+31)&(~31)) != Length )
					//{
					//	dbgprintf("DIP:DVDLowRead Offset:%08X Size:%08d Dst:%08X\n", Offset, Length, Buffer  );
					//	dbgprintf("DIP:Failed to read %d bytes, only got %d\n", Length, read );
					//	break;
					//}

					if( (Buffer > 0x01700000 && Length != 0x2000 && Patch == 0) )
					{
						DoPatches( (char*)(0x3100), DOLSize, 0x80000000 );

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
								write32( 0xCC, 5 );
							} break;
						}
							
						dbgprintf("Video:%08x\n", *(u32*)0xCC );
						
						//u32 newval = (0x0006950 - 0x003191BC);
						//newval&= 0x03FFFFFC;
						//newval|= 0x48000000;
						//write32( 0x003191BC, newval );

						//write32( 0xF8, 0x09a7ec80 );
						//write32( 0xFC, 0x1cf7c580 );
						
						//memcpy( (void*)0x0317268, ARAMTransfer, sizeof(ARAMTransfer) );
						
						//write32( 0x0317268, 0x3CC0C000 );	// lis     %r6, 0xCC00

						//write32( 0x031726C, 0xA00624E0 );	// lhz     %r0, 0x5020(%r6)
						//write32( 0x0317284, 0xB00624A0 );	// sth     %r0, 0x5020(%r6)

						//write32( 0x031728C, 0x38862480 );	// addi    %r4, %r6, 0x5000
						//
						//write32( 0x0317290, 0xA0A624E2 );	// lhz     %r5, 0x5022(%r6)
						//write32( 0x031729C, 0xB00624A2 );	// sth     %r0, 0x5022(%r6)

						//write32( 0x03172A0, 0x38A62480 );	// addi    %r5, %r6, 0x5000
						//
						//write32( 0x03172A8, 0xA0C624E4 );	// lhz     %r6, 0x5024(%r6)
						//write32( 0x03172BC, 0xA0C90066 );	// lhz     %r6, 0x26(%r9)

						//write32( 0x03172D4, 0xA0E40048 );	// lhz     %r7, 0x28(%r4)
						//write32( 0x03172E4, 0xA0E40048 );	// lhz     %r7, 0x28(%r4)

						//write32( 0x03172F4, 0xA085004A );	// lhz     %r4, 0x2A(%r5)						
						
						Patch = 1;
					} else if ( Buffer == 0x01300000 && Patch )
					{
						DoPatches( (char*)(Buffer), Length, 0x80000000 );
					}
					//if(Patch)
					//{
					//	dbgprintf("Read(0x%08X,%08d,0x%08X)\n", Offset, Length, Buffer );
					//	
					//	for( i = 0; i < 0x30; i+=4 )
					//		dbgprintf("0x%08X:0x%08X\n", 0x03a71a0 + i, read32( 0x03a71a0 + i ) );
					//}
					
					write32( DI_SDMA_ADR, P2C(read32(DI_SDMA_ADR) + read32(DI_SDMA_LEN)) );
					write32( DI_SDMA_LEN, 0 );

					set32( DI_SSTATUS, 0x10 );

					while( read32(DI_SCONTROL) & 1 )
						clear32( DI_SCONTROL, 1 );

				} break;
				default:
				{
					dbgprintf("DIP:Unknown CMD:%08X %08X %08X %08X %08X %08X\n", read32(DI_CMD_0), read32(DI_CMD_1), read32(DI_CMD_2), read32(DI_IMM), read32(DI_DMA_ADR), read32(DI_DMA_LEN) );
					while(1);
				} break;
			}

			clear32( HW_GPIO_OUT, 1<<5 );

		} else {
			;//dbgprintf("DIP:DI_CONTROL:%08X:%08X\n", read32(DI_CONTROL), read32(DI_CONTROL) );
		}
	}
}
