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

#ifndef _DSP_
#define _DSP_

#include "string.h"
#include "global.h"
#include "ipc.h"
#include "alloc.h"
#include "ff.h"
#include "vsprintf.h"

#define		DSP_BASE			0x00002480

#define		DSP_DSP_MAILBOX_HIGH	(DSP_BASE + 0x00)
#define		DSP_DSP_MAILBOX_LOW		(DSP_BASE + 0x02)

#define		DSP_CPU_MAILBOX_HIGH	(DSP_BASE + 0x04)
#define		DSP_CPU_MAILBOX_LOW		(DSP_BASE + 0x06)

#define		DSP_STATUS				(DSP_BASE + 0x0A)
#define		DSP_SIZE				(DSP_BASE + 0x12)
#define		DSP_MODE				(DSP_BASE + 0x16)
#define		DSP_REFRESH				(DSP_BASE + 0x1A)

#define		DSP_DMA_MADDR			(DSP_BASE + 0x20)
#define		DSP_DMA_MADDR_HIGH		(DSP_BASE + 0x20)
#define		DSP_DMA_MADDR_LOW		(DSP_BASE + 0x22)
#define		DSP_DMA_AADDR			(DSP_BASE + 0x24)
#define		DSP_DMA_AADDR_HIGH		(DSP_BASE + 0x24)
#define		DSP_DMA_AADDR_LOW		(DSP_BASE + 0x26)
#define		DSP_DMA_COUNT			(DSP_BASE + 0x28)
#define		DSP_DMA_COUNT_HIGH		(DSP_BASE + 0x28)
#define		DSP_DMA_COUNT_LOW		(DSP_BASE + 0x2A)
#define		DSP_DMA_START_HIGH		(DSP_BASE + 0x30)
#define		DSP_DMA_START_LOW		(DSP_BASE + 0x32)

#define		DSP_DMA_CONTROL			(DSP_BASE + 0x36)
#define		DSP_DMA_BYTES_LEFT		(DSP_BASE + 0x3A)


#define		DSP_SBASE				(DSP_BASE + 0x40)

#define		DSP_SDSP_MAILBOX_HIGH	(DSP_SBASE + 0x00)
#define		DSP_SDSP_MAILBOX_LOW	(DSP_SBASE + 0x02)

#define		DSP_SCPU_MAILBOX_HIGH	(DSP_SBASE + 0x04)
#define		DSP_SCPU_MAILBOX_LOW	(DSP_SBASE + 0x06)

#define		DSP_SSTATUS				(DSP_SBASE + 0x0A)
#define		DSP_SSIZE				(DSP_SBASE + 0x12)
#define		DSP_SMODE				(DSP_SBASE + 0x16)
#define		DSP_SREFRESH			(DSP_SBASE + 0x1A)

#define		DSP_SDMA_MADDR			(DSP_SBASE + 0x20)
#define		DSP_SDMA_MADDR_HIGH		(DSP_SBASE + 0x20)
#define		DSP_SDMA_MADDR_LOW		(DSP_SBASE + 0x22)
#define		DSP_SDMA_AADDR			(DSP_SBASE + 0x24)
#define		DSP_SDMA_AADDR_HIGH		(DSP_SBASE + 0x24)
#define		DSP_SDMA_AADDR_LOW		(DSP_SBASE + 0x26)
#define		DSP_SDMA_COUNT			(DSP_SBASE + 0x28)
#define		DSP_SDMA_COUNT_HIGH		(DSP_SBASE + 0x28)
#define		DSP_SDMA_COUNT_LOW		(DSP_SBASE + 0x2A)
#define		DSP_SDMA_START_HIGH		(DSP_SBASE + 0x30)
#define		DSP_SDMA_START_LOW		(DSP_SBASE + 0x32)

#define		DSP_SDMA_CONTROL		(DSP_SBASE + 0x36)
#define		DSP_SDMA_BYTES_LEFT		(DSP_SBASE + 0x3A)

void DSPInit( void );
void DSPUpdateRegisters( void );

#endif
