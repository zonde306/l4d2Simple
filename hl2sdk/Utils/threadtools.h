#pragma once
#include <Windows.h>
#include <cstdint>
#include <intrin.h>

inline bool ThreadInterlockedAssignIf(long volatile *p, long value, long comperand)
{
	Assert((size_t)p % 4 == 0);
	return (_InterlockedCompareExchange(p, value, comperand) == comperand);
}

inline long ThreadInterlockedExchange(long volatile *p, long value)
{
	Assert((size_t)p % 4 == 0);
	return _InterlockedExchange(p, value);
}

class CThreadFastMutex
{
public:
	CThreadFastMutex()
		: m_ownerID(0),
		m_depth(0)
	{
	}

private:
	FORCEINLINE bool TryLockInline(const uint32_t threadId) volatile
	{
		if (threadId != m_ownerID && !ThreadInterlockedAssignIf((volatile long *)&m_ownerID, (long)threadId, 0))
			return false;

		_ReadWriteBarrier();
		++m_depth;
		return true;
	}

	inline bool TryLock(const uint32_t threadId) volatile
	{
		return TryLockInline(threadId);
	}

	inline void Lock(const uint32_t threadId, unsigned nSpinSleepTime) volatile
	{
		while (!TryLock(threadId))
			Sleep(nSpinSleepTime);
	}

public:
	inline bool TryLock() volatile
	{
#ifdef _DEBUG_OUTPUT
		if (m_depth == INT_MAX)
			DebuggerBreak();

		if (m_depth < 0)
			DebuggerBreak();
#endif
		return TryLockInline(GetCurrentThreadId());
	}

#ifndef _DEBUG_OUTPUT 
	FORCEINLINE
#endif
		void Lock(unsigned int nSpinSleepTime = 0) volatile
	{
		const uint32_t threadId = GetCurrentThreadId();

		if (!TryLockInline(threadId))
		{
			SuspendThread(GetCurrentThread());
			Lock(threadId, nSpinSleepTime);
		}
#ifdef _DEBUG_OUTPUT
		if (m_ownerID != ThreadGetCurrentId())
			DebuggerBreak();

		if (m_depth == INT_MAX)
			DebuggerBreak();

		if (m_depth < 0)
			DebuggerBreak();
#endif
	}

#ifndef _DEBUG_OUTPUT
	FORCEINLINE
#endif
		void Unlock() volatile
	{
#ifdef _DEBUG_OUTPUT
		if (m_ownerID != ThreadGetCurrentId())
			DebuggerBreak();

		if (m_depth <= 0)
			DebuggerBreak();
#endif

		--m_depth;
		if (!m_depth)
		{
			_ReadWriteBarrier();
			ThreadInterlockedExchange((long*)&m_ownerID, 0);
		}
	}

#ifdef WIN32
	inline bool TryLock() const volatile { return (const_cast<CThreadFastMutex *>(this))->TryLock(); }
	inline void Lock(unsigned nSpinSleepTime = 1) const volatile { (const_cast<CThreadFastMutex *>(this))->Lock(nSpinSleepTime); }
	inline void Unlock() const	volatile { (const_cast<CThreadFastMutex *>(this))->Unlock(); }
#endif
	// To match regular CThreadMutex:
	inline bool AssertOwnedByCurrentThread() { return true; }
	inline void SetTrace(bool) {}

	inline uint32_t GetOwnerId() const { return m_ownerID; }
	inline int	GetDepth() const { return m_depth; }
private:
	volatile uint32_t m_ownerID;
	int				m_depth;
};

template <class MUTEX_TYPE = CThreadMutex>
class CAutoLockT
{
public:
	FORCEINLINE CAutoLockT(MUTEX_TYPE &lock)
		: m_lock(lock)
	{
		m_lock.Lock();
	}

	FORCEINLINE CAutoLockT(const MUTEX_TYPE &lock)
		: m_lock(const_cast<MUTEX_TYPE &>(lock))
	{
		m_lock.Lock();
	}

	FORCEINLINE ~CAutoLockT()
	{
		m_lock.Unlock();
	}


private:
	MUTEX_TYPE & m_lock;

	// Disallow copying
	CAutoLockT<MUTEX_TYPE>(const CAutoLockT<MUTEX_TYPE> &);
	CAutoLockT<MUTEX_TYPE> &operator=(const CAutoLockT<MUTEX_TYPE> &);
};

template <int size>	struct CAutoLockTypeDeducer {};
// template <> struct CAutoLockTypeDeducer<sizeof(CThreadMutex)> { typedef CThreadMutex Type_t; };
// template <> struct CAutoLockTypeDeducer<sizeof(CThreadNullMutex)> { typedef CThreadNullMutex Type_t; };
template <> struct CAutoLockTypeDeducer<sizeof(CThreadFastMutex)> { typedef CThreadFastMutex Type_t; };
// template <> struct CAutoLockTypeDeducer<sizeof(CAlignedThreadFastMutex)> { typedef CAlignedThreadFastMutex Type_t; };

#define AUTO_LOCK_( type, mutex ) \
	CAutoLockT< type > UNIQUE_ID( static_cast<const type &>( mutex ) )

#define AUTO_LOCK( mutex ) \
	AUTO_LOCK_( CAutoLockTypeDeducer<sizeof(mutex)>::Type_t, mutex )
