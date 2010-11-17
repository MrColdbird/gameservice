/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2009 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#define __CELL_ASSERT__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// libgcm
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>

// gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;

using namespace cell::Gcm;

namespace CellGcmUtil{

/*
typedef struct CellGcmTileInfo{
 uint32_t tile;   = (location + 1) | (bank<<4) | ((offset/0x10000)<<16) | (location<<31);
 uint32_t limit;  = (((offset+size-1)/0x10000)<<16) | (location<<31);
 uint32_t pitch;  = (pitch/0x100)<<8;
 uint32_t format; = base | ((base + (size-1)/0x10000)<<13) | (comp<<26) | (1<<30);
}CellGcmTileInfo;
*/

int cellGcmUtilUnpackTileInfo(int index, const CellGcmTileInfo *raw_tile_info, CellGcmUtilUnpackedTileInfo *outTileInfo)
{
	assert(index < CELL_GCM_MAX_TILE_INDEX);
	assert(index >= 0);
	if(index < 0 || index >= CELL_GCM_MAX_TILE_INDEX) return CELL_GCM_ERROR_INVALID_VALUE;

	assert(raw_tile_info != 0);
	if(raw_tile_info == 0) return CELL_GCM_ERROR_INVALID_VALUE;

	assert(outTileInfo != 0);
	if(outTileInfo == 0) return CELL_GCM_ERROR_INVALID_VALUE;


	const uint32_t FORMAT_BASE_TAG_MASK  = 0x00001FFF;
	const uint32_t FORMAT_LIMIT_TAG_MASK = 0x03FFE000;
	const uint32_t FORMAT_COMP_TAG_MASK  = 0x3C000000;

	const uint32_t FORMAT_BASE_TAG_RSHIFT  = 0;
	const uint32_t FORMAT_LIMIT_TAG_RSHIFT	= 13;
	const uint32_t FORMAT_COMP_TAG_RSHIFT	= 26;

	const uint32_t PITCH_PITCH_MASK = 0xFFFFFF00;

	const uint32_t TILE_BANK_MASK    = 0x0000FFF0;
	const uint32_t TILE_BANK_RSHIFT  = 4;

	const uint32_t TILE_REGION_MASK = 0x00000003;
	const uint32_t TILE_OFFSET_MASK = 0x7FFF0000;

	const uint32_t LIMIT_LIMIT_MASK  = 0x7FFF0000;


	const CellGcmTileInfo *tile_info = raw_tile_info + index;
	
	uint32_t region = tile_info->tile & TILE_REGION_MASK;

	outTileInfo->enabled = (region != 0);
	outTileInfo->location = region - 1;
	outTileInfo->offset = (tile_info->tile & TILE_OFFSET_MASK);

	uint32_t limit = (tile_info->limit & LIMIT_LIMIT_MASK);
	outTileInfo->size = (limit - outTileInfo->offset) + 0x00010000;

	outTileInfo->pitch =  tile_info->pitch & PITCH_PITCH_MASK;
	outTileInfo->comp =  (tile_info->format & FORMAT_COMP_TAG_MASK) >> FORMAT_COMP_TAG_RSHIFT;

	outTileInfo->base =  (tile_info->format & FORMAT_BASE_TAG_MASK ) >> FORMAT_BASE_TAG_RSHIFT;
	outTileInfo->limit = (tile_info->format & FORMAT_LIMIT_TAG_MASK) >> FORMAT_LIMIT_TAG_RSHIFT;
	outTileInfo->bank =  (tile_info->tile   & TILE_BANK_MASK       ) >> TILE_BANK_RSHIFT;

	return CELL_OK;
}

} // namespace CellGcmUtil
