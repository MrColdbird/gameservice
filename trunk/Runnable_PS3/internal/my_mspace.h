/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2008 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#ifndef __CELL_GCMUTIL_INTERNAL__
#error ONLY USE FROM MEMORY.CPP!!
#endif

#ifndef __CELL_GCMUTIL_MY_MSPACE_H__
#define __CELL_GCMUTIL_MY_MSPACE_H__

typedef void* my_mspace;
const uint32_t MY_SPACE_DEFAULT_ALIGN = 16;

my_mspace my_mspace_create(void *base, size_t capacity);
int my_mspace_destroy(my_mspace msp);
void *my_mspace_malloc(my_mspace msp, uint32_t size);
void my_mspace_free(my_mspace msp, void *ptr);
void *my_mspace_calloc(my_mspace msp, uint32_t nelem, uint32_t size);
void *my_mspace_memalign(my_mspace msp, uint32_t boundary, uint32_t size);
void *my_mspace_realloc(my_mspace msp, void *ptr, uint32_t size);
void *my_mspace_reallocalign(my_mspace msp, void *ptr, uint32_t size, uint32_t boundary);
int my_mspace_is_heap_empty(my_mspace msp);

void my_mspace_dump_status(my_mspace msp);

#endif /* __CELL_GCMUTIL_MY_MSPACE_H__ */
