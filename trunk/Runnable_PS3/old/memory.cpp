/*   SCE CONFIDENTIAL                                       */
/*   PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*   Copyright (C) 2010 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#define __CELL_ASSERT__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cell/gcm.h>

#include "gcmutil.h"

using namespace cell::Gcm;

#ifdef NDEBUG
#define DPRINTF(format, arg...)
#else
#define DPRINTF(format, arg...) PRINTF(format, ## arg)
#endif
#define PRINTF(format, arg...) printf(format, ## arg)

static size_t sLocalBaseAddress = 0;
static size_t sLocalCurrentAddress = 0;
static size_t sLocalSize = 0;

void cellGcmUtilInitializeLocalMemory(const size_t base, const size_t size)
{
	DPRINTF("initialize video memory: %x %x\n", base, size);
	sLocalCurrentAddress = sLocalBaseAddress = base;
	sLocalSize = size;
}

void *cellGcmUtilAllocateLocalMemory(const size_t size, const size_t alignment)
{
	DPRINTF("size %x, alignment %x\n", size, alignment);

	void *p = (void*)((sLocalCurrentAddress + alignment - 1) & ~(alignment - 1));
	sLocalCurrentAddress = (size_t)p + size;

	DPRINTF("video memory: %x %x\n", (uint32_t)p, (uint32_t)sLocalCurrentAddress);

	if (sLocalCurrentAddress > sLocalBaseAddress + sLocalSize) {
		PRINTF("Out of Local Memory: %x\n", sLocalCurrentAddress);
		exit(1);
	}

	return p;
}

size_t cellGcmUtilGetAllocatedLocalMemorySize(void)
{
	return (sLocalCurrentAddress - sLocalBaseAddress);
}

static size_t sMainBaseAddress = 0;
static size_t sMainCurrentAddress = 0;
static size_t sMainSize = 0;
static uint32_t sMainBaseOffset = 0;

uint32_t cellGcmUtilGetOffsetOfMainMemory(void* ea)
{
	assert((size_t)ea < sMainBaseAddress + sMainSize);
	return (size_t)ea - sMainBaseAddress + sMainBaseOffset;
}

void* cellGcmUtilGetMainMemoryBaseAddress(void)
{
	assert(sMainBaseAddress != 0);
	return (void*) sMainBaseAddress;
}

void cellGcmUtilInitializeMainMemory(const size_t size)
{
	// allocate system memory
	sMainCurrentAddress = sMainBaseAddress = (size_t)(uint32_t *)memalign(0x100000, size);
	DPRINTF("initialize system memory: %x %x\n", sMainBaseAddress, size);
	CELL_GCMUTIL_ASSERTS(sMainBaseAddress != NULL,"memalign()");
	sMainSize = size;
	// map system memory in RSX's address space
	CELL_GCMUTIL_CHECK_ASSERT(cellGcmMapMainMemory((void*)sMainBaseAddress, size, &sMainBaseOffset));

	DPRINTF("offset of system memory: %x\n", sMainBaseOffset);

	// register clean up function for main memory
	if( atexit(cellGcmUtilFreeMainMemory) != 0 )
	{
		PRINTF("atexit() failed.\n");
		exit(1);
	}
}

void cellGcmUtilFreeMainMemory(void) 
{
	if( sMainBaseAddress ) free( (void*)sMainBaseAddress );

	DPRINTF( "Main memory allocated freed\n" );
}

void cellGcmUtilResetMainMemory(void)
{
	assert(sMainBaseAddress != 0);
	sMainCurrentAddress = sMainBaseAddress;
}

void *cellGcmUtilAllocateMainMemory(const size_t size, const size_t alignment)
{
	DPRINTF("size %x, alignment %x\n", size, alignment);

	void *p = (void*)((sMainCurrentAddress + alignment - 1) & ~(alignment - 1));
	sMainCurrentAddress = (size_t)p + size;

	DPRINTF("system memory: %x %x\n", (uint32_t)p, (uint32_t)sMainCurrentAddress);

	if (sMainCurrentAddress > sMainBaseAddress + sMainSize) {
		PRINTF("Out of Main Memory: %x\n", sMainCurrentAddress);
		exit(1);
	}

	return p;
}

size_t cellGcmUtilGetAllocatedMainMemorySize(void)
{
	return (sMainCurrentAddress - sMainBaseAddress);
}
