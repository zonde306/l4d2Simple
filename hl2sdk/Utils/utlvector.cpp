#include "utlvector.h"
#include "strtools.h"
#include "../definitions.h"

// TODO (Ilya): It seems like all the functions in CUtlVector are simple enough that they should be inlined.

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template< typename T, class A >
inline CUtlVector<T, A>::CUtlVector(int growSize, int initSize) :
	m_Memory(growSize, initSize), m_Size(0)
{
	ResetDbgInfo();
}

template< typename T, class A >
inline CUtlVector<T, A>::CUtlVector(T* pMemory, int allocationCount, int numElements) :
	m_Memory(pMemory, allocationCount), m_Size(numElements)
{
	ResetDbgInfo();
}

template< typename T, class A >
inline CUtlVector<T, A>::~CUtlVector()
{
	Purge();
}

template< typename T, class A >
inline CUtlVector<T, A>& CUtlVector<T, A>::operator=(const CUtlVector<T, A> &other)
{
	int nCount = other.Count();
	SetSize(nCount);
	for (int i = 0; i < nCount; i++)
	{
		(*this)[i] = other[i];
	}
	return *this;
}


//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template< typename T, class A >
inline T& CUtlVector<T, A>::operator[](int i)
{
	// Do an inline unsigned check for maximum debug-build performance.
	Assert((unsigned)i < (unsigned)m_Size);
	return m_Memory[i];
}

template< typename T, class A >
inline const T& CUtlVector<T, A>::operator[](int i) const
{
	// Do an inline unsigned check for maximum debug-build performance.
	Assert((unsigned)i < (unsigned)m_Size);
	return m_Memory[i];
}

template< typename T, class A >
inline T& CUtlVector<T, A>::Element(int i)
{
	// Do an inline unsigned check for maximum debug-build performance.
	Assert((unsigned)i < (unsigned)m_Size);
	return m_Memory[i];
}

template< typename T, class A >
inline const T& CUtlVector<T, A>::Element(int i) const
{
	// Do an inline unsigned check for maximum debug-build performance.
	Assert((unsigned)i < (unsigned)m_Size);
	return m_Memory[i];
}

template< typename T, class A >
inline T& CUtlVector<T, A>::Head()
{
	Assert(m_Size > 0);
	return m_Memory[0];
}

template< typename T, class A >
inline const T& CUtlVector<T, A>::Head() const
{
	Assert(m_Size > 0);
	return m_Memory[0];
}

template< typename T, class A >
inline T& CUtlVector<T, A>::Tail()
{
	Assert(m_Size > 0);
	return m_Memory[m_Size - 1];
}

template< typename T, class A >
inline const T& CUtlVector<T, A>::Tail() const
{
	Assert(m_Size > 0);
	return m_Memory[m_Size - 1];
}


//-----------------------------------------------------------------------------
// Count
//-----------------------------------------------------------------------------
template< typename T, class A >
inline int CUtlVector<T, A>::Size() const
{
	return m_Size;
}

template< typename T, class A >
inline int CUtlVector<T, A>::Count() const
{
	return m_Size;
}


//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template< typename T, class A >
inline bool CUtlVector<T, A>::IsValidIndex(int i) const
{
	return (i >= 0) && (i < m_Size);
}


//-----------------------------------------------------------------------------
// Returns in invalid index
//-----------------------------------------------------------------------------
template< typename T, class A >
inline int CUtlVector<T, A>::InvalidIndex()
{
	return -1;
}


//-----------------------------------------------------------------------------
// Grows the vector
//-----------------------------------------------------------------------------
template< typename T, class A >
void CUtlVector<T, A>::GrowVector(int num)
{
	if (m_Size + num > m_Memory.NumAllocated())
	{
		// MEM_ALLOC_CREDIT_CLASS();
		m_Memory.Grow(m_Size + num - m_Memory.NumAllocated());
	}

	m_Size += num;
	ResetDbgInfo();
}


//-----------------------------------------------------------------------------
// Sorts the vector
//-----------------------------------------------------------------------------
template< typename T, class A >
void CUtlVector<T, A>::Sort(int(__cdecl *pfnCompare)(const T *, const T *))
{
	typedef int(__cdecl *QSortCompareFunc_t)(const void *, const void *);
	if (Count() <= 1)
		return;

	if (Base())
	{
		qsort(Base(), Count(), sizeof(T), (QSortCompareFunc_t)(pfnCompare));
	}
	else
	{
		Assert(0);
		// this path is untested
		// if you want to sort vectors that use a non-sequential memory allocator,
		// you'll probably want to patch in a quicksort algorithm here
		// I just threw in this bubble sort to have something just in case...

		for (int i = m_Size - 1; i >= 0; --i)
		{
			for (int j = 1; j <= i; ++j)
			{
				if (pfnCompare(&Element(j - 1), &Element(j)) < 0)
				{
					V_swap(Element(j - 1), Element(j));
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Makes sure we have enough memory allocated to store a requested # of elements
//-----------------------------------------------------------------------------
template< typename T, class A >
void CUtlVector<T, A>::EnsureCapacity(int num)
{
	// MEM_ALLOC_CREDIT_CLASS();
	m_Memory.EnsureCapacity(num);
	ResetDbgInfo();
}


//-----------------------------------------------------------------------------
// Makes sure we have at least this many elements
//-----------------------------------------------------------------------------
template< typename T, class A >
void CUtlVector<T, A>::EnsureCount(int num)
{
	if (Count() < num)
		AddMultipleToTail(num - Count());
}


//-----------------------------------------------------------------------------
// Shifts elements
//-----------------------------------------------------------------------------
template< typename T, class A >
void CUtlVector<T, A>::ShiftElementsRight(int elem, int num)
{
	Assert(IsValidIndex(elem) || (m_Size == 0) || (num == 0));
	int numToMove = m_Size - elem - num;
	if ((numToMove > 0) && (num > 0))
		Q_memmove(&Element(elem + num), &Element(elem), numToMove * sizeof(T));
}

template< typename T, class A >
void CUtlVector<T, A>::ShiftElementsLeft(int elem, int num)
{
	Assert(IsValidIndex(elem) || (m_Size == 0) || (num == 0));
	int numToMove = m_Size - elem - num;
	if ((numToMove > 0) && (num > 0))
	{
		Q_memmove(&Element(elem), &Element(elem + num), numToMove * sizeof(T));

#ifdef _DEBUG
		Q_memset(&Element(m_Size - num), 0xDD, num * sizeof(T));
#endif
	}
}


//-----------------------------------------------------------------------------
// Adds an element, uses default constructor
//-----------------------------------------------------------------------------
template< typename T, class A >
inline int CUtlVector<T, A>::AddToHead()
{
	return InsertBefore(0);
}

template< typename T, class A >
inline int CUtlVector<T, A>::AddToTail()
{
	return InsertBefore(m_Size);
}

template< typename T, class A >
inline int CUtlVector<T, A>::InsertAfter(int elem)
{
	return InsertBefore(elem + 1);
}

template< typename T, class A >
int CUtlVector<T, A>::InsertBefore(int elem)
{
	// Can insert at the end
	Assert((elem == Count()) || IsValidIndex(elem));

	GrowVector();
	ShiftElementsRight(elem);
	Construct(&Element(elem));
	return elem;
}


//-----------------------------------------------------------------------------
// Adds an element, uses copy constructor
//-----------------------------------------------------------------------------
template< typename T, class A >
inline int CUtlVector<T, A>::AddToHead(const T& src)
{
	// Can't insert something that's in the list... reallocation may hose us
	Assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
	return InsertBefore(0, src);
}

template< typename T, class A >
inline int CUtlVector<T, A>::AddToTail(const T& src)
{
	// Can't insert something that's in the list... reallocation may hose us
	Assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
	return InsertBefore(m_Size, src);
}

template< typename T, class A >
inline int CUtlVector<T, A>::InsertAfter(int elem, const T& src)
{
	// Can't insert something that's in the list... reallocation may hose us
	Assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
	return InsertBefore(elem + 1, src);
}

template< typename T, class A >
int CUtlVector<T, A>::InsertBefore(int elem, const T& src)
{
	// Can't insert something that's in the list... reallocation may hose us
	Assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));

	// Can insert at the end
	Assert((elem == Count()) || IsValidIndex(elem));

	GrowVector();
	ShiftElementsRight(elem);
	CopyConstruct(&Element(elem), src);
	return elem;
}


//-----------------------------------------------------------------------------
// Adds multiple elements, uses default constructor
//-----------------------------------------------------------------------------
template< typename T, class A >
inline int CUtlVector<T, A>::AddMultipleToHead(int num)
{
	return InsertMultipleBefore(0, num);
}

template< typename T, class A >
inline int CUtlVector<T, A>::AddMultipleToTail(int num, const T *pToCopy)
{
	// Can't insert something that's in the list... reallocation may hose us
	Assert((Base() == NULL) || !pToCopy || (pToCopy + num < Base()) || (pToCopy >= (Base() + Count())));

	return InsertMultipleBefore(m_Size, num, pToCopy);
}

template< typename T, class A >
int CUtlVector<T, A>::InsertMultipleAfter(int elem, int num)
{
	return InsertMultipleBefore(elem + 1, num);
}


template< typename T, class A >
void CUtlVector<T, A>::SetCount(int count)
{
	RemoveAll();
	AddMultipleToTail(count);
}

template< typename T, class A >
inline void CUtlVector<T, A>::SetSize(int size)
{
	SetCount(size);
}

template< typename T, class A >
void CUtlVector<T, A>::SetCountNonDestructively(int count)
{
	int delta = count - m_Size;
	if (delta > 0) AddMultipleToTail(delta);
	else if (delta < 0) RemoveMultipleFromTail(-delta);
}

template< typename T, class A >
void CUtlVector<T, A>::CopyArray(const T *pArray, int size)
{
	// Can't insert something that's in the list... reallocation may hose us
	Assert((Base() == NULL) || !pArray || (Base() >= (pArray + size)) || (pArray >= (Base() + Count())));

	SetSize(size);
	for (int i = 0; i < size; i++)
	{
		(*this)[i] = pArray[i];
	}
}

template< typename T, class A >
void CUtlVector<T, A>::Swap(CUtlVector< T, A > &vec)
{
	m_Memory.Swap(vec.m_Memory);
	V_swap(m_Size, vec.m_Size);

#ifndef _X360
	V_swap(m_pElements, vec.m_pElements);
#endif
}

template< typename T, class A >
int CUtlVector<T, A>::AddVectorToTail(CUtlVector const &src)
{
	Assert(&src != this);

	int base = Count();

	// Make space.
	AddMultipleToTail(src.Count());

	// Copy the elements.	
	for (int i = 0; i < src.Count(); i++)
	{
		(*this)[base + i] = src[i];
	}

	return base;
}

template< typename T, class A >
inline int CUtlVector<T, A>::InsertMultipleBefore(int elem, int num, const T *pToInsert)
{
	if (num == 0)
		return elem;

	// Can insert at the end
	Assert((elem == Count()) || IsValidIndex(elem));

	GrowVector(num);
	ShiftElementsRight(elem, num);

	// Invoke default constructors
	for (int i = 0; i < num; ++i)
		Construct(&Element(elem + i));

	// Copy stuff in?
	if (pToInsert)
	{
		for (int i = 0; i < num; i++)
		{
			Element(elem + i) = pToInsert[i];
		}
	}

	return elem;
}


//-----------------------------------------------------------------------------
// Finds an element (element needs operator== defined)
//-----------------------------------------------------------------------------
template< typename T, class A >
int CUtlVector<T, A>::Find(const T& src) const
{
	for (int i = 0; i < Count(); ++i)
	{
		if (Element(i) == src)
			return i;
	}
	return -1;
}

template< typename T, class A >
bool CUtlVector<T, A>::HasElement(const T& src) const
{
	return (Find(src) >= 0);
}


//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------
template< typename T, class A >
void CUtlVector<T, A>::FastRemove(int elem)
{
	Assert(IsValidIndex(elem));

	Destruct(&Element(elem));
	if (m_Size > 0)
	{
		if (elem != m_Size - 1)
			memcpy(reinterpret_cast<void*>(&Element(elem)), reinterpret_cast<void*>(&Element(m_Size - 1)), sizeof(T));
		--m_Size;
	}
}

template< typename T, class A >
void CUtlVector<T, A>::Remove(int elem)
{
	Destruct(&Element(elem));
	ShiftElementsLeft(elem);
	--m_Size;
}

template< typename T, class A >
bool CUtlVector<T, A>::FindAndRemove(const T& src)
{
	int elem = Find(src);
	if (elem != -1)
	{
		Remove(elem);
		return true;
	}
	return false;
}

template< typename T, class A >
bool CUtlVector<T, A>::FindAndFastRemove(const T& src)
{
	int elem = Find(src);
	if (elem != -1)
	{
		FastRemove(elem);
		return true;
	}
	return false;
}

template< typename T, class A >
void CUtlVector<T, A>::RemoveMultiple(int elem, int num)
{
	Assert(elem >= 0);
	Assert(elem + num <= Count());

	for (int i = elem + num; --i >= elem; )
		Destruct(&Element(i));

	ShiftElementsLeft(elem, num);
	m_Size -= num;
}

template< typename T, class A >
void CUtlVector<T, A>::RemoveMultipleFromHead(int num)
{
	Assert(num <= Count());

	for (int i = num; --i >= 0; )
		Destruct(&Element(i));

	ShiftElementsLeft(0, num);
	m_Size -= num;
}

template< typename T, class A >
void CUtlVector<T, A>::RemoveMultipleFromTail(int num)
{
	Assert(num <= Count());

	for (int i = m_Size - num; i < m_Size; i++)
		Destruct(&Element(i));

	m_Size -= num;
}

template< typename T, class A >
void CUtlVector<T, A>::RemoveAll()
{
	for (int i = m_Size; --i >= 0; )
	{
		Destruct(&Element(i));
	}

	m_Size = 0;
}


//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

template< typename T, class A >
inline void CUtlVector<T, A>::Purge()
{
	RemoveAll();
	m_Memory.Purge();
	ResetDbgInfo();
}


template< typename T, class A >
inline void CUtlVector<T, A>::PurgeAndDeleteElements()
{
	for (int i = 0; i < m_Size; i++)
	{
		delete Element(i);
	}
	Purge();
}

template< typename T, class A >
inline void CUtlVector<T, A>::Compact()
{
	m_Memory.Purge(m_Size);
}

template< typename T, class A >
inline int CUtlVector<T, A>::NumAllocated() const
{
	return m_Memory.NumAllocated();
}


//-----------------------------------------------------------------------------
// Data and memory validation
//-----------------------------------------------------------------------------
#ifdef DBGFLAG_VALIDATE
template< typename T, class A >
void CUtlVector<T, A>::Validate(CValidator &validator, char *pchName)
{
	validator.Push(typeid(*this).name(), this, pchName);

	m_Memory.Validate(validator, "m_Memory");

	validator.Pop();
}
#endif // DBGFLAG_VALIDATE
