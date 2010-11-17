/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2008 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/process.h>
#include <cell/sysmodule.h>

// libgcm
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>

// libdbgfont
#include <cell/dbgfont.h>

// gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;


using namespace cell::Gcm;

namespace{
	const float WORD_X_MAX = 80.0f;
	const float WORD_Y_MAX = 32.0f;
	float m_pos_x = 0.0f;
	float m_pos_y = 0.0f;
	float m_margin = 0.0f;
	float m_size = 1.0f;
	uint32_t m_color = 0xffffffff;

	uint32_t dbgfont_argb2abgr(uint32_t color)
	{
		return (color & 0xFF00FF00) | ((color >> 16) & 0x000000FF) | ((color << 16) & 0x00FF0000);
	}

	void dbgfont_puts(char *str)
	{
		cellDbgFontPuts(m_pos_x, m_pos_y, m_size, dbgfont_argb2abgr(m_color), str);

		uint32_t str_len = strlen(str);
		m_pos_x += str_len * m_size / WORD_X_MAX;	
	}
	void dbgfont_puts_format(char *str)
	{
		char *p = strchr(str, '\n');
		if(p != NULL){
			str[p - str] = '\0';
			dbgfont_puts(str);
			
			// new line
			m_pos_x = m_margin;
			m_pos_y += m_size / WORD_Y_MAX;

			dbgfont_puts_format(++p);
		}else{
			dbgfont_puts(str);
		}
	}
} // no name namespace

namespace CellGcmUtil{

uint32_t cellGcmUtilDbgfontGetRequireSize(int32_t word_count)
{
	const int32_t vertex_size = word_count * CELL_DBGFONT_VERTEX_SIZE;
	const int32_t total_size = CELL_DBGFONT_FRAGMENT_SIZE + CELL_DBGFONT_TEXTURE_SIZE + vertex_size;

	return total_size;
}

bool cellGcmUtilDbgfontInit(int32_t word_count, void* addr, uint32_t size, uint8_t location)
{
	// init dbgfont
	if(size < cellGcmUtilDbgfontGetRequireSize(word_count)) return false;
	if(!addr) return false;
	if(location != CELL_GCM_LOCATION_LOCAL && location != CELL_GCM_LOCATION_MAIN) return false;

	CellVideoOutResolution resolution = cellGcmUtilGetResolution();
	
	CellDbgFontConfigGcm cfg;
	memset(&cfg, 0, sizeof(CellDbgFontConfigGcm));

	if(location == CELL_GCM_LOCATION_MAIN){
		cfg.mainBufAddr = (sys_addr_t)addr;
		cfg.mainBufSize  = size;
	}else{
		cfg.localBufAddr = (sys_addr_t)addr;
		cfg.localBufSize = size;
	}

	cfg.screenWidth  = resolution.width;
	cfg.screenHeight = resolution.height;
	cfg.option = CELL_DBGFONT_VERTEX_LOCAL|
			CELL_DBGFONT_TEXTURE_LOCAL|
			CELL_DBGFONT_SYNC_OFF |
			CELL_DBGFONT_VIEWPORT_ON |
			CELL_DBGFONT_MINFILTER_LINEAR |
			CELL_DBGFONT_MAGFILTER_NEAREST;

	if(cellDbgFontInitGcm(&cfg) != CELL_OK){
		return false;
	}

	return true;
}

void cellGcmUtilDbgfontEnd()
{
	// end dbgfong
	cellDbgFontExitGcm();
}

void cellGcmUtilSetPrintPos(float x, float y)
{
	if(x >= 0.0f && x <= 1.0f){
		m_pos_x = x;
		m_margin = x;
	}
	if(y >= 0.0f && y <= 1.0f){
		m_pos_y = y;
	}
}
void cellGcmUtilPrintf(const char* format, ...)
{
	char buffer[1024];
	va_list marker;
	va_start(marker,format);
	std::vsprintf(buffer, format, marker);
	va_end(marker);

	dbgfont_puts_format(buffer);
}
void cellGcmUtilPrintfColor(uint32_t color, const char* format, ...)
{
	char buffer[1024];
	va_list marker;
	va_start(marker,format);
	std::vsprintf(buffer, format, marker);
	va_end(marker);

	uint32_t color_backup = m_color;
	cellGcmUtilSetPrintColor(color);
	dbgfont_puts_format(buffer);
	cellGcmUtilSetPrintColor(color_backup);
}
void cellGcmUtilPuts(const char* str)
{
	char buffer[1024];
	char *ptr = buffer;
	bool is_local_alloc = strlen(str) > 1023;
	if(is_local_alloc){
		size_t len = strlen(str) + 1;
		ptr = new char[len];
		memcpy(ptr, str, len);
		ptr[len - 1] = 0;
	}else{
		strcpy(ptr, str);
	}

	dbgfont_puts_format(ptr);

	if(is_local_alloc){
		delete [] ptr, ptr = 0;
	}
}
void cellGcmUtilSetPrintSize(float size)
{
	m_size = size;
}
void cellGcmUtilSetPrintColor(uint32_t color)
{
	m_color = color;
}
void cellGcmUtilGetPrintPos(float *x, float *y)
{
	if(x) *x = m_pos_x;
	if(y) *y = m_pos_y;
}

} // namespace CellGcmUtil
