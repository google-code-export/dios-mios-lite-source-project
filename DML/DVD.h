#ifndef _DVD_
#define _DVD_

#include "global.h"
#include "HW.h"
#include "Config.h"
#include "ff.h"
#include "dol.h"

typedef struct
{
	u32		SlotID;
	u32		Region;
	u32		Gamecount;
	u32		Config;
	u8		GameInfo[][0x80];
} DVDConfig;

void DVDReadConfig( void );
s32 DVDSelectGame( void );

#endif
