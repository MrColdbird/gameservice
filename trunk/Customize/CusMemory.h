#ifndef __CUSMEMORY_H__
#define __CUSMEMORY_H__

#undef GS_USE_ENGINE_MEMORY_SCHEME

// ======================================================== 
// trigger the customized memory management
// ======================================================== 
#define GS_USE_ENGINE_MEMORY_SCHEME 0

// ======================================================== 
// Customize memory allignment here:
// Replace all the codes in micro GS_USE_ENGINE_MEMORY_SCHEME
// in order to use your memory managerment
// ======================================================== 
#if GS_USE_ENGINE_MEMORY_SCHEME
#ifdef _DEBUG
#define EE_MEMORY_DEBUGGER
#endif
#include <efd/MemoryDefines.h>
#include <efd/Asserts.h>
#endif

#undef GS_NEW
#if GS_USE_ENGINE_MEMORY_SCHEME
#define GS_NEW          EE_EXTERNAL_NEW
#define GS_DELETE       EE_EXTERNAL_DELETE 
#define GS_MALLOC       EE_EXTERNAL_MALLOC
#define GS_FREE         EE_EXTERNAL_FREE
#else
#define GS_NEW          new
#define GS_DELETE       delete
#define GS_MALLOC       malloc
#define GS_FREE         free
#endif

#if GS_USE_ENGINE_MEMORY_SCHEME
#define GS_Assert(pred)	EE_ASSERT(pred)
#else
#define GS_Assert(pred)	assert(pred)
#endif

//namespace GameService
//{

//GS_BYTE* Alloc(GS_SIZET size);
//
//GS_VOID Free(GS_VOID* ptr);
//
//GS_BYTE* Realloc( GS_BYTE* ptr, GS_DWORD size, GS_DWORD oldsize );

//} // namespace




#endif // __CUSMEMORY_H__
