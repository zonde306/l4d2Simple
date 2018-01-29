#pragma once
#include "../definitions.h"
#include <Windows.h>
#include <cstdint>
#include <cmath>
#include <intrin.h>

#define NTAB 32

#pragma warning(push)
#pragma warning( disable:4251 )

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


//-----------------------------------------------------------------------------
// A generator of uniformly distributed random numbers
//-----------------------------------------------------------------------------
class IUniformRandomStream
{
public:
	// Sets the seed of the random number generator
	virtual void	SetSeed(int iSeed) = 0;

	// Generates random numbers
	virtual float	RandomFloat(float flMinVal = 0.0f, float flMaxVal = 1.0f) = 0;
	virtual int		RandomInt(int iMinVal, int iMaxVal) = 0;
	virtual float	RandomFloatExp(float flMinVal = 0.0f, float flMaxVal = 1.0f, float flExponent = 1.0f) = 0;
};


//-----------------------------------------------------------------------------
// The standard generator of uniformly distributed random numbers
//-----------------------------------------------------------------------------
class CUniformRandomStream : public IUniformRandomStream
{
public:
	CUniformRandomStream();

	// Sets the seed of the random number generator
	virtual void	SetSeed(int iSeed);

	// Generates random numbers
	virtual float	RandomFloat(float flMinVal = 0.0f, float flMaxVal = 1.0f);
	virtual int		RandomInt(int iMinVal, int iMaxVal);
	virtual float	RandomFloatExp(float flMinVal = 0.0f, float flMaxVal = 1.0f, float flExponent = 1.0f);

private:
	int		GenerateRandomNumber();

	int m_idum;
	int m_iy;
	int m_iv[NTAB];

	CThreadFastMutex m_mutex;
};


//-----------------------------------------------------------------------------
// A generator of gaussian distributed random numbers
//-----------------------------------------------------------------------------
class CGaussianRandomStream
{
public:
	// Passing in NULL will cause the gaussian stream to use the
	// installed global random number generator
	CGaussianRandomStream(IUniformRandomStream *pUniformStream = NULL);

	// Attaches to a random uniform stream
	void	AttachToStream(IUniformRandomStream *pUniformStream = NULL);

	// Generates random numbers
	float	RandomFloat(float flMean = 0.0f, float flStdDev = 1.0f);

private:
	IUniformRandomStream * m_pUniformStream;
	bool	m_bHaveValue;
	float	m_flRandomValue;

	CThreadFastMutex m_mutex;
};
