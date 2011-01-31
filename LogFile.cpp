#include "stdafx.h"
#include "LogFile.h"

#if defined(_PS3)
#include <cell/sysmodule.h>
#include <sys/paths.h>
#include <cell/cell_fs.h>
#include <sys/timer.h>
#include <sys/time.h>

// define the file system you want to dump log:
//#define USE_HOSTFS_HOME

#ifdef USE_CFS
#define MOUNT_POINT SYS_DEV_HDD0"/game"
#endif
#ifdef USE_FAT
#define MOUNT_POINT SYS_DEV_MS
#endif
#ifdef USE_HOSTFS_HOME
#define MOUNT_POINT SYS_APP_HOME
#endif
#ifdef USE_HOSTFS_ROOT
#define MOUNT_POINT SYS_HOST_ROOT"/tmp"
#endif
#ifdef USE_DISCFS
#define MOUNT_POINT SYS_DEV_BDVD"/PS3_GAME/USRDIR"
#endif
#ifndef MOUNT_POINT
#define MOUNT_POINT SYS_APP_HOME
#endif
#endif // PS3

namespace GameService
{

#if defined(_XBOX) || defined(_XENON)
GS_CHAR LogFile::G_FileName[GS_EFileIndex_MAX][64] = 
{
	"game:\\GameServiceLog.txt",
	"game:\\Data\\GFxAssets\\GS_TmpDLCImgFile.png",
};
#elif defined(_PS3)
GS_CHAR LogFile::G_FileName[GS_EFileIndex_MAX][64] = 
{
	MOUNT_POINT"/GameServiceLog.txt",
	MOUNT_POINT"/Data/GFxAssets/GS_TmpDLCImgFile.png",
};
#else
GS_CHAR LogFile::G_FileName[GS_EFileIndex_MAX][64] = 
{
	"D:\\GameServiceLog.txt",
	"D:\\GS_TmpDLCImgFile.png"
};
#endif

const GS_CHAR* LogFile::GetFileName(GS_INT fileIndex)
{
    if (fileIndex < GS_EFileIndex_Log || fileIndex >= GS_EFileIndex_MAX)
        return NULL;

    return G_FileName[fileIndex];
}

LogFile::LogFile(GS_BOOL isDumpLog)
{
#if defined(_XBOX) || defined(_XENON)
    for (GS_UINT i=0; i<GS_EFileIndex_MAX; i++)
    {
        if (GS_EFileIndex_Log == i && !isDumpLog)
        {
            m_hFile[i] = INVALID_HANDLE_VALUE;
            continue;
        }

		if (GS_EFileIndex_TmpImg == i)
			continue;

        m_hFile[i] = CreateFile( G_FileName[i], GENERIC_WRITE, 0,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    }
#elif defined(_PS3)
    // initialize
    for (GS_UINT i=0; i<GS_EFileIndex_MAX; i++)
    {
        m_iFile[i] = -1;
    }

	int ret = -1;
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	if (ret < 0) {
        // DebugOutput("cellSysmoduleLoadModule(CELL_SYSMODULE_FS) failed (0x%x)\n", ret);
		return;
	}

    // make sure path mount finished
    CellFsStat status;
    for (int i = 0; i < 15; i++) {
        ret = cellFsStat(MOUNT_POINT, &status);
        if (ret == CELL_FS_SUCCEEDED) {
            break;
        }
        sys_timer_sleep(1);
    }

    for (GS_UINT i=0; i<GS_EFileIndex_MAX; i++)
    {
        if (GS_EFileIndex_Log == i && !isDumpLog)
        {
            m_iFile[i] = -1;
            continue;
        }

        ret = cellFsOpen(G_FileName[i], CELL_FS_O_RDWR|CELL_FS_O_CREAT|CELL_FS_O_TRUNC, &m_iFile[i], NULL, 0);
        if (ret != CELL_FS_SUCCEEDED)
        {
            // DebugOutput("cellSysmoduleLoadModule(CELL_SYSMODULE_FS) failed (0x%x)\n", ret);
            m_iFile[i] = -1;
            return;
        }
    }
#endif
}

LogFile::~LogFile()
{
#if defined(_XBOX) || defined(_XENON)
    for (GS_UINT i=0; i<GS_EFileIndex_MAX; i++)
    {
        if( m_hFile[i] != INVALID_HANDLE_VALUE )
        {
            CloseHandle( m_hFile[i] );
        }
    }
#elif defined(_PS3)
    int ret = -1;
	ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_FS);
	if (ret < 0) {
        // DebugOutput("cellSysmoduleUnloadModule(CELL_SYSMODULE_FS) failed (0x%x)\n", ret);
	}
#endif
}

GS_BOOL LogFile::DumpLog(const GS_CHAR* log)
{
    GS_CHAR final_log[1024];

#if defined(_XBOX) || defined(_XENON)
    if( INVALID_HANDLE_VALUE == m_hFile[GS_EFileIndex_Log] )
        return FALSE;

    // add time stamp into all logs
    SYSTEMTIME local_time;
    GetLocalTime( &local_time );
    sprintf_s(final_log, 1024, "%d.%d.%d-%d:%d:%d\t%s\n", 
              local_time.wYear,local_time.wMonth,local_time.wDay,local_time.wHour,local_time.wMinute,local_time.wSecond,
              log);

    // Write to the file
    DWORD dwWritten;
    if( WriteFile( m_hFile[GS_EFileIndex_Log], ( VOID* )final_log, strlen( final_log ), &dwWritten, NULL ) == 0 )
    {
        // DebugOutput( "WriteSaveGame: WriteFile failed. Error = %08x\n", GetLastError() );
        return FALSE;
    }

#elif defined(_PS3)
    if (-1 == m_iFile[GS_EFileIndex_Log])
        return FALSE;

    // add time stamp into all logs
    struct tm *date;
    time_t now;
    time( &now );
    date = localtime( &now );

    sprintf(final_log, "%d.%d.%d-%d:%d:%d\t%s\n", 
            date->tm_year+1900,date->tm_mon+1,date->tm_mday,date->tm_hour,date->tm_min,date->tm_sec,
            log);

    // Write to the file
    int ret = -1;
    uint64_t sw;
    ret = cellFsWrite(m_iFile[GS_EFileIndex_Log], (const void *)final_log, strlen(final_log), &sw);
    if (ret != CELL_FS_SUCCEEDED)
    {
        // DebugOutput("[GameService] - LogFile::Write failed (0x%x)\n", ret);
		return FALSE;
    }
#endif

	return TRUE;
}

GS_BOOL LogFile::WriteLocalFile(GS_INT fileIndex, GS_BYTE* data, GS_DWORD size)
{
    if (fileIndex < GS_EFileIndex_Log || fileIndex >= GS_EFileIndex_MAX)
        return FALSE;

    // Write to the file
#if defined(_XBOX)
	if (GS_EFileIndex_TmpImg == fileIndex)
	{
		m_hFile[fileIndex] = CreateFile( G_FileName[fileIndex], GENERIC_WRITE, 0,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	}

    DWORD dwWritten;
    if( WriteFile( m_hFile[fileIndex], ( VOID* )data, size, &dwWritten, NULL ) == 0 )
    {
        // DebugOutput( "WriteSaveGame: WriteFile failed. Error = %08x\n", GetLastError() );
        return FALSE;
    }

	if (GS_EFileIndex_TmpImg == fileIndex)
	{
		if (0 == FlushFileBuffers(m_hFile[fileIndex]))
			return FALSE;

		CloseHandle( m_hFile[fileIndex] );
		m_hFile[fileIndex] = INVALID_HANDLE_VALUE;
	}

#elif defined(_PS3)
    int ret = -1;
    uint64_t sw;
    ret = cellFsWrite(m_iFile[fileIndex], (const void *)data, size, &sw);
    if (ret != CELL_FS_SUCCEEDED)
    {
        // DebugOutput("[GameService] - LogFile::Write failed (0x%x)\n", ret);
		return FALSE;
    }
#endif
    return TRUE;
}

} // namespace
