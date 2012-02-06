#ifndef _CFG_
#define _CFG_

#include "string.h"
#include "global.h"
#include "ipc.h"
#include "alloc.h"
#include "vsprintf.h"
#include "HW.h"

#define	EXI_BASE	0x0D806800

void exi_select(int channel, int device, int freq);
void exi_deselect(int channel);

#define EXI_READ  0
#define EXI_WRITE 1

void exi_imm(int channel, void *data, int len, int mode, int zero);
void exi_sync(int channel);
void exi_imm_ex(int channel, void *data, int len, int mode);

void	SRAM_Init( void );
void	SRAM_Flush( void );

u8		SRAM_GetVideoMode( void );
void	SRAM_SetVideoMode( u8 VideoMode );

enum VideoModes
{
	GCVideoModeNone		= 0,
	GCVideoModePAL60	= 1,
	GCVideoModePROG		= 2,
};

typedef struct GC_SRAM 
{
/* 0x00 */	u16 CheckSum1;
/* 0x02 */	u16 CheckSum2;
/* 0x04 */	u32 ead0;
/* 0x08 */	u32 ead1;
/* 0x0C */	u32 CounterBias;
/* 0x10 */	u8	DisplayOffsetH;
/* 0x11 */	u8	BootMode;	// Also holds a PAL60 flag
/* 0x12 */	u8	Language;
/* 0x13 */	u8	Flags;
		/*
			bit			desc			0		1
			0			Prog mode		off		on
			1			EU60			off		on
			2			Sound mode		Mono	Stereo
			3			always 1
			4			always 0
			5			always 1
			6			-\_ Video mode
			7			-/
		*/
/* 0x14 */	u8	FlashID[2*12];
/* 0x2C */	u32	WirelessKBID;
/* 0x30 */	u16	WirlessPADID[4];
/* 0x38 */	u8	LastDVDError;
/* 0x39 */	u8	Reserved;
/* 0x3A */	u16	FlashIDChecksum[2];
/* 0x3C */	u16	Unused;
} GC_SRAM;

#endif