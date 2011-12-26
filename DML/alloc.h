#include "utils.h"
#include "memory.h"
#include "HW.h"

#ifndef _ALLOC_
#define _ALLOC_

#define _AHEAP_SIZE_TOTAL	0x8000
#define _AHEAP_INFO_SIZE	0x0100
#define _AHEAP_SIZE		_AHEAP_SIZE_TOTAL-_AHEAP_INFO_SIZE

typedef struct
{
	u8 *Offset;
	u32 Size;
} HeapInfoEntry;

void HeapInit( void );
void *malloc( u32 size );
void *malloca( u32 size, u32 align );
void free( void *ptr );

#endif
