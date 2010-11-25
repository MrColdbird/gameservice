#ifndef __CUSMEMORY_H__
#define __CUSMEMORY_H__

#pragma once

void* operator new(size_t size, GameService::GS_MemoryOP type);

void* operator new[](size_t size, GameService::GS_MemoryOP type);

namespace GameService
{

GS_BYTE* Alloc(GS_SIZET size);

GS_VOID Free(GS_VOID* ptr);

GS_BYTE* Realloc( GS_BYTE* ptr, GS_DWORD size, GS_DWORD oldsize );

template <class T>
GS_VOID Delete(T*& ptr)
{
    if (ptr)
    {
        ptr->~T();
        Free(ptr);
    }

    ptr = NULL;
}
template <class T>
GS_VOID DeleteThis(T* ptr)
{
    if (ptr)
    {
        ptr->~T();
        Free(ptr);
    }
}

GS_VOID Assert(GS_BOOL value);

} // namespace


#endif // __CUSMEMORY_H__
