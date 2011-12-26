/*

	DIOS-MIOS

	Copyright (C) 2010  crediar

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

#ifndef _DIP_
#define _DIP_

#include "string.h"
#include "global.h"
#include "ipc.h"
#include "alloc.h"
#include "ff.h"
#include "vsprintf.h"
#include "HW.h"
#include "dol.h"

#define DI_SUCCESS	1
#define DI_ERROR	2
#define DI_FATAL	64

enum opcodes
{
	DVD_IDENTIFY			= 0x12,
	DVD_READ_DISCID			= 0x70,
	DVD_LOW_READ			= 0x71,
	DVD_WAITFORCOVERCLOSE	= 0x79,
	DVD_READ_PHYSICAL		= 0x80,
	DVD_READ_COPYRIGHT		= 0x81,
	DVD_READ_DISCKEY		= 0x82,
	DVD_GETCOVER			= 0x88,
	DVD_RESET				= 0x8A,
	DVD_OPEN_PARTITION		= 0x8B,
	DVD_CLOSE_PARTITION		= 0x8C,
	DVD_READ_UNENCRYPTED	= 0x8D,
	DVD_REPORTKEY			= 0xA4,
	DVD_LOW_SEEK			= 0xAB,
	DVD_READ				= 0xD0,
	DVD_READ_CONFIG			= 0xD1,
	DVD_READ_BCA			= 0xDA,
	DVD_GET_ERROR			= 0xE0,
	DVD_SET_MOTOR			= 0xE3,

	DVD_SELECT_GAME			= 0x23,
	DVD_GET_GAMECOUNT		= 0x24,
	DVD_EJECT_DISC			= 0x27,
	DVD_INSERT_DISC			= 0x28,
	DVD_READ_GAMEINFO		= 0x30,
};

enum GameRegion 
{
	JAP=0,
	USA,
	EUR,
	KOR,
	ASN,
	LTN,
};

typedef struct
{
	u32		SlotID;
	u32		Region;
	u32		Gamecount;
	u32		Config;
	u8		GameInfo[][0x80];
} DIConfig;

#define		DI_BASE		0x00002F00

#define		DI_STATUS	(DI_BASE+0x00)
#define		DI_COVER	(DI_BASE+0x04)
#define		DI_CMD_0	(DI_BASE+0x08)
#define		DI_CMD_1	(DI_BASE+0x0C)
#define		DI_CMD_2	(DI_BASE+0x10)
#define		DI_DMA_ADR	(DI_BASE+0x14)
#define		DI_DMA_LEN	(DI_BASE+0x18)
#define		DI_CONTROL	(DI_BASE+0x1C)
#define		DI_IMM		(DI_BASE+0x20)
#define		DI_CONFIG	(DI_BASE+0x24)

#define		DI_SHADOW	(DI_BASE + 0x30)

#define		DI_SSTATUS	(DI_SHADOW+0x00)
#define		DI_SCOVER	(DI_SHADOW+0x04)
#define		DI_SCMD_0	(DI_SHADOW+0x08)
#define		DI_SCMD_1	(DI_SHADOW+0x0C)
#define		DI_SCMD_2	(DI_SHADOW+0x10)
#define		DI_SDMA_ADR	(DI_SHADOW+0x14)
#define		DI_SDMA_LEN	(DI_SHADOW+0x18)
#define		DI_SCONTROL	(DI_SHADOW+0x1C)
#define		DI_SIMM		(DI_SHADOW+0x20)
#define		DI_SCONFIG	(DI_SHADOW+0x24)


//typedef struct
//{
//	u32 Status;			//	0x00
//	u32 Cover;			//	0x04
//	u32 Command0;		//	0x08
//	u32 Command1;		//	0x0C
//	u32 Command2;		//	0x10
//	u32 DMA_Address;	//	0x14
//	u32 DMA_Length;		//	0x18
//	u32 Control;		//	0x1C
//	u32 Immedite;		//	0x20
//	u32 Config;			//	0x24
//
//} DiscInterface;


typedef struct
{
	union
	{
		struct
		{
			u32 Type		:8;
			u32 NameOffset	:24;
		};
		u32 TypeName;
	};
	union
	{
		struct		// File Entry
		{
			u32 FileOffset;
			u32 FileLength;
		};
		struct		// Dir Entry
		{
			u32 ParentOffset;
			u32 NextOffset;
		};
		u32 entry[2];
	};
} FEntry;

typedef struct
{
	u32 Offset;
	u32 Size;
	FIL File;
} FileCache;

#define FILECACHE_MAX	1

typedef struct
{
	u8 *data;
	u32 len;
} vector;

typedef struct
{
	u32 TMDSize;
	u32 TMDOffset;
	u32	CertChainSize;
	u32 CertChainOffset;
	u32 H3TableOffset;
	u32	DataOffset;
	u32 DataSize;
} PartitionInfo;

void DIInit( void );
void DIUpdateRegisters( void );
u32 LowReadDiscID( void *data );
u32 DVDLowGetError( void );
void DVDReadConfig( void );
s32 DVDSelectGame( int SlotID );

#endif