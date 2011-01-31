// ======================================================================================
// File         : Array.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:45:30 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef __GAMESERVICE_ARRAY_H__
#define __GAMESERVICE_ARRAY_H__

#include "Customize/CusMemory.h"

// ======================================================== 
// Trigger 
// if use vector choose std or other
// ======================================================== 
#undef GS_USE_STD_VECTOR
#define GS_USE_STD_VECTOR

// define vector: 
#ifdef GS_USE_STD_VECTOR
// use std::vector 
#include <vector>
#if defined(_PS3)
//typename std::vector<T>;
#define GS_VECTOR typename std::vector<T>
#else
#define GS_VECTOR std::vector<T>
#endif
#else
// or engine vector
#include <efd/Foundation.h>
#if defined(_PS3)
#define GS_VECTOR typename efd::vector<T>
#else
#define GS_VECTOR efd::vector<T>
#endif
#endif

namespace GameService
{

#if defined(_XBOX) || defined(_XENON)
#define _FORCE_INLINE //__forceinline
#elif defined(_PS3)
#define _FORCE_INLINE inline
#else
#define _FORCE_INLINE __forceinline
#endif

enum {INDEX_NONE	= -1         };

template< class T > class TArray
{
public:
    GS_VECTOR& GetData() 
    {
        return m_Vector;
    }

	_FORCE_INLINE TArray()
	{ 
        m_Vector.clear();
    }

	_FORCE_INLINE TArray( GS_INT InNum )
	{
		m_Vector.reserve(InNum);
	}

	_FORCE_INLINE TArray(const TArray& Other) : TArray()
	{
		Copy(Other);
	}

	virtual ~TArray()
	{
        m_Vector.clear();
	}

	// common functions
	_FORCE_INLINE GS_DWORD GetTypeSize() const
	{
		return sizeof(T);
	}

	_FORCE_INLINE T& operator()( GS_UINT i )
	{
		GS_Assert((i>=0));
		GS_Assert(i<m_Vector.size()||m_Vector.size()==0); // m_ArrayNum==0 is workaround for &MyArray(0) abuse
		return m_Vector[i];
	}

	_FORCE_INLINE const T& operator()( GS_UINT i ) const
	{
		GS_Assert(i>=0);
		GS_Assert(i<m_Vector.size()||m_Vector.size()==0); // m_ArrayNum==0 is workaround for &MyArray(0) abuse
		return m_Vector[i];
	}

	_FORCE_INLINE T& Last( GS_INT c=0 )
	{
		GS_Assert(m_Vector.size()>0);
		GS_Assert(c<m_Vector.size());
		return m_Vector.back();
	}
	_FORCE_INLINE const T& Last( GS_INT c=0 ) const
	{
		GS_Assert(m_Vector.size()>0);
		GS_Assert(c<m_Vector.size());
		return m_Vector.back();
	}

	_FORCE_INLINE GS_BOOL FindItem( const T& Item, GS_INT& Index ) const
	{
        GS_VECTOR::iterator it;
		for( it=m_Vector.begin(); it != m_Vector.end(); it++ )
			if( *it == Item )
				return 1;
		return 0;
	}
	_FORCE_INLINE GS_INT FindItemIndex( const T& Item ) const
	{
        GS_VECTOR::iterator it;
		GS_INT index = 0;
		for( it=m_Vector.begin(); it != m_Vector.end(); it++,index++ )
			if( *it == Item )
				return index;
		return INDEX_NONE;
	}
	_FORCE_INLINE GS_BOOL ContainsItem( const T& Item ) const
	{
		return ( FindItemIndex(Item) != INDEX_NONE );
	}

	GS_BOOL operator==(const TArray<T>& OtherArray) const
	{
        return (m_Vector == OtherArray.m_Vector);
	}
	GS_BOOL operator!=(const TArray<T>& OtherArray) const
	{
        return (m_Vector == OtherArray.m_Vector);
	}
	TArray<T>& operator+=( const TArray<T>& Other )
	{
		Append( Other );
		return *this;
	}
	TArray<T>& operator=( const TArray<T>& Other )
	{
		Copy( Other );
		return *this;
	}

	// Add, Insert, Remove, Empty interface.
	_FORCE_INLINE GS_BOOL IsValidIndex( GS_INT i ) const
	{
		return i>=0 && i<m_Vector.size();
	}

	_FORCE_INLINE GS_UINT Num() const
	{
		GS_Assert(m_Vector.size()>=0);
		return m_Vector.size();
	}

	void Shrink( GS_INT ElementSize )
	{
        GS_Assert(m_Vector.size() > 0);
        if (m_Vector.size() != m_Vector.capacity())
        {
            m_Vector.resize(m_Vector.size());
        }
	}
	void Remove( GS_UINT Index, GS_UINT Count=1 )
	{
		GS_Assert(Index>=0);
		GS_Assert(Index<=m_Vector.size());
		GS_Assert(Index+Count<=m_Vector.size());
        for (GS_UINT i=0;i<Count; i++)
        {
            m_Vector.erase(m_Vector.begin()+Index);
        }
	}

	void Empty( GS_INT Slack=0 )
	{
        m_Vector.clear();
        GS_Assert(m_Vector.empty());
	}

	GS_BOOL IsEmpty()
	{
		return (m_Vector.empty());
	}

	void Append(const TArray<T>& Source)
	{
        for (GS_INT i=0;i<Source.Num();i++)
        {
            m_Vector.push_back(Source(i));
        }
	}

	GS_INT AddItem( const T& Item )
	{
        m_Vector.push_back(Item);
        return m_Vector.size()-1;
	}

	GS_INT AddUniqueItem( const T& Item )
	{
        GS_INT index=0;
        GS_VECTOR::iterator it;
		for( it=m_Vector.begin(); it!=m_Vector.end(); it++,index++ )
			if( *it==Item )
				return index;

		return AddItem( Item );
	}
	void Reserve(GS_INT Number)
	{
        m_Vector.reserve(Number);
	}
	GS_INT RemoveItem( const T& Item )
	{
		GS_INT OriginalNum=m_Vector.size();
        GS_INT index=0;
		GS_VECTOR::iterator it;
		for( it=m_Vector.begin(); it!=m_Vector.end(); it++,index++ )
			if( *it==Item )
				m_Vector.erase(m_Vector.begin()+index);

        return OriginalNum - m_Vector.size();
	}

protected:
	void Copy(const TArray<T>& Source)
	{
		m_Vector = Source.m_Vector;
	}

private:
	std::vector<T>   m_Vector;
};

} // namespace GameService

#endif // __GAMESERVICE_ARRAY_H__
