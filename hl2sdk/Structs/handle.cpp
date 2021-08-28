#include "handle.h"
#include "../Interfaces/IClientEntityList.h"
#include "../interfaces.h"

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
CHandle<T>::CHandle(const CBaseHandle &handle) : CBaseHandle(handle)
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
CHandle<T>::operator T* ()
{
	return Get();
}

template<class T>
CHandle<T>::operator T* () const
{
	return Get();
}

template<class T>
bool CHandle<T>::operator !() const
{
	return !Get();
}

template<class T>
bool CHandle<T>::operator==(T* val) const
{
	return Get() == val;
}

template<class T>
bool CHandle<T>::operator!=(T* val) const
{
	return Get() != val;
}

template<class T>
void CHandle<T>::Set(const T* pVal)
{
	CBaseHandle::Set(reinterpret_cast<const IHandleEntity*>(pVal));
}

template<class T>
const CBaseHandle& CHandle<T>::operator=(const T* val)
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

CBaseHandle::CBaseHandle(const CBaseHandle& other)
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
	return (m_Index != INVALID_EHANDLE_INDEX && m_Index > 0);
}

int CBaseHandle::GetEntryIndex() const
{
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

bool CBaseHandle::operator !=(const CBaseHandle& other) const
{
	return m_Index != other.m_Index;
}

bool CBaseHandle::operator ==(const CBaseHandle& other) const
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

bool CBaseHandle::operator <(const CBaseHandle& other) const
{
	return m_Index < other.m_Index;
}

bool CBaseHandle::operator <(const IHandleEntity* pEntity) const
{
	unsigned long otherIndex = (pEntity) ? pEntity->GetRefEHandle().m_Index : INVALID_EHANDLE_INDEX;
	return m_Index < otherIndex;
}

const CBaseHandle& CBaseHandle::operator=(const IHandleEntity* pEntity)
{
	return Set(pEntity);
}

const CBaseHandle& CBaseHandle::Set(const IHandleEntity* pEntity)
{
	if (pEntity)
		*this = pEntity->GetRefEHandle();
	else
		m_Index = INVALID_EHANDLE_INDEX;

	return *this;
}

IHandleEntity* CBaseHandle::Get() const
{
	if (!IsValid())
		return nullptr;

	return g_pInterface->EntList->GetClientEntityFromHandle(*this);
}