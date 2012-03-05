#include "Config.h"

DML_CFG *DMLCfg;

void ConfigInit( DML_CFG *Cfg )
{
	DMLCfg = (DML_CFG*)malloc( sizeof( DML_CFG ) );

	memset32( DMLCfg, 0, sizeof(DML_CFG) );

	//If a loader supplied any options we use them otherwise use the code defines
	if( Cfg->Magicbytes == 0xD1050CF6 && Cfg->Version == CONFIG_VERSION )
	{
		memcpy( DMLCfg, Cfg, sizeof( DML_CFG ) );

	} else {
		
		DMLCfg->Config = 0;
#ifdef CHEATHOOK
		DMLCfg->Config |= DML_CFG_DEBUGGER;
#endif
#ifdef DEBUGGERWAIT
		DMLCfg->Config |= DML_CFG_DEBUGWAIT;
#endif
#ifdef CARDMODE
		DMLCfg->Config |= DML_CFG_NMM;
#endif
#ifdef CARDDEBUG
		DMLCfg->Config |= DML_CFG_NMM_DEBUG;
#endif
#ifdef ACTIVITYLED
		DMLCfg->Config |= DML_CFG_ACTIVITY_LED;
#endif
#ifdef PADHOOK
		DMLCfg->Config |= DML_CFG_PADHOOK;
#endif
		DMLCfg->VideoMode	= DML_VID_DML_AUTO;
		DMLCfg->Version		= CONFIG_VERSION;
		DMLCfg->Magicbytes	= 0xD1050CF6;
	}

	//Check if a memcard is inserted in Slot A
	if( (read32(0xD806800) & 0x1000) == 0x1000 )
	{
		DMLCfg->Config &= ~DML_CFG_NMM;		// disable NMM
	}
}

inline u32 ConfigGetConfig( u32 Config )
{
	return !!(DMLCfg->Config & Config);
}
u32 ConfigGetVideMode( void )
{
	return DMLCfg->VideoMode;
}

char *ConfigGetGamePath( void )
{
	return DMLCfg->GamePath;
}
char *ConfigGetCheatPath( void )
{
	return DMLCfg->CheatPath;
}

u8 SRAM[64];
void SRAM_Checksum( unsigned short *buf, unsigned short *c1, unsigned short *c2) 
{ 
	u32 i; 
    *c1 = 0; *c2 = 0; 
    for (i = 0;i<4;++i) 
    { 
        *c1 += buf[0x06 + i]; 
        *c2 += (buf[0x06 + i] ^ 0xFFFF); 
    }
	//dbgprintf("New Checksum: %04X %04X\n", *c1, *c2 );
} 

void SRAM_Read( void *sram )
{
	u32 data = 0x20000100;

	exi_select( 0, 1, 3);
	exi_imm( 0, &data, 4, EXI_WRITE, 0 );
	exi_sync( 0 );
	exi_imm_ex( 0, SRAM, 64, EXI_READ );
	exi_deselect( 0 );
}
void SRAM_Write( void *sram )
{
	u32 data = 0xA0000100;

	exi_select( 0, 1, 3);
	exi_imm( 0, &data, 4, EXI_WRITE, 0 );
	exi_sync( 0 );
	exi_imm_ex( 0, SRAM, 64, EXI_WRITE );
	exi_deselect( 0 );
}

void SRAM_Init( void )
{
	SRAM_Read(SRAM);
//	hexdump( SRAM, 64 );
}
void SRAM_SetVideoMode( u8 VideoMode )
{
	GC_SRAM *sram = (GC_SRAM*)SRAM;
	
	if( VideoMode == GCVideoModePAL60 )
	{
		sram->Flags		&= 0x7C;
		sram->BootMode	&= 0xBF;

		sram->BootMode	|= 0x40;
		sram->Flags		|= 0x01;

	} else if( VideoMode == GCVideoModeNTSC )
	{
		sram->Flags		&= 0x7C;
		sram->BootMode	&= 0xBF;		

	} else if( VideoMode == GCVideoModePROG )
	{
		sram->Flags		|= 0x80;
	}
}
GC_SRAM *SRAM_Unlock( void )
{
	return (GC_SRAM*)SRAM;
}
u8 SRAM_GetVideoMode( void )
{
	GC_SRAM *sram = (GC_SRAM*)SRAM;

	if( sram->BootMode & 0x40 )
		return GCVideoModePAL60;

	return sram->Flags & 3;
}
void SRAM_Flush( void )
{
	SRAM_Checksum( (unsigned short *)SRAM, (unsigned short *)SRAM, (unsigned short *)(SRAM+2) );
	SRAM_Write(SRAM);	
}

/* exi_select: enable chip select, set speed */
void exi_select(int channel, int device, int freq)
{
	volatile unsigned long *exi = (volatile unsigned long *)EXI_BASE;
	long d;
	// exi_select
	d = exi[channel * 5];
	d &= 0x405;
	d |= ((1<<device)<<7) | (freq << 4);
	exi[channel*5] = d;
}

/* disable chipselect */
void exi_deselect(int channel)
{
	volatile unsigned long *exi = (volatile unsigned long *)EXI_BASE;
	exi[channel * 5] &= 0x405;
}
/* dirty way for asynchronous reads */
static void *exi_last_addr;
static int   exi_last_len;

/* mode?Read:Write len bytes to/from channel */
/* when read, data will be written back in exi_sync */
void exi_imm(int channel, void *data, int len, int mode, int zero)
{
	volatile unsigned long *exi = (volatile unsigned long *)EXI_BASE;
	if (mode == EXI_WRITE)
		exi[channel * 5 + 4] = *(unsigned long*)data;

	exi[channel * 5 + 3] = ((len-1)<<4)|(mode<<2)|1;
	
	if (mode == EXI_READ)
	{
		exi_last_addr = data;
		exi_last_len = len;
	} else
	{
		exi_last_addr = 0;
		exi_last_len = 0;
	}
}

/* Wait until transfer is done, write back data */
void exi_sync(int channel)
{
	volatile unsigned long *exi = (volatile unsigned long *)EXI_BASE;
	while (exi[channel * 5 + 3] & 1);

	if (exi_last_addr)
	{	
		int i;
		unsigned long d;
		d = exi[channel * 5 + 4];
		for (i=0; i<exi_last_len; ++i)
			((unsigned char*)exi_last_addr)[i] = (d >> ((3-i)*8)) & 0xFF;
	}
}

/* simple wrapper for transfers > 4bytes */
void exi_imm_ex(int channel, void *data, int len, int mode)
{
	unsigned char *d = (unsigned char*)data;
	while (len)
	{
		int tc = len;
		if (tc > 4)
			tc = 4;
		exi_imm(channel, d, tc, mode, 0);
		exi_sync(channel);
		len-=tc;
		d+=tc;
	}
}
