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

#define __CELL_GCMUTIL_CALL_INTERNAL__
#include "gcmutil_internal.h"
#undef __CELL_GCMUTIL_CALL_INTERNAL__


struct my_mspace_imp
{
	uint32_t addr;
	uint32_t size;
	uint32_t num;
	uint32_t reserved;
};
struct my_mspace_buffer_info
{
	uint32_t offset;
	uint32_t size;
	uint32_t align;
	uint32_t reserved;
};

static my_mspace_buffer_info* get_first_info(my_mspace_imp *mspi)
{
	uint8_t *info = reinterpret_cast<uint8_t*>(mspi) - sizeof(my_mspace_buffer_info);
	my_mspace_buffer_info *first_info = reinterpret_cast<my_mspace_buffer_info*>(info);
	return first_info;
}

static my_mspace_buffer_info* get_info(my_mspace_imp *mspi, uint32_t index)
{
	return get_first_info(mspi) - (index + 1);
}

static my_mspace_buffer_info* get_prev_info(my_mspace_imp *mspi, uint32_t index)
{
	return get_first_info(mspi) - index;
}

static uint32_t get_align(uint32_t base, uint32_t alignment)
{
	return (base + alignment - 1) & ~(alignment - 1);
}

my_mspace my_mspace_create(void *base, size_t capacity)
{
	if(!base) return 0;
	if(capacity < sizeof(my_mspace_imp)) return 0;
	if(capacity > 0xFFFFFFFFUL) return 0;

	uint32_t capc = static_cast<uint32_t>(capacity);

	my_mspace_imp *mspi = reinterpret_cast<my_mspace_imp*>(reinterpret_cast<uint8_t*>(base) + capc - sizeof(my_mspace_imp));
	mspi->addr = reinterpret_cast<uint32_t>(base);
	mspi->size = capc;
	mspi->num = 0;
	mspi->reserved = 0;

	my_mspace_buffer_info *first_info = get_first_info(mspi);
	first_info->offset = 0;
	first_info->size = 0;
	first_info->align = MY_SPACE_DEFAULT_ALIGN;
	first_info->reserved = 0;

	return mspi;
}

int my_mspace_destroy(my_mspace msp)
{
	if(!msp) return 0;

	my_mspace_imp *mspi = reinterpret_cast<my_mspace_imp*>(msp);

	if(mspi->num != 0) return 1;
	
	mspi->addr = 0;
	mspi->size = 0;
	mspi->num = 0;
	mspi->reserved = 0;

	return 0;
}

void *my_mspace_malloc(my_mspace msp, uint32_t size)
{
	return my_mspace_memalign(msp, MY_SPACE_DEFAULT_ALIGN, size);
}

void my_mspace_free(my_mspace msp, void *ptr)
{
	if(ptr == 0) return;

	my_mspace_imp *mspi = reinterpret_cast<my_mspace_imp*>(msp);
	uint32_t addr = reinterpret_cast<uint32_t>(ptr);

	for(uint32_t i = 0; i < mspi->num; ++i){
		my_mspace_buffer_info *info = get_info(mspi, i);
		
		if(mspi->addr + info->offset != addr) continue;
		
		my_mspace_buffer_info *last = get_info(mspi, mspi->num - 1);

		memmove(last + 1, last, reinterpret_cast<uint8_t*>(info) - reinterpret_cast<uint8_t*>(last));
		memset(last, 0, sizeof(my_mspace_buffer_info));

		--mspi->num;

		break;
	}
}

void *my_mspace_calloc(my_mspace msp, uint32_t nelem, uint32_t size)
{
	uint32_t alloc_size = size * nelem;
	void *ptr = my_mspace_memalign(msp, MY_SPACE_DEFAULT_ALIGN, alloc_size);
	memset(ptr, 0, alloc_size);
	return ptr;
}

void *my_mspace_memalign(my_mspace msp, uint32_t boundary, uint32_t size)
{
	my_mspace_imp *mspi = reinterpret_cast<my_mspace_imp*>(msp);
	
	for(uint32_t i = 0; i < mspi->num; ++i){
		my_mspace_buffer_info *info = get_info(mspi, i);
		my_mspace_buffer_info *prev = get_prev_info(mspi, i);
		
		// try insert
		uint32_t offset = get_align(mspi->addr + prev->offset + prev->size, boundary) - mspi->addr;
		uint32_t end_offset = offset + size;

		if(end_offset > info->offset) continue;

		my_mspace_buffer_info *last = get_info(mspi, mspi->num - 1);

		memmove(get_info(mspi, mspi->num), last, reinterpret_cast<uint8_t*>(prev) - reinterpret_cast<uint8_t*>(last));

		info->align = boundary;
		info->offset = offset;
		info->size = size;
		info->reserved = 0;

		++mspi->num;

		return reinterpret_cast<void*>(mspi->addr + offset);
	}

	// append last
	{
		my_mspace_buffer_info *info = get_info(mspi, mspi->num);
		my_mspace_buffer_info *last = get_prev_info(mspi, mspi->num);

		uint32_t offset = get_align(mspi->addr + last->offset + last->size, boundary) - mspi->addr;
		uint32_t end_offset = offset + size;

		if(end_offset > mspi->size) return 0;

		info->align = boundary;
		info->offset = offset;
		info->size = size;
		info->reserved = 0;

		++mspi->num;

		return reinterpret_cast<void*>(mspi->addr + offset);
	}
}

void *my_mspace_realloc(my_mspace msp, void *ptr, uint32_t size)
{
	return my_mspace_reallocalign(msp, ptr, size, MY_SPACE_DEFAULT_ALIGN);
}

void *my_mspace_reallocalign(my_mspace msp, void *ptr, uint32_t size, uint32_t boundary)
{
	(void)msp;
	(void)ptr;
	(void)size;
	(void)boundary;
	return 0;
}

int my_mspace_is_heap_empty(my_mspace msp)
{
	my_mspace_imp *mspi = reinterpret_cast<my_mspace_imp*>(msp);
	return (mspi->num == 0);
}

void my_mspace_dump_status(my_mspace msp)
{
	my_mspace_imp *mspi = reinterpret_cast<my_mspace_imp*>(msp);

	fprintf(stderr, "my_mspace: base=%08x, size=%08x, num=%08x\n", mspi->addr, mspi->size, mspi->num);
	for(uint32_t i = 0; i < mspi->num; ++i){
		my_mspace_buffer_info *info = get_info(mspi, i);
		fprintf(stderr, "            ptr=%08x, size=%08x, align=%08x\n",mspi->addr + info->offset, info->size, info->align);
	}
}
