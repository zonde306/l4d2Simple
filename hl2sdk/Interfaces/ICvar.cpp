#include "ICvar.h"

inline ICvar::Iterator::Iterator(ICvar *icvar)
{
	m_pIter = icvar->FactoryInternalIterator();
}

inline ICvar::Iterator::~Iterator(void)
{
	delete m_pIter;
}

inline void ICvar::Iterator::SetFirst(void)
{
	m_pIter->SetFirst();
}

inline void ICvar::Iterator::Next(void)
{
	m_pIter->Next();
}

inline bool ICvar::Iterator::IsValid(void)
{
	return m_pIter->IsValid();
}

inline ConCommandBase * ICvar::Iterator::Get(void)
{
	return m_pIter->Get();
}

