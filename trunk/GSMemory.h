// ======================================================================================
// File         : GSMemory.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:45:30 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef __GAMESERVICE_GSMEMORY_H__
#define __GAMESERVICE_GSMEMORY_H__

#pragma warning( disable : 4127 )

void* operator new(size_t size, GameService::GS_MemoryOP type);
void* operator new[](size_t size, GameService::GS_MemoryOP type);

namespace GameService
{

// ======================================================== 
// New Implementation
// ======================================================== 
extern GS_MemoryOP GSOPType;


// ======================================================== 
// Old Implementation:
// ======================================================== 
#if 0
#define GS_ENGINE_BYTES_ALIGNMENT_4
//#define GS_ENGINE_BYTES_ALIGNMENT_8

#ifdef GS_ENGINE_BYTES_ALIGNMENT_4
#define GS_MEMORY_ALIGNMENT_BYTES 4
#define GS_MEMORY_ALIGNMENT_TYPE GS_UINT
#elif defined(GS_ENGINE_BYTES_ALIGNMENT_8)
#define GS_MEMORY_ALIGNMENT_BYTES 8
#define GS_MEMORY_ALIGNMENT_TYPE GS_UINT64
#endif

// use 2 bytes header to restore array size
template <class T>
T* GS_New()
{
	GS_BYTE* ptr = Alloc(sizeof(T) + GS_MEMORY_ALIGNMENT_BYTES);
	GS_MEMORY_ALIGNMENT_TYPE* header = (GS_MEMORY_ALIGNMENT_TYPE*)ptr;
	*header = 1;

	ptr += GS_MEMORY_ALIGNMENT_BYTES;
	return new(ptr) T;
}

// with class copy constructor parameter
template <class T>
T* GS_New(T self)
{
	GS_BYTE* ptr = Alloc(sizeof(T) + GS_MEMORY_ALIGNMENT_BYTES);
	GS_MEMORY_ALIGNMENT_TYPE* header = (GS_MEMORY_ALIGNMENT_TYPE*)ptr;
	*header = 1;

	ptr += GS_MEMORY_ALIGNMENT_BYTES;
	return new(ptr) T(self);
}

// with 1 class constructor parameter
template <class T, class P1>
T* GS_New_Param(P1 param)
{
	GS_BYTE* ptr = Alloc(sizeof(T) + GS_MEMORY_ALIGNMENT_BYTES);
	GS_MEMORY_ALIGNMENT_TYPE* header = (GS_MEMORY_ALIGNMENT_TYPE*)ptr;
	*header = 1;

	ptr += GS_MEMORY_ALIGNMENT_BYTES;
	return new(ptr) T(param);
}
// with 2 class constructor parameter
template <class T, class P1, class P2>
T* GS_New_Param(P1 param1, P2 param2)
{
	GS_BYTE* ptr = Alloc(sizeof(T) + GS_MEMORY_ALIGNMENT_BYTES);
	GS_MEMORY_ALIGNMENT_TYPE* header = (GS_MEMORY_ALIGNMENT_TYPE*)ptr;
	*header = 1;

	ptr += GS_MEMORY_ALIGNMENT_BYTES;
	return new(ptr) T(param1,param2);
}

template <class T>
T* GS_NewArray(GS_UINT arraySize)
{
	GS_BYTE* ptr = Alloc(sizeof(T)*arraySize + GS_MEMORY_ALIGNMENT_BYTES);
	GS_MEMORY_ALIGNMENT_TYPE* header = (GS_MEMORY_ALIGNMENT_TYPE*)ptr;
	*header = arraySize;

	ptr += GS_MEMORY_ALIGNMENT_BYTES;
	return new(ptr) T[arraySize];
}

template <class T>
GS_VOID GS_Delete(T* ptr)
{
	if (ptr)
	{
		GS_BYTE* offset = (GS_BYTE*)ptr;
		offset -= GS_MEMORY_ALIGNMENT_BYTES;
		GS_MEMORY_ALIGNMENT_TYPE* header = (GS_MEMORY_ALIGNMENT_TYPE*)offset;
		T* content = (T*)ptr;
		for (GS_MEMORY_ALIGNMENT_TYPE i=0; i<*header; i++,content++)
		{
			if (content)
			{
				content->~T();
			}
		}
		Free(offset);
	}
}
#endif //0

} // namespace GameService

#endif // __GAMESERVICE_GSMEMORY_H__
