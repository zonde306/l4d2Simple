#include "handle.h"
#include "../Interfaces/IClientEntityList.h"

template<class T>
CHandle<T>::CHandle()
{
}

template<class T>
CHandle<T>::CHandle(int iEntry, int iSerialNumber)
{
	Init(iEntry, iSerialNumber);
}

template<class T>
CHandle<T>::CHandle(const CBaseHandle &handle)
	: CBaseHandle(handle)
{
}

template<class T>
CHandle<T>::CHandle(T *pObj)
{
	Term();
	Set(pObj);
}

template<class T>
CHandle<T> CHandle<T>::FromIndex(int index)
{
	CHandle<T> ret;
	ret.m_Index = index;
	return ret;
}

template<class T>
T* CHandle<T>::Get() const
{
	return (T*)CBaseHandle::Get();
}

template<class T>
CHandle<T>::operator T *()
{
	return Get();
}

template<class T>
CHandle<T>::operator T *() const
{
	return Get();
}

template<class T>
bool CHandle<T>::operator !() const
{
	return !Get();
}

template<class T>
bool CHandle<T>::operator==(T *val) const
{
	return Get() == val;
}

template<class T>
bool CHandle<T>::operator!=(T *val) const
{
	return Get() != val;
}

template<class T>
void CHandle<T>::Set(const T* pVal)
{
	CBaseHandle::Set(reinterpret_cast<const IHandleEntity*>(pVal));
}

template<class T>
const CBaseHandle& CHandle<T>::operator=(const T *val)
{
	Set(val);
	return *this;
}

template<class T>
T* CHandle<T>::operator -> () const
{
	return Get();
}

CBaseHandle::CBaseHandle()
{
	m_Index = INVALID_EHANDLE_INDEX;
}

CBaseHandle::CBaseHandle(const CBaseHandle &other)
{
	m_Index = other.m_Index;
}

CBaseHandle::CBaseHandle(unsigned long value)
{
	m_Index = value;
}

CBaseHandle::CBaseHandle(int iEntry, int iSerialNumber)
{
	Init(iEntry, iSerialNumber);
}

void CBaseHandle::Init(int iEntry, int iSerialNumber)
{
	m_Index = (unsigned long)(iEntry | (iSerialNumber << NUM_SERIAL_NUM_SHIFT_BITS));
}

void CBaseHandle::Term()
{
	m_Index = INVALID_EHANDLE_INDEX;
}

bool CBaseHandle::IsValid() const
{
	return m_Index != INVALID_EHANDLE_INDEX;
}

int CBaseHandle::GetEntryIndex() const
{
	// There is a hack here: due to a bug in the original implementation of the 
	// entity handle system, an attempt to look up an invalid entity index in 
	// certain cirumstances might fall through to the the mask operation below.
	// This would mask an invalid index to be in fact a lookup of entity number
	// NUM_ENT_ENTRIES, so invalid ent indexes end up actually looking up the
	// last slot in the entities array. Since this slot is always empty, the 
	// lookup returns NULL and the expected behavior occurs through this unexpected
	// route.
	// A lot of code actually depends on this behavior, and the bug was only exposed
	// after a change to NUM_SERIAL_NUM_BITS increased the number of allowable
	// static props in the world. So the if-stanza below detects this case and 
	// retains the prior (bug-submarining) behavior.
	if (!IsValid())
		return NUM_ENT_ENTRIES - 1;
	return m_Index & ENT_ENTRY_MASK;
}

int CBaseHandle::GetSerialNumber() const
{
	return m_Index >> NUM_SERIAL_NUM_SHIFT_BITS;
}

int CBaseHandle::ToInt() const
{
	return (int)m_Index;
}

bool CBaseHandle::operator !=(const CBaseHandle &other) const
{
	return m_Index != other.m_Index;
}

bool CBaseHandle::operator ==(const CBaseHandle &other) const
{
	return m_Index == other.m_Index;
}

bool CBaseHandle::operator ==(const IHandleEntity* pEnt) const
{
	return Get() == pEnt;
}

bool CBaseHandle::operator !=(const IHandleEntity* pEnt) const
{
	return Get() != pEnt;
}

bool CBaseHandle::operator <(const CBaseHandle &other) const
{
	return m_Index < other.m_Index;
}

bool CBaseHandle::operator <(const IHandleEntity *pEntity) const
{
	unsigned long otherIndex = (pEntity) ? pEntity->GetRefEHandle().m_Index : INVALID_EHANDLE_INDEX;
	return m_Index < otherIndex;
}

const CBaseHandle& CBaseHandle::operator=(const IHandleEntity *pEntity)
{
	return Set(pEntity);
}

const CBaseHandle& CBaseHandle::Set(const IHandleEntity *pEntity)
{
	if (pEntity)
		*this = pEntity->GetRefEHandle();
	else
		m_Index = INVALID_EHANDLE_INDEX;

	return *this;
}

namespace interfaces
{
	extern IClientEntityList* EntList;
};

IHandleEntity * CBaseHandle::Get() const
{
	if (m_Index == INVALID_EHANDLE_INDEX)
		return nullptr;
	
	return interfaces::EntList->GetClientEntity(m_Index);
}

