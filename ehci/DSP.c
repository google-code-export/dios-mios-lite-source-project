#include "DSP.h"

void DSPInit( void )
{
	memset32( (void*)DSP_BASE, 0xdeadbeef, 0x40 );
	memset32( (void*)(DSP_SBASE), 0, 0x40 );
}

// Note: starlet can only 32bit access MEM1

void DSPUpdateRegisters( void )
{
	//if( read32( DSP_DMA_MADDR ) != 0xdeadbeef )
	//{
	//	dbgprintf("DSP:MADDR:0x%08X\n",  read32( DSP_DMA_MADDR ) );

	//	//check what u16 part changed
	//	if( (read32( DSP_DMA_MADDR ) >> 16) == 0xdead )
	//	{
	//		u32 value = read32(DSP_SDMA_MADDR);
	//		value &= 0xFFFF0000;
	//		value |= read32( DSP_DMA_MADDR ) & 0xFFFF0000;
	//		write32( DSP_SDMA_MADDR, value );

	//	} else {

	//		u32 value = read32(DSP_SDMA_MADDR);
	//		value &= 0xFFFF;
	//		value |= read32( DSP_DMA_MADDR ) & 0xFFFF;
	//		write32( DSP_SDMA_MADDR, value );

	//	}

	//	write32( DSP_DMA_MADDR, 0xdeadbeef );
	//}
	//
	//if( read32( DSP_DMA_AADDR ) != 0xdeadbeef )
	//{
	//	
	//	dbgprintf("DSP:AADDR:0x%08X\n",  read32( DSP_DMA_AADDR ) );

	//	//check what u16 part changed
	//	if( (read32( DSP_DMA_AADDR ) >> 16) == 0xdead )
	//	{
	//		u32 value = read32(DSP_SDMA_AADDR);
	//		value &= 0xFFFF0000;
	//		value |= read32( DSP_DMA_AADDR ) & 0xFFFF0000;
	//		write32( DSP_SDMA_AADDR, value );

	//	} else {

	//		u32 value = read32(DSP_SDMA_AADDR);
	//		value &= 0xFFFF;
	//		value |= read32( DSP_DMA_AADDR ) & 0xFFFF;
	//		write32( DSP_SDMA_AADDR, value );

	//	}

	//	write32( DSP_DMA_AADDR, 0xdeadbeef );
	//}

	//if( read32( DSP_DMA_COUNT ) != 0xdeadbeef )
	//{
	//	
	//	dbgprintf("DSP:COUNT:0x%08X\n",  read32( DSP_DMA_COUNT ) );

	//	//check what u16 part changed
	//	if( (read32( DSP_DMA_COUNT ) >> 16) == 0xdead )
	//	{
	//		u32 value = read32(DSP_SDMA_COUNT);
	//		value &= 0xFFFF0000;
	//		value |= read32( DSP_DMA_COUNT ) & 0xFFFF0000;
	//		write32( DSP_SDMA_COUNT, value );

	//	} else {

	//		u32 value = read32(DSP_SDMA_COUNT);
	//		value &= 0xFFFF;
	//		value |= read32( DSP_DMA_COUNT ) & 0xFFFF;

	//		memcpy( P2C(read32(DSP_SDMA_MADDR)),  read32(DSP_SDMA_AADDR) | 0x10000000, read32(DSP_SDMA_COUNT) );

	//		dbgprintf("DSP:DMA Transfer: 0x%08X->0x%08X Count:0x%08X\n", P2C(read32(DSP_SDMA_MADDR)), read32(DSP_SDMA_AADDR), read32(DSP_SDMA_COUNT) );

	//		write32( DSP_SDMA_COUNT, value );
	//	}

	//	write32( DSP_DMA_COUNT, 0xdeadbeef );
	//}

	//if( read32( DSP_DMA_MADDR ) != 0xdeadbeef )
	//{
	//	//dbgprintf("DSP:MADDR:0x%08X\n",  read32( DSP_DMA_MADDR ) );
	//	
	//	write32( DSP_SDMA_MADDR, read32( DSP_DMA_MADDR ) );
	//	write32( DSP_DMA_MADDR, 0xdeadbeef );
	//}
	//if( read32( DSP_DMA_AADDR ) != 0xdeadbeef )
	//{
	//	//dbgprintf("DSP:AADDR:0x%08X\n",  read32( DSP_DMA_AADDR ) );
	//	
	//	write32( DSP_SDMA_AADDR, read32( DSP_DMA_AADDR ) );
	//	write32( DSP_DMA_AADDR, 0xdeadbeef );
	//}
	if( read32( DSP_DMA_COUNT ) != 0xdeadbeef )
	{
		while( read32( DSP_DMA_MADDR ) == 0xdeadbeef );
		write32( DSP_SDMA_MADDR, read32( DSP_DMA_MADDR ) );
		write32( DSP_DMA_MADDR, 0xdeadbeef );

		while( read32( DSP_DMA_AADDR ) == 0xdeadbeef );
		write32( DSP_SDMA_AADDR, read32( DSP_DMA_AADDR ) );
		write32( DSP_DMA_AADDR, 0xdeadbeef );

		//dbgprintf("DSP:COUNT:0x%08X\n",  read32( DSP_DMA_COUNT ) );
		
		set32( HW_GPIO_OUT, 1<<5 );
		if( read32(DSP_DMA_COUNT) & (1<<31) )
		{
			dbgprintf("DSP:DMA Transfer: 0x%08X->0x%08X Count:0x%08X\n", read32(DSP_SDMA_AADDR), read32(DSP_SDMA_MADDR), P2C(read32(DSP_DMA_COUNT)) );
			memcpy( (void*)P2C(read32(DSP_SDMA_MADDR)), (void*)(read32(DSP_SDMA_AADDR) | 0x11000000), P2C(read32(DSP_DMA_COUNT)) );

			//hexdump( (void*)P2C(read32(DSP_SDMA_MADDR)), P2C(read32(DSP_DMA_COUNT)) );
			
		} else {
				
			dbgprintf("DSP:DMA Transfer: 0x%08X->0x%08X Count:0x%08X\n", read32(DSP_SDMA_MADDR), read32(DSP_SDMA_AADDR), P2C(read32(DSP_DMA_COUNT)) );
			memcpy( (void*)(read32(DSP_SDMA_AADDR) | 0x11000000),  (void*)P2C(read32(DSP_SDMA_MADDR)), P2C(read32(DSP_DMA_COUNT)) );

		}
		clear32( HW_GPIO_OUT, 1<<5 );


		write32( DSP_SDMA_COUNT, read32( DSP_DMA_COUNT ) );
		write32( DSP_DMA_COUNT, 0xdeadbeef );
	}
	
}