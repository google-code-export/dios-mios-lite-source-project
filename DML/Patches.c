#include "Patches.h"

#include "CardPatches.c"
#include "DVDPatches.c"
#include "FwritePatches.c"
#include "CheatCode.c"

// Audio streaming replacement functions copied from Swiss r92
u32 __dvdLowAudioStatusNULL[17] = {
        // execute function(1); passed in on r4
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7C8903A6,     //  mtctr       r4
        0x3C80CC00,     //  lis         r4, 0xCC00
        0x2E830000,     //  cmpwi       cr5, r3, 0
        0x4196000C,     //  beq-        cr5, +0xC ?
        0x38600001,     //  li          r3, 1
        0x48000008,     //  b           +0x8 ?
        0x38600000,     //  li          r3, 0
        0x90646020,     //  stw         r3, 0x6020 (r4)
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctrl
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020      //  blr
};

u32 __dvdLowAudioConfigNULL[10] = {
        // execute callback(1); passed in on r5 without actually touching the drive!
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7CA903A6,     //  mtctr       r5
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctrl
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020      //  blr
};

u32 __dvdLowReadAudioNULL[] = {
        // execute callback(1); passed in on r6 without actually touching the drive!
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7CC903A6,     //  mtctr       r6
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctr;
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020
};


unsigned char OSReportDM[] =
{
	0x7C, 0x08, 0x02, 0xA6, 0x90, 0x01, 0x00, 0x04, 0x90, 0xE1, 0x00, 0x08, 0x3C, 0xE0, 0xC0, 0x00, 
	0x90, 0x67, 0x18, 0x60, 0x90, 0x87, 0x18, 0x64, 0x90, 0xA7, 0x18, 0x68, 0x90, 0xC7, 0x18, 0x6C, 
	0x90, 0xE7, 0x18, 0x70, 0x91, 0x07, 0x18, 0x74, 0x80, 0x07, 0x18, 0x60, 0x7C, 0x00, 0x18, 0x00, 
	0x41, 0x82, 0xFF, 0xF8, 0x80, 0xE1, 0x00, 0x08, 0x80, 0x01, 0x00, 0x04, 0x7C, 0x08, 0x03, 0xA6, 
	0x4E, 0x80, 0x00, 0x20, 
} ;

FuncPattern FPatterns[] =
{
	//{ 0xBC,			20,     3,      3,      4,      7,	DVDReadAsync,			sizeof(DVDReadAsync),			"DVDReadAsync",					0,		0 },
	//{ 0x114,        23,     2,      6,      9,      8,	DVDRead,				sizeof(DVDRead),				"DVDRead",						0,		0 },
	//{ 0xD8,			17,     12,     5,      3,      2,	DVDReadAbsAsyncPrioAsync,		sizeof(DVDReadAbsAsyncPrioAsync),	"DVDReadAbsAsyncPrio",			0,		0 },
	{ 0xD8,			17,     12,     5,      3,      2,	(u8*)NULL,		0xdead0006,	"DVDReadAbsAsyncPrio",			0,		0 },

	{ 0xCC,			17,     10,     5,      3,      2,	DVDInquiryAsync,			sizeof(DVDInquiryAsync),		"DVDInquiryAsync",				0,		0 },
	{ 0xC8,			16,     9,      5,      3,      3,	DVDSeekAbsAsyncPrio,		sizeof(DVDSeekAbsAsyncPrio),	"DVDSeekAbsAsyncPrio",			0,		0 },
	//{ 0xD4,			13,     8,      11,     2,      7,	(u8*)NULL,					0xdead0004,						"AIResetStreamSampleCount",			0,		0 },
	//{ 0xC0,			15,     9,      5,      3,      2,	DVDPrepareStreamAbsAsync,	sizeof(DVDPrepareStreamAbsAsync),"DVDPrepareStreamAbsAsync",	0,		0 },
	//{ 0xB8,			15,     7,      5,      3,      2,	(u8*)NULL,					0xdead0010,						"DVDStream*",					0,		0 },

	//{ 0x2DC,        56,     23,     9,      21,     16, __DVDInterruptHandler,		sizeof(__DVDInterruptHandler),	"__DVDInterruptHandler",		2,		0 },
	//{ 0x2F0,        60,     27,     9,      21,     17, __DVDInterruptHandler,		sizeof(__DVDInterruptHandler),	"__DVDInterruptHandler",		2,		0 },

	{ 0x308,        40,     18,     10,     23,     17,	patch_fwrite_GC,			sizeof(patch_fwrite_GC),		"__fwrite A",					1,		0 },
	{ 0x338,        48,     20,     10,     24,     16,	patch_fwrite_GC,			sizeof(patch_fwrite_GC),		"__fwrite B",					1,		0 },
	{ 0x2D8,        41,     17,     8,      21,     13,	patch_fwrite_GC,			sizeof(patch_fwrite_GC),		"__fwrite C",					1,		0 },

	{ 0x824,        111,    44,     13,     53,     64,	(u8*)NULL,					0xdead0000,						"VIInit",						0,		0 },
	
	{ 0x94, 		18, 	10, 	2, 		0, 		2, 		(u8*)__dvdLowReadAudioNULL, sizeof(__dvdLowReadAudioNULL), "DVDLowReadAudio", 0, 0 },
	{ 0x88, 		18, 	8, 		2, 		0, 		2, 		(u8*)__dvdLowAudioStatusNULL, sizeof(__dvdLowAudioStatusNULL), "DVDLowAudioStatus", 0, 0 },
	{ 0x98, 		19, 	8, 		2, 		1, 		3, 		(u8*)__dvdLowAudioConfigNULL, sizeof(__dvdLowAudioConfigNULL), "DVDLowAudioConfig", 0, 0 },
};		


FuncPattern LPatterns[] =
{	
	{ 0xFC,			20,     4,      7,      6,      7,	DVDReadAbsAsyncPrio,	sizeof(DVDReadAbsAsyncPrio),	"DVDReadAbsAsyncPrio",		1,		0 },
	{ 0xD8,			17,     12,     5,      3,      2,	DVDReadAbsAsyncPrio,	sizeof(DVDReadAbsAsyncPrio),	"DVDReadAbsAsyncPrio",		1,		0 },
	{ 0x11C,        26,     9,      7,      3,      3,	DVDReadAbsAsyncPrio,	sizeof(DVDReadAbsAsyncPrio),	"DVDReadAbsAsyncPrioForBS",	2,		0 },
	{ 0xCC,			16,     11,     5,      3,      2,	DVDReadAbsAsyncPrio,	sizeof(DVDReadAbsAsyncPrio),	"DVDReadAbsAsyncPrioForBS",	2,		0 },


	{ 0x270,        70,     6,      13,     12,     13,	dDVDReadAbs,			sizeof(dDVDReadAbs),			"DVDReadAbs",				0,		0 },
	{ 0x280,        50,     22,     8,      18,     12, __DVDInterruptHandler,	sizeof(__DVDInterruptHandler), "__DVDInterruptHandler",		4,		0 },
	{ 0x2DC,        56,     23,     9,      21,     16, __DVDInterruptHandler,	sizeof(__DVDInterruptHandler), "__DVDInterruptHandler",		4,		0 },

#ifdef fwrite_patch
	{ 0x308,        40,     18,     10,     23,     17,	patch_fwrite_GC,		sizeof(patch_fwrite_GC),		"__fwrite A",				5,		0 },
	{ 0x338,        48,     20,     10,     24,     16,	patch_fwrite_GC,		sizeof(patch_fwrite_GC),		"__fwrite B",				5,		0 },
	{ 0x2D8,        41,     17,     8,      21,     13,	patch_fwrite_GC,		sizeof(patch_fwrite_GC),		"__fwrite C",				5,		0 },
#endif
};

FuncPattern CPatterns[] =
{
	{ 0x14C,        28,     12,     7,      12,     4,	CARDFreeBlocks,		sizeof(CARDFreeBlocks),	"CARDFreeBlocks A",		1,		0 },
	//{ 0x11C,        24,     10,     7,      10,     4,	CARDFreeBlocks,		sizeof(CARDFreeBlocks),	"CARDFreeBlocks B",		1,		0 },
	//{ 0x94,			11,     6,      3,      5,      4,	__CARDSync,			sizeof(__CARDSync),		"__CARDSync",			0,		0 },
	//{ 0x50,			6,      3,      2,      2,      2,	CARDCheck,			sizeof(CARDCheck),		"CARDCheck",			0,		0 },
	////{ 0x24,			4,      2,      1,      0,      2,	CARDCheckAsync,		sizeof(CARDCheckAsync),	"CARDCheckAsync",	0,		0 },
	//{ 0x58C,        82,     11,     18,     41,     57,	CARDCheckEX,		sizeof(CARDCheckEX),	"CARDCheckExAsync",		0,		0 },
	//{ 0x34,			4,      2,      1,      2,      2,	CARDProbe,			sizeof(CARDProbe),		"CARDProbe",			2,		0 },
	////{ 0x1C,			2,      2,      1,      0,      2,	CARDProbe,			sizeof(CARDProbe),		"CARDProbe B",			2,		0 },	//This is causing more trouble than a hack...
	//{ 0x178,        20,     6,      6,      20,     4,	CARDProbeEX,		sizeof(CARDProbeEX),	"CARDProbeEx A",		3,		0 },
	//{ 0x198,        22,     6,      5,      19,     4,	CARDProbeEX,		sizeof(CARDProbeEX),	"CARDProbeEx B",		3,		0 },
	//{ 0x160,        17,     6,      5,      18,     4,	CARDProbeEX,		sizeof(CARDProbeEX),	"CARDProbeEx C",		3,		0 },
	//{ 0x19C,        32,     14,     11,     12,     3,	CARDMountAsync,		sizeof(CARDMountAsync),	"CARDMountAsync A",		4,		0 },
	//{ 0x184,        30,     14,     11,     10,     3,	CARDMountAsync,		sizeof(CARDMountAsync),	"CARDMountAsync B",		4,		0 },	
	//{ 0x174,        23,     6,      7,      14,     5,	CARDOpen,			sizeof(CARDOpen),		"CARDOpen A",			5,		0 },
	//{ 0x118,        14,     6,      6,      11,     4,	CARDOpen,			sizeof(CARDOpen),		"CARDOpen B",			5,		0 },
	//{ 0x170,        23,     6,      7,      14,     5,	CARDOpen,			sizeof(CARDOpen),		"CARDOpen C",			5,		0 },	
	//{ 0x15C,        27,     6,      5,      15,     6,	CARDFastOpen,		sizeof(CARDFastOpen),	"CARDFastOpen",			0,		0 },
	//{ 0x50,			8,      4,      2,      2,      3,	CARDClose,			sizeof(CARDClose),		"CARDClose",			0,		0 },
	//{ 0x21C,        44,     6,      13,     19,     12,	CARDCreate,			sizeof(CARDCreate),		"CARDCreateAsync A",	6,		0 },
	//{ 0x214,        42,     6,      13,     19,     12,	CARDCreate,			sizeof(CARDCreate),		"CARDCreateAsync B",	6,		0 },	
	//{ 0x10C,        25,     6,      9,      9,      5,	CARDDelete,			sizeof(CARDDelete),		"CARDDeleteAsync",		0,		0 },
	//{ 0x144,        27,     3,      8,      10,     9,	CARDRead,			sizeof(CARDRead),		"CARDReadAsync A",		7,		0 },
	//{ 0x140,        30,     7,      7,      10,     10,	CARDRead,			sizeof(CARDRead),		"CARDReadAsync B",		7,		0 },
	//{ 0x140,        27,     3,      8,      10,     9,	CARDRead,			sizeof(CARDRead),		"CARDReadAsync C",		7,		0 },	
	//{ 0x110,        24,     4,      8,      9,      6,	CARDWrite,			sizeof(CARDWrite),		"CARDWriteAsync A",		8,		0 },
	//{ 0x10C,        23,     4,      8,      9,      6,	CARDWrite,			sizeof(CARDWrite),		"CARDWriteAsync B",		8,		0 },	
	//{ 0x128,        25,     9,      9,      6,      5,	CARDGetStats,		sizeof(CARDGetStats),	"CARDGetStatus A",		9,		0 },
	//{ 0x110,        25,     9,      8,      6,      5,	CARDGetStats,		sizeof(CARDGetStats),	"CARDGetStatus B",		9,		0 },
	//{ 0x124,        25,     9,      9,      6,      5,	CARDGetStats,		sizeof(CARDGetStats),	"CARDGetStatus C",		9,		0 },	
	//{ 0x170,        29,     9,      9,      12,     5,	CARDSetStats,		sizeof(CARDSetStats),	"CARDSetStatusAsync A",	10,		0 },
	//{ 0x16C,        29,     9,      9,      12,     5,	CARDSetStats,		sizeof(CARDSetStats),	"CARDSetStatusAsync B",	10,		0 },	
	//{ 0xC0,			22,     5,      2,      5,      10,	CARDGetSerialNo,	sizeof(CARDGetSerialNo),"CARDGetSerialNo",		0,		0 },
	//{ 0x84,			12,     5,      3,      4,      2,	CARDGetEncoding,	sizeof(CARDGetEncoding),"CARDGetEncoding",		0,		0 },
	//{ 0x80,			11,     5,      3,      4,      2,	CARDGetMemSize,		sizeof(CARDGetMemSize),	"CARDGetMemSize",		0,		0 },
};

u32 CardLowestOff = 0;

void PatchB( u32 dst, u32 src )
{
	u32 newval = (dst - src);
	newval&= 0x03FFFFFC;
	newval|= 0x48000000;
	write32( src, newval );	
}
void PatchBL( u32 dst, u32 src )
{
	u32 newval = (dst - src);
	newval&= 0x03FFFFFC;
	newval|= 0x48000001;
	write32( src, newval );	
}
void PatchGCIPL( void )
{
	memcpy( (void*)0x1304048, mDVDReadAbsAsyncPrioShift, sizeof(mDVDReadAbsAsyncPrioShift) );
	PatchB( 0x1304048, 0x1306794 );
	memcpy( (void*)0x1303D7C, __DVDInterruptHandler, sizeof(__DVDInterruptHandler) );

	memcpy( (void*)0x1306864, DVDReadDiscIDAsync, sizeof(DVDReadDiscIDAsync) );		
	
	memcpy( (void*)0x1304320, OSReportDM, sizeof(OSReportDM) );

	write32( 0x1301AE8, 0x60000000 );	// This BS2Report seems to break loading for some reason
	
//OSReport
	PatchB( 0x1304320, 0x1301CEC );
//BS2Report
	PatchB( 0x1304320, 0x1301DA0 );
//BS2Panic
	PatchB( 0x1304320, 0x1301D3C );
//UNKReport
	PatchB( 0x1304320, 0x13117AC );
}

void create_Pattern(u8 *Data, u32 Length, FuncPattern *FP)
{
	u32 i;

	memset( FP, 0, sizeof(FuncPattern) );

	for( i = 0; i < Length; i+=4 )
	{
		u32 word =  read32( (u32)Data + i );
		
		if( (word & 0xFC000003) ==  0x48000001 )
			FP->FCalls++;

		if( (word & 0xFC000003) ==  0x48000000 )
			FP->Branch++;
		if( (word & 0xFFFF0000) ==  0x40800000 )
			FP->Branch++;
		if( (word & 0xFFFF0000) ==  0x41800000 )
			FP->Branch++;
		if( (word & 0xFFFF0000) ==  0x40810000 )
			FP->Branch++;
		if( (word & 0xFFFF0000) ==  0x41820000 )
			FP->Branch++;
		
		if( (word & 0xFC000000) ==  0x80000000 )
			FP->Loads++;
		if( (word & 0xFF000000) ==  0x38000000 )
			FP->Loads++;
		if( (word & 0xFF000000) ==  0x3C000000 )
			FP->Loads++;
		
		if( (word & 0xFC000000) ==  0x90000000 )
			FP->Stores++;
		if( (word & 0xFC000000) ==  0x94000000 )
			FP->Stores++;

		if( (word & 0xFF000000) ==  0x7C000000 )
			FP->Moves++;

		if( word == 0x4E800020 )
			break;
	}

	FP->Length = i;

	//if( (u32)Data == 0x336E08 )
	//{
	//	dbgprintf("Length: 0x%02X\n", FP.Length );
	//	dbgprintf("Loads : %d\n", FP.Loads );
	//	dbgprintf("Stores: %d\n", FP.Stores );
	//	dbgprintf("FCalls: %d\n", FP.FCalls );
	//	dbgprintf("Branch: %d\n", FP.Branch );
	//	dbgprintf("Moves : %d\n", FP.Moves );
	//	dbgprintf("Res   : %d\n", memcmp( &FP, FunctionPattern, sizeof(u32) * 6 ) );	
	//}
}

bool compare_Pattern( FuncPattern *FP1, FuncPattern *FP2 )
{
	if( memcmp( FP1, FP2, sizeof(u32) * 6 ) == 0 )
		return true;
	else
		return false;
}

void DoCardPatches( char *ptr, u32 size, u32 SectionOffset )
{
	u32 i,j,k,offset,fail,FoundCardFuncStart=0;
	FuncPattern temp_FP;

	dbgprintf("DoCardPatches( 0x%p, %d, 0x%X)\n", ptr, size, SectionOffset );
	
	for( i=0; i < size; i+=4 )
	{
		if( read32( (u32)ptr + i ) != 0x7C0802A6 )	// MFLR
			continue;
		
		create_Pattern((u8*)(ptr+i), size, &temp_FP);

		for( j=0; j < sizeof(CPatterns)/sizeof(FuncPattern); ++j )
		{
			if( CPatterns[j].PatchLength == 0 )
				continue;
			
			if( CPatterns[j].Found )				// Skip already found patches
				continue;

			if( compare_Pattern( &temp_FP, &(CPatterns[j]) ) )
			{
				if( CPatterns[j].Patch == CARDFreeBlocks )
				{
					if( CardLowestOff == 0 )
					{
						dbgprintf("CardLowestOff:0x%08X\n", i );
						CardLowestOff = i;
					}

					//Check for CARDGetResultCode which is always (when used) above CARDFreeBlocks
					if( read32( (u32)ptr + i - 0x30 ) == 0x2C030000 )
					{
						dbgprintf("Found [CARDGetResultCode] @ 0x%08X\n", (u32)ptr + i - 0x30 + SectionOffset );
						memcpy( ptr + i - 0x30, CARDGetResultCode, sizeof(CARDGetResultCode) );
					}

					FoundCardFuncStart = 1;
				}

				if( FoundCardFuncStart == 0 )
					continue;

				dbgprintf("Found [%s] @ 0x%08X\n", CPatterns[j].Name, (u32)ptr + i + SectionOffset );
				CPatterns[j].Found = (u32)ptr + i;

				// If this is a patch group set all others of this group as found aswell
				if( CPatterns[j].Group )
				{
					for( k=0; k < sizeof(CPatterns)/sizeof(FuncPattern); ++k )
					{
						if( CPatterns[k].Group == CPatterns[j].Group )
						{
							if( !CPatterns[k].Found )	//Don't overwrite the offset!
								CPatterns[k].Found = -1;	// Usually this holds the offset, to determinate it from a REALLY found pattern we set it -1 which still counts a logical TRUE
							
							//dbgprintf("Setting [%s] to found!\n", CPatterns[k].Name );
						}
					}
				}

				//If by now no CARDProbe is found it won't be so set it to found to prevent CARDProbe B false hits
				if( CPatterns[j].Patch == CARDRead )
				{
					for( k=0; k < sizeof(CPatterns)/sizeof(FuncPattern); ++k )
					{
						if( CPatterns[k].Patch == CARDProbe )
						{
							if( !CPatterns[k].Found )	//Don't overwrite the offset!
								CPatterns[k].Found = -1;
						}
					}
				}


				if( strstr( CPatterns[j].Name, "Async" ) != NULL )
				{
					//dbgprintf("Async!\n");

					//Most games only use the normal functions so we patch a branch over to async and clear the CB
					//Find function call to our function

					offset = (u32)ptr + i;
					fail = 0;
					
					while(fail < 3)
					{
						//dbgprintf("[%08X] %08X %08X(%08X)\n", offset, read32( offset ) & 0xFC000003,read32( offset ) & 0x03FFFFFC ,(read32( offset ) & 0x03FFFFFC ) + offset);
						if( (read32( offset ) & 0xFC000003 ) == 0x48000001 )
						{
							if( (((read32( offset ) & 0x03FFFFFC ) + offset) & 0x03FFFFFC) == (u32)ptr+i ) 
								break;
						}

						if( read32( offset ) == 0x4E800020 )
							fail++;

						offset+=4;
					}

					if( fail < 3 )
					{
						dbgprintf("Found function call to [%s] @ 0x%08X\n", CPatterns[j].Name, offset + SectionOffset );
			
						//Now find function start 
						offset -= 4;
						while(1)
						{
							if( read32( offset ) == 0x7C0802A6 )
								break;

							offset-=4;
						}
					
						dbgprintf("Found function start of [%s(Sync)] @ 0x%08X\n", CPatterns[j].Name, offset + SectionOffset );

						//This patches a li rX, 0 before the Async function call for the Sync call
						//Since this register of the cb is different per function we do this haxx
						if( (read32( offset + 0x04 ) & 0x0000F000 ) == 0x00008000 )	// lis
						{
							write32( offset, read32( offset + 0x0C ) & 0xFBE00000 );

							//Forge a branch to the async function

							offset += 4;

							u32 newval = ((u32)ptr + i) - offset;
							newval&= 0x03FFFFFC;
							newval|= 0x48000000;
							write32( offset, newval );
						} else {
							dbgprintf("Unhandled Async cb case!\n");
						}

					} else {
						dbgprintf("No sync function found!\n");
					}

					memcpy( ptr + i, CPatterns[j].Patch, CPatterns[j].PatchLength );

				} else {
					memcpy( ptr + i, CPatterns[j].Patch, CPatterns[j].PatchLength );					
				}
			}
		}
	}
	
	for( j=0; j < sizeof(CPatterns)/sizeof(FuncPattern); ++j )
	{
		if( CPatterns[j].Found == 0 )
			dbgprintf("Pattern %s not found!\n", CPatterns[j].Name );
	}

	return;

}
void DoPatchesLoader( char *ptr, u32 size )
{
	u32 i=0,j=0,k=0;
	//u32 offset=0,read;
	FuncPattern temp_FP;

	for( j=0; j < sizeof(LPatterns)/sizeof(FuncPattern); ++j )
		LPatterns[j].Found = 0;	

	dbgprintf("DoPatchesLoader( 0x%p, %d )\n", ptr, size );

	for( i=0; i < size; i+=4 )
	{
		if( read32( (u32)ptr + i ) != 0x4E800020 )
			continue;

		i+=4;

		create_Pattern((u8*)(ptr+i), size, &temp_FP);

		for( j=0; j < sizeof(LPatterns)/sizeof(FuncPattern); ++j )
		{
			if( LPatterns[j].Found ) //Skip already found patches
				continue;

			if( compare_Pattern( &temp_FP, &(LPatterns[j]) ) )
			{
				dbgprintf("Patch:Found [%s]: 0x%08X\n", LPatterns[j].Name, ((u32)ptr + i) | 0x80000000 );

				memcpy( (void*)(ptr+i), LPatterns[j].Patch, LPatterns[j].PatchLength );

				LPatterns[j].Found = 1;

				// If this is a patch group set all others of this group as found aswell
				if( LPatterns[j].Group &&  LPatterns[j].Group != 0xdeadbeef )
				{
					for( k=0; k < sizeof(LPatterns)/sizeof(FuncPattern); ++k )
					{
						if( LPatterns[k].Group == LPatterns[j].Group )
						{
							if( !LPatterns[k].Found )	//Don't overwrite the offset!
								LPatterns[k].Found = -1;	// Usually this holds the offset, to determinate it from a REALLY found pattern we set it -1 which still counts a logical TRUE
							
							//dbgprintf("Setting [%s] to found!\n", LPatterns[k].Name );
						}
					}
				}
			}
		}
	}

}
void DoPatches( char *ptr, u32 size, u32 SectionOffset, u32 UseCache )
{
	u32 magicword;
	FIL PCache;
	u32 i=0,j=0,k=0,read;
	//u32 offset=0;
	FuncPattern temp_FP;
	
	//u32 __DVDIHOffset	= 0;
	//u32 DVDLROffset		= 0;
	PatchCache PC;

	dbgprintf("DoPatches( 0x%p, %d, 0x%X)\n", ptr, size, SectionOffset );

	//EXIControl(1);

//Hacks and other stuff

	//Eternal Darkness MemcardReport
	if( read32(0) == 0x47454450 )
	{
		u32 newval = 0x0200C0C - 0x0170414;
		newval&= 0x03FFFFFC;
		newval|= 0x48000000;
		write32( 0x0170414, newval );
	}

//Note: ORing the values prevents an early break out when a single patterns has multiple hits

#ifdef CODES
		u32 DebuggerHook = 0x0;		
		for( i=0; i < size; i+=4 )
		{
			//GC VI Hook
			if( read32( (u32)ptr + i ) == 0x906402E4 && read32( (u32)ptr + i + 4 ) == 0x38000000 
				&& read32( (u32)ptr + i + 8 ) == 0x900402E0 && read32( (u32)ptr + i + 12 ) == 0x909E0004 )
			{
				j = 0;
				while( read32( (u32)ptr + i + j ) != 0x4E800020 )
					j+=4;
				DebuggerHook = (u32)ptr + i + j;
				write32( 0x0D800070, 1 );
				dbgprintf("Debugger:[Patches.c] DebuggerHook: %08X\n", DebuggerHook);
				write32( 0x0D800070, 0 );
			}
		}

		if( DebuggerHook > 0x0)		
		{ 	
			u32 newval = 0x18A8 - DebuggerHook;
			newval &= 0x03FFFFFC;
			newval |= 0x48000000;
			write32( DebuggerHook, newval );
			write32( 0x0D800070, 1 );
			dbgprintf("Debugger:[Patches.c] Change GC VI hook blr at %08X: %08X\n",  DebuggerHook, newval);
			write32( 0x0D800070, 0 );
		}
#endif
	
	//cache stuff
	static char cachename[32] ALIGNED(32);
	sprintf(cachename, "cache%08x.bin", UseCache);

	if (UseCache && f_open( &PCache, cachename, FA_READ | FA_OPEN_EXISTING ) == FR_OK)
	{
		dbgprintf("Patch:Found cache file\n");
		
		if( PCache.fsize == 0 )
		{
			dbgprintf("Patch:But file size is 0\n");
			f_close( &PCache );
			goto SPatches;
		}
		f_read( &PCache, &magicword, 4, &read );
		if (magicword != 0xC0DE0000)
		{
			dbgprintf("Patch:Wrong magic word: %08x\n");
			f_close( &PCache );
			goto SPatches;		
		}
	} else {

SPatches:
		//Reset all 'Founds'
		//if( UseCache == 0 )
		//{
			for( j=0; j < sizeof(FPatterns)/sizeof(FuncPattern); ++j )
				FPatterns[j].Found = 0;
		//}

		// Open the cache file to write the patch locations into it
		f_open( &PCache, cachename, FA_WRITE | FA_READ | FA_CREATE_ALWAYS );
		
		magicword = 0xC0DE0000;
		f_write( &PCache, &magicword, 4, &read );

		memset( &PC, 0, sizeof( PatchCache ) );

		//DoCardPatches( ptr, size, SectionOffset );
		
		//PC.Offset  = CardLowestOff;
		//PC.PatchID = 0xdead0002;

		//f_write( &PCache, &PC, sizeof( PatchCache ), &read );
		
		// DVD read patch
		bool dvd_read_patched = false;
		for( i=0; ((i < size) && (!dvd_read_patched)); i+=4 )
		{
			if( read32( (u32)ptr + i ) == 0x3C60A800 ||	// Games
				read32( (u32)ptr + i ) == 0x3C00A800	// DolLoaders
				) 
			{
				u32 Loader = 0;

				if( read32( (u32)ptr + i ) == 0x3C00A800 )
					Loader = 1;

				int j=0;
				while( read32( (u32)ptr + i - j ) != 0x7C0802A6 )
					j+=4;

				//Check if there is a lis %rX, 0xCC00 in this function
				//At least Sunshine has one false hit on lis r3,0xA800
				int k=0;
				while( 1 )
				{
					if( read32( (u32)ptr + i + k - j ) == 0x4E800020 )
						break;
					if( (read32( (u32)ptr + i + k - j ) & 0xF81FFFFF) == 0x3800CC00 )
						break;

					k += 4;
				}
			
				if( read32( (u32)ptr + i + k - j ) == 0x4E800020 )
					continue;
				
				PC.Offset  = (u32)ptr + i - j;
				PC.PatchID = 0xdead0005;
				
				dbgprintf("Patch:Found [DVDLowRead]: 0x%08X\n", PC.Offset + SectionOffset );

				f_write( &PCache, &PC, sizeof( PatchCache ), &read );
				dvd_read_patched = true;
			}			
		}
		if (!dvd_read_patched)
		{
			dbgprintf("Patch:Critical Error [DVDLowRead] not patched\n");
		}

		for( i=0; i < size; i+=4 )
		{
			if( read32( (u32)ptr + i ) != 0x4E800020 )
				continue;

			i+=4;

			create_Pattern((u8*)(ptr+i), size, &temp_FP);

			for( j=0; j < sizeof(FPatterns)/sizeof(FuncPattern); ++j )
			{
				if( FPatterns[j].Found ) //Skip already found patches
					continue;
				
				if( compare_Pattern( &temp_FP, &(FPatterns[j]) ) )
				{
					dbgprintf("Patch:Found [%s]: 0x%08X\n", FPatterns[j].Name, (u32)ptr + i + SectionOffset );
					
					PC.Offset  = (u32)ptr + i;
					PC.PatchID = j;
					
					f_write( &PCache, &PC, sizeof( PatchCache ), &read );
					
					if( FPatterns[j].PatchLength != 0xdead0010 )
					{
						FPatterns[j].Found = 1;
					}

					// If this is a patch group set all others of this group as found aswell
					if( FPatterns[j].Group )
					{
						for( k=0; k < sizeof(FPatterns)/sizeof(FuncPattern); ++k )
						{
							if( FPatterns[k].Group == FPatterns[j].Group )
							{
								if( !FPatterns[k].Found )		// Don't overwrite the offset!
									FPatterns[k].Found = -1;	// Usually this holds the offset, to determinate it from a REALLY found pattern we set it -1 which still counts a logical TRUE
							
								//dbgprintf("Setting [%s] to found!\n", FPatterns[k].Name );
							}
						}
					}
				}
			}
		}
	}

//Apply patches
	f_sync( &PCache );
	f_lseek( &PCache, 0 );

	f_read( &PCache, &magicword, 4, &read );
	if (magicword != 0xC0DE0000)
	{
		dbgprintf("Patch:Wrong magic word(how?): %08x\n");
	}

	for( i=0; i < PCache.fsize / sizeof(PatchCache); ++i )
	{
		f_read( &PCache, &PC, sizeof(PatchCache), &read );

		u32 PatchLength;
		
#if defined(CHEATHOOK) || !defined(fwrite_patch)
		// This pattern is not removed when the non fwrite version is built, so both versions can use the same cache
		// And using the fwrite patch is not a good idea when using the Ocarina code handler
		if( FPatterns[PC.PatchID].Patch == patch_fwrite_GC )
		{
			dbgprintf("Patch:Skipping Patch[%s]: 0x%08X \n", FPatterns[PC.PatchID].Name, PC.Offset | 0x80000000 );
			break;
		}
#endif
		if( (PC.PatchID & 0xFFFF0000) == 0xdead0000 )
		{
			PatchLength = PC.PatchID;
		} else {
			dbgprintf("Patch:Applying Patch[%s]: 0x%08X \n", FPatterns[PC.PatchID].Name, PC.Offset | 0x80000000 );
			PatchLength = FPatterns[PC.PatchID].PatchLength;
		}

		switch( PatchLength )
		{
			case 0xdead0000:
			{
				write32( PC.Offset + 0x54 + 16, read32( PC.Offset + 0x54 + 4 ) + (1<<21) );
			} break;
			case 0xdead0001:
			{
#ifdef CHEATHOOK
				dbgprintf("   Hook@0x%08x\n", PC.Offset + SectionOffset );

				memcpy( (void*)0x1800, kenobigc, sizeof(kenobigc) ); 
		
				write32( P2C(read32(0x1808)), 1 );

				u32 newval = 0x18A8 - PC.Offset;
				newval&= 0x03FFFFFC;
				newval|= 0x48000000;
				write32( PC.Offset, newval );
#endif
			} break;
			case 0xdead0002:
			{
				//Don't call it again
				if( CardLowestOff == 0 )
				{
					CardLowestOff = 0xdeadbeef;
					//DoCardPatches( ptr+PC.Offset, size, SectionOffset );					
				}
			} break;
			/*	This should not be required anymore
			case 0xdead0004:	// Audiostreaming hack
			{
				switch( read32(0) >> 8 )
				{
					case 0x474544:	// Eternal Darkness
					break;
					default:
					{
						write32( PC.Offset + 0xB4, 0x60000000 );
						write32( PC.Offset + 0xC0, 0x60000000 );
					} break;
				}
			} break;
			*/
			case 0xdead0005:
			{
				dbgprintf("Patch:Applying Patch[DVDLowRead]: 0x%08X \n", PC.Offset | 0x80000000 );
								
				//Search for __OSGetSystemTime function call
				j=0;
				while( read32( PC.Offset + j ) != 0x4E800020 )
				{
					if( (read32( PC.Offset + j ) & 0xFC000003) == 0x48000001 )
					{
						u32 dst = read32( PC.Offset + j ) & 0x03FFFFFC;
							dst = ~dst;
							dst = (dst + 1) & 0x03FFFFFC;
							dst = PC.Offset + j -dst;

						dbgprintf("DIP:__OSGetSystemTime @ %08X->%08X\n", PC.Offset + j, dst );
						memcpy( (void*)0x2E00, DVDLowRead, sizeof(DVDLowRead) );
						PatchBL( dst, 0x2E1C );
						PatchBL( 0x2E00, PC.Offset + j );

						break;
					}
					j+=4;
				}
				
				//Search for lis rX, 0xA800
				j=0;
				while( read32( PC.Offset + j ) != 0x4E800020 )
				{
					if( (read32( PC.Offset + j ) & 0x0000FFFF) == 0x0000A800 )
					{
						write32( PC.Offset + j, (read32(PC.Offset + j) & 0xFFFF0000) | 0xE000 );
						dbgprintf("DIP:lis rX, 0xA800 @ %08X\n", PC.Offset + j );
						break;
					}
					j+=4;					
				}
				//Search for li rX, 3
				j=0;
				while( read32( PC.Offset + j ) != 0x4E800020 )
				{
					if( (read32( PC.Offset + j ) & 0x0000FFFF) == 0x00000003 )
					{
						write32( PC.Offset + j, (read32(PC.Offset + j) & 0xFFFF0000) | 1 );
						dbgprintf("DIP:li  rX, 3 @ %08X\n", PC.Offset + j );
						break;
					}
					j+=4;
				}
			} break;
			case 0xdead0006:
			{				
				//Search for DCInvalidateRange function call
				j=0;
				while( read32( PC.Offset + j ) != 0x4E800020 )
				{
					if( (read32( PC.Offset + j ) & 0xFC000003) == 0x48000001 )
					{
						dbgprintf("DIP:DCInvalidateRange @ %08X\n", (PC.Offset + j) | 0x80000000 );
						memcpy( (void*)0x2E30, DVDReadAbsAsyncPrio, sizeof(DVDReadAbsAsyncPrio) );
						PatchBL( 0x2E30, PC.Offset + j );

						break;
					}
					j+=4;
				}

				//Patch branches
				j=0;
				u32 count=0;
				while( read32( PC.Offset + j ) != 0x4E800020 )
				{
					if( (read32( PC.Offset + j ) & 0xFF000003) == 0x41000000 )
					{
						dbgprintf("DIP:Branch @ %08X\n", (PC.Offset + j) | 0x80000000 );

						if( count == 0 )
						{
							write32( PC.Offset + j, 0x60000000 );

						} else if( count == 1 )
						{
							write32( PC.Offset + j, (read32(PC.Offset + j) & 0x0000FFFF) | 0x48000000 );

						} else {
							break;
						}
						count++;
					}
					j+=4;
				}
			} break;
			//case 0xdead0010:
			//{
			//	switch( read32(PC.Offset + 8) & 0xF )
			//	{
			//		case 0x06:		// DVDPrepareStreamAbsAsync
			//		{
			//			dbgprintf("TODO:Applying Patch[DVDPrepareStreamAbsAsync]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDCancelStreamAsync, sizeof(DVDCancelStreamAsync) );
			//			write32( PC.Offset + 0x40, 0x3CA5000 | (read32(PC.Offset + 8) & 0xF) );
			//		} break;
			//		case 0x07:		// DVDCancelStreamAsync
			//		{
			//			dbgprintf("Patch:Applying Patch[DVDCancelStreamAsync]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDCancelStreamAsync, sizeof(DVDCancelStreamAsync) );
			//		} break;
			//		case 0x08:		// DVDStopStreamAtEndAsync
			//		{
			//			dbgprintf("Patch:Applying Patch[DVDStopStreamAtEndAsync]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDStopStreamAtEndAsync, sizeof(DVDStopStreamAtEndAsync) );
			//		} break;
			//		case 0x09:		// DVDGetStreamErrorStatus
			//		{
			//			dbgprintf("TODO:Applying Patch[DVDGetStreamErrorStatus]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDCancelStreamAsync, sizeof(DVDCancelStreamAsync) );
			//			write32( PC.Offset + 0x40, 0x3CA5000 | (read32(PC.Offset + 8) & 0xF) );
			//		} break;
			//		case 0x0A:		// DVDGetStreamPlayAddrAsync
			//		{
			//			dbgprintf("Patch:Applying Patch[DVDGetStreamPlayAddrAsync]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDGetStreamPlayAddrAsync, sizeof(DVDGetStreamPlayAddrAsync) );
			//		} break;
			//		case 0x0B:		// DVDGetStreamStartAddr
			//		{
			//			dbgprintf("TODO:Applying Patch[DVDGetStreamStartAddr]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDCancelStreamAsync, sizeof(DVDCancelStreamAsync) );
			//			write32( PC.Offset + 0x40, 0x3CA5000 | (read32(PC.Offset + 8) & 0xF) );
			//		} break;
			//		case 0x0C:		// DVDGetStreamLength
			//		{
			//			dbgprintf("TODO:Applying Patch[DVDGetStreamLength]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDCancelStreamAsync, sizeof(DVDCancelStreamAsync) );
			//			write32( PC.Offset + 0x40, 0x3CA5000 | (read32(PC.Offset + 8) & 0xF) );
			//		} break;
			//		case 0x0D:		// __DVDAudioBufferConfig
			//		{
			//			dbgprintf("TODO:Applying Patch[__DVDAudioBufferConfig]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDCancelStreamAsync, sizeof(DVDCancelStreamAsync) );
			//			write32( PC.Offset + 0x40, 0x3CA5000 | (read32(PC.Offset + 8) & 0xF) );
			//		} break;
			//		case 0x0F:		// DVDChangeDiskAsyncForBS
			//		{
			//			dbgprintf("TODO:Applying Patch[DVDChangeDiskAsyncForBS]: 0x%08X \n", PC.Offset | 0x80000000 );
			//			memcpy( (void*)(PC.Offset), DVDCancelStreamAsync, sizeof(DVDCancelStreamAsync) );
			//			write32( PC.Offset + 0x40, 0x3CA5000 | (read32(PC.Offset + 8) & 0xF) );
			//		} break;						
			//		default:
			//		{
			//			dbgprintf("Patch:Unhandled StreamFunction:%X\n", read32(PC.Offset + 8) & 0xF );
			//		} break;
			//	}
			//} break;
			default:
			{
				memcpy( (void*)(PC.Offset), FPatterns[PC.PatchID].Patch, FPatterns[PC.PatchID].PatchLength );
				
				if ((FPatterns[PC.PatchID].Patch == (u8 *)__dvdLowAudioStatusNULL) && ((read32(0) >> 8) == 0x47494B))
				{
					write32(PC.Offset + 36, 0x38600001);
					dbgprintf("Patch:LowAudioStatus patched for Ikaruga\n");
				}
			} break;
		}
		
	}

//Write PatchCache to file
	f_close( &PCache );

}