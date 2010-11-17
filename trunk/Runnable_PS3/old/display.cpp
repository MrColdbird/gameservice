/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2010 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#define __CELL_ASSERT__

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/timer.h>
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>
#include <assert.h>

#include "gcmutil.h"

using namespace cell::Gcm;

#if 0 // [
#  define DPRINTF(x) { \
     printf( "%s(%d):", __FILE__, __LINE__ ); \
     printf x; \
     fflush( stdout ); \
   }
#else // ][
#  define DPRINTF(x)
#endif // ]

static uint32_t sLabelVal=1;

static uint32_t sFrameIndex=0;

static bool     sIsTiledSet = false;
static bool     sIsZcullSet = false;
static bool     sIsSurfaceSet = false;

static uint32_t sDisplayWidth;	
static uint32_t sDisplayHeight;	
static float    sDisplayAspectRatio;

static uint32_t sResolutionId;

static void    *sColorBaseAddress;
static void    *sDepthBaseAddress;
static void    *sColorAddress[GCM_UTIL_COLOR_BUF_NUM];
static void    *sDepthAddress;
static uint32_t sColorPitch;
static uint32_t sDepthPitch;
static uint32_t sColorTiledPitch;
static uint32_t sDepthTiledPitch;
static uint32_t sColorOffset[GCM_UTIL_COLOR_BUF_NUM];
static uint32_t sDepthOffset;
static uint32_t sColorSize;
static uint32_t sDepthSize;

static uint32_t sColorDepth = 4;
static uint32_t sZetaDepth = 4;

static CellGcmSurface sSurface;

// video out format
static uint8_t sVideoOutFormat;
static uint8_t sGcmColorFormat;

// status flag
static bool sIsFP16Supported = false; 
static bool sIsDisplayInitialized = false;

// for fp16 quad clear
static float    sClearColor[4];
//static float*   sQuadBuffer;
static uint32_t sQuadBufferOffset;

static CGprogram sVertexProgram;
static CGprogram sFragmentProgram;
static uint32_t  sFragmentProgramOffset;
//static void*     sFragmentProgramUcode;
static void*     sVertexProgramUcode;

static uint32_t  sQuadClearPositionIndex;
static uint32_t  sQuadClearColorIndex;

// shader binary embedded in object
//extern struct _CGprogram _binary_quad_clear_vpshader_vpo_start;
//extern struct _CGprogram _binary_quad_clear_fpshader_fpo_start;

// Utility function that enables FP16 surface render target.
// NOTE: 
//  This function needs to be called before cellGcmUtilInitDisplay()
//
// Returns true on success.
// Returns false on failure (i.e. called after Init Display)
//
bool cellGcmUtilEnableFP16Surface(void)
{
	if( sIsDisplayInitialized ) {
		printf( "Error: Please call cellGcmUtilEnableFP16Surface() before "
		        "cellGcmUtilInitDisplay() is called\n" );
		return false;
	}

	//sIsFP16Supported = true;

	return sIsFP16Supported;
}

// Utility function that does GPU 'finish'.
//
void cellGcmUtilFinish(void)
{
	cellGcmSetWriteBackEndLabel( GCM_UTIL_LABEL_INDEX, 
	                             sLabelVal );
	cellGcmFlush();

	while( *(cellGcmGetLabelAddress(GCM_UTIL_LABEL_INDEX)) != sLabelVal)
	{
		sys_timer_usleep(30);
	}
	++sLabelVal;
}

// Utility to let RSX wait for complete RSX pipeline idle
//
void cellGcmUtilWaitForIdle(void)
{
	// set write label command in push buffer, and wait
	// NOTE: this is for RSX to wailt
	cellGcmSetWriteBackEndLabel(GCM_UTIL_LABEL_INDEX, sLabelVal);
	cellGcmSetWaitLabel(GCM_UTIL_LABEL_INDEX, sLabelVal);

	// increment label value for 
	++sLabelVal;

	cellGcmUtilFinish();
}

static bool setVideoOutConfigure(void)
{
	uint32_t pitch = 0;

	assert( sVideoOutFormat == CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8 
	     || sVideoOutFormat == CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8B8G8R8 
	     || sVideoOutFormat == CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_R16G16B16X16_FLOAT );

	// Get Offset for color buffer & depth buffer
	for (int i = 0; i < GCM_UTIL_COLOR_BUF_NUM; i++) {
		sColorAddress[i]
			= (void *)((uint32_t)sColorBaseAddress + (i*sColorSize));

		CELL_GCMUTIL_CHECK_ASSERT(cellGcmAddressToOffset(sColorAddress[i], &sColorOffset[i]));
	}

	sDepthAddress = sDepthBaseAddress;
	CELL_GCMUTIL_CHECK_ASSERT(cellGcmAddressToOffset(sDepthAddress, &sDepthOffset));

	// There are two cases.
	//  1. Scan-out buffer is Tiled buffer.
	//  2. Scan-out buffer is not Tiled buffer.
	//
	if( sIsTiledSet ) 
		pitch = sColorTiledPitch;
	else
		pitch = sColorPitch;

	for (int i = 0; i < GCM_UTIL_COLOR_BUF_NUM; i++) {
		CELL_GCMUTIL_CHECK_ASSERT(cellGcmSetDisplayBuffer(i, sColorOffset[i], pitch, sDisplayWidth, sDisplayHeight));
	}

	// Also needs to tell sysutil what our buffer is like.
	//
	CellVideoOutConfiguration videocfg;
	memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
	videocfg.resolutionId = sResolutionId; 
	videocfg.format = sVideoOutFormat;
	videocfg.pitch = pitch;

	// Need to make sure RSX is completely stopped
	cellGcmUtilWaitForIdle();

	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0));

	CellVideoOutState videoState;
	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState));

	// get screen aspect
	switch( videoState.displayMode.aspect ) {
	case CELL_VIDEO_OUT_ASPECT_4_3:
		DPRINTF(("GCM UTIL: Display aspect ratio is 4:3\n" ));
		sDisplayAspectRatio = 4.0f/3.0f;
		break;
	case CELL_VIDEO_OUT_ASPECT_16_9:
		DPRINTF(("GCM UTIL: Display aspect ratio is 16:9\n" ));
		sDisplayAspectRatio = 16.0f/9.0f;
		break;
	default:
		DPRINTF(("GCM UTIL: Unknown aspect ratio %x\n", videoState.displayMode.aspect));
		sDisplayAspectRatio = 16.0f/9.0f;
	}

	return true;
}
/*
static bool quadClearInit(void) 
{
	int ret;

	// Allocate 4 vertices (each with 2 float) in local memory
	sQuadBuffer = (float*)cellGcmUtilAllocateLocalMemory(sizeof(float)*4*2, 128);
	if( ! sQuadBuffer ) {
		DPRINTF(( "Failed to allocate vertices for quad clear.\n" ));
		return false;
	}

	ret = cellGcmAddressToOffset((void*)sQuadBuffer, &sQuadBufferOffset);
	if( ret != CELL_OK ) {
		DPRINTF(( "Failed to get IO offset of quad vertices.\n" ));
		return false;
	}
	
	// set vertices for fullscreen quad
	sQuadBuffer[0] = -1.f;  sQuadBuffer[1] = -1.f;
	sQuadBuffer[2] =  1.f;  sQuadBuffer[3] = -1.f;
	sQuadBuffer[4] = -1.f;  sQuadBuffer[5] =  1.f;
	sQuadBuffer[6] =  1.f;  sQuadBuffer[7] =  1.f;

	// set up shader for full screen quad
	sVertexProgram   = &_binary_quad_clear_vpshader_vpo_start;
	sFragmentProgram = &_binary_quad_clear_fpshader_fpo_start;

	cellGcmCgInitProgram(sVertexProgram);
	cellGcmCgInitProgram(sFragmentProgram);

	uint32_t ucode_size;
	void *ucode;
	cellGcmCgGetUCode(sFragmentProgram, &ucode, &ucode_size);
	sFragmentProgramUcode = cellGcmUtilAllocateLocalMemory(ucode_size, 64);
	if( ! sFragmentProgramUcode ) {
		DPRINTF(( "Failed to allocate local memory for fragment ucode.\n" ));
		return false;
	}
	memcpy(sFragmentProgramUcode, ucode, ucode_size); 

	// get fragment program offset
	ret = cellGcmAddressToOffset(sFragmentProgramUcode, &sFragmentProgramOffset);
	if( ret != CELL_OK ) {
		DPRINTF(( "Failed to get offset of fragment shader.\n" ));
		return false;
	}

	cellGcmCgGetUCode(sVertexProgram, &ucode, &ucode_size);
	sVertexProgramUcode = ucode;


	// get Vertex Attribute index
	CGparameter position = cellGcmCgGetNamedParameter(sVertexProgram, "position");
	CGparameter color = cellGcmCgGetNamedParameter(sVertexProgram, "color");

	sQuadClearPositionIndex = 
		cellGcmCgGetParameterResource(sVertexProgram, position) - CG_ATTR0;
	sQuadClearColorIndex = 
		cellGcmCgGetParameterResource(sVertexProgram, color) - CG_ATTR0;
	
	return true;
}
*/
static bool allocateBuffer(void)
{
	// if fp16 surface is enable, alocate double size of memory.
	//
	uint32_t fp16factor = (sIsFP16Supported)? 2: 1;

	// For Tiled region, frame buffer needs to be on 64 KB boundary
	sColorBaseAddress = cellGcmUtilAllocateLocalMemory(GCM_UTIL_COLOR_BUF_NUM*sColorSize*fp16factor, 0x10000);
	if( ! sColorBaseAddress ) {
		DPRINTF(( "Failed to allocate memory for color buffers\n" ));
		return false;
	}

	sDepthBaseAddress = cellGcmUtilAllocateLocalMemory(sDepthSize, 0x10000);
	if( ! sDepthBaseAddress ) {
		DPRINTF(( "Failed to allocate memory for depth buffer\n" ));
		return false;
	}

	// if FP16 surface is to be used, prepare quad clear
	if( sIsFP16Supported ) {
		/*quadClearInit() ;
		if( ret != true )*/ return false;
	}

	return true;
}

static bool setDisplayFormat( uint8_t gcm_format )
{
	switch( gcm_format ) {
	case CELL_GCM_SURFACE_A8R8G8B8:
		sColorDepth = 4;
		sVideoOutFormat = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
		break;
	case CELL_GCM_SURFACE_A8B8G8R8:
		sColorDepth = 4;
		sVideoOutFormat = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8B8G8R8;
		break;
	case CELL_GCM_SURFACE_F_W16Z16Y16X16:
		if( sIsFP16Supported ) 
			sVideoOutFormat = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_R16G16B16X16_FLOAT;
		else {
			DPRINTF(( "Error: Please call cellGcmUtilEnableFP16Surface() "
			        "before cellGcmUtilInitDisplay() is called.\n" ));
			return false;
		}
		sColorDepth = 8;
		break;
	default:
		DPRINTF(( "Error: Unsupported format for display buffer\n" ));
		return false;
	}
	sZetaDepth = 4;

	// set library internal gcm format
	//
	sGcmColorFormat = gcm_format;
	
	sColorPitch = sDisplayWidth * sColorDepth;
	sDepthPitch = sDisplayWidth * sZetaDepth;

	sColorTiledPitch = cellGcmGetTiledPitchSize(sColorPitch);
	sDepthTiledPitch = cellGcmGetTiledPitchSize(sDepthPitch);

	sColorSize =  sColorTiledPitch * ((sDisplayHeight+63)&~63);
	sDepthSize =  sDepthTiledPitch * ((sDisplayHeight+63)&~63);

	// round up to 64KB size
	sColorSize = (sColorSize+0xFFFF) & ~0xFFFF;
	sDepthSize = (sDepthSize+0xFFFF) & ~0xFFFF;

	return true;
}

//
// Description:
//   - initialize display
//   - register color/depth buffers
//
// Return value:
//    0 on success.
//   -1 on failure.
//
bool cellGcmUtilInitDisplay(void)
{
	int32_t ret;
	CellVideoOutResolution resolution;

	// Get display width and height from system setting
	//
	CellVideoOutState videoState;
	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState));

	CELL_GCMUTIL_CHECK_ASSERT(cellVideoOutGetResolution(videoState.displayMode.resolutionId, &resolution));

	// assign display setting
	sResolutionId = videoState.displayMode.resolutionId;
	sDisplayWidth = resolution.width;
	sDisplayHeight = resolution.height;


	// set default format type to A8R8G8B8
	ret = setDisplayFormat(CELL_GCM_SURFACE_A8R8G8B8);
	if( ret != true ) {
		DPRINTF(( "Failed to set display format to 0x%x.\n", CELL_GCM_SURFACE_A8R8G8B8));
		return false;
	}

	// allocate buffer 
	ret = allocateBuffer();
	if( ret != true ) {
		DPRINTF(( "Failed to allocate buffer.\n" ));
		return false;
	}

	// Notify sytem the type of buffer we use
	//
	ret = setVideoOutConfigure();
	if( ret != true ) {
		DPRINTF(( "Failed to set video out configuration.\n" ));
		return false;
	}

	// set default sync mode to vsync
	//
	cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);

	sIsDisplayInitialized = true;

	return sIsDisplayInitialized;
}


// Description:
//   - Re-initialize display
//   - register color/depth buffers
//
// Return value:
//    0 on success.
//   -1 on failure.
//
bool cellGcmUtilResetDisplayBuffer(uint8_t gcm_format)
{
	// check format
	// RSX only supports scanout in following formats:
	//   - A8R8G8B8
	//   - A8B8G8R8
	//   - F_W16Z16Y16X16
	//
	int ret;

	// need to disable tiled region if we change
	// surface format.
	//
	if( sGcmColorFormat != gcm_format && sIsTiledSet ) {
		cellGcmUtilSetInvalidateTile();
	}
		
	// set new format setting
	ret = setDisplayFormat( gcm_format );
	if( ret != true ) {
		DPRINTF(( "Failed to set display format.\n" ));
		return false;
	}

	// Notify new color format to system.
	//
	ret = setVideoOutConfigure();
	if( ret != true ) {
		DPRINTF(( "Failed to video out configuration.\n" ));
		return false;
	}

	// reset render target
	cellGcmUtilResetRenderTarget();

	return true;
}

void cellGcmUtilSetTile(void)
{
	if( ! sIsTiledSet ) {

		// cellGcmSetTile needs to be called when RSX is idle.
		//
		cellGcmUtilWaitForIdle();

		int ret = 0;
		(void) ret;
		for(int i=0; i<GCM_UTIL_COLOR_BUF_NUM; i++ ) {
			CELL_GCMUTIL_CHECK(cellGcmSetTileInfo(i, CELL_GCM_LOCATION_LOCAL, sColorOffset[i], sColorSize,
						   sColorTiledPitch, CELL_GCM_COMPMODE_DISABLED, 0, 0));
		
			CELL_GCMUTIL_CHECK(cellGcmBindTile(i));
		}
		CELL_GCMUTIL_CHECK(cellGcmSetTileInfo(GCM_UTIL_COLOR_BUF_NUM, CELL_GCM_LOCATION_LOCAL, sDepthOffset, sDepthSize,
			sDepthTiledPitch, CELL_GCM_COMPMODE_Z32_SEPSTENCIL, 0, 2));

		CELL_GCMUTIL_CHECK(cellGcmBindTile(GCM_UTIL_COLOR_BUF_NUM));

		sIsTiledSet = true;

		// pitch size might be changed, so reset render target.
		cellGcmUtilResetRenderTarget();

		// pitch size might be changed, so reset display buffer.
		setVideoOutConfigure();
	}
}

void cellGcmUtilSetInvalidateTile(void) 
{

	if( sIsTiledSet ) {

		// cellGcmInvalidateTile needs to be called when RSX is idle.
		//
		cellGcmUtilWaitForIdle();

		int i;
		for( i=0; i<GCM_UTIL_COLOR_BUF_NUM; i++ ) {
			cellGcmUnbindTile(i);
		}
		cellGcmUnbindTile(i);

		sIsTiledSet = false;

		// pitch size might be changed, so reset render target.
		cellGcmUtilResetRenderTarget();

		// pitch size might be changed, so reset display buffer.
		setVideoOutConfigure();
	}
}

void cellGcmUtilSetZcull(void)
{
	if( ! sIsZcullSet ) {

		// cellGcmBindZcull needs to be called when RSX is idle.
		//
		cellGcmUtilWaitForIdle();

		// zcull width & height must be 64 aligned
		CELL_GCMUTIL_CHECK_ASSERT(cellGcmBindZcull(0, sDepthOffset,
						(sDisplayWidth+0x3f)&(~0x3f), (sDisplayHeight+0x3f)&(~0x3f), 0,
						CELL_GCM_ZCULL_Z24S8, CELL_GCM_SURFACE_CENTER_1,
						CELL_GCM_ZCULL_LESS, CELL_GCM_ZCULL_LONES,
						CELL_GCM_SCULL_SFUNC_LESS, 0x80, 0xff));

		sIsZcullSet = true;
	}
}

void cellGcmUtilUnsetZcull(void)
{
	if( sIsZcullSet ) {

		// cellGcmUnbindZcull needs to be called when RSX is idle.
		//
		cellGcmUtilWaitForIdle();

		// zcull width & height must be 64 aligned
		cellGcmUnbindZcull(0);

		sIsZcullSet = false;
	}
}

CellGcmSurface* cellGcmUtilGetCurrentSurface(void)
{
	if( sIsSurfaceSet ) 
		return &sSurface;
	else {
		return NULL;
	}
}

static inline void setupSurface( uint32_t index )
{
	sSurface.colorFormat 	= sGcmColorFormat;
	sSurface.colorTarget	= CELL_GCM_SURFACE_TARGET_0;
	sSurface.colorLocation[0]	= CELL_GCM_LOCATION_LOCAL;
	sSurface.colorOffset[0] 	= sColorOffset[index];
	sSurface.colorPitch[0] 	    = sIsTiledSet? sColorTiledPitch: sColorPitch;

	sSurface.colorLocation[1]	= CELL_GCM_LOCATION_LOCAL;
	sSurface.colorLocation[2]	= CELL_GCM_LOCATION_LOCAL;
	sSurface.colorLocation[3]	= CELL_GCM_LOCATION_LOCAL;
	sSurface.colorOffset[1] 	= 0;
	sSurface.colorOffset[2] 	= 0;
	sSurface.colorOffset[3] 	= 0;
	sSurface.colorPitch[1]	= 64;
	sSurface.colorPitch[2]	= 64;
	sSurface.colorPitch[3]	= 64;

	sSurface.depthFormat 	= CELL_GCM_SURFACE_Z24S8;
	sSurface.depthLocation	= CELL_GCM_LOCATION_LOCAL;
	sSurface.depthOffset	= sDepthOffset;
	sSurface.depthPitch 	= sIsTiledSet? sDepthTiledPitch: sDepthPitch;

	sSurface.type		= CELL_GCM_SURFACE_PITCH;
	sSurface.antialias	= CELL_GCM_SURFACE_CENTER_1;

	sSurface.width 		= sDisplayWidth;
	sSurface.height 	= sDisplayHeight;
	sSurface.x 		= 0;
	sSurface.y 		= 0;
}

void cellGcmUtilResetRenderTarget(void)
{
	setupSurface( sFrameIndex );
	cellGcmSetSurface(&sSurface);

	sIsSurfaceSet = true;
}

void cellGcmUtilSwapRenderTarget(void)
{
	sFrameIndex = (sFrameIndex+1)%GCM_UTIL_COLOR_BUF_NUM;

	cellGcmUtilResetRenderTarget();
}

/* wait until flip */
static void waitFlip(void)
{
	while (cellGcmGetFlipStatus()!=0){
		sys_timer_usleep(300);
	}
	cellGcmResetFlipStatus();
}

void cellGcmUtilFlipDisplay(void)
{
	static int first=1;

	// wait until the previous flip executed
	if (first!=1) waitFlip();
	else cellGcmResetFlipStatus();

	if(cellGcmSetFlip(sFrameIndex) != CELL_OK) return;
	cellGcmFlush();

	cellGcmSetWaitFlip();

	// New render target
	cellGcmUtilSwapRenderTarget();

	first=0;
}

// Utility to set clear color for float16 surface
//
// NOTE: color is given in argb order
void cellGcmUtilSetClearColor(const uint32_t color)
{
	// reoder argb -> rgba
	sClearColor[0] = ((float)(uint8_t)(color >> 16))/255.f;  // a
	sClearColor[1] = ((float)(uint8_t)(color >>  8))/255.f;  // r
	sClearColor[2] = ((float)(uint8_t)(color >>  0))/255.f;  // g
	sClearColor[3] = ((float)(uint8_t)(color >> 24))/255.f;  // b
}

static void quadClear(void) 
{
	// set shaders
	cellGcmSetVertexProgram(sVertexProgram, sVertexProgramUcode);
	cellGcmSetFragmentProgram(sFragmentProgram, sFragmentProgramOffset);
	cellGcmSetVertexDataArray(sQuadClearPositionIndex,
			0, 
			sizeof(float)*2, 
			2, 
			CELL_GCM_VERTEX_F, 
			CELL_GCM_LOCATION_LOCAL, 
			sQuadBufferOffset);
	cellGcmSetVertexData4f(sQuadClearColorIndex, 
	        sClearColor );

	// set polygon fill mode
	cellGcmSetFrontPolygonMode( CELL_GCM_POLYGON_MODE_FILL );

	// 
	cellGcmSetDrawArrays(CELL_GCM_PRIMITIVE_TRIANGLE_STRIP, 0, 4);
}

// Utility to clear float16 surface
//
void cellGcmUtilSetClearSurface(const uint32_t mask)
{
	uint32_t color_mask = 0;

	// set color mask
	if( mask | CELL_GCM_CLEAR_R )
		color_mask |= CELL_GCM_COLOR_MASK_R;
	if( mask | CELL_GCM_CLEAR_G )
		color_mask |= CELL_GCM_COLOR_MASK_G;
	if( mask | CELL_GCM_CLEAR_B )
		color_mask |= CELL_GCM_COLOR_MASK_B;
	if( mask | CELL_GCM_CLEAR_A )
		color_mask |= CELL_GCM_COLOR_MASK_A;

	if( color_mask ) {
		cellGcmSetColorMask( color_mask ); 

		// clear color buffer by drawing full screen quad
		quadClear();

		// restore color mask
		cellGcmSetColorMask( CELL_GCM_COLOR_MASK_R |
							 CELL_GCM_COLOR_MASK_G |
							 CELL_GCM_COLOR_MASK_B |
							 CELL_GCM_COLOR_MASK_A );
	}

	uint32_t depth_stencil_mask = 0;

	// set depth mask
	if( mask | CELL_GCM_CLEAR_Z )
		depth_stencil_mask |= CELL_GCM_CLEAR_Z;
	if( mask | CELL_GCM_CLEAR_S )
		depth_stencil_mask |= CELL_GCM_CLEAR_S;

	if( mask ) {
		// clear depth & stencil buffer
		cellGcmSetClearSurface( depth_stencil_mask );
	}
}

//
// Getters
//
void* cellGcmUtilGetColorBaseAddress(void)
{
	return sColorBaseAddress;
}

void* cellGcmUtilGetDepthBaseAddress(void)
{
	return sDepthBaseAddress;
}

uint32_t cellGcmUtilGetDisplayWidth(void)
{
	return sDisplayWidth;
}

uint32_t cellGcmUtilGetDisplayHeight(void)
{
	return sDisplayHeight;
}

uint32_t cellGcmUtilGetColorBufferSize(void)
{
	return sColorSize;
}

uint32_t cellGcmUtilGetDepthBufferSize(void)
{
	return sDepthSize;
}

bool cellGcmUtilIsFP16Supported(void)
{
	return sIsFP16Supported;
}

float cellGcmUtilGetDisplayAspectRatio(void)
{
	return sDisplayAspectRatio;
}

