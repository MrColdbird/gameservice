/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2010 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#define __CELL_ASSERT__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/process.h>
#include <cell/sysmodule.h>

// libgcm
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>

// gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;

#define __CELL_GCMUTIL_CALL_INTERNAL__
#include "internal/gcmutil_internal.h"
#undef __CELL_GCMUTIL_CALL_INTERNAL__


namespace{
	my_mspace mspMain = 0;
	my_mspace mspLocal = 0;

	// for unmapped
	bool my_unmap_is_heap_empty();
	void my_unmap_dump_status();
	void my_unmap_destroy();
	void my_unmap_free(Memory_t *memory);

	const uint32_t UNMAPPED_OFFSET = 0xFFFFFFFF;

	struct my_memory_marker{
		void* addr;
		uint32_t size;
	} *mmmUnmap = 0;
	uint32_t unmapMarkerSize = 0;
	uint32_t unmapMarkerNumber = 0;
	size_t unmapBufferSize = 0;

	void relocateUnmapMaker()
	{
		uint32_t RELOC_STRID = 128;
		uint32_t reSize = unmapMarkerSize + RELOC_STRID;

		my_memory_marker *reUnmap = new my_memory_marker[reSize];
		if(reUnmap == 0){
			delete [] mmmUnmap, mmmUnmap = 0;
			unmapMarkerSize = 0;
		}

		memcpy(reUnmap, mmmUnmap, unmapMarkerSize * sizeof(my_memory_marker));

		delete [] mmmUnmap, mmmUnmap = 0;
		mmmUnmap = reUnmap;
	}
	void registUnmapMaker(Memory_t *memory)
	{
		if(!memory) return;
		if(memory->offset != UNMAPPED_OFFSET) return;

		if(mmmUnmap == 0 || (unmapMarkerSize == unmapMarkerNumber + 1)){
			relocateUnmapMaker();
		}

		if(mmmUnmap){
			mmmUnmap[unmapMarkerNumber].addr = memory->addr;
			mmmUnmap[unmapMarkerNumber].size = memory->size;
		}
		++unmapMarkerNumber;
		unmapBufferSize += memory->size;
	}
	void releaseUnmapMarker(Memory_t *memory)
	{
		if(!memory) return;
		if(memory->offset != UNMAPPED_OFFSET) return;

		if(mmmUnmap == 0 && unmapMarkerNumber > 0){
			--unmapMarkerNumber;
			unmapBufferSize -= memory->size;
			return;
		}
		
		for(uint32_t i = 0; i < unmapMarkerNumber; ++i)
		{
			if(mmmUnmap[i].addr != memory->addr) continue;

			memmove(mmmUnmap + i, mmmUnmap + i + 1, (unmapMarkerNumber - i - 1) * sizeof(my_memory_marker));
			--unmapMarkerNumber;
			unmapBufferSize -= memory->size;
			break;
		}

		if(unmapMarkerNumber == 0 && unmapBufferSize == 0)
		{
			delete [] mmmUnmap, mmmUnmap = 0;
			unmapMarkerSize = 0;
		}
	}
	bool my_unmap_is_heap_empty(){
		return (mmmUnmap == 0 && unmapMarkerNumber == 0);
	}
	void my_unmap_dump_status()
	{
		fprintf(stderr, " my_unmap: base=--------, size=%08x, num=%08x\n", unmapBufferSize, unmapMarkerNumber);
		if(mmmUnmap){
			for(uint32_t i = 0; i < unmapMarkerNumber; ++i){
				fprintf(stderr, "            ptr=%08x, size=%08x, align=--------\n", reinterpret_cast<uint32_t>(mmmUnmap[i].addr), mmmUnmap[i].size);
			}
		}
	}
	void my_unmap_free(Memory_t *memory)
	{
		if(!memory) return;
		if(memory->offset != UNMAPPED_OFFSET) return;

		releaseUnmapMarker(memory);

		free(memory->addr);
	}
	void my_unmap_destroy()
	{
		if(mmmUnmap){
			for(uint32_t i = 0; i < unmapMarkerNumber; ++i){
				free(mmmUnmap[i].addr);
				mmmUnmap[i].addr = 0;
				mmmUnmap[i].size = 0;
			}
			delete [] mmmUnmap, mmmUnmap = 0;
			unmapMarkerSize = 0;
		}
		unmapBufferSize = 0;
		unmapMarkerNumber = 0;
	}

} // no name namespace

namespace CellGcmUtil{

bool cellGcmUtilInitLocal(void* local_base_addr, size_t local_size)
{
	mspLocal = my_mspace_create((void *)local_base_addr, local_size);

	return (mspLocal != 0);
}

bool cellGcmUtilAllocateLocal(uint32_t size, uint32_t alignment, Memory_t *memory)
{
	// error
	assert(memory != 0);

	// warning
	if(memory == 0) return false;
	
	void *addr = 0;
	addr = my_mspace_memalign(mspLocal, alignment, size);
	assert(addr != 0);
	if(addr == 0)
	{
		printf("cellGcmUtilAllocateLocal() failed. Cannot allocate memory (size=0x%x)\n", size);
		return false;
	}

	memset(memory, 0, sizeof(Memory_t));
	memory->location = CELL_GCM_LOCATION_LOCAL;
	memory->addr = addr;
	memory->size = size;

	int ret = cellGcmAddressToOffset(addr, &memory->offset);
	if(ret != CELL_OK)
	{
		printf("cellGcmUtilAllocateLocal() failed. cellGcmAddressToOffset() failed: addr=0x%x\n", reinterpret_cast<uint32_t>(addr));
		cellGcmUtilFree(memory);
		return false;
	}	

	return true;
}

bool cellGcmUtilInitMain(void* main_base_addr, size_t main_size)
{
	mspMain = my_mspace_create((void *)main_base_addr, main_size);

	return (mspMain != 0);
}

bool cellGcmUtilAllocateMain(uint32_t size, uint32_t alignment, Memory_t *memory)
{
	// error
	assert(memory != 0);

	// warning
	if(memory == 0) return false;
	if(size == 0) return false;

	void *addr = 0;
	addr = my_mspace_memalign(mspMain, alignment, size);
	assert(addr != 0);
	if(addr == 0)
	{
		printf("cellGcmUtilAllocateMain() failed. Cannot allocate memory (size=0x%x)\n", size);
		return false;
	}

	memset(memory, 0, sizeof(Memory_t));
	memory->location = CELL_GCM_LOCATION_MAIN;
	memory->addr = addr;
	memory->size = size;

	int ret = cellGcmAddressToOffset(addr, &memory->offset);
	if(ret != CELL_OK)
	{
		printf("cellGcmUtilAllocateMain() failed. cellGcmAddressToOffset() failed: addr=0x%x\n", reinterpret_cast<uint32_t>(addr));
		cellGcmUtilFree(memory);
		return false;
	}

	return true;
}

void cellGcmUtilFree(Memory_t *memory)
{
	if(memory == 0) return;

	if(memory->location == CELL_GCM_LOCATION_MAIN){
		if(memory->offset == UNMAPPED_OFFSET){
			my_unmap_free(memory);
		}else{
            my_mspace_free(mspMain, memory->addr);
		}
	}else if(memory->location == CELL_GCM_LOCATION_LOCAL){
		my_mspace_free(mspLocal, memory->addr);
	}

	memset(memory, 0, sizeof(Memory_t));
}

void cellGcmUtilFinalizeMemory()
{
	if(my_mspace_is_heap_empty(mspMain)){
		my_mspace_destroy(mspMain);
	}else{
		fprintf(stderr, "#### Main Memory Leaked!! ####\n");
		my_mspace_dump_status(mspMain);
	}

	if(my_mspace_is_heap_empty(mspLocal)){
		my_mspace_destroy(mspLocal);
	}else{
		fprintf(stderr, "#### Local Memory Leaked!! ####\n");
		my_mspace_dump_status(mspLocal);
	}

	if(!my_unmap_is_heap_empty()){
		fprintf(stderr, "#### Unmapped Memory Leaked!! ####\n");
		my_unmap_dump_status();
		my_unmap_destroy();
	}
}

bool cellGcmUtilAllocate(uint32_t size, uint32_t alignment, uint8_t location, Memory_t *memory)
{
	if(location == CELL_GCM_LOCATION_MAIN){
		return cellGcmUtilAllocateMain(size, alignment, memory);
	}else if(location == CELL_GCM_LOCATION_LOCAL){
		return cellGcmUtilAllocateLocal(size, alignment, memory);
	}

	return false;
}

bool cellGcmUtilReallocate(Memory_t *memory, uint8_t location, uint32_t alignment)
{
	// error
	assert(memory != 0);

	if(memory == 0) return false;

	Memory_t reMemory;
	if(cellGcmUtilAllocate(memory->size, alignment, location, &reMemory) == false) return false;

	memcpy(reMemory.addr, memory->addr, memory->size);

	cellGcmUtilFree(memory);

	*memory = reMemory;

	return true;
}

bool cellGcmUtilAllocateUnmappedMain(uint32_t size, uint32_t alignment, Memory_t *memory)
{
	// error
	assert(memory != 0);

	// warning
	if(memory == 0) return false;
	if(size == 0) return false;

	void *addr = 0;
	addr = memalign(alignment, size);
	CELL_GCMUTIL_ASSERTS(addr != NULL,"memalign()");

	if(addr == 0)
	{
		printf("cellGcmUtilAllocateUnmappedMain() failed. Cannot allocate memory (size=0x%x)\n", size);
		return false;
	}

	memset(memory, 0, sizeof(Memory_t));
	memory->location = CELL_GCM_LOCATION_MAIN;
	memory->addr = addr;
	memory->size = size;

	memory->offset = UNMAPPED_OFFSET;

	registUnmapMaker(memory);

	return true;
}

} // namespace CellGcmUtil
