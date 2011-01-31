// ======================================================================================
// File         : Utils.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:59 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"

namespace GameService
{

#if defined(_PS3)
int32_t STACK_SIZE = 0x4000; /* 16KB */
int32_t EXIT_CODE = 0xbee;
int32_t THREAD_PRIO = 1002;
#endif

//--------------------------------------------------------------------------------------
// Name: DebugSpewV()
// Desc: Internal helper function
//--------------------------------------------------------------------------------------
#if defined(_XBOX) || defined(_XENON) || defined(_WINDOWS)
static GS_VOID DebugOutputV( const GS_CHAR* strFormat, const va_list pArgList )
{
#ifdef _DEBUG
    GS_CHAR str[2048];
    // Use the secure CRT to avoid buffer overruns. Specify a count of
    // _TRUNCATE so that too long strings will be silently truncated
    // rather than triggering an error.
    _vsnprintf_s( str, _TRUNCATE, strFormat, pArgList );
#if !defined(_WINDOWS)
    OutputDebugStringA( str );
#endif
#endif
}
#endif

//--------------------------------------------------------------------------------------
// Name: DebugSpew()
// Desc: Prints formatted debug spew
//--------------------------------------------------------------------------------------
#if defined(_XBOX) || defined(_XENON)
#ifdef  _Printf_format_string_  // VC++ 2008 and later support this annotation
GS_VOID CDECL DebugOutput( _In_z_ _Printf_format_string_ const GS_CHAR* strFormat, ... )
#else
GS_VOID CDECL DebugOutput( const GS_CHAR* strFormat, ... )
#endif
{
#ifdef _DEBUG
    va_list pArgList;
    va_start( pArgList, strFormat );
    DebugOutputV( strFormat, pArgList );
    va_end( pArgList );
#endif
}
#elif defined(_PS3)
GS_VOID DebugOutput(const char * format, ...)
{
#ifdef _DEBUG
    va_list pArgList;
    va_start( pArgList, format );
    printf( format, pArgList );
    va_end( pArgList );
#endif
}
#else
GS_VOID DebugOutput(const char * format, ...)
{
}
#endif

//--------------------------------------------------------------------------------------
// Name: FatalError()
// Desc: Prints formatted debug spew and breaks into the debugger. Exits the application.
//--------------------------------------------------------------------------------------
#if defined(_XBOX) || defined(_XENON)
#ifdef  _Printf_format_string_  // VC++ 2008 and later support this annotation
GS_VOID CDECL FatalError( _In_z_ _Printf_format_string_ const GS_CHAR* strFormat, ... )
#else
GS_VOID CDECL FatalError( const GS_CHAR* strFormat, ... )
#endif
{
#ifdef _DEBUG
    va_list pArgList;
    va_start( pArgList, strFormat );
    DebugOutputV( strFormat, pArgList );
    va_end( pArgList );

    GS_Assert(0);

    // exit(0);
#endif
}
#elif defined(_PS3)
GS_VOID FatalError( const char* strFormat, ... )
{
#ifdef _DEBUG
    va_list pArgList;
    va_start( pArgList, strFormat );
    printf( strFormat, pArgList );
    va_end( pArgList );

    GS_Assert(0);
#endif
}
#else
GS_VOID FatalError( const char* strFormat, ... )
{
}
#endif

} // namespace GameService
