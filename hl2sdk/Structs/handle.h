#pragma once

#define NUM_ENT_ENTRY_BITS			(11 + 2)
#define NUM_ENT_ENTRIES				(1 << NUM_ENT_ENTRY_BITS)
#define INVALID_EHANDLE_INDEX		0xFFFFFFFF
#define NUM_SERIAL_NUM_BITS			16 // (32 - NUM_ENT_ENTRY_BITS)
#define NUM_SERIAL_NUM_SHIFT_BITS	(32 - NUM_SERIAL_NUM_BITS)
#define ENT_ENTRY_MASK				(( 1 << NUM_SERIAL_NUM_BITS) - 1)


class CBaseHandle;

class IHandleEntity
{
public:
	virtual ~IHandleEntity() {}
	virtual void SetRefEHandle(const CBaseHandle &handle) = 0;
	virtual const CBaseHandle& GetRefEHandle() const = 0;
};

class CBaseHandle
{
public:
	CBaseHandle();
	CBaseHandle(const CBaseHandle &other);
	CBaseHandle(unsigned long value);
	CBaseHandle(int iEntry, int iSerialNumber);

	void Init(int iEntry, int iSerialNumber);
	void Term();

	// Even if this returns true, Get() still can return return a non-null value.
	// This just tells if the handle has been initted with any values.
	bool IsValid() const;

	int GetEntryIndex() const;
	int GetSerialNumber() const;

	int ToInt() const;
	bool operator !=(const CBaseHandle &other) const;
	bool operator ==(const CBaseHandle &other) const;
	bool operator ==(const IHandleEntity* pEnt) const;
	bool operator !=(const IHandleEntity* pEnt) const;
	bool operator <(const CBaseHandle &other) const;
	bool operator <(const IHandleEntity* pEnt) const;

	// Assign a value to the handle.
	const CBaseHandle& operator=(const IHandleEntity *pEntity);
	const CBaseHandle& Set(const IHandleEntity *pEntity);

	IHandleEntity* Get() const;
protected:
	unsigned long  m_Index;
};

template< class T >
class CHandle : public CBaseHandle
{
public:
	CHandle();
	CHandle(int iEntry, int iSerialNumber);
	CHandle(const CBaseHandle &handle);
	CHandle(T *pVal);

	static CHandle<T> FromIndex(int index);

	T*        Get() const;
	void Set(const T* pVal);

	operator T*();
	operator T*() const;

	bool operator !() const;
	bool operator==(T *val) const;
	bool operator!=(T *val) const;
	const CBaseHandle& operator=(const T *val);

	T*        operator->() const;
};

