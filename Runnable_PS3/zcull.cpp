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
typedef struct CellGcmZcullInfo{
 uint32_t region;  = (1) | (zFormat<<4) | (aaFormat<<8);
 uint32_t size;    = ((widht>>6)<<22) | ((height>>6)<<6);
 uint32_t start;   = cullStart&(~0xfff);
 uint32_t offset;  = offset;
 uint32_t status0; = (0) | (zCullDir<<1) | (zCullFormat<<2) | ((sFunc&0xf)<<12) | (sRef<<16) | (sMask<<24);
 uint32_t status1; = (0x2000) | (0x20<<16);
}CellGcmZcullInfo;
*/

int cellGcmUtilUnpackZcullInfo(int index, const CellGcmZcullInfo *raw_zcull_info, CellGcmUtilUnpackedZcullInfo *outZcullInfo)
{
	assert(index < CELL_GCM_MAX_ZCULL_INDEX);
	assert(index >= 0);
	if(index < 0 || index >= CELL_GCM_MAX_ZCULL_INDEX) return CELL_GCM_ERROR_INVALID_VALUE;

	assert(raw_zcull_info != 0);
	if(raw_zcull_info == 0) return CELL_GCM_ERROR_INVALID_VALUE;

	assert(outZcullInfo != 0);
	if(outZcullInfo == 0) return CELL_GCM_ERROR_INVALID_VALUE;

	
	const uint32_t REGION_Z_FORMAT_MASK  = 0x000000F0;
	const uint32_t REGION_AA_FORMAT_MASK = 0x00000F00;
	
	const uint32_t REGION_Z_FORMAT_RSHIFT  = 4;
	const uint32_t REGION_AA_FORMAT_RSHIFT = 8;
	
	const uint32_t SIZE_WIDTH_MASK  = 0xFFC00000;
	const uint32_t SIZE_HEIGHT_MASK = 0x0000FFC0;
	
	const uint32_t SIZE_WIDTH_RSHIFT  = 16;
	const uint32_t SIZE_HEIGHT_RSHIFT = 0;

	const uint32_t START_CULL_START_MASK = 0xFFFFF000;

	const uint32_t STATUS0_ZCULL_DIR_MASK    = 0x00000002;
	const uint32_t STATUS0_ZCULL_FORMAT_MASK = 0x00000FFC;
	const uint32_t STATUS0_SFUNC_MASK        = 0x0000F000;
	const uint32_t STATUS0_SREF_MASK         = 0x00FF0000;
	const uint32_t STATUS0_SMASK_MASK        = 0xFF000000;

	const uint32_t STATUS0_ZCULL_DIR__RSHIFT   = 1;
	const uint32_t STATUS0_ZCULL_FORMAT_RSHIFT = 2;
	const uint32_t STATUS0_SFUNC_RSHIFT        = 12;
	const uint32_t STATUS0_SREF_RSHIFT         = 16;
	const uint32_t STATUS0_SMASK_RSHIFT        = 24;


	const CellGcmZcullInfo *zcull_info = raw_zcull_info + index;

	outZcullInfo->enabled = (zcull_info->region != 0);
	
	outZcullInfo->offset = zcull_info->offset;

	outZcullInfo->width  = (zcull_info->size & SIZE_WIDTH_MASK ) >> SIZE_WIDTH_RSHIFT;
	outZcullInfo->height = (zcull_info->size & SIZE_HEIGHT_MASK) >> SIZE_HEIGHT_RSHIFT;

	outZcullInfo->cullStart = (zcull_info->start & START_CULL_START_MASK);
	
	outZcullInfo->zFormat  = (zcull_info->region & REGION_Z_FORMAT_MASK ) >> REGION_Z_FORMAT_RSHIFT;
	outZcullInfo->aaFormat = (zcull_info->region & REGION_AA_FORMAT_MASK) >> REGION_AA_FORMAT_RSHIFT;

	outZcullInfo->zcullDir    = (zcull_info->status0 & STATUS0_ZCULL_DIR_MASK    ) >> STATUS0_ZCULL_DIR__RSHIFT;
	outZcullInfo->zcullFormat = (zcull_info->status0 & STATUS0_ZCULL_FORMAT_MASK ) >> STATUS0_ZCULL_FORMAT_RSHIFT;
	outZcullInfo->sFunc       = (zcull_info->status0 & STATUS0_SFUNC_MASK        ) >> STATUS0_SFUNC_RSHIFT;
	outZcullInfo->sRef        = (zcull_info->status0 & STATUS0_SREF_MASK         ) >> STATUS0_SREF_RSHIFT;
	outZcullInfo->sMask       = (zcull_info->status0 & STATUS0_SMASK_MASK        ) >> STATUS0_SMASK_RSHIFT;

	return CELL_OK;
}

} // namespace CellGcmUtil
