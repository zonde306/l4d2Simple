#pragma once
#include "utlmemory.h"
#include "utlblockmemory.h"
#include <corecrt_malloc.h>
#include <cstring>

#define FOR_EACH_VEC( vecName, iteratorName ) \
	for ( int iteratorName = 0; iteratorName < (vecName).Count(); iteratorName++ )
#define FOR_EACH_VEC_BACK( vecName, iteratorName ) \
	for ( int iteratorName = (vecName).Count()-1; iteratorName >= 0; iteratorName-- )

class IUniformRandomStream;
template <class T>
inline void Construct(T* pMemory)
{
	::new(pMemory) T;
}

template <class T>
inline void CopyConstruct(T* pMemory, T const& src)
{
	::new(pMemory) T(src);
}

template <class T>
inline void Destruct(T* pMemory)
{
	pMemory->~T();

#ifdef _DEBUG_OUTPUT
	memset(pMemory, 0xDD, sizeof(T));
#endif
}


//-----------------------------------------------------------------------------
// The CUtlVector class:
// A growable array class which doubles in size by default.
// It will always keep all elements consecutive in memory, and may move the
// elements around in memory (via a PvRealloc) when elements are inserted or
// removed. Clients should therefore refer to the elements of the vector
// by index (they should *never* maintain pointers to elements in the vector).
//-----------------------------------------------------------------------------
template< class T, class A = CUtlMemory<T> >
class CUtlVector
{
	typedef A CAllocator;
public:
	typedef T ElemType_t;
	typedef T* iterator;
	typedef const T* const_iterator;

	// constructor, destructor
	explicit CUtlVector(int growSize = 0, int initSize = 0);
	explicit CUtlVector(T* pMemory, int allocationCount, int numElements = 0);
	~CUtlVector();

	// Copy the array.
	CUtlVector<T, A>& operator=(const CUtlVector<T, A> &other);

	// element access
	T& operator[](int i);
	const T& operator[](int i) const;
	T& Element(int i);
	const T& Element(int i) const;
	T& Head();
	const T& Head() const;
	T& Tail();
	const T& Tail() const;

	// STL compatible member functions. These allow easier use of std::sort
	// and they are forward compatible with the C++ 11 range-based for loops.
	iterator begin() { return Base(); }
	const_iterator begin() const { return Base(); }
	iterator end() { return Base() + Count(); }
	const_iterator end() const { return Base() + Count(); }

	// Gets the base address (can change when adding elements!)
	T* Base() { return m_Memory.Base(); }
	const T* Base() const { return m_Memory.Base(); }

	// Returns the number of elements in the vector
	// SIZE IS DEPRECATED!
	int Count() const;
	int Size() const;	// don't use me!

						/// are there no elements? For compatibility with lists.
	inline bool IsEmpty(void) const
	{
		return (Count() == 0);
	}

	// Is element index valid?
	bool IsValidIndex(int i) const;
	static int InvalidIndex();

	// Adds an element, uses default constructor
	int AddToHead();
	int AddToTail();
	int InsertBefore(int elem);
	int InsertAfter(int elem);

	// Adds an element, uses copy constructor
	int AddToHead(const T& src);
	int AddToTail(const T& src);
	int InsertBefore(int elem, const T& src);
	int InsertAfter(int elem, const T& src);

	// Adds multiple elements, uses default constructor
	int AddMultipleToHead(int num);
	int AddMultipleToTail(int num, const T *pToCopy = NULL);
	int InsertMultipleBefore(int elem, int num, const T *pToCopy = NULL);	// If pToCopy is set, then it's an array of length 'num' and
	int InsertMultipleAfter(int elem, int num);

	// Calls RemoveAll() then AddMultipleToTail.
	void SetSize(int size);
	void SetCount(int count);
	void SetCountNonDestructively(int count); //sets count by adding or removing elements to tail TODO: This should probably be the default behavior for SetCount

											  // Calls SetSize and copies each element.
	void CopyArray(const T *pArray, int size);

	// Fast swap
	void Swap(CUtlVector< T, A > &vec);

	// Add the specified array to the tail.
	int AddVectorToTail(CUtlVector<T, A> const &src);

	// Finds an element (element needs operator== defined)
	int Find(const T& src) const;

	bool HasElement(const T& src) const;

	// Makes sure we have enough memory allocated to store a requested # of elements
	void EnsureCapacity(int num);

	// Makes sure we have at least this many elements
	void EnsureCount(int num);

	// Element removal
	void FastRemove(int elem);	// doesn't preserve order
	void Remove(int elem);		// preserves order, shifts elements
	bool FindAndRemove(const T& src);	// removes first occurrence of src, preserves order, shifts elements
	bool FindAndFastRemove(const T& src);	// removes first occurrence of src, doesn't preserve order
	void RemoveMultiple(int elem, int num);	// preserves order, shifts elements
	void RemoveMultipleFromHead(int num); // removes num elements from tail
	void RemoveMultipleFromTail(int num); // removes num elements from tail
	void RemoveAll();				// doesn't deallocate memory

									// Memory deallocation
	void Purge();

	// Purges the list and calls delete on each element in it.
	void PurgeAndDeleteElements();

	// Compacts the vector to the number of elements actually in use 
	void Compact();

	// Set the size by which it grows when it needs to allocate more memory.
	void SetGrowSize(int size) { m_Memory.SetGrowSize(size); }

	int NumAllocated() const;	// Only use this if you really know what you're doing!

	void Sort(int(__cdecl *pfnCompare)(const T *, const T *));

#ifdef DBGFLAG_VALIDATE
	void Validate(CValidator &validator, char *pchName);		// Validate our internal structures
#endif // DBGFLAG_VALIDATE

protected:
	// Grows the vector
	void GrowVector(int num = 1);

	// Shifts elements....
	void ShiftElementsRight(int elem, int num = 1);
	void ShiftElementsLeft(int elem, int num = 1);

	CAllocator m_Memory;
	int m_Size;

#ifndef _X360
	// For easier access to the elements through the debugger
	// it's in release builds so this can be used in libraries correctly
	T *m_pElements;

	inline void ResetDbgInfo()
	{
		m_pElements = Base();
	}
#else
	inline void ResetDbgInfo() {}
#endif

private:
	// Can't copy this unless we explicitly do it!
	// Use CCopyableUtlVector<T> to get this functionality
	CUtlVector(CUtlVector const& vec);
};


// this is kind of ugly, but until C++ gets templatized typedefs in C++0x, it's our only choice
template < class T >
class CUtlBlockVector : public CUtlVector< T, CUtlBlockMemory< T, int > >
{
public:
	explicit CUtlBlockVector(int growSize = 0, int initSize = 0)
		: CUtlVector< T, CUtlBlockMemory< T, int > >(growSize, initSize) {}

private:
	// Private and unimplemented because iterator semantics are not currently supported
	// on CUtlBlockVector, due to its non-contiguous allocations.
	// typename is require to disambiguate iterator as a type versus other possibilities.
	typedef CUtlVector< T, CUtlBlockMemory< T, int > > Base;
	typename Base::iterator begin();
	typename Base::const_iterator begin() const;
	typename Base::iterator end();
	typename Base::const_iterator end() const;
};

//-----------------------------------------------------------------------------
// The CUtlVectorFixed class:
// A array class with a fixed allocation scheme
//-----------------------------------------------------------------------------

template< class BASE_UTLVECTOR, class MUTEX_TYPE = CThreadFastMutex >
class CUtlVectorMT : public BASE_UTLVECTOR, public MUTEX_TYPE
{
	typedef BASE_UTLVECTOR BaseClass;
public:
	MUTEX_TYPE Mutex_t;

	// constructor, destructor
	explicit CUtlVectorMT(int growSize = 0, int initSize = 0) : BaseClass(growSize, initSize) {}
	explicit CUtlVectorMT(typename BaseClass::ElemType_t* pMemory, int numElements) : BaseClass(pMemory, numElements) {}
};


//-----------------------------------------------------------------------------
// The CUtlVectorFixed class:
// A array class with a fixed allocation scheme
//-----------------------------------------------------------------------------
template< class T, size_t MAX_SIZE >
class CUtlVectorFixed : public CUtlVector< T, CUtlMemoryFixed<T, MAX_SIZE > >
{
	typedef CUtlVector< T, CUtlMemoryFixed<T, MAX_SIZE > > BaseClass;
public:

	// constructor, destructor
	explicit CUtlVectorFixed(int growSize = 0, int initSize = 0) : BaseClass(growSize, initSize) {}
	explicit CUtlVectorFixed(T* pMemory, int numElements) : BaseClass(pMemory, numElements) {}
};


//-----------------------------------------------------------------------------
// The CUtlVectorFixedGrowable class:
// A array class with a fixed allocation scheme backed by a dynamic one
//-----------------------------------------------------------------------------
template< class T, size_t MAX_SIZE >
class CUtlVectorFixedGrowable : public CUtlVector< T, CUtlMemoryFixedGrowable<T, MAX_SIZE > >
{
	typedef CUtlVector< T, CUtlMemoryFixedGrowable<T, MAX_SIZE > > BaseClass;

public:
	// constructor, destructor
	explicit CUtlVectorFixedGrowable(int growSize = 0) : BaseClass(growSize, MAX_SIZE) {}
};


//-----------------------------------------------------------------------------
// The CUtlVectorConservative class:
// A array class with a conservative allocation scheme
//-----------------------------------------------------------------------------
template< class T >
class CUtlVectorConservative : public CUtlVector< T, CUtlMemoryConservative<T> >
{
	typedef CUtlVector< T, CUtlMemoryConservative<T> > BaseClass;
public:

	// constructor, destructor
	explicit CUtlVectorConservative(int growSize = 0, int initSize = 0) : BaseClass(growSize, initSize) {}
	explicit CUtlVectorConservative(T* pMemory, int numElements) : BaseClass(pMemory, numElements) {}
};


//-----------------------------------------------------------------------------
// The CUtlVectorUltra Conservative class:
// A array class with a very conservative allocation scheme, with customizable allocator
// Especialy useful if you have a lot of vectors that are sparse, or if you're
// carefully packing holders of vectors
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable : 4200) // warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable : 4815 ) // warning C4815: 'staticData' : zero-sized array in stack object will have no elements
#define sizeMalloc(p) (*(((unsigned int *)p)-1) & ~(0x01|0x02))

class CUtlVectorUltraConservativeAllocator
{
public:
	static void *Alloc(size_t nSize)
	{
		return malloc(nSize);
	}

	static void *Realloc(void *pMem, size_t nSize)
	{
		return realloc(pMem, nSize);
	}

	static void Free(void *pMem)
	{
		free(pMem);
	}

	static size_t GetSize(void *pMem)
	{
		return sizeMalloc(pMem);
	}

};

template <typename T, typename A = CUtlVectorUltraConservativeAllocator >
class CUtlVectorUltraConservative : private A
{
public:
	CUtlVectorUltraConservative()
	{
		m_pData = StaticData();
	}

	~CUtlVectorUltraConservative()
	{
		RemoveAll();
	}

	int Count() const
	{
		return m_pData->m_Size;
	}

	static int InvalidIndex()
	{
		return -1;
	}

	inline bool IsValidIndex(int i) const
	{
		return (i >= 0) && (i < Count());
	}

	T& operator[](int i)
	{
		Assert(IsValidIndex(i));
		return m_pData->m_Elements[i];
	}

	const T& operator[](int i) const
	{
		Assert(IsValidIndex(i));
		return m_pData->m_Elements[i];
	}

	T& Element(int i)
	{
		Assert(IsValidIndex(i));
		return m_pData->m_Elements[i];
	}

	const T& Element(int i) const
	{
		Assert(IsValidIndex(i));
		return m_pData->m_Elements[i];
	}

	void EnsureCapacity(int num)
	{
		int nCurCount = Count();
		if (num <= nCurCount)
		{
			return;
		}
		if (m_pData == StaticData())
		{
			m_pData = (Data_t *)A::Alloc(sizeof(int) + (num * sizeof(T)));
			m_pData->m_Size = 0;
		}
		else
		{
			int nNeeded = sizeof(int) + (num * sizeof(T));
			int nHave = A::GetSize(m_pData);
			if (nNeeded > nHave)
			{
				m_pData = (Data_t *)A::Realloc(m_pData, nNeeded);
			}
		}
	}

	int AddToTail(const T& src)
	{
		int iNew = Count();
		EnsureCapacity(Count() + 1);
		m_pData->m_Elements[iNew] = src;
		m_pData->m_Size++;
		return iNew;
	}

	void RemoveAll()
	{
		if (Count())
		{
			for (int i = m_pData->m_Size; --i >= 0; )
			{
				Destruct(&m_pData->m_Elements[i]);
			}
		}
		if (m_pData != StaticData())
		{
			A::Free(m_pData);
			m_pData = StaticData();

		}
	}

	void PurgeAndDeleteElements()
	{
		if (m_pData != StaticData())
		{
			for (int i = 0; i < m_pData->m_Size; i++)
			{
				delete Element(i);
			}
			RemoveAll();
		}
	}

	void FastRemove(int elem)
	{
		Assert(IsValidIndex(elem));

		Destruct(&Element(elem));
		if (Count() > 0)
		{
			if (elem != m_pData->m_Size - 1)
				memcpy(&Element(elem), &Element(m_pData->m_Size - 1), sizeof(T));
			--m_pData->m_Size;
		}
		if (!m_pData->m_Size)
		{
			A::Free(m_pData);
			m_pData = StaticData();
		}
	}

	void Remove(int elem)
	{
		Destruct(&Element(elem));
		ShiftElementsLeft(elem);
		--m_pData->m_Size;
		if (!m_pData->m_Size)
		{
			A::Free(m_pData);
			m_pData = StaticData();
		}
	}

	int Find(const T& src) const
	{
		int nCount = Count();
		for (int i = 0; i < nCount; ++i)
		{
			if (Element(i) == src)
				return i;
		}
		return -1;
	}

	bool FindAndRemove(const T& src)
	{
		int elem = Find(src);
		if (elem != -1)
		{
			Remove(elem);
			return true;
		}
		return false;
	}


	bool FindAndFastRemove(const T& src)
	{
		int elem = Find(src);
		if (elem != -1)
		{
			FastRemove(elem);
			return true;
		}
		return false;
	}

	struct Data_t
	{
		int m_Size;
		T m_Elements[0];
	};

	Data_t *m_pData;
private:
	void ShiftElementsLeft(int elem, int num = 1)
	{
		int Size = Count();
		Assert(IsValidIndex(elem) || (Size == 0) || (num == 0));
		int numToMove = Size - elem - num;
		if ((numToMove > 0) && (num > 0))
		{
			Q_memmove(&Element(elem), &Element(elem + num), numToMove * sizeof(T));

#ifdef _DEBUG
			Q_memset(&Element(Size - num), 0xDD, num * sizeof(T));
#endif
		}
	}



	static Data_t *StaticData()
	{
		static Data_t staticData;
		Assert(staticData.m_Size == 0);
		return &staticData;
	}
};

#pragma warning(pop)


//-----------------------------------------------------------------------------
// The CCopyableUtlVector class:
// A array class that allows copy construction (so you can nest a CUtlVector inside of another one of our containers)
//  WARNING - this class lets you copy construct which can be an expensive operation if you don't carefully control when it happens
// Only use this when nesting a CUtlVector() inside of another one of our container classes (i.e a CUtlMap)
//-----------------------------------------------------------------------------
template< typename T, typename A = CUtlMemory<T> >
class CCopyableUtlVector : public CUtlVector< T, A >
{
	typedef CUtlVector< T, A > BaseClass;
public:
	explicit CCopyableUtlVector(int growSize = 0, int initSize = 0) : BaseClass(growSize, initSize) {}
	explicit CCopyableUtlVector(T* pMemory, int numElements) : BaseClass(pMemory, numElements) {}
	virtual ~CCopyableUtlVector() {}
	CCopyableUtlVector(CCopyableUtlVector const& vec) { this->CopyArray(vec.Base(), vec.Count()); }
};

// A vector class for storing pointers, so that the elements pointed to by the pointers are deleted
// on exit.
template<class T> class CUtlVectorAutoPurge : public CUtlVector< T, CUtlMemory< T, int> >
{
public:
	~CUtlVectorAutoPurge(void)
	{
		this->PurgeAndDeleteElements();
	}

};

// easy string list class with dynamically allocated strings. For use with V_SplitString, etc.
// Frees the dynamic strings in destructor.
class CUtlStringList : public CUtlVectorAutoPurge< char *>
{
public:
	void CopyAndAddToTail(char const *pString)			// clone the string and add to the end
	{
		char *pNewStr = new char[1 + strlen(pString)];
		strcpy(pNewStr, pString);
		AddToTail(pNewStr);
	}

	static int __cdecl SortFunc(char * const * sz1, char * const * sz2)
	{
		return strcmp(*sz1, *sz2);
	}

	inline void PurgeAndDeleteElements()
	{
		for (int i = 0; i < m_Size; i++)
		{
			delete[] Element(i);
		}
		Purge();
	}

	~CUtlStringList(void)
	{
		this->PurgeAndDeleteElements();
	}
};

// <Sergiy> placing it here a few days before Cert to minimize disruption to the rest of codebase
class CSplitString : public CUtlVector<char*, CUtlMemory<char*, int> >
{
public:
	CSplitString(const char *pString, const char *pSeparator);
	CSplitString(const char *pString, const char **pSeparators, int nSeparators);
	~CSplitString();
	//
	// NOTE: If you want to make Construct() public and implement Purge() here, you'll have to free m_szBuffer there
	//
private:
	void Construct(const char *pString, const char **pSeparators, int nSeparators);
	void PurgeAndDeleteElements();
private:
	char *m_szBuffer; // a copy of original string, with '\0' instead of separators
};


