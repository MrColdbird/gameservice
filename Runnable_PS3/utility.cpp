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

using namespace cell::Gcm;

namespace{

inline int32_t max(int32_t a, int32_t b)
{
	return (a > b? a: b);
}

} // no name namespace

namespace CellGcmUtil{

// texture.cpp

bool cellGcmUtilInit(int32_t cb_size, int32_t main_size)
{
	const int32_t SIZE_4KB = 0x00001000;
	const int32_t SIZE_1MB = 0x00100000;

	// invalid param check
	if(cb_size < 0) return false;
	if(main_size < 0) return false;

	// calc rquire host size
	int32_t host_size = cellGcmAlign(SIZE_1MB, cb_size + max(SIZE_4KB, main_size));

	// using optimized default command buffer
	CELL_GCMUTIL_CHECK_ASSERT(cellGcmInitDefaultFifoMode(CELL_GCM_DEFAULT_FIFO_MODE_OPTIMIZE));

	// init gcm
	void* host_addr = memalign(SIZE_1MB, host_size);
	CELL_GCMUTIL_ASSERTS(host_addr != NULL,"memalign()");

	if(host_addr == 0)
	{
		printf("cellGcmUtilInit() failed. Cannot allocate memory (size=0x%x)\n", host_size);
		return false;
	}

	CELL_GCMUTIL_CHECK_ASSERT(cellGcmInit(cb_size, host_size, host_addr));

	// init local memory allocater
	CellGcmConfig config;
	cellGcmGetConfiguration(&config);
	if(cellGcmUtilInitLocal(config.localAddress, config.localSize) == false)
	{
		printf("cellGcmUtilInit() failed. cellGcmUtilInitLocal() failed.\n");
		return false;
	}

	// init main memory allocater
	if(cellGcmUtilInitMain(reinterpret_cast<uint8_t*>(host_addr) + cb_size, host_size - cb_size) == false)
	{
		printf("cellGcmUtilInit() failed. cellGcmUtilInitMain() failed.\n");
		return false;
	}

	return true;
}

void cellGcmUtilEnd()
{
	cellGcmUtilFinalizeMemory();
}

bool cellGcmUtilOffsetToAddress(uint8_t location, uint32_t offset, void **address)
{
	if(address == 0) return false;

	void *addr = 0;
	if(location == CELL_GCM_LOCATION_MAIN){
		if(cellGcmIoOffsetToAddress(offset, &addr) != CELL_OK) return false;
		*address = addr;
		return true;
	}else if(location == CELL_GCM_LOCATION_LOCAL){
		CellGcmConfig config;
		cellGcmGetConfiguration(&config);
		*address = reinterpret_cast<uint8_t*>(config.localAddress) + offset;
		return true;
	}

	return false;
}

Viewport_t cellGcmUtilGetViewport2D(uint16_t width, uint16_t height)
{
	Viewport_t vp;
	vp.x = 0;
	vp.y = 0;
	vp.width= width;
	vp.height= height;
	vp.min = 0.0f;
	vp.max = 1.0f;
	vp.scale[0] = vp.width;
	vp.scale[1] = vp.height;
	vp.scale[2] = (vp.max - vp.min) * 0.5f;
	vp.scale[3] = 0.0f;
	vp.offset[0] = 0.0f;
	vp.offset[1] = 0.0f;
	vp.offset[2] = (vp.max + vp.min) * 0.5f;
	vp.offset[3] = 0.0f;

	return vp;
}

Viewport_t cellGcmUtilGetViewportGL(uint32_t surface_height,
		   uint32_t vpX, uint32_t vpY,
		   uint32_t vpWidth, uint32_t vpHeight,
		   float vpZMin, float vpZMax)
{
	Viewport_t vp;

    vp.x = vpX;
    vp.y = surface_height - vpY - vpHeight;
    vp.width  = vpWidth;
    vp.height = vpHeight;
    vp.min = vpZMin;
    vp.max = vpZMax;
    vp.scale[0] = vp.width  * 0.5f;
    vp.scale[1] = vp.height * -0.5f;
    vp.scale[2] = (vp.max - vp.min) * 0.5f;
    vp.scale[3] = 0.0f;
    vp.offset[0] = vp.x + vp.width  * 0.5f;
    vp.offset[1] = vp.y + vp.height * 0.5f;
    vp.offset[2] = (vp.max + vp.min) * 0.5f;
    vp.offset[3] = 0.0f;

	return vp;
}

Viewport_t cellGcmUtilGetViewportDX(uint32_t vpX, uint32_t vpY,
		   uint32_t vpWidth, uint32_t vpHeight,
		   float vpZMin, float vpZMax, bool bPixelCenterInteger = true)
{
	Viewport_t vp;

	float half_offset = (bPixelCenterInteger? 0.5f: 0.0f);
   
    vp.x = vpX;
    vp.y = vpY;
    vp.width  = vpWidth;
    vp.height = vpHeight;
    vp.min = vpZMin;
    vp.max = vpZMax;
    vp.scale[0] = vp.width  * 0.5f;
    vp.scale[1] = vp.height * 0.5f;
    vp.scale[2] = (vp.max - vp.min) * 0.5f;
    vp.scale[3] = 0.0f;
    vp.offset[0] = vp.x + vp.width * 0.5f + half_offset;
    vp.offset[1] = vp.y + vp.height * 0.5f + half_offset;
    vp.offset[2] = (vp.max + vp.min) * 0.5f;
    vp.offset[3] = 0.0f;
    
    return vp;
}

} // namespace CellGcmUtil
