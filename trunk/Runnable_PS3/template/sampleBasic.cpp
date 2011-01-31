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

// libdbgfont
#include <cell/dbgfont.h>

// vectormath
#include <vectormath/cpp/vectormath_aos.h>
using namespace Vectormath::Aos;

// file
#include <cell/cell_fs.h>
#include <sys/paths.h>

// using gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;

#include "sampleBasic.h"
using namespace CellGcmUtil::SampleBasic;

// sample navigator
#include "snaviutil.h"

// global variable
SampleApp_t CellGcmUtil::SampleBasic::gSampleApp;

namespace{
	Memory_t sMemDbgFont;

	const uint32_t U_SEC  = (1000 * 1000);
	float ave_usec(U_SEC);
	float ave_et = 0.0f;
	float ave_proc = 0.0f;
	CTimer sFpsTimer, sDrawTimer, sProcTimer;

	const uint32_t TIME_STAMP_BASE_INDEX = 1024;
	enum{
		TIME_STAMP_DRAW_START = 0,
		TIME_STAMP_DRAW_END,
		TIME_STAMP_NUMBER,
	};
	uint32_t sTimeStampIndex = 0;
	uint32_t sTimeStampNumber = 0;

	CellGcmReportData* sReportAddr = 0;
	Memory_t sReportAddressMain;

	void waitFlip()
	{
		// PPU wait for RSX(TM) flip finish
		while(cellGcmGetFlipStatus() != 0){
			sys_timer_usleep(200);
		}
	}

	void flip(void)
	{
		// wait flip
		waitFlip();
		
		// flip
		cellGcmResetFlipStatus();
		if(cellGcmSetFlip(gSampleApp.nFrameIndex) != CELL_OK) return;
		
		// RSX(TM) wait for flip
		cellGcmSetWaitFlip();
		
		// run RSX(TM)
		cellGcmFlush();
		
		// set next render target
		++gSampleApp.nFrameIndex;
		gSampleApp.nFrameIndex = gSampleApp.nFrameIndex % gSampleApp.nFrameNumber;

		// set render target
		cellGcmSetSurface(&gSampleApp.p_vSurface[gSampleApp.nFrameIndex]);

		// set next timestamp
		++sTimeStampIndex;
		sTimeStampIndex = sTimeStampIndex % sTimeStampNumber;
	}

	void setTimeStamp(uint32_t index)
	{
		cellGcmSetReportLocation(CELL_GCM_LOCATION_MAIN);

		uint32_t actIndex = TIME_STAMP_BASE_INDEX + TIME_STAMP_NUMBER * sTimeStampIndex + index;
		sReportAddr[actIndex].zero = 1; // Write dummy data for sync
		cellGcmSetTimeStamp(actIndex);
	}

	bool getTimeStamp(uint32_t index, uint64_t *timer)
	{
		uint32_t pastTimeStampIndex = (sTimeStampIndex==sTimeStampNumber-1? 0: sTimeStampIndex+1);
		uint32_t pastIndex = TIME_STAMP_BASE_INDEX + TIME_STAMP_NUMBER * pastTimeStampIndex + index;
		
		// Check dummy data for sync
		if(sReportAddr[pastIndex].zero != 0) return false;
		
		if(timer) *timer = sReportAddr[pastIndex].timer;
		
		return true;
	}

	uint64_t getTimeStampCount(uint32_t index_begin, uint32_t index_end)
	{
		uint64_t tb(0), te(0);
		if(getTimeStamp(index_begin, &tb) && getTimeStamp(index_end, &te))
		{
			return (te - tb) / 1000;
		}

		return 0;
	}

	void updateTime()
	{
		const uint32_t NUM_TEST_COUNT = 12;

		sFpsTimer.UpdateUSec(cellGcmGetLastFlipTime());
		sDrawTimer.UpdateCountUSec(getTimeStampCount(TIME_STAMP_DRAW_START, TIME_STAMP_DRAW_END));

		ave_usec = sFpsTimer.GetTime(NUM_TEST_COUNT);
		ave_et = sDrawTimer.GetTime(NUM_TEST_COUNT);
		ave_proc = sProcTimer.GetTime(NUM_TEST_COUNT);

		if(ave_usec == 0.0f) ave_usec = U_SEC;
	}

	bool initReport()
	{
		const int32_t MAIN_REPORT_ALIGN = 0x00100000; //  1MB
		const int32_t MAIN_REPORT_SIZE  = 0x00100000; // 1MB (MAIN_REPORT_SIZE_MAX  = 0x01000000; // 16MB)

		if(cellGcmUtilAllocateUnmappedMain(MAIN_REPORT_SIZE, MAIN_REPORT_ALIGN, &sReportAddressMain) == false)
		{
			return false;
		}

		int ret = 0;
		ret = cellGcmMapEaIoAddress(sReportAddressMain.addr, 0x0e000000, MAIN_REPORT_SIZE);
		if(ret != CELL_OK)
		{
			printf("cellGcmMapEaIoAddress() failed. (0x%x)\n", ret);
			cellGcmUtilFree(&sReportAddressMain);
			return false;
		}

		// Report to Main Memory
		cellGcmSetReportLocation(CELL_GCM_LOCATION_MAIN);
		sReportAddr = cellGcmGetReportDataAddressLocation(0, CELL_GCM_LOCATION_MAIN);

		return true;
	}

	void endReport()
	{
		int ret = cellGcmUnmapEaIoAddress(sReportAddressMain.addr);
		if(ret != CELL_OK)
		{
			printf("cellGcmUnmapEaIoAddress() failed. (0x%x)\n", ret);
		}
		cellGcmUtilFree(&sReportAddressMain);
	}

	bool onSampleInit(void)
	{
		if(onInit() == false){
			printf("onInit() failed.\n");
			return false;
		}

		// init pad
		cellPadUtilPadInit();
		cellPadUtilSetSensorMode(true);
		cellPadUtilSetPressMode(true);
		cellPadUtilSetSensorFilter(CELL_PADFILTER_IIR_CUTOFF_2ND_LPF_BT_010);

		// init dbgfont
		const int32_t word_count = 32 * 80 * 16;
		int32_t rq_size = cellGcmUtilDbgfontGetRequireSize(word_count);
		
		if(cellGcmUtilAllocateLocal(rq_size, 128, &sMemDbgFont) == false){
			printf("Cannot allocate memory for Dbgfont.\n");
			return false;
		}
		if(cellGcmUtilDbgfontInit(word_count, sMemDbgFont.addr, sMemDbgFont.size, sMemDbgFont.location) == false){
			printf("cellGcmUtilDbgfontInit() failed.\n");
			return false;
		}

		// set the flip status to "flip occurred"
		cellGcmSetFlipStatus();

		CellVideoOutResolution reso = cellGcmUtilGetResolution();
		printf("VideoOutResolution: %dx%d\n", reso.width, reso.height);
		for(uint32_t i = 0; i < gSampleApp.nFrameNumber; ++i){
			printf("VideoOutBuffer[%d]: format=0x%02x offset=0x%08x\n", i, gSampleApp.p_vSurface[i].colorFormat, gSampleApp.p_vSurface[i].colorOffset[0]);
		}

		if(initReport() == false)
		{
			printf("initReport() failed.\n");
			return false;
		}

		sTimeStampIndex = 0;
		sTimeStampNumber = gSampleApp.nFrameNumber * 2;

		return true;
	}

	void onSampleUpdate(void)
	{
		if(!cellPadUtilUpdate())return;

		if(gSampleApp.isPause){
			if(cellPadUtilButtonPressed(0, CELL_UTIL_BUTTON_SELECT | CELL_UTIL_BUTTON_START)){
				gSampleApp.isWantReboot = true;
			}
		}

		if(cellPadUtilButtonPressedOnce(0, CELL_UTIL_BUTTON_START)){
			gSampleApp.isPause = !gSampleApp.isPause;
		}

		onUpdate();
	}

	void onSampleFrame(void)
	{
		onFrame();
	}

	void onSampleDbgfont(void)
	{
		onDbgfont();

		// draw dbgfont
		cellDbgFontDrawGcm();

		cellGcmFlush();
	}

	void onSampleDraw(void)
	{
		setTimeStamp(TIME_STAMP_DRAW_START);

		onDraw();
		
		setTimeStamp(TIME_STAMP_DRAW_END);

		cellGcmFlush();

		onSampleDbgfont();
	}

	void onSampleFinish(void)
	{
		cellGcmUtilDbgfontEnd();
		cellGcmUtilFree(&sMemDbgFont);

		cellPadUtilPadEnd();

		{
			cellGcmFinish(0x00000001);
			cellGcmSetReferenceCommand(0xFFFFFFFF);

			// Black Out
			cellGcmSetSurface(&gSampleApp.p_vSurface[gSampleApp.nFrameIndex]);
			cellGcmSetClearColor(0xFF000000);
			cellGcmSetClearSurface(CELL_GCM_CLEAR_Z | CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A);
			flip();
			cellGcmSetWaitFlip();
			cellGcmFinish(0x00000002);

			// Wait for last flip
			waitFlip();
		}

		onFinish();
		
		endReport();

		cellGcmUtilEnd();
	}

} // namespace SampleBasic

void sysutilEventCallback(uint64_t status, uint64_t param, void* userdata)
{
	(void) param;
	(void) userdata;

	switch(status) {
	case CELL_SYSUTIL_REQUEST_EXITGAME:
		gSampleApp.isKeepRunning = false;	
		break;
	case CELL_SYSUTIL_DRAWING_BEGIN:
	case CELL_SYSUTIL_DRAWING_END:
		break;
	case CELL_SYSUTIL_SYSTEM_MENU_OPEN:
		gSampleApp.isSysMenu = true;
		break;
	case CELL_SYSUTIL_SYSTEM_MENU_CLOSE:
		gSampleApp.isSysMenu = false;
		break;
	case CELL_SYSUTIL_BGMPLAYBACK_PLAY:
	case CELL_SYSUTIL_BGMPLAYBACK_STOP:
		break;
	default:
		break;
	}
}

void CellGcmUtil::SampleBasic::sampleDrawSimplePerf()
{
	float dx(0.0f), dy(0.0f);
	cellGcmUtilGetPrintPos(&dx, &dy);

	cellGcmUtilSetPrintPos(0.83f, 0.9f);
	cellGcmUtilSetPrintSize(0.5f);
	cellGcmUtilPrintf("      FPS: %.2f\n", U_SEC / ave_usec);
	cellGcmUtilPrintf("DRAW_TIME: %.3f us\n", ave_et);
	cellGcmUtilPrintf("PROC_TIME: %.3f ms\n", ave_proc / 1000.0f);
	if(gSampleApp.isSysMenu){
		cellGcmUtilPrintf("   STATUS: InGame XMB\n");
	}else if(gSampleApp.isPause){
		cellGcmUtilPrintf("   STATUS: PAUSE\n");
	}
	cellGcmUtilSetPrintPos(dx, dy);
}

#include "..\..\Interface.h"
int CellGcmUtil::SampleBasic::sampleMain(int argc, char *argv[])
{
	// setup sysutil event callback
	int ret = cellSysutilRegisterCallback(0, sysutilEventCallback, NULL);
	if(ret != CELL_OK)
	{
		printf("cellSysutilRegisterCallback() failed. (0x%x)\n", ret);
		return -1;
	}

	// init sample app struct
	memset(&gSampleApp, 0, sizeof(gSampleApp));

	if(!onSampleInit()){
		printf("onSampleInit() failed.\n");
		return -1;
	}

	GameService::Initialize(
		FALSE,	// require online
		2,		// achievement count
		0,		// version id
		0,		// trial version
		TRUE	// dump log
#if defined(_PS3)
		, 5000	// free space available in KB for PS3
		, 0		// enable ingame marketplace
#endif
		);

	gSampleApp.isKeepRunning = true;
	while(gSampleApp.isKeepRunning){
		cellSysutilCheckCallback();

		sProcTimer.StartCount();
		onSampleUpdate();
		onSampleFrame();
		onSampleDraw();
		sProcTimer.EndCount();

		updateTime();

		GameService::Update();

		flip();

		if(gSampleApp.isWantReboot){
			gSampleApp.isKeepRunning = false;
		}

		// for sample navigator
		if(cellSnaviUtilIsExitRequested(cellPadUtilGetPadData(0))){
			gSampleApp.isKeepRunning = false;
		}
	}

	GameService::Destroy();

	onSampleFinish();

	if(gSampleApp.isWantReboot && argc > 0){
		printf("reboot: %s\n", argv[0]);
		if(cellGcmUtilIsFileExsits(argv[0])){
			exitspawn(argv[0], NULL, NULL, NULL, NULL, 1001, SYS_PROCESS_PRIMARY_STACK_SIZE_32K);
		}else{
			fprintf(stderr, "File Not Found!: %s\n", argv[0]);
		}
	}
	
	printf("--\n");

	return 0;
}
