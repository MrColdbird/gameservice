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
	uint8_t dispGetSurfaceDepth(uint8_t format)
	{
		uint32_t depth = 4;
		switch(format){
			case CELL_GCM_SURFACE_B8:
				depth = 1;
				break;
			case CELL_GCM_SURFACE_X1R5G5B5_Z1R5G5B5:
			case CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5:
			case CELL_GCM_SURFACE_R5G6B5:
			case CELL_GCM_SURFACE_G8B8:
				depth = 2;
				break;

			case CELL_GCM_SURFACE_X8R8G8B8_Z8R8G8B8:
			case CELL_GCM_SURFACE_X8R8G8B8_O8R8G8B8:
			case CELL_GCM_SURFACE_A8R8G8B8:
			case CELL_GCM_SURFACE_F_X32:
			case CELL_GCM_SURFACE_X8B8G8R8_Z8B8G8R8:
			case CELL_GCM_SURFACE_X8B8G8R8_O8B8G8R8:
			case CELL_GCM_SURFACE_A8B8G8R8:
				depth = 4;
				break;
			case CELL_GCM_SURFACE_F_W16Z16Y16X16:
				depth = 8;
				break;
			case CELL_GCM_SURFACE_F_W32Z32Y32X32:
				depth = 16;
				break;
			default:
				depth = 4;
		}

		return depth;
	}

	uint8_t dispGetDepthDepth(uint8_t format)
	{
		if(format == CELL_GCM_SURFACE_Z16) return 2;
		if(format == CELL_GCM_SURFACE_Z24S8) return 4;
		return 0;
	}

	uint8_t dispGetVideoFormat(uint8_t format)
	{
		switch(format){
			case CELL_GCM_SURFACE_X8R8G8B8_Z8R8G8B8:
			case CELL_GCM_SURFACE_X8R8G8B8_O8R8G8B8:
			case CELL_GCM_SURFACE_A8R8G8B8:
				return CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;

			case CELL_GCM_SURFACE_X8B8G8R8_Z8B8G8R8:
			case CELL_GCM_SURFACE_X8B8G8R8_O8B8G8R8:
			case CELL_GCM_SURFACE_A8B8G8R8:
				return CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;

			case CELL_GCM_SURFACE_F_W16Z16Y16X16:
				return CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_R16G16B16X16_FLOAT;
			
			case CELL_GCM_SURFACE_B8:
			case CELL_GCM_SURFACE_X1R5G5B5_Z1R5G5B5:
			case CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5:
			case CELL_GCM_SURFACE_R5G6B5:
			case CELL_GCM_SURFACE_G8B8:
			case CELL_GCM_SURFACE_F_X32:
			case CELL_GCM_SURFACE_F_W32Z32Y32X32:
			default:
				return CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
		}
	}


} // no name namespace

namespace CellGcmUtil{

CellVideoOutResolution cellGcmUtilGetResolution()
{
	// check video output settings
	CellVideoOutState state;
	CellVideoOutResolution resolution = {0, 0};
	
	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &state));
	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutGetResolution(state.displayMode.resolutionId, &resolution));

	return resolution;
}

float cellGcmUtilGetAspectRatio()
{
	// check video output settings
	CellVideoOutState state;
	float aspect = 16.0f / 9.0f;
	
    CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &state));

	if(state.displayMode.aspect == CELL_VIDEO_OUT_ASPECT_4_3){
		aspect = 4.0f / 3.0f;
	}

	return aspect;
}

bool cellGcmUtilInitDisplay(uint8_t color_format, uint8_t depth_format, uint8_t buffer_number, Memory_t *buffer, CellGcmSurface *surface)
{
	if(buffer == 0) return false;
	if(surface == 0) return false;

	Memory_t *bufFrame = buffer;
	Memory_t &bufDepth = buffer[buffer_number];

	uint8_t color_depth = dispGetSurfaceDepth(color_format);
	uint8_t z_depth = dispGetDepthDepth(depth_format);

	// check video output settings
	CellVideoOutState state;
	CellVideoOutResolution resolution;
	
    CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &state));
	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutGetResolution(state.displayMode.resolutionId, &resolution));
	
	// allocate local memory
	uint32_t pitch_frame = cellGcmGetTiledPitchSize(resolution.width * color_depth);
	uint32_t pitch_depth = cellGcmGetTiledPitchSize(resolution.width * z_depth);
	
	// width and height must be a multiple of 64 for BindZcull
	uint32_t zcull_width = cellGcmAlign(CELL_GCM_ZCULL_ALIGN_WIDTH, resolution.width);
	uint32_t zcull_height = cellGcmAlign(CELL_GCM_ZCULL_ALIGN_HEIGHT, resolution.height);

	for(int32_t i = 0; i < buffer_number; ++i){
		// size should be multiples of 64K bytes for SetTileInfo
		uint32_t buf_size = cellGcmAlign(CELL_GCM_TILE_ALIGN_SIZE, pitch_frame * zcull_height);

		// offset should be aligned on 64K bytes boundary for SetTileInfo
		if(cellGcmUtilAllocateLocal(buf_size, CELL_GCM_TILE_ALIGN_OFFSET, &bufFrame[i]) == false){
			return false;
		}
	}

	{
		// size should be multiples of 64K bytes for SetTileInfo
		uint32_t buf_size = cellGcmAlign(CELL_GCM_TILE_ALIGN_SIZE, pitch_depth * zcull_height);

		// offset should be aligned on 64K bytes boundary for SetTileInfo
		if(cellGcmUtilAllocateLocal(buf_size, CELL_GCM_TILE_ALIGN_OFFSET, &bufDepth) == false){
			return false;
		}
	}

	// regist output buffer
	for(int32_t i = 0; i < buffer_number; ++i){
		CELL_GCMUTIL_CHECK_ASSERT(cellGcmSetDisplayBuffer(i, bufFrame[i].offset, pitch_frame, resolution.width, resolution.height));
	}
	
	// regist tile
	for(int32_t i = 0; i < buffer_number; ++i){
		CELL_GCMUTIL_CHECK(cellGcmSetTileInfo(i, bufFrame[i].location, bufFrame[i].offset, bufFrame[i].size, pitch_frame, CELL_GCM_COMPMODE_DISABLED, NULL, 0));
		CELL_GCMUTIL_CHECK(cellGcmBindTile(i));
	}
	CELL_GCMUTIL_CHECK(cellGcmSetTileInfo(buffer_number, bufDepth.location, bufDepth.offset, bufDepth.size, pitch_depth, CELL_GCM_COMPMODE_Z32_SEPSTENCIL, NULL, 2));
	CELL_GCMUTIL_CHECK(cellGcmBindTile(buffer_number));

	// regist zcull
	CELL_GCMUTIL_CHECK(cellGcmBindZcull(0, bufDepth.offset, zcull_width, zcull_height, 0,
				CELL_GCM_ZCULL_Z24S8, CELL_GCM_SURFACE_CENTER_1,
				CELL_GCM_ZCULL_LESS, CELL_GCM_ZCULL_LONES,
				CELL_GCM_SCULL_SFUNC_LESS, 0x80, 0xff));

	// regist suface
	for(int32_t i = 0; i < buffer_number; ++i){
		surface[i].colorFormat 	= color_format;
		surface[i].colorTarget	= CELL_GCM_SURFACE_TARGET_0;

		// frame buffer
		surface[i].colorLocation[0]	= bufFrame[i].location;
		surface[i].colorOffset[0]	 	= bufFrame[i].offset;
		surface[i].colorPitch[0]		= pitch_frame;

		// not be used
		surface[i].colorLocation[1]	= CELL_GCM_LOCATION_LOCAL;
		surface[i].colorLocation[2]	= CELL_GCM_LOCATION_LOCAL;
		surface[i].colorLocation[3]	= CELL_GCM_LOCATION_LOCAL;
		surface[i].colorOffset[1] 	= 0;
		surface[i].colorOffset[2] 	= 0;
		surface[i].colorOffset[3] 	= 0;
		surface[i].colorPitch[1]	= 64;
		surface[i].colorPitch[2]	= 64;
		surface[i].colorPitch[3]	= 64;

		// depth buffer
		surface[i].depthFormat 	= depth_format;
		surface[i].depthLocation	= bufDepth.location;
		surface[i].depthOffset	= bufDepth.offset;
		surface[i].depthPitch 	= pitch_depth;

		surface[i].type		= CELL_GCM_SURFACE_PITCH;
		surface[i].antialias	= CELL_GCM_SURFACE_CENTER_1;

		surface[i].width 	= resolution.width;
		surface[i].height 	= resolution.height;
		surface[i].x 		= 0;
		surface[i].y 		= 0;
	}

	CellVideoOutConfiguration videocfg;
	memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
	videocfg.resolutionId = state.displayMode.resolutionId; 
	videocfg.format = dispGetVideoFormat(color_format);
	videocfg.pitch = pitch_frame;

	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0));
	
	return true;
}

uint8_t getResolutionId(uint32_t width, uint32_t height)
{
	if(width == 1920) return CELL_VIDEO_OUT_RESOLUTION_1080;

	if(width == 1440) return CELL_VIDEO_OUT_RESOLUTION_1440x1080;

	if(width == 1280){
		if(height == 1080) return CELL_VIDEO_OUT_RESOLUTION_1280x1080;
		return CELL_VIDEO_OUT_RESOLUTION_720;
	}

	if(width == 960) return CELL_VIDEO_OUT_RESOLUTION_960x1080;

	if(width == 720){
		if(height == 576) return CELL_VIDEO_OUT_RESOLUTION_576;
		return CELL_VIDEO_OUT_RESOLUTION_480;
	}

	printf("ERROR: reso=%dx%d\n", width, height);
	assert(false);

	return CELL_VIDEO_OUT_RESOLUTION_1080;
}

uint8_t getAspect(uint32_t width, uint32_t height)
{
	// check video output settings
	CellVideoOutState state;
    if(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &state) != CELL_OK){
		assert(false);
		return CELL_VIDEO_OUT_ASPECT_16_9;
	}

	if(width == 1920) return CELL_VIDEO_OUT_ASPECT_16_9;

	if(width == 1440) return state.displayMode.aspect;

	if(width == 1280){
		if(height == 1080) return state.displayMode.aspect;
		return CELL_VIDEO_OUT_ASPECT_16_9;
	}

	if(width == 960) return state.displayMode.aspect;

	if(width == 720){
		if(height == 576) return CELL_VIDEO_OUT_ASPECT_4_3;
		return CELL_VIDEO_OUT_ASPECT_4_3;
	}

	assert(false);

	return CELL_VIDEO_OUT_RESOLUTION_1080;
}

bool cellGcmUtilSetDisplayBuffer(uint8_t number, CellGcmTexture *texture_array)
{
	if(texture_array == 0) return false;

	int ret = 0;

	for(int32_t i = 0; i < number; ++i){
		CELL_GCMUTIL_CHECK_ASSERT(cellGcmSetDisplayBuffer(i, texture_array[i].offset, texture_array[i].pitch, texture_array[i].width, texture_array[i].height));
	}

	CellVideoOutConfiguration videocfg;
	memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
	videocfg.resolutionId = getResolutionId(texture_array[0].width, texture_array[0].height);
	videocfg.aspect = getAspect(texture_array[0].width, texture_array[0].height);
	videocfg.format = dispGetVideoFormat(texture_array[0].format);
	videocfg.pitch = texture_array[0].pitch;

	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0));
	return (ret == CELL_OK);
}

} // namespace CellGcmUtil
