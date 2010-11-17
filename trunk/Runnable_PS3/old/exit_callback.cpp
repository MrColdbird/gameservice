/*   SCE CONFIDENTIAL                                       */
/*   PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*   Copyright (C) 2008 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>

#include "gcmutil.h"

using namespace cell::Gcm;

static void sysutil_exit_callback(uint64_t status, uint64_t param, void* userdata);
static void rsx_exit_routine(void);

static bool sKeepRunning = true;
static bool sInitialized = false;
static void (*sExitCallback)(void) = 0;


int cellGcmUtilInitCallback(void (*exit_callback)(void))
{
	int ret;

	// register user's exit callback if exist 
	if( exit_callback != NULL ) {
		sExitCallback = exit_callback;
	}

	// register sysutil exit callback
	ret = cellSysutilRegisterCallback(0, sysutil_exit_callback, NULL);
	if( ret != CELL_OK ) {
		printf( "Registering sysutil callback failed...: error=0x%x\n", ret );
		return ret;
	}

	// 
	sInitialized = true;

	// 
	return ret;
}

// 
// Returns true if exit event not received.
// Returns false if exit event received or callback is not registered.
//  
bool cellGcmUtilCheckCallback(void)
{
	int ret;

	if( sInitialized ) {
		// check system event
		ret = cellSysutilCheckCallback();
		if( ret != CELL_OK ) {
			printf( "cellSysutilCheckCallback() failed...: error=0x%x\n", ret );
		}
	}
	else {
		printf( "GCM UTIL: Callbacks are not registered yet."
		        "          Please call cellGcmUtilInitCallback().\n" );
		return false;
	}
	
	if(sKeepRunning == false){
		// call user's exit callback if exist 
		if(sExitCallback){
			sExitCallback();
		}
		// call RSX's exit routine
		rsx_exit_routine();
	}

	return sKeepRunning;
}

void rsx_exit_routine(void)
{
	// Let RSX wait for final flip
	cellGcmSetWaitFlip();

	// Let PPU wait for all commands done (include waitFlip)
	cellGcmUtilFinish();
}

void sysutil_exit_callback(uint64_t status, uint64_t param, void* userdata)
{
	(void) param;
	(void) userdata;

	switch(status) {
	case CELL_SYSUTIL_REQUEST_EXITGAME:
		sKeepRunning = false;
		break;
	case CELL_SYSUTIL_DRAWING_BEGIN:
	case CELL_SYSUTIL_DRAWING_END:
	case CELL_SYSUTIL_SYSTEM_MENU_OPEN:
	case CELL_SYSUTIL_SYSTEM_MENU_CLOSE:
	case CELL_SYSUTIL_BGMPLAYBACK_PLAY:
	case CELL_SYSUTIL_BGMPLAYBACK_STOP:
	default:
		break;
	}
}

