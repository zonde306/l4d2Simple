#pragma once

#include <cstdint>

template <typename T>
inline T WordSwapAsm(T w)
{
	__asm
	{
		mov ax, w
		xchg al, ah
	}
}

template <typename T>
inline T DWordSwapAsm(T dw)
{
	__asm
	{
		mov eax, dw
		bswap eax
	}
}

#define IsPC() true

#define BigShort(val) WordSwap(val)
#define BigWord(val) WordSwap(val)
#define BigLong(val) DWordSwap(val)
#define BigDWord(val) DWordSwap(val)

#define LittleShort(val) (val)
#define LittleWord(val) (val)
#define LittleLong(val) (val)
#define LittleDWord(val) (val)
#define LittleQWord(val) (val)

#define SwapShort(val) BigShort(val)
#define SwapWord(val) BigWord(val)
#define SwapLong(val) BigLong(val)
#define SwapDWord(val) BigDWord(val)

#define BigFloat(pOut, pIn) SafeSwapFloat(pOut, pIn)
#define LittleFloat(pOut, pIn) (*pOut = *pIn)
#define SwapFloat(pOut, pIn) BigFloat(pOut, pIn)

__forceinline unsigned long LoadLittleDWord(const unsigned long *base, unsigned int dwordIndex)
{
	return LittleDWord(base[dwordIndex]);
}

__forceinline void StoreLittleDWord(unsigned long *base, unsigned int dwordIndex, unsigned long dword)
{
	base[dwordIndex] = LittleDWord(dword);
}

typedef enum
{
	BITBUFERROR_VALUE_OUT_OF_RANGE = 0,
	BITBUFERROR_BUFFER_OVERRUN,
	BITBUFERROR_NUM_ERRORS
} BitBufErrorType;

typedef void(*BitBufErrorHandler)(BitBufErrorType errorType, const char *pDebugName);

#if defined( _DEBUG )
extern void InternalBitBufErrorHandler(BitBufErrorType errorType, const char *pDebugName);
#define CallErrorHandler( errorType, pDebugName ) InternalBitBufErrorHandler( errorType, pDebugName );
#else
#define CallErrorHandler( errorType, pDebugName ) ((void)0)
#endif

void SetBitBufErrorHandler(BitBufErrorHandler fn);

inline int BitByte(int bits)
{
	return (bits + 7) >> 3;
}

namespace bitbuf
{
	inline uint32_t ZigZagEncode32(int32_t n)
	{
		return(n << 1) ^ (n >> 31);
	}

	inline int32_t ZigZagDecode32(uint32_t n)
	{
		return(n >> 1) ^ -static_cast<int32_t>(n & 1);
	}

	inline uint64_t ZigZagEncode64(int64_t n)
	{
		return(n << 1) ^ (n >> 63);
	}

	inline int64_t ZigZagDecode64(uint64_t n)
	{
		return(n >> 1) ^ -static_cast<int64_t>(n & 1);
	}

	const int kMaxVarintBytes = 10;
	const int kMaxVarint32Bytes = 5;
}

class Vector;

class bf_write
{
public:
	bf_write();

	bf_write(void* pData, int nBytes, int nMaxBits = -1);
	bf_write(const char* pDebugName, void* pData, int nBytes, int nMaxBits = -1);

	void StartWriting(void* pData, int nBytes, int iStartBit = 0, int nMaxBits = -1);
	void Reset();

	unsigned char* GetBasePointer()
	{
		return (unsigned char*)m_pData;
	}

	void SetAssertOnOverflow(bool bAssert);

	const char* GetDebugName();
	void SetDebugName(const char* pDebugName);

public:
	void SeekToBit(int bitPos);

public:
	void WriteOneBit(int nValue);
	void WriteOneBitNoCheck(int nValue);
	void WriteOneBitAt(int iBit, int nValue);
	void WriteUBitLong(unsigned int data, int numbits, bool bCheckRange = true);
	void WriteSBitLong(int data, int numbits);
	void WriteBitLong(unsigned int data, int numbits, bool bSigned);
	bool WriteBits(const void* pIn, int nBits);
	void WriteUBitVar(unsigned int data);
	void WriteVarInt32(uint32_t data);
	void WriteVarInt64(uint64_t data);
	void WriteSignedVarInt32(int32_t data);
	void WriteSignedVarInt64(int64_t data);

	int ByteSizeVarInt32(uint32_t data);
	int ByteSizeVarInt64(uint64_t data);
	int ByteSizeSignedVarInt32(int32_t data);
	int ByteSizeSignedVarInt64(int64_t data);

	bool WriteBitsFromBuffer(class bf_read* pIn, int nBits);

	void WriteBitAngle(float fAngle, int numbits);
	void WriteBitCoord(const float f);
	void WriteBitCoordMP(const float f, bool bIntegral, bool bLowPrecision);
	void WriteBitFloat(float val);
	void WriteBitVec3Coord(const Vector& fa);
	void WriteBitNormal(float f);
	void WriteBitVec3Normal(const Vector& fa);
	void WriteBitAngles(const Vector& fa);

public:
	void WriteChar(int val);
	void WriteByte(int val);
	void WriteShort(int val);
	void WriteWord(int val);
	void WriteLong(long val);
	void WriteLongLong(int64_t val);
	void WriteFloat(float val);
	bool WriteBytes(const void* pBuf, int nBytes);
	bool WriteString(const char* pStr);

public:
	int GetNumBytesWritten() const;
	int GetNumBitsWritten() const;
	int GetMaxNumBits();
	int GetNumBitsLeft();
	int GetNumBytesLeft();

	unsigned char* GetData();
	const unsigned char* GetData() const;

	bool CheckForOverflow(int nBits);

	inline bool IsOverflowed() const
	{
		return m_bOverflow;
	}

	void SetOverflowFlag();

public:
	unsigned long* m_pData;
	int m_nDataBytes;
	int m_nDataBits;
	int m_iCurBit;

private:
	bool m_bOverflow;
	bool m_bAssertOnOverflow;
	const char* m_pDebugName;
};

class bf_read
{
public:
	bf_read();

	bf_read(const void* pData, int nBytes, int nBits = -1);
	bf_read(const char* pDebugName, const void* pData, int nBytes, int nBits = -1);

	void StartReading(const void* pData, int nBytes, int iStartBit = 0, int nBits = -1);
	void Reset();
	void SetAssertOnOverflow(bool bAssert);

	const char* GetDebugName() const
	{
		return m_pDebugName;
	}

	void SetDebugName(const char* pName);
	void ExciseBits(int startbit, int bitstoremove);

public:
	int ReadOneBit();

protected:
	unsigned int CheckReadUBitLong(int numbits);
	int ReadOneBitNoCheck();
	bool CheckForOverflow(int nBits);

public:
	const unsigned char* GetBasePointer()
	{
		return m_pData;
	}

	__forceinline int TotalBytesAvailable(void) const
	{
		return m_nDataBytes;
	}

	void ReadBits(void* pOut, int nBits);
	int ReadBitsClamped_ptr(void* pOut, size_t outSizeBytes, size_t nBits);

	template <typename T, size_t N>
	int ReadBitsClamped(T(&pOut)[N], size_t nBits)
	{
		return ReadBitsClamped_ptr(pOut, N * sizeof(T), nBits);
	}

	float ReadBitAngle(int numbits);

	unsigned int ReadUBitLong(int numbits);
	unsigned int ReadUBitLongNoInline(int numbits);
	unsigned int PeekUBitLong(int numbits);

	int ReadSBitLong(int numbits);

	unsigned int ReadUBitVar();
	unsigned int ReadUBitVarInternal(int encodingType);

	uint32_t ReadVarInt32();
	uint64_t ReadVarInt64();
	int32_t ReadSignedVarInt32();
	int64_t ReadSignedVarInt64();

	unsigned int ReadBitLong(int numbits, bool bSigned);

	float ReadBitCoord();
	float ReadBitCoordMP(bool bIntegral, bool bLowPrecision);
	float ReadBitFloat();
	float ReadBitNormal();
	void ReadBitVec3Coord(Vector& fa);
	void ReadBitVec3Normal(Vector& fa);
	void ReadBitAngles(Vector& fa);

public:
	__forceinline int ReadChar()
	{
		return (char)ReadUBitLong(8);
	}

	__forceinline int ReadByte()
	{
		return ReadUBitLong(8);
	}

	__forceinline int ReadShort()
	{
		return (short)ReadUBitLong(16);
	}

	__forceinline int ReadWord()
	{
		return ReadUBitLong(16);
	}

	__forceinline long ReadLong()
	{
		return ReadUBitLong(32);
	}

	int64_t ReadLongLong();
	float ReadFloat();
	bool ReadBytes(void* pOut, int nBytes);
	bool ReadString(char* pStr, int bufLen, bool bLine = false, int* pOutNumChars = NULL);
	char* ReadAndAllocateString(bool* pOverflow = 0);

	int CompareBits(bf_read* other, int bits);
	int CompareBitsAt(int offset, bf_read* other, int otherOffset, int bits);

public:
	int GetNumBytesLeft();
	int GetNumBytesRead();
	int GetNumBitsLeft();
	int GetNumBitsRead() const;

	inline bool IsOverflowed() const
	{
		return m_bOverflow;
	}

	inline bool Seek(int iBit);
	inline bool SeekRelative(int iBitDelta);

	void SetOverflowFlag();


public:
	const unsigned char* m_pData;
	int m_nDataBytes;
	int m_nDataBits;
	int m_iCurBit;

private:
	bool m_bOverflow;
	bool m_bAssertOnOverflow;
	const char* m_pDebugName;
};