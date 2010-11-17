/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2008 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#ifndef __CELL_GCMUTIL_TEMPLATE_SAMPLE_BASIC_H__
#define __CELL_GCMUTIL_TEMPLATE_SAMPLE_BASIC_H__

// gcmutil
#include "gcmutil.h"

namespace CellGcmUtil{

	namespace SampleBasic{

		extern struct SampleApp_t{
			bool isKeepRunning;
			bool isWantReboot;
			bool isPause;
			bool isSysMenu;
			uint32_t nFrameNumber;
			uint32_t nFrameIndex;
			CellGcmSurface *p_vSurface;
		} gSampleApp;

		void sampleDrawSimplePerf();
		int sampleMain(int argc, char *argv[]);

	} // namespace SampleBasic

} // namespace CellGcmUtil

// for basic sample template
extern bool onInit(void);
extern void onFinish(void);
extern void onUpdate(void);
extern void onFrame(void);
extern void onDraw(void);
extern void onDbgfont(void);

#endif /* __CELL_GCMUTIL_TEMPLATE_SAMPLE_BASIC_H__ */
