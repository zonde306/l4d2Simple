#pragma once
#include "../definitions.h"

//-----------------------------------------------------------------------------
// The CUtlBlockMemory class:
// A growable memory class that allocates non-sequential blocks, but is indexed sequentially
//-----------------------------------------------------------------------------
template< class T, class I >
class CUtlBlockMemory
{
public:
	// constructor, destructor
	CUtlBlockMemory(int nGrowSize = 0, int nInitSize = 0);
	~CUtlBlockMemory();

	// Set the size by which the memory grows - round up to the next power of 2
	void Init(int nGrowSize = 0, int nInitSize = 0);

	// here to match CUtlMemory, but only used by ResetDbgInfo, so it can just return NULL
	T* Base() { return NULL; }
	const T* Base() const { return NULL; }

	class Iterator_t
	{
	public:
		Iterator_t(I i) : index(i) {}
		I index;

		bool operator==(const Iterator_t it) const { return index == it.index; }
		bool operator!=(const Iterator_t it) const { return index != it.index; }
	};
	Iterator_t First() const { return Iterator_t(IsIdxValid(0) ? 0 : InvalidIndex()); }
	Iterator_t Next(const Iterator_t &it) const { return Iterator_t(IsIdxValid(it.index + 1) ? it.index + 1 : InvalidIndex()); }
	I GetIndex(const Iterator_t &it) const { return it.index; }
	bool IsIdxAfter(I i, const Iterator_t &it) const { return i > it.index; }
	bool IsValidIterator(const Iterator_t &it) const { return IsIdxValid(it.index); }
	Iterator_t InvalidIterator() const { return Iterator_t(InvalidIndex()); }

	// element access
	T& operator[](I i);
	const T& operator[](I i) const;
	T& Element(I i);
	const T& Element(I i) const;

	// Can we use this index?
	bool IsIdxValid(I i) const;
	static I InvalidIndex() { return (I)-1; }

	void Swap(CUtlBlockMemory< T, I > &mem);

	// Size
	int NumAllocated() const;
	int Count() const { return NumAllocated(); }

	// Grows memory by max(num,growsize) rounded up to the next power of 2, and returns the allocation index/ptr
	void Grow(int num = 1);

	// Makes sure we've got at least this much memory
	void EnsureCapacity(int num);

	// Memory deallocation
	void Purge();

	// Purge all but the given number of elements
	void Purge(int numElements);

protected:
	int Index(int major, int minor) const { return (major << m_nIndexShift) | minor; }
	int MajorIndex(int i) const { return i >> m_nIndexShift; }
	int MinorIndex(int i) const { return i & m_nIndexMask; }
	void ChangeSize(int nBlocks);
	int NumElementsInBlock() const { return m_nIndexMask + 1; }

	T** m_pMemory;
	int m_nBlocks;
	int m_nIndexMask : 27;
	int m_nIndexShift : 5;
};
