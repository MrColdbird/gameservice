/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2010 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */
 
#define __CELL_ASSERT__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/process.h>
#include <sys/ppu_thread.h>

#include <sys/paths.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <sysutil/sysutil_sysparam.h>

#include <sys/sys_time.h>

#include <cell/pad.h>

#include "snaviutil.h"

// for global
char gCellSnaviUtilTempBufferForAdjstPath[256];

#define DPRINTF(x)

static bool sIsInitialized = false;
static bool sIsNavigatorEnable = false;
static bool sIsInitPad = false;
static const char *sSpawnPath = 0;
static const char *sSpawnName = 0;
static const char *sSnaviPath = 0;

const int32_t PAD_MAX = 7;
const int32_t MAIN_PRIO = 1001;

const char SIGN_DATA[] = "SNAVI_SPAWN_DATA";
const size_t SIGN_DATA_SIZE = 17;
const size_t SPAWN_STACK_MAX = 0x1000; // 4KB

const int32_t CellSnaviUtilSnaviContext_VERSION = 1;

typedef struct CellSnaviUtilNavigateInfo{
	const char *name;
	const char *navi_path;
	const char *sample_path;
} CellSnaviUtilNavigateInfo;

typedef struct CellSnaviUtilSnaviContext{
	char signature[32];
	int32_t version;
	int32_t offset;
	int32_t size;
	
	CellSnaviUtilNavigateInfo info;
} CellSnaviUtilSnaviContext;

// static function
static void naviutil_init(void);
static void exit_callback(void);
static void sysutil_exit_callback(uint64_t status, uint64_t param, void* userinfo);
static sys_addr_t get_stack_addr(void);
static bool is_exit_input(const CellPadData *pPad);

// public function
bool cellSnaviUtilCheckValidation(const CellSnaviUtilSnaviContext *info);
bool cellSnaviUtilGetSnaviContext(CellSnaviUtilNavigateInfo **pp_info);
int32_t cellSnaviUtilSetSnaviContext(CellSnaviUtilNavigateInfo *info, char *buffer, size_t size);
void cellSnaviUtilGetStackInfo(sys_addr_t *addr, size_t *size);


void naviutil_init(void)
{
	/* check info set by the navigator */
	CellSnaviUtilNavigateInfo *pinfo;
	bool ret = cellSnaviUtilGetSnaviContext(&pinfo);
	
	if(ret){
		sIsNavigatorEnable = true;
		
		sSpawnName = pinfo->name;
		sSpawnPath = pinfo->sample_path;
		sSnaviPath = pinfo->navi_path;
		
		/* catch exit */
		atexit(exit_callback);
		
		/* catch exit by the PS button */
		cellSysutilRegisterCallback(3, sysutil_exit_callback, NULL);
		
		printf("Execute from Navigator: %s\n", sSpawnName);
		printf("  Sample Home: %s\n", sSpawnPath);
	}else{
		DPRINTF("Navigator Disabled.\n");
	}
	
	sIsInitialized = true;
}

bool is_exit_input(const CellPadData *pPad)
{
	if(!pPad) return false;
	if(pPad->len == 0) return false;
	
	bool ret = (pPad->button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_START && 
				pPad->button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_SELECT && 
				pPad->button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1 && 
				pPad->button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L1 && 
				pPad->button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2 && 
				pPad->button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2);
	
	return ret;
}

bool cellSnaviUtilIsExitRequested(const CellPadData *pPad)
{
	if(!sIsInitialized){
		naviutil_init();
	}
	
	if(!sIsNavigatorEnable) return false;
	
	if(pPad == 0){
		CellPadInfo2 pad_info;
		int32_t cr = cellPadGetInfo2(&pad_info);
		if(cr == CELL_PAD_ERROR_UNINITIALIZED && sIsInitPad == false){
			cellPadInit(PAD_MAX);
			cr = cellPadGetInfo2(&pad_info);
			sIsInitPad = true;
		}
		if(cr != CELL_PAD_OK) return false;
		
		
		/* check all pad */
		for(uint32_t i = 0; i < pad_info.max_connect; ++i){
			if((pad_info.port_status[i] && CELL_PAD_STATUS_CONNECTED)==0) continue;
			
			CellPadData cpd;
			int32_t err = cellPadGetData(i, &cpd);
			
			if(err != CELL_PAD_OK) continue;
			
			if(is_exit_input(&cpd)){
				return true;
			}
		}
	}else{
		return is_exit_input(pPad);
	}
	
	return false;
}

const char* cellSnaviUtilAdjustPath(const char* app_home_path, char* buffer)
{
	if(buffer == 0) return app_home_path;
	if(app_home_path == 0) return app_home_path;

	if(!sIsInitialized){
		naviutil_init();
	}
	
	if(!sIsNavigatorEnable) return app_home_path;

	if(strncmp(app_home_path, SYS_APP_HOME, strlen(SYS_APP_HOME)) == 0){
		strcpy(buffer, sSpawnPath);
		strcat(buffer, app_home_path + strlen(SYS_APP_HOME));
		printf("  Adjust Path: %s -> %s\n", app_home_path, buffer);

		return buffer;
	}

	return app_home_path;
}

void exit_callback(void)
{
	if(!sIsInitialized){
		naviutil_init();
	}
	
	if(!sIsNavigatorEnable) return;
	
	if(sIsInitPad){
		cellPadEnd();
	}
	
	bool file_exists = false;
	const char *fname = sSnaviPath;
	{
		int fd = open(fname, O_RDONLY);
		
		if(fd > 0){
			file_exists = true;
			close(fd);
		}
	}
	
	if(!file_exists) return;
	
	/* refer to the data received when starting this program */
	sys_addr_t data = 0;
	size_t data_size = 0;
	cellSnaviUtilGetStackInfo(&data, &data_size);
	
	/* excute navigator */
	exitspawn(fname, 0, 0, data, data_size, MAIN_PRIO, 
								SYS_PROCESS_PRIMARY_STACK_SIZE_32K);
}

void sysutil_exit_callback(uint64_t status, uint64_t param, void* userdata)
{
	(void) param;
	(void) userdata;
	
	switch(status) {
	case CELL_SYSUTIL_REQUEST_EXITGAME:
		/* don't return to the navigator if it ended with the PS button */
		sIsNavigatorEnable = false;
		break;
	
	case CELL_SYSUTIL_DRAWING_BEGIN:
	case CELL_SYSUTIL_DRAWING_END:
	default:
		break;
	}
}


sys_addr_t get_stack_addr(void)
{
	sys_addr_t addr;
	sys_ppu_thread_stack_t stack_info;
	
	sys_ppu_thread_get_stack_information(&stack_info);
	addr = stack_info.pst_addr + stack_info.pst_size - SPAWN_STACK_MAX;
	
	return addr;
}

void cellSnaviUtilGetStackInfo(sys_addr_t *addr, size_t *size)
{
	if(addr) *addr = get_stack_addr();
	if(size) *size = SPAWN_STACK_MAX;
}

bool cellSnaviUtilCheckValidation(const CellSnaviUtilSnaviContext *context)
{
	if(!context) return false;
	
	int32_t cmp = strncmp(context->signature, SIGN_DATA, SIGN_DATA_SIZE - 1);
	
	return (cmp == 0);
}

bool cellSnaviUtilGetSnaviContext(CellSnaviUtilNavigateInfo **pp_info)
{
	sys_addr_t addr = get_stack_addr();
	
	CellSnaviUtilSnaviContext *context = (CellSnaviUtilSnaviContext *)addr;
	*pp_info = &context->info;
	CellSnaviUtilNavigateInfo *ret_info = *pp_info;
	
	ret_info->name = 0;
	ret_info->navi_path = 0;
	ret_info->sample_path = 0;
	
	/* invalid context */
	if(!cellSnaviUtilCheckValidation(context)) return false;
	
	/* set ptr */
	char *ptr_info = (char *)(addr + context->offset);
	
	/* get navigator path */
	ret_info->navi_path = ptr_info;
	ptr_info += strlen(ret_info->navi_path) + 1;
	
	/* get sample name */
	ret_info->name = ptr_info;
	ptr_info += strlen(ret_info->name) + 1;

	/* get sample path */
	ret_info->sample_path = ptr_info;
	ptr_info += strlen(ret_info->sample_path) + 1;

	return true;
}

int32_t cellSnaviUtilSetSnaviContext(CellSnaviUtilNavigateInfo *info, char *buffer, size_t size)
{
	/* calc size */
	size_t path_len = strlen(info->navi_path) + 1;
	size_t name_len = strlen(info->name) + 1;
	size_t sample_len = strlen(info->sample_path) + 1;
	size_t header_size = sizeof(CellSnaviUtilSnaviContext);
	size_t info_size = sample_len + name_len + path_len + 1;
	size_t all_size = info_size + header_size;
	
	/* buffer shortage */
	if(all_size > size){
		return size - all_size;
	}
	
	if(!buffer) return 0;
	
	
	char *ptr = buffer;
	
	/* set context */
	CellSnaviUtilSnaviContext *context = (CellSnaviUtilSnaviContext *)ptr;

	/* set signature */
	memmove(context->signature, SIGN_DATA, SIGN_DATA_SIZE);
	
	/* set header */
	context->version = CellSnaviUtilSnaviContext_VERSION;
	context->offset = sizeof(CellSnaviUtilSnaviContext);
	context->size = info_size;
	
	ptr += context->offset;
	
	/* set info */
	memmove(ptr, info->navi_path, path_len);
	ptr += path_len;
	
	memmove(ptr, info->name, name_len);
	ptr += name_len;

	memmove(ptr, info->sample_path, sample_len);
	ptr += sample_len;
	
	ptr[0] = '\0';

	return all_size;
}

