// ======================================================================================
// File         : Array.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:45:30 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef __GAMESERVICE_ARRAY_H__
#define __GAMESERVICE_ARRAY_H__

#include "Customize/CusMemory.h"

namespace GameService
{

#if defined(_XBOX)
#define _FORCE_INLINE //__forceinline
#elif defined(_PS3)
#define _FORCE_INLINE inline
#else
#define _FORCE_INLINE __forceinline
#endif

// Copied from unreal:
enum {INDEX_NONE	= -1         };

/**
 * Base type information for atomic types which pass by value.
 */
template<typename T>
class TTypeInfoAtomicBase
{
public:
	typedef T ConstInitType;
	enum { NeedsConstructor = 0	};
	enum { NeedsDestructor = 0	};
};

/**
 * Base type information for constructed types which pass by reference.
 */
template<typename T>
class TTypeInfoConstructedBase
{
public:
	typedef const T& ConstInitType;
	enum { NeedsConstructor = 1	};
	enum { NeedsDestructor = 1	};
};

/**
 * The default behaviour is for types to behave as constructed types.
 */
template<typename T>
class TTypeInfo : public TTypeInfoConstructedBase<T>{};

/**
 * C-style pointers require no construction.
 */
template<typename T>
class TTypeInfo<T*>: public TTypeInfoAtomicBase<T*> {};

template <> class TTypeInfo<GS_BYTE>		: public TTypeInfoAtomicBase<GS_BYTE>	{};
template <> class TTypeInfo<GS_INT>		: public TTypeInfoAtomicBase<GS_INT> {};
#if !defined(_PS3)
template <> class TTypeInfo<GS_DWORD>		: public TTypeInfoAtomicBase<GS_DWORD> {};
#endif
template <> class TTypeInfo<GS_WORD>		: public TTypeInfoAtomicBase<GS_WORD> {};
template <> class TTypeInfo<GS_FLOAT>		: public TTypeInfoAtomicBase<GS_FLOAT> {};
template <> class TTypeInfo<GS_DOUBLE>		: public TTypeInfoAtomicBase<GS_DOUBLE> {};
// End Copy


// Dynamic Template Array:
template< class T > class TArray
{
public:
	_FORCE_INLINE GS_BYTE* GetData()
	{
		return m_Data;
	}
	_FORCE_INLINE const GS_BYTE* GetData() const
	{
		return m_Data;
	}

	enum {ResizeFactor1=3, ResizeFactor2=8, ResizeFactor3=1};
	typedef T ElementType;

	_FORCE_INLINE TArray()
	: m_Data( NULL )
	, m_ArrayNum( 0 )
	, m_ArrayMax( 0 )
	{}

	_FORCE_INLINE TArray( GS_INT InNum )
	: m_Data( NULL )
	, m_ArrayNum( InNum )
	, m_ArrayMax( InNum )
	{
		ArrayRealloc(sizeof(T));
	}

	_FORCE_INLINE TArray(const TArray& Other) : TArray()
	{
		Copy(Other);
	}

	virtual ~TArray()
	{
		assert(m_ArrayNum>=0);
		assert(m_ArrayMax>=m_ArrayNum);
		Remove( 0, m_ArrayNum );

		if (m_Data)
		{
			Free(m_Data);
		}

		m_Data = NULL;
		m_ArrayNum = m_ArrayMax = 0;
	}

	// Array Realloc function
	void ArrayRealloc( GS_DWORD size, GS_DWORD oldNum=0 )
	{
		if (m_Data || m_ArrayMax)
		{
			m_Data = Realloc(m_Data, m_ArrayMax*size, oldNum*size);
		}
	}

	// common functions
	_FORCE_INLINE T* GetTypedData()
	{
		return (T*)m_Data;
	}

	_FORCE_INLINE const T* GetTypedData() const
	{
		return (T*)m_Data;
	}

	_FORCE_INLINE GS_DWORD GetTypeSize() const
	{
		return sizeof(T);
	}

	_FORCE_INLINE T& operator()( GS_INT i )
	{
		assert(i>=0);
		assert(i<m_ArrayNum||m_ArrayNum==0); // m_ArrayNum==0 is workaround for &MyArray(0) abuse
		assert(m_ArrayMax>=m_ArrayNum);
		return ((T*)m_Data)[i];
	}

	_FORCE_INLINE const T& operator()( GS_INT i ) const
	{
		assert(i>=0);
		assert(i<m_ArrayNum||m_ArrayNum==0); // m_ArrayNum==0 is workaround for &MyArray(0) abuse
		assert(m_ArrayMax>=m_ArrayNum);
		return ((T*)m_Data)[i];
	}

	_FORCE_INLINE T& Top()
	{
		return Last();
	}
	_FORCE_INLINE const T& Top() const
	{
		return Last();
	}
	_FORCE_INLINE T& Last( GS_INT c=0 )
	{
		assert(m_Data);
		assert(c<m_ArrayNum);
		assert(m_ArrayMax>=m_ArrayNum);
		return ((T*)m_Data)[m_ArrayNum-c-1];
	}
	_FORCE_INLINE const T& Last( GS_INT c=0 ) const
	{
		assert(m_Data);
		assert(c<m_ArrayNum);
		assert(m_ArrayMax>=m_ArrayNum);
		return ((T*)m_Data)[m_ArrayNum-c-1];
	}

	_FORCE_INLINE GS_BOOL FindItem( const T& Item, GS_INT& Index ) const
	{
		for( Index=0; Index<m_ArrayNum; Index++ )
			if( (*this)(Index)==Item )
				return 1;
		return 0;
	}
	_FORCE_INLINE GS_INT FindItemIndex( const T& Item ) const
	{
		for( GS_INT Index=0; Index<m_ArrayNum; Index++ )
			if( (*this)(Index)==Item )
				return Index;
		return INDEX_NONE;
	}
	_FORCE_INLINE GS_BOOL ContainsItem( const T& Item ) const
	{
		return ( FindItemIndex(Item) != INDEX_NONE );
	}

	GS_BOOL operator==(const TArray<T>& OtherArray) const
	{
		if(Num() != OtherArray.Num())
			return 0;
		for(GS_INT Index = 0;Index < Num();Index++)
		{
			if(!((*this)(Index) == OtherArray(Index)))
				return 0;
		}
		return 1;
	}
	GS_BOOL operator!=(const TArray<T>& OtherArray) const
	{
		if(Num() != OtherArray.Num())
			return 1;
		for(GS_INT Index = 0;Index < Num();Index++)
		{
			if(!((*this)(Index) == OtherArray(Index)))
				return 1;
		}
		return 0;
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
		return i>=0 && i<m_ArrayNum;
	}

	_FORCE_INLINE GS_INT Num() const
	{
		assert(m_ArrayNum>=0);
		assert(m_ArrayMax>=m_ArrayNum);
		return m_ArrayNum;
	}

	void InsertZeroed( GS_INT Index, GS_INT Count, GS_INT ElementSize )
	{
		Insert( Index, Count, ElementSize );
		memset( (GS_BYTE*)m_Data+Index*ElementSize, 0, Count*ElementSize );
	}

	void Insert( GS_INT Index, GS_INT Count, GS_INT ElementSize )
	{
		assert(Count>=0);
		assert(m_ArrayNum>=0);
		assert(m_ArrayMax>=m_ArrayNum);
		assert(Index>=0);
		assert(Index<=m_ArrayNum);

		GS_INT OldNum = m_ArrayNum;
		if( (m_ArrayNum+=Count)>m_ArrayMax )
		{
			GS_DWORD old_num = m_ArrayMax;
			m_ArrayMax = m_ArrayNum + ResizeFactor1*m_ArrayNum/ResizeFactor2 + ResizeFactor3;
			ArrayRealloc( ElementSize, old_num );
		}
		memmove
		(
			(GS_BYTE*)m_Data + (Index+Count )*ElementSize,
			(GS_BYTE*)m_Data + (Index       )*ElementSize,
						  (OldNum-Index)*ElementSize
		);
	}
	GS_INT Add( GS_INT Count, GS_INT ElementSize )
	{
		assert(Count>=0);
		assert(m_ArrayNum>=0);
		assert(m_ArrayMax>=m_ArrayNum);

		const GS_INT Index = m_ArrayNum;
		if( (m_ArrayNum+=Count)>m_ArrayMax )
		{
			GS_DWORD old_num = m_ArrayMax;
			m_ArrayMax = m_ArrayNum + ResizeFactor1*m_ArrayNum/ResizeFactor2 + ResizeFactor3;
			ArrayRealloc( ElementSize, old_num );
		}

		return Index;
	}
	GS_INT AddZeroed( GS_INT ElementSize, GS_INT n=1 )
	{
		GS_INT Index = Add( n, ElementSize );
		memset( (GS_BYTE*)m_Data+Index*ElementSize, 0, n*ElementSize );
		return Index;
	}
	void Shrink( GS_INT ElementSize )
	{
		assert(m_ArrayNum>=0);
		assert(m_ArrayMax>=m_ArrayNum);
		if( m_ArrayMax != m_ArrayNum )
		{
			GS_DWORD old_num = m_ArrayMax;
			m_ArrayMax = m_ArrayNum;
			if (old_num > m_ArrayMax)
			{
				old_num = m_ArrayMax;
			}
			ArrayRealloc( ElementSize, old_num );
		}
	}
	void Remove( GS_INT Index, GS_INT Count=1 )
	{
		assert(Index>=0);
		assert(Index<=m_ArrayNum);
		assert(Index+Count<=m_ArrayNum);
		if( TTypeInfo<T>::NeedsDestructor )
			for( GS_INT i=Index; i<Index+Count; i++ )
				(&(*this)(i))->~T();

		// Base array impl
		if( Count )
		{
			memmove
			(
				(GS_BYTE*)m_Data + (Index      ) * sizeof(T),
				(GS_BYTE*)m_Data + (Index+Count) * sizeof(T),
				(m_ArrayNum - Index - Count ) * sizeof(T)
			);
			m_ArrayNum -= Count;
		}
		assert(m_ArrayNum>=0);
		assert(m_ArrayMax>=m_ArrayNum);
	}

	void Empty( GS_INT Slack=0 )
	{
		if( TTypeInfo<T>::NeedsDestructor )
			for( GS_INT i=0; i<m_ArrayNum; i++ )
				(&(*this)(i))->~T();

		// Base array impl
		m_ArrayNum = 0;
		// only reallocate if we need to, I don't trust realloc to the same size to work
		if (m_ArrayMax != Slack)
		{
			m_ArrayMax = Slack;
			ArrayRealloc( sizeof(T) );
		}	
	}

	void Append(const TArray<T>& Source)
	{
		// Do nothing if the source and target match, or the source is empty.
		if ( this != &Source && Source.Num() > 0 )
		{
			// Allocate memory for the new(GSOPType) elements.
			Reserve( m_ArrayNum + Source.m_ArrayNum );

			if ( TTypeInfo<T>::NeedsConstructor )
			{
				// Construct each element.
				for ( GS_INT Index = 0 ; Index < Source.m_ArrayNum ; ++Index )
				{
					// @todo DB: optimize by calling placement new(GSOPType) directly, to avoid
					// @todo DB: the superfluous checks in TArray placement new(GSOPType).

					//::new(GSOPType)(*this) T(Source(Index));
					const GS_INT _index = Add(1, sizeof(T));
					((T*)m_Data)[_index] = Source(Index);

				}
			}
			else
			{
				// Do a bulk copy.
				memcpy( (GS_BYTE*)m_Data + m_ArrayNum * sizeof(T), Source.m_Data, sizeof(T) * Source.m_ArrayNum );
				m_ArrayNum += Source.m_ArrayNum;
			}
		}
	}

	GS_INT AddItem( const T& Item )
	{
		//::new(GSOPType)(*this) T(Item);
		const GS_INT index = Add(1, sizeof(T));
		((T*)m_Data)[index] = Item;
		return Num() - 1;
	}

	GS_INT AddUniqueItem( const T& Item )
	{
		for( GS_INT Index=0; Index<m_ArrayNum; Index++ )
			if( (*this)(Index)==Item )
				return Index;
		return AddItem( Item );
	}
	void Reserve(GS_INT Number)
	{
		if (Number > m_ArrayMax)
		{
			GS_DWORD old_num = m_ArrayMax;
			m_ArrayMax = Number;
			ArrayRealloc( sizeof(T), old_num );
		}
	}
	GS_INT RemoveItem( const T& Item )
	{
		GS_INT OriginalNum=m_ArrayNum;
		for( GS_INT Index=0; Index<m_ArrayNum; Index++ )
			if( (*this)(Index)==Item )
				Remove( Index-- );
		return OriginalNum - m_ArrayNum;
	}

	// Iterator.
	class TIterator
	{
	public:
		TIterator( TArray<T>& InArray ) : m_Array(InArray), m_Index(-1) 
		{ ++*this; }
		void operator++()      
		{ ++m_Index; }
		void RemoveCurrent()   
		{ m_Array.Remove(m_Index--); }
		GS_INT GetIndex()   const 
		{ return m_Index; }
		operator GS_BOOL() const 
		{ return m_Array.IsValidIndex(m_Index); }
		T& operator*()   const 
		{ return m_Array(m_Index); }
		T* operator->()  const 
		{ return &m_Array(m_Index); }
		T& GetCurrent()  const 
		{ return m_Array( m_Index ); }
		T& GetPrev()     const 
		{ return m_Array( m_Index ? m_Index-1 : m_Array.Num()-1 ); }
		T& GetNext()     const 
		{ return m_Array( m_Index<m_Array.Num()-1 ? m_Index+1 : 0 ); }

	private:
		TArray<T>& m_Array;
		GS_INT m_Index;
	};

protected:
	void Copy(const TArray<T>& Source)
	{
		if (this != &Source)
		{
			// Just empty our array if there is nothing to copy
			if (Source.m_ArrayNum > 0)
			{
				// Presize the array so there are no extra allocs/memcpys
				Empty(Source.m_ArrayNum);
				// Determine whether we need per element construction or bulk
				// copy is fine
				if (TTypeInfo<T>::NeedsConstructor)
				{
					// Use the inplace new(GSOPType) to copy the element to an array element
					for (GS_INT Index = 0; Index < Source.m_ArrayNum; Index++)
					{
						//::new(GSOPType)(*this) T(Source(Index));
						const GS_INT _index = Add(1, sizeof(T));
						((T*)m_Data)[_index] = Source(Index);
					}
				}
				else
				{
					// Use the much faster path for types that allow it
					memcpy(m_Data,&Source(0),sizeof(T) * Source.m_ArrayNum);
					m_ArrayNum = Source.m_ArrayNum;
				}
			}
			else
			{
				Empty();
			}
		}
	}

private:
	GS_BYTE* m_Data;
	GS_INT	  m_ArrayNum;
	GS_INT	  m_ArrayMax;

};

// not used since compile error...... LiChen
//template <class T> 
//void* operator new(GSOPType)( size_t Size, TArray<T>& Array )
//{
//	const GS_INT Index = Array.Add(1, sizeof(T));
//	return &Array(Index);
//}
//template <class T> 
//void* operator new(GSOPType)( size_t Size, TArray<T>& Array, GS_INT Index )
//{
//	Array.Insert(Index, 1, sizeof(T));
//	return &Array(Index);
//}

} // namespace GameService

#endif // __GAMESERVICE_ARRAY_H__
