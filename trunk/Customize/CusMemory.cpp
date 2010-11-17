#include "stdafx.h"
#include "CusMemory.h"

// ======================================================== 
// Customize memory allignment here:
// Replace all the codes in micro GS_USE_ENGINE_MEMORY_SCHEME
// in order to use your memory managerment
// ======================================================== 
#if GS_USE_ENGINE_MEMORY_SCHEME
#include "BASe/MEMory/MEM.h"

void* operator new(size_t size, GameService::GS_MemoryOP type)
{
	return GameService::Alloc(size);
}

void* operator new[](size_t size, GameService::GS_MemoryOP type)
{
	return GameService::Alloc(size);
}

namespace GameService
{
#if defined(_XBOX) || defined(_XENON)
static GS_BOOL G_AllocAttrInit = 0;
static GS_DWORD G_AllocAttr = -1; 
#endif

GS_BYTE* Alloc(GS_SIZET size)
{
#if GS_USE_ENGINE_MEMORY_SCHEME
	return (GS_BYTE*)MEM_p_Alloc(size);
#else
    // use default malloc
    return new GS_BYTE[size];
#endif
}

GS_VOID Free(GS_VOID* ptr)
{
#if GS_USE_ENGINE_MEMORY_SCHEME
	MEM_Free((GS_BYTE*)ptr);
#else
    // use default free
    if (ptr)
    {
        delete [] ptr;
    }

	return;
#endif
}

GS_BYTE* Realloc( GS_BYTE* ptr, GS_DWORD size, GS_DWORD oldsize )
{
#if GS_USE_ENGINE_MEMORY_SCHEME
	if (0 == size)
	{
		if(ptr)
		{
			Free(ptr);
		}
		return 0;
	}
	else if (0 == ptr)
	{
		return (GS_BYTE*)Alloc(size);
	}

	GS_BYTE* newptr = Alloc(size);
	if (oldsize != 0)
	{
		memmove(newptr, ptr, oldsize);
	}
	Free(ptr);

	return newptr;

    //return (GS_BYTE*)MEM_p_Realloc(ptr,size);

#else
	GS_BYTE* ret = NULL;
    if ((ret=(GS_BYTE*)realloc(ptr,size)) == NULL)
    {
        if (size != 0)
        {
            // not enough memory
            Master::G()->Log("[GameService] - Not enough Memory in Realloc !!! \n");
            return ptr;
        }
        else
        {
            // old memory released
            // return NULL?
        }
    }
    return ret;
#endif
}	
#endif

} // namespace GameService
