#include "utlblockmemory.h"

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template< class T, class I >
CUtlBlockMemory<T, I>::CUtlBlockMemory(int nGrowSize, int nInitAllocationCount)
	: m_pMemory(0), m_nBlocks(0), m_nIndexMask(0), m_nIndexShift(0)
{
	Init(nGrowSize, nInitAllocationCount);
}

template< class T, class I >
CUtlBlockMemory<T, I>::~CUtlBlockMemory()
{
	Purge();
}


//-----------------------------------------------------------------------------
// Fast swap
//-----------------------------------------------------------------------------
template< class T, class I >
void CUtlBlockMemory<T, I>::Swap(CUtlBlockMemory< T, I > &mem)
{
	swap(m_pMemory, mem.m_pMemory);
	swap(m_nBlocks, mem.m_nBlocks);
	swap(m_nIndexMask, mem.m_nIndexMask);
	swap(m_nIndexShift, mem.m_nIndexShift);
}


//-----------------------------------------------------------------------------
// Set the size by which the memory grows - round up to the next power of 2
//-----------------------------------------------------------------------------
template< class T, class I >
void CUtlBlockMemory<T, I>::Init(int nGrowSize /* = 0 */, int nInitSize /* = 0 */)
{
	Purge();

	if (nGrowSize == 0)
	{
		// default grow size is smallest size s.t. c++ allocation overhead is ~6% of block size
		nGrowSize = (127 + sizeof(T)) / sizeof(T);
	}
	nGrowSize = SmallestPowerOfTwoGreaterOrEqual(nGrowSize);
	m_nIndexMask = nGrowSize - 1;

	m_nIndexShift = 0;
	while (nGrowSize > 1)
	{
		nGrowSize >>= 1;
		++m_nIndexShift;
	}
	Assert(m_nIndexMask + 1 == (1 << m_nIndexShift));

	Grow(nInitSize);
}


//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template< class T, class I >
inline T& CUtlBlockMemory<T, I>::operator[](I i)
{
	Assert(IsIdxValid(i));
	T *pBlock = m_pMemory[MajorIndex(i)];
	return pBlock[MinorIndex(i)];
}

template< class T, class I >
inline const T& CUtlBlockMemory<T, I>::operator[](I i) const
{
	Assert(IsIdxValid(i));
	const T *pBlock = m_pMemory[MajorIndex(i)];
	return pBlock[MinorIndex(i)];
}

template< class T, class I >
inline T& CUtlBlockMemory<T, I>::Element(I i)
{
	Assert(IsIdxValid(i));
	T *pBlock = m_pMemory[MajorIndex(i)];
	return pBlock[MinorIndex(i)];
}

template< class T, class I >
inline const T& CUtlBlockMemory<T, I>::Element(I i) const
{
	Assert(IsIdxValid(i));
	const T *pBlock = m_pMemory[MajorIndex(i)];
	return pBlock[MinorIndex(i)];
}


//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
template< class T, class I >
inline int CUtlBlockMemory<T, I>::NumAllocated() const
{
	return m_nBlocks * NumElementsInBlock();
}


//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template< class T, class I >
inline bool CUtlBlockMemory<T, I>::IsIdxValid(I i) const
{
	return (i >= 0) && (MajorIndex(i) < m_nBlocks);
}

template< class T, class I >
void CUtlBlockMemory<T, I>::Grow(int num)
{
	if (num <= 0)
		return;

	int nBlockSize = NumElementsInBlock();
	int nBlocks = (num + nBlockSize - 1) / nBlockSize;

	ChangeSize(m_nBlocks + nBlocks);
}

template< class T, class I >
void CUtlBlockMemory<T, I>::ChangeSize(int nBlocks)
{
	UTLBLOCKMEMORY_TRACK_FREE(); // this must stay before the recalculation of m_nBlocks, since it implicitly uses the old value

	int nBlocksOld = m_nBlocks;
	m_nBlocks = nBlocks;

	UTLBLOCKMEMORY_TRACK_ALLOC(); // this must stay after the recalculation of m_nBlocks, since it implicitly uses the new value

	if (m_pMemory)
	{
		// free old blocks if shrinking
		// Only possible if m_pMemory is non-NULL (and avoids PVS-Studio warning)
		for (int i = m_nBlocks; i < nBlocksOld; ++i)
		{
			UTLBLOCKMEMORY_TRACK_FREE();
			free((void*)m_pMemory[i]);
		}

		// // MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T**)realloc(m_pMemory, m_nBlocks * sizeof(T*));
		Assert(m_pMemory);
	}
	else
	{
		// // MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T**)malloc(m_nBlocks * sizeof(T*));
		Assert(m_pMemory);
	}

	if (!m_pMemory)
	{
		Error("CUtlBlockMemory overflow!\n");
	}

	// allocate new blocks if growing
	int nBlockSize = NumElementsInBlock();
	for (int i = nBlocksOld; i < m_nBlocks; ++i)
	{
		// // MEM_ALLOC_CREDIT_CLASS();
		m_pMemory[i] = (T*)malloc(nBlockSize * sizeof(T));
		Assert(m_pMemory[i]);
	}
}


//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
template< class T, class I >
inline void CUtlBlockMemory<T, I>::EnsureCapacity(int num)
{
	Grow(num - NumAllocated());
}


//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template< class T, class I >
void CUtlBlockMemory<T, I>::Purge()
{
	if (!m_pMemory)
		return;

	for (int i = 0; i < m_nBlocks; ++i)
	{
		UTLBLOCKMEMORY_TRACK_FREE();
		free((void*)m_pMemory[i]);
	}
	m_nBlocks = 0;

	UTLBLOCKMEMORY_TRACK_FREE();
	free((void*)m_pMemory);
	m_pMemory = 0;
}

template< class T, class I >
void CUtlBlockMemory<T, I>::Purge(int numElements)
{
	Assert(numElements >= 0);

	int nAllocated = NumAllocated();
	if (numElements > nAllocated)
	{
		// Ensure this isn't a grow request in disguise.
		Assert(numElements <= nAllocated);
		return;
	}

	if (numElements <= 0)
	{
		Purge();
		return;
	}

	int nBlockSize = NumElementsInBlock();
	int nBlocksOld = m_nBlocks;
	int nBlocks = (numElements + nBlockSize - 1) / nBlockSize;

	// If the number of blocks is the same as the allocated number of blocks, we are done.
	if (nBlocks == m_nBlocks)
		return;

	ChangeSize(nBlocks);
}

