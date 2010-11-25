// ======================================================================================
// File         : Utils.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:49:04 PM | Thursday,July
// Description  : 
// ======================================================================================

#pragma once
#ifndef GAMESERVICE_UTILS_H
#define GAMESERVICE_UTILS_H

namespace GameService
{

#if defined(_XBOX) || defined(_XENON) || defined(_WINDOWS)
typedef void                GS_VOID;
typedef char                GS_CHAR;
typedef int                 GS_BOOL;
typedef unsigned char       GS_BYTE;
typedef int                 GS_INT;
typedef unsigned int        GS_UINT;
typedef unsigned long       GS_DWORD;
typedef unsigned short      GS_WORD;
typedef unsigned short      GS_UINT16;
typedef float               GS_FLOAT;
typedef double              GS_DOUBLE;
typedef signed __int64      GS_INT64;
typedef unsigned __int64    GS_UINT64;  
typedef unsigned long       GS_SIZET;
typedef unsigned __int64 GS_MemoryOP;

#elif defined(_PS3)
typedef void            GS_VOID;
typedef char            GS_CHAR;
typedef uint32_t        GS_BOOL;
typedef uint8_t         GS_BYTE;
typedef int32_t         GS_INT;
typedef uint32_t        GS_UINT;
typedef int32_t         GS_DWORD;
typedef int16_t         GS_WORD;
typedef uint16_t        GS_UINT16;
typedef float           GS_FLOAT;
typedef double          GS_DOUBLE;
typedef int64_t         GS_INT64;
typedef uint64_t        GS_UINT64;  
typedef unsigned long   GS_SIZET;
typedef uint64_t		GS_MemoryOP;

#define TRUE 1
#define FALSE 0

extern int32_t STACK_SIZE; /* 16KB */
extern int32_t EXIT_CODE;
extern int32_t THREAD_PRIO;

#endif

// define GS data type

//--------------------------------------------------------------------------------------
// Debug spew and error handling routines
//--------------------------------------------------------------------------------------
#if defined(_XBOX) || defined(_XENON) || defined(_WINDOWS)
#ifdef  _Printf_format_string_  // VC++ 2008 and later support this annotation
GS_VOID CDECL DebugOutput( _In_z_ _Printf_format_string_ const GS_CHAR*, ... );  // Un-modified debug spew
GS_VOID CDECL __declspec(noreturn) FatalError( _In_z_ _Printf_format_string_ const GS_CHAR*, ... ); // Debug spew with a forced break and exit
#else
GS_VOID CDECL DebugOutput( const GS_CHAR*, ... );  // Un-modified debug spew
GS_VOID CDECL __declspec(noreturn) FatalError( const GS_CHAR*, ... ); // Debug spew with a forced break and exit
#endif
#elif defined(_PS3)
GS_VOID DebugOutput(const char * format, ...);
GS_VOID FatalError( const char*, ... );
#endif

// Macros for printing warnings/errors with prepended file and line numbers
#define GS_PrintWarning GameService::DebugOutput( "%s(%d): warning: ", __FILE__, __LINE__ ), GameService::DebugOutput
#define GS_PrintError   GameService::DebugOutput( "%s(%d): error: ",   __FILE__, __LINE__ ), GameService::DebugOutput

// Avoid compiler warnings for unused variables
#define GameService_Unused( x )   ((GS_VOID)(x))

// Assert in debug but still execute code in release
// Useful for validating expected return values from functions
#ifdef _DEBUG
    #define GameService_Verify( e ) Assert( e )
#else
    #define GameService_Verify( e ) GameService_Unused( e )
#endif    

extern GS_MemoryOP GSOPType;

} // namespace GameService

#endif
