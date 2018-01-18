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

	bool TryLock(const uint32_t threadId) volatile
	{
		return TryLockInline(threadId);
	}

	void Lock(const uint32_t threadId, unsigned nSpinSleepTime) volatile
	{
		while (!TryLock(threadId))
			Sleep(nSpinSleepTime);
	}

public:
	bool TryLock() volatile
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
	bool TryLock() const volatile { return (const_cast<CThreadFastMutex *>(this))->TryLock(); }
	void Lock(unsigned nSpinSleepTime = 1) const volatile { (const_cast<CThreadFastMutex *>(this))->Lock(nSpinSleepTime); }
	void Unlock() const	volatile { (const_cast<CThreadFastMutex *>(this))->Unlock(); }
#endif
	// To match regular CThreadMutex:
	bool AssertOwnedByCurrentThread() { return true; }
	void SetTrace(bool) {}

	uint32_t GetOwnerId() const { return m_ownerID; }
	int	GetDepth() const { return m_depth; }
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


#define IA 16807
#define IM 2147483647
#define IQ 127773
#define IR 2836
#define NDIV (1+(IM-1)/NTAB)
#define MAX_RANDOM_RANGE 0x7FFFFFFFUL

// fran1 -- return a random floating-point number on the interval [0,1)
//
#define AM (1.0/IM)
#define EPS 1.2e-7f
#define RNMX (1.0f-EPS)

//-----------------------------------------------------------------------------
// globals
//-----------------------------------------------------------------------------
static CUniformRandomStream s_UniformStream;
static CGaussianRandomStream s_GaussianStream;
static IUniformRandomStream *s_pUniformStream = &s_UniformStream;


//-----------------------------------------------------------------------------
// Installs a global random number generator, which will affect the Random functions above
//-----------------------------------------------------------------------------
void InstallUniformRandomStream(IUniformRandomStream *pStream)
{
	s_pUniformStream = pStream ? pStream : &s_UniformStream;
}


//-----------------------------------------------------------------------------
// A couple of convenience functions to access the library's global uniform stream
//-----------------------------------------------------------------------------
void RandomSeed(int iSeed)
{
	s_pUniformStream->SetSeed(iSeed);
}

float RandomFloat(float flMinVal, float flMaxVal)
{
	return s_pUniformStream->RandomFloat(flMinVal, flMaxVal);
}

float RandomFloatExp(float flMinVal, float flMaxVal, float flExponent)
{
	return s_pUniformStream->RandomFloatExp(flMinVal, flMaxVal, flExponent);
}

int RandomInt(int iMinVal, int iMaxVal)
{
	return s_pUniformStream->RandomInt(iMinVal, iMaxVal);
}

float RandomGaussianFloat(float flMean, float flStdDev)
{
	return s_GaussianStream.RandomFloat(flMean, flStdDev);
}


//-----------------------------------------------------------------------------
//
// Implementation of the uniform random number stream
//
//-----------------------------------------------------------------------------
CUniformRandomStream::CUniformRandomStream()
{
	SetSeed(0);
}

void CUniformRandomStream::SetSeed(int iSeed)
{
	m_mutex.Lock();
	m_idum = ((iSeed < 0) ? iSeed : -iSeed);
	m_iy = 0;
	m_mutex.Unlock();
}

int CUniformRandomStream::GenerateRandomNumber()
{
	m_mutex.Lock();
	int j;
	int k;

	if (m_idum <= 0 || !m_iy)
	{
		if (-(m_idum) < 1)
			m_idum = 1;
		else
			m_idum = -(m_idum);

		for (j = NTAB + 7; j >= 0; j--)
		{
			k = (m_idum) / IQ;
			m_idum = IA * (m_idum - k * IQ) - IR * k;
			if (m_idum < 0)
				m_idum += IM;
			if (j < NTAB)
				m_iv[j] = m_idum;
		}
		m_iy = m_iv[0];
	}
	k = (m_idum) / IQ;
	m_idum = IA * (m_idum - k * IQ) - IR * k;
	if (m_idum < 0)
		m_idum += IM;
	j = m_iy / NDIV;

	// We're seeing some strange memory corruption in the contents of s_pUniformStream. 
	// Perhaps it's being caused by something writing past the end of this array? 
	// Bounds-check in release to see if that's the case.
	if (j >= NTAB || j < 0)
	{
		j = (j % NTAB) & 0x7fffffff;
	}

	m_iy = m_iv[j];
	m_iv[j] = m_idum;
	m_mutex.Unlock();

	return m_iy;
}

float CUniformRandomStream::RandomFloat(float flLow, float flHigh)
{
	// float in [0,1)
	float fl = AM * GenerateRandomNumber();
	if (fl > RNMX)
	{
		fl = RNMX;
	}
	return (fl * (flHigh - flLow)) + flLow; // float in [low,high)
}

float CUniformRandomStream::RandomFloatExp(float flMinVal, float flMaxVal, float flExponent)
{
	// float in [0,1)
	float fl = AM * GenerateRandomNumber();
	if (fl > RNMX)
	{
		fl = RNMX;
	}
	if (flExponent != 1.0f)
	{
		fl = powf(fl, flExponent);
	}
	return (fl * (flMaxVal - flMinVal)) + flMinVal; // float in [low,high)
}

int CUniformRandomStream::RandomInt(int iLow, int iHigh)
{
	//ASSERT(lLow <= lHigh);
	unsigned int maxAcceptable;
	unsigned int x = iHigh - iLow + 1;
	unsigned int n;
	if (x <= 1 || MAX_RANDOM_RANGE < x - 1)
	{
		return iLow;
	}

	// The following maps a uniform distribution on the interval [0,MAX_RANDOM_RANGE]
	// to a smaller, client-specified range of [0,x-1] in a way that doesn't bias
	// the uniform distribution unfavorably. Even for a worst case x, the loop is
	// guaranteed to be taken no more than half the time, so for that worst case x,
	// the average number of times through the loop is 2. For cases where x is
	// much smaller than MAX_RANDOM_RANGE, the average number of times through the
	// loop is very close to 1.
	//
	maxAcceptable = MAX_RANDOM_RANGE - ((MAX_RANDOM_RANGE + 1) % x);
	do
	{
		n = GenerateRandomNumber();
	} while (n > maxAcceptable);

	return iLow + (n % x);
}


//-----------------------------------------------------------------------------
//
// Implementation of the gaussian random number stream
// We're gonna use the Box-Muller method (which actually generates 2
// gaussian-distributed numbers at once)
//
//-----------------------------------------------------------------------------
CGaussianRandomStream::CGaussianRandomStream(IUniformRandomStream *pUniformStream)
{
	AttachToStream(pUniformStream);
}


//-----------------------------------------------------------------------------
// Attaches to a random uniform stream
//-----------------------------------------------------------------------------
void CGaussianRandomStream::AttachToStream(IUniformRandomStream *pUniformStream)
{
	m_mutex.Lock();
	m_pUniformStream = pUniformStream;
	m_bHaveValue = false;
	m_mutex.Unlock();
}


//-----------------------------------------------------------------------------
// Generates random numbers
//-----------------------------------------------------------------------------
float CGaussianRandomStream::RandomFloat(float flMean, float flStdDev)
{
	m_mutex.Lock();
	IUniformRandomStream *pUniformStream = m_pUniformStream ? m_pUniformStream : s_pUniformStream;
	float fac, rsq, v1, v2;

	if (!m_bHaveValue)
	{
		// Pick 2 random #s from -1 to 1
		// Make sure they lie inside the unit circle. If they don't, try again
		do
		{
			v1 = 2.0f * pUniformStream->RandomFloat() - 1.0f;
			v2 = 2.0f * pUniformStream->RandomFloat() - 1.0f;
			rsq = v1 * v1 + v2 * v2;
		} while ((rsq > 1.0f) || (rsq == 0.0f));

		// The box-muller transformation to get the two gaussian numbers
		fac = sqrtf(-2.0f * log(rsq) / rsq);

		// Store off one value for later use
		m_flRandomValue = v1 * fac;
		m_bHaveValue = true;

		m_mutex.Unlock();
		return flStdDev * (v2 * fac) + flMean;
	}
	else
	{
		m_bHaveValue = false;

		m_mutex.Unlock();
		return flStdDev * m_flRandomValue + flMean;
	}
}

