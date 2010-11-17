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

// system
#include <sys/process.h>
#include <sysutil/sysutil_sysparam.h>
#include <cell/sysmodule.h>

// libgcm
#include <cell/gcm.h>
using namespace cell::Gcm;

// vectormath
#include <vectormath/cpp/vectormath_aos.h>
using namespace Vectormath::Aos;

// file
#include <cell/cell_fs.h>
#include <sys/paths.h>

// using gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;

// using basic sample template
#include "template/sampleBasic.h"
using namespace CellGcmUtil::SampleBasic;


namespace{
	// sample display settings
	const int32_t COLOR_BUF_NUM = 2;
	CellGcmSurface sSurface[COLOR_BUF_NUM];
	Memory_t sFrameBuffer;
	Memory_t sDepthBuffer;
	CellGcmTexture sFrameTexture[COLOR_BUF_NUM];
	CellGcmTexture sDepthTexture;

	// triangle data
	struct MyVertex_t{
		float x, y, z;
		uint32_t argb;

		MyVertex_t(): x(0.0f), y(0.0f), z(0.0f), argb(0xFF000000){}
	};
	Memory_t sVertex;
	const uint32_t VTX_NUMBER = 3;

	float sDisplayAspectRatio = 0.0f;

	Shader_t sVShader;
	Shader_t sFShader;
	Memory_t sFShaderUCode;

	// vertex shader constants
	const uint32_t VS_UF_MVP_MATRIX	= 0;	// 	uniform float4x4 ModelViewProjMatrix : C0,

	// shader files
	#define VERTEX_SHADER	SYS_APP_HOME "/vs_basic.vpo"
	#define FRAGMENT_SHADER	SYS_APP_HOME "/fs_basic.fpo"

} // namespace

namespace{

bool vertexInit(void)
{
	if(cellGcmUtilAllocateLocal(VTX_NUMBER * sizeof(MyVertex_t), 128, &sVertex) == false) return false;
	memset(sVertex.addr, 0, VTX_NUMBER * sizeof(MyVertex_t));

	MyVertex_t *vertices = reinterpret_cast<MyVertex_t*>(sVertex.addr);

	vertices[0].x = -1.0f; 
	vertices[0].y = -1.0f; 
	vertices[0].z = -1.0f; 
	vertices[0].argb=0xff00ff00;

	vertices[1].x =  1.0f; 
	vertices[1].y = -1.0f; 
	vertices[1].z = -1.0f; 
	vertices[1].argb=0xff0000ff;

	vertices[2].x = -1.0f; 
	vertices[2].y =  1.0f; 
	vertices[2].z = -1.0f; 
	vertices[2].argb=0xffff0000;

	return true;
}

void vertexEnd()
{
	cellGcmUtilFree(&sVertex);
}

void vertexDraw()
{
	// begin
	{
		// setup vertex attribute
		cellGcmSetInvalidateVertexCache();
		cellGcmSetVertexDataArray(CELL_GCMUTIL_ATTR_POSITION, 0, sizeof(MyVertex_t), 3, CELL_GCM_VERTEX_F, sVertex.location, sVertex.offset);
		cellGcmSetVertexDataArray(CELL_GCMUTIL_ATTR_COLOR,    0, sizeof(MyVertex_t), 3, CELL_GCM_VERTEX_F, sVertex.location, sVertex.offset + sizeof(float) * 3);
	}

	// draw
	{
		cellGcmSetDrawArrays(CELL_GCM_PRIMITIVE_TRIANGLES, 0, 3);
	}

	// end
	{
		// invalidate vertex attribute
		cellGcmSetVertexDataArray(CELL_GCMUTIL_ATTR_POSITION, 0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
		cellGcmSetVertexDataArray(CELL_GCMUTIL_ATTR_COLOR,    0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}
}

} // namespace


bool onInit(void)
{
	// init libgcm and gcmutil
	const int32_t CB_SIZE   = 0x00800000; //   8MB
	const int32_t MAIN_SIZE = 0x08000000; // 128MB
	if(!cellGcmUtilInit(CB_SIZE, MAIN_SIZE)) return false;

	// init display
	CellVideoOutResolution reso = cellGcmUtilGetResolution();
	const uint8_t color_format = CELL_GCM_TEXTURE_A8R8G8B8;
	const uint8_t depth_format = CELL_GCM_TEXTURE_DEPTH24_D8;
	bool bRet = false;
	bRet = cellGcmUtilCreateTiledTexture(reso.width, reso.height, color_format, CELL_GCM_LOCATION_LOCAL, CELL_GCM_COMPMODE_C32_2X1, COLOR_BUF_NUM, sFrameTexture, &sFrameBuffer);
	if(bRet == false)
	{
		printf("cellGcmUtilCreateTiledTexture() failed.\n");
		return false;
	}

	bRet = cellGcmUtilCreateDepthTexture(reso.width, reso.height, depth_format, CELL_GCM_LOCATION_LOCAL, CELL_GCM_SURFACE_CENTER_1, true, true, &sDepthTexture, &sDepthBuffer);
	if(bRet == false)
	{
		printf("cellGcmUtilCreateDepthTexture() failed.\n");
		return false;
	}

	// frame texture tu surface
	for(int32_t i = 0; i < COLOR_BUF_NUM; ++i){
		sSurface[i] = cellGcmUtilTextureToSurface(&sFrameTexture[i], &sDepthTexture);
	}

	// regist output buffer
	bRet = cellGcmUtilSetDisplayBuffer(COLOR_BUF_NUM, sFrameTexture);
	if(bRet == false)
	{
		printf("cellGcmUtilSetDisplayBuffer() failed.\n");
		return false;
	}

	// setup sample template
	gSampleApp.nFrameNumber = COLOR_BUF_NUM;
	gSampleApp.nFrameIndex = 0;
	gSampleApp.p_vSurface = sSurface;
	
	// get display aspect ratio
	sDisplayAspectRatio = cellGcmUtilGetAspectRatio();

	// init shader
	if(!cellGcmUtilLoadShader(VERTEX_SHADER, &sVShader)) return false;
	if(!cellGcmUtilLoadShader(FRAGMENT_SHADER, &sFShader)) return false;
	if(!cellGcmUtilGetFragmentUCode(&sFShader, CELL_GCM_LOCATION_LOCAL, &sFShaderUCode)) return false;

	// init geometry
	if(vertexInit() == false) return false;

	return true;
}

void onFinish(void)
{
	// free gemometry
	vertexEnd();

	// free shader
	cellGcmUtilDestroyShader(&sVShader);
	cellGcmUtilDestroyShader(&sFShader);
	cellGcmUtilFree(&sFShaderUCode);

	// free display buffer
	cellGcmUtilFree(&sFrameBuffer);
	cellGcmUtilFree(&sDepthBuffer);
}

void onUpdate(void)
{
	// nothing to do
}

void onFrame(void)
{
	if(!gSampleApp.isPause && !gSampleApp.isSysMenu){
		// nothing to do
	}
}

void setDrawEnv(void)
{
	// depth test
	cellGcmSetDepthTestEnable(CELL_GCM_TRUE);
	cellGcmSetDepthFunc(CELL_GCM_LESS);
	
	// blend op
	cellGcmSetBlendEnable(CELL_GCM_TRUE);
	cellGcmSetBlendEquation(CELL_GCM_FUNC_ADD, CELL_GCM_FUNC_ADD);
	cellGcmSetBlendFunc(CELL_GCM_SRC_ALPHA, CELL_GCM_ONE_MINUS_SRC_ALPHA, CELL_GCM_ONE, CELL_GCM_ONE_MINUS_SRC_ALPHA);

	// set clear color
	cellGcmSetClearColor(0xFF404040);

	cellGcmSetFrontFace(CELL_GCM_CCW);
	cellGcmSetCullFace(CELL_GCM_BACK);
	cellGcmSetCullFaceEnable(CELL_GCM_TRUE);
	cellGcmSetZMinMaxControl(CELL_GCM_TRUE, CELL_GCM_TRUE, CELL_GCM_FALSE);
	
	// set view port
	Viewport_t vp = cellGcmUtilGetViewportGL(sSurface[0].height, 0, 0, sSurface[0].width, sSurface[0].height, 0.0f, 1.0f);
	cellGcmSetViewport(vp.x, vp.y, vp.width, vp.height, vp.min, vp.max, vp.scale, vp.offset);
	
	// clear surface
	cellGcmSetClearSurface(	CELL_GCM_CLEAR_Z | CELL_GCM_CLEAR_A |
							CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B);
}

void onDraw(void)
{
	// environment
	Matrix4 matrixProj = Matrix4::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0, 10000.0f);
	Matrix4 matrixView = Matrix4::translation(Vector3(0.0f, 0.0f, -4.0f)) * Matrix4::scale(Vector3(1.0f / sDisplayAspectRatio, 1.0f, 1.0f));
	Matrix4 matrixModel = Matrix4::identity();

	Matrix4 matrixModelViewProj = transpose(matrixProj * matrixView * matrixModel);
	
	// set shader
	cellGcmSetVertexProgram(sVShader.program, sVShader.ucode);
	cellGcmSetFragmentProgram(sFShader.program, sFShaderUCode.offset);

	// draw objects
	{
		setDrawEnv();
		cellGcmSetVertexProgramConstants(VS_UF_MVP_MATRIX, 16, reinterpret_cast<const float*>(&matrixModelViewProj));

		vertexDraw();
	}
}

void onDbgfont(void)
{
	// draw info
	cellGcmUtilSetPrintSize(0.75f);
	cellGcmUtilSetPrintPos(0.04f, 0.04f);
	cellGcmUtilSetPrintColor(0xffffffff);
	cellGcmUtilPrintf("GCM Graphics Sample\n");

	cellGcmUtilSetPrintSize(0.5f);
	cellGcmUtilSetPrintPos(0.05f, CELL_GCMUTIL_POSFIX);

	const char* const SAMPLE_NAME = "New Basic";
	if(!gSampleApp.isPause && !gSampleApp.isSysMenu){
		cellGcmUtilPrintf("%s\n", SAMPLE_NAME);
	}else if(gSampleApp.isSysMenu){
		cellGcmUtilPrintf("%s: In Game XMB\n", SAMPLE_NAME);
	}else if(gSampleApp.isPause){
		cellGcmUtilPrintf("%s: PAUSE\n", SAMPLE_NAME);
	}

	// draw perf
	if(!gSampleApp.isSysMenu){
		sampleDrawSimplePerf();
	}
}


SYS_PROCESS_PARAM(1001, 0x10000);

int main(int argc, char *argv[])
{
	// call sample template
	return sampleMain(argc, argv);
}
