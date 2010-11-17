/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2009 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/process.h>
#include <cell/sysmodule.h>

// timer
#include <sys/time_util.h>
#include <sys/sys_time.h>
#define U_SEC (1000 * 1000)

// libgcm
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>

// gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;


// for debug
#ifdef NDEBUG
#define ASSERT(x)
#else
#define ASSERT(x) assert(x)
#endif

using CellGcmUtil::CTimer;

CTimer::CTimer(): m_count(0), m_start(0), m_is_timebase(false), m_time(0.0f), sample_count(0), m_prev(0){}
CTimer::~CTimer(){}

void CTimer::StartCount()
{
	uint64_t tb_cur = 0;
	SYS_TIMEBASE_GET(tb_cur);

	m_start = tb_cur;
}
void CTimer::EndCount()
{
	uint64_t tb_cur = 0;
	SYS_TIMEBASE_GET(tb_cur);

	m_count += tb_cur - m_start;

	m_is_timebase = true;
}
void CTimer::UpdateTimeBase()
{
	uint64_t tb_cur = 0;
	SYS_TIMEBASE_GET(tb_cur);

	m_count += tb_cur - m_prev;
	m_prev = tb_cur;

	m_is_timebase = true;
}
void CTimer::UpdateUSec(system_time_t now_time)
{
	m_count += now_time - m_prev;
	m_prev = now_time;

	m_is_timebase = false;
}
void CTimer::UpdateCountUSec(uint64_t count)
{
	m_count += count;

	m_is_timebase = false;
}
float CTimer::GetTime(uint32_t sample)
{
	++sample_count;

	uint64_t tb_freq = sys_time_get_timebase_frequency();

	if(sample_count == sample){
		float count_per_sample = m_count / (float)sample;
		if(m_is_timebase){
			m_time = count_per_sample / (float)(tb_freq / U_SEC);
		}else{
			m_time = count_per_sample;
		}

		m_count = 0;
		sample_count = 0;
	}

	return m_time;
}

uint32_t CTimer::GetCount()
{	
	if(m_is_timebase){
		uint64_t tb_freq = sys_time_get_timebase_frequency();
		return m_count * U_SEC / tb_freq;
	}else{
		return m_count;
	}
}
