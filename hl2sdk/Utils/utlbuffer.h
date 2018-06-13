#pragma once
#include "../definitions.h"
#include "../Structs/netprop.h"
#include "utlmemory.h"
#include <Windows.h>

/*
typedef enum _fieldtypes
{
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions

	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *

	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)

	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)

	// NOTE: Use float arrays for local transformations that don't need to be fixed up.
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)

	FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX,		// a model index
	FIELD_MATERIALINDEX,	// a material index (using the material precache string table)

	FIELD_VECTOR2D,			// 2 floats
	FIELD_INTEGER64,		// 64bit integer


	FIELD_TYPECOUNT,		// MUST BE LAST
} fieldtype_t;

enum
{
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,

	// Must be last
	TD_OFFSET_COUNT,
};

struct datamap_t;
struct typedescription_t
{
	fieldtype_t			fieldType;
	const char			*fieldName;
	int					fieldOffset; // Local offset value
	unsigned short		fieldSize;
	short				flags;
	// the name of the variable in the map/fgd data, or the name of the action
	const char			*externalName;
	// pointer to the function set for save/restoring of custom data types
	void		*pSaveRestoreOps;
	// for associating function with string names
	void*			inputFunc;
	// For embedding additional datatables inside this one
	datamap_t			*td;

	// Stores the actual member variable size in bytes
	int					fieldSizeInBytes;

	// FTYPEDESC_OVERRIDE point to first baseclass instance if chains_validated has occurred
	struct typedescription_t *override_field;

	// Used to track exclusion of baseclass fields
	int					override_count;

	// Tolerance for field errors for float fields
	float				fieldTolerance;

	// For raw fields (including children of embedded stuff) this is the flattened offset
	int					flatOffset[TD_OFFSET_COUNT];
	unsigned short		flatGroup;
};

struct datamap_t
{
	typedescription_t	*dataDesc;
	int					dataNumFields;
	char const			*dataClassName;
	datamap_t			*baseMap;

	int					m_nPackedSize;
	void	*m_pOptimizedDataMap;
};
*/

class CByteswap
{
public:
	CByteswap()
	{
		// Default behavior sets the target endian to match the machine native endian (no swap).
		SetTargetBigEndian(IsMachineBigEndian());
	}

	//-----------------------------------------------------------------------------
	// Write a single field.
	//-----------------------------------------------------------------------------
	void SwapFieldToTargetEndian(void* pOutputBuffer, void *pData, typedescription_t *pField);

	//-----------------------------------------------------------------------------
	// Write a block of fields.  Works a bit like the saverestore code.  
	//-----------------------------------------------------------------------------
	void SwapFieldsToTargetEndian(void *pOutputBuffer, void *pBaseData, datamap_t *pDataMap);

	// Swaps fields for the templated type to the output buffer.
	template<typename T> inline void SwapFieldsToTargetEndian(T* pOutputBuffer, void *pBaseData, unsigned int objectCount = 1)
	{
		for (unsigned int i = 0; i < objectCount; ++i, ++pOutputBuffer)
		{
			SwapFieldsToTargetEndian((void*)pOutputBuffer, pBaseData, &T::m_DataMap);
			pBaseData = (byte*)pBaseData + sizeof(T);
		}
	}

	// Swaps fields for the templated type in place.
	template<typename T> inline void SwapFieldsToTargetEndian(T* pOutputBuffer, unsigned int objectCount = 1)
	{
		SwapFieldsToTargetEndian<T>(pOutputBuffer, (void*)pOutputBuffer, objectCount);
	}

	//-----------------------------------------------------------------------------
	// True if the current machine is detected as big endian. 
	// (Endienness is effectively detected at compile time when optimizations are
	// enabled)
	//-----------------------------------------------------------------------------
	static bool IsMachineBigEndian()
	{
		short nIsBigEndian = 1;

		// if we are big endian, the first byte will be a 0, if little endian, it will be a one.
		return (bool)(0 == *(char *)&nIsBigEndian);
	}

	//-----------------------------------------------------------------------------
	// Sets the target byte ordering we are swapping to or from.
	//
	// Braindead Endian Reference:
	//		x86 is LITTLE Endian
	//		PowerPC is BIG Endian
	//-----------------------------------------------------------------------------
	inline void SetTargetBigEndian(bool bigEndian)
	{
		m_bBigEndian = bigEndian;
		m_bSwapBytes = IsMachineBigEndian() != bigEndian;
	}

	// Changes target endian
	inline void FlipTargetEndian(void)
	{
		m_bSwapBytes = !m_bSwapBytes;
		m_bBigEndian = !m_bBigEndian;
	}

	// Forces byte swapping state, regardless of endianess
	inline void ActivateByteSwapping(bool bActivate)
	{
		SetTargetBigEndian(IsMachineBigEndian() != bActivate);
	}

	//-----------------------------------------------------------------------------
	// Returns true if the target machine is the same as this one in endianness.
	//
	// Used to determine when a byteswap needs to take place.
	//-----------------------------------------------------------------------------
	inline bool IsSwappingBytes(void)	// Are bytes being swapped?
	{
		return m_bSwapBytes;
	}

	inline bool IsTargetBigEndian(void)	// What is the current target endian?
	{
		return m_bBigEndian;
	}

	//-----------------------------------------------------------------------------
	// IsByteSwapped()
	//
	// When supplied with a chunk of input data and a constant or magic number
	// (in native format) determines the endienness of the current machine in
	// relation to the given input data.
	//
	// Returns:
	//		1  if input is the same as nativeConstant.
	//		0  if input is byteswapped relative to nativeConstant.
	//		-1 if input is not the same as nativeConstant and not byteswapped either.
	//
	// ( This is useful for detecting byteswapping in magic numbers in structure 
	// headers for example. )
	//-----------------------------------------------------------------------------
	template<typename T> inline int SourceIsNativeEndian(T input, T nativeConstant)
	{
		// If it's the same, it isn't byteswapped:
		if (input == nativeConstant)
			return 1;

		int output;
		LowLevelByteSwap<T>(&output, &input);
		if (output == nativeConstant)
			return 0;

		Assert(0);		// if we get here, input is neither a swapped nor unswapped version of nativeConstant.
		return -1;
	}

	//-----------------------------------------------------------------------------
	// Swaps an input buffer full of type T into the given output buffer.
	//
	// Swaps [count] items from the inputBuffer to the outputBuffer.
	// If inputBuffer is omitted or NULL, then it is assumed to be the same as
	// outputBuffer - effectively swapping the contents of the buffer in place.
	//-----------------------------------------------------------------------------
	template<typename T> inline void SwapBuffer(T* outputBuffer, T* inputBuffer = NULL, int count = 1)
	{
		Assert(count >= 0);
		Assert(outputBuffer);

		// Fail gracefully in release:
		if (count <= 0 || !outputBuffer)
			return;

		// Optimization for the case when we are swapping in place.
		if (inputBuffer == NULL)
		{
			inputBuffer = outputBuffer;
		}

		// Swap everything in the buffer:
		for (int i = 0; i < count; i++)
		{
			LowLevelByteSwap<T>(&outputBuffer[i], &inputBuffer[i]);
		}
	}

	//-----------------------------------------------------------------------------
	// Swaps an input buffer full of type T into the given output buffer.
	//
	// Swaps [count] items from the inputBuffer to the outputBuffer.
	// If inputBuffer is omitted or NULL, then it is assumed to be the same as
	// outputBuffer - effectively swapping the contents of the buffer in place.
	//-----------------------------------------------------------------------------
	template<typename T> inline void SwapBufferToTargetEndian(T* outputBuffer, T* inputBuffer = NULL, int count = 1)
	{
		Assert(count >= 0);
		Assert(outputBuffer);

		// Fail gracefully in release:
		if (count <= 0 || !outputBuffer)
			return;

		// Optimization for the case when we are swapping in place.
		if (inputBuffer == NULL)
		{
			inputBuffer = outputBuffer;
		}

		// Are we already the correct endienness? ( or are we swapping 1 byte items? )
		if (!m_bSwapBytes || (sizeof(T) == 1))
		{
			// If we were just going to swap in place then return.
			if (!inputBuffer)
				return;

			// Otherwise copy the inputBuffer to the outputBuffer:
			memcpy(outputBuffer, inputBuffer, count * sizeof(T));
			return;

		}

		// Swap everything in the buffer:
		for (int i = 0; i < count; i++)
		{
			LowLevelByteSwap<T>(&outputBuffer[i], &inputBuffer[i]);
		}
	}

private:
	//-----------------------------------------------------------------------------
	// The lowest level byte swapping workhorse of doom.  output always contains the 
	// swapped version of input.  ( Doesn't compare machine to target endianness )
	//-----------------------------------------------------------------------------
	template<typename T> static void LowLevelByteSwap(T *output, T *input)
	{
		T temp = *output;
#if defined( _X360 )
		// Intrinsics need the source type to be fixed-point
		DWORD* word = (DWORD*)input;
		switch (sizeof(T))
		{
		case 8:
		{
			__storewordbytereverse(*word, 0, &temp);
			__storewordbytereverse(*(word + 1), 4, &temp);
		}
		break;

		case 4:
			__storewordbytereverse(*word, 0, &temp);
			break;

		case 2:
			__storeshortbytereverse(*input, 0, &temp);
			break;

		default:
			Assert("Invalid size in CByteswap::LowLevelByteSwap" && 0);
		}
#else
		for (int i = 0; i < (int)sizeof(T); i++)
		{
			((unsigned char*)&temp)[i] = ((unsigned char*)input)[sizeof(T) - (i + 1)];
		}
#endif
		memcpy(output, &temp, sizeof(T));
	}

	unsigned int m_bSwapBytes : 1;
	unsigned int m_bBigEndian : 1;
};

//-----------------------------------------------------------------------------
// Description of character conversions for string output
// Here's an example of how to use the macros to define a character conversion
// BEGIN_CHAR_CONVERSION( CStringConversion, '\\' )
//	{ '\n', "n" },
//	{ '\t', "t" }
// END_CHAR_CONVERSION( CStringConversion, '\\' )
//-----------------------------------------------------------------------------
class CUtlCharConversion
{
public:
	struct ConversionArray_t
	{
		char m_nActualChar;
		const char *m_pReplacementString;
	};

	CUtlCharConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray);
	char GetEscapeChar() const;
	const char *GetDelimiter() const;
	int GetDelimiterLength() const;

	const char *GetConversionString(char c) const;
	int GetConversionLength(char c) const;
	int MaxConversionLength() const;

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion(const char *pString, int *pLength);

protected:
	struct ConversionInfo_t
	{
		int m_nLength;
		const char *m_pReplacementString;
	};

	char m_nEscapeChar;
	const char *m_pDelimiter;
	int m_nDelimiterLength;
	int m_nCount;
	int m_nMaxConversionLength;
	char m_pList[255];
	ConversionInfo_t m_pReplacements[255];
};

//-----------------------------------------------------------------------------
// Character conversions for C strings
//-----------------------------------------------------------------------------
class CUtlCStringConversion : public CUtlCharConversion
{
public:
	CUtlCStringConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray);

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion(const char *pString, int *pLength);

private:
	char m_pConversion[255];
};


//-----------------------------------------------------------------------------
// Character conversions for no-escape sequence strings
//-----------------------------------------------------------------------------
class CUtlNoEscConversion : public CUtlCharConversion
{
public:
	CUtlNoEscConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray) :
		CUtlCharConversion(nEscapeChar, pDelimiter, nCount, pArray) {}

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion(const char *pString, int *pLength) { *pLength = 0; return 0; }
};

struct characterset_t
{
	char set[256];
};

//-----------------------------------------------------------------------------
// Macro to set overflow functions easily
//-----------------------------------------------------------------------------
#define SetUtlBufferOverflowFuncs( _get, _put )	\
SetOverflowFuncs( static_cast <UtlBufferOverflowFunc_t>( _get ), static_cast <UtlBufferOverflowFunc_t>( _put ) )
#undef GetObject

//-----------------------------------------------------------------------------
// Command parsing..
//-----------------------------------------------------------------------------
class CUtlBuffer
{
public:
	enum SeekType_t
	{
		SEEK_HEAD = 0,
		SEEK_CURRENT,
		SEEK_TAIL
	};

	// flags
	enum BufferFlags_t
	{
		TEXT_BUFFER = 0x1,			// Describes how get + put work (as strings, or binary)
		EXTERNAL_GROWABLE = 0x2,	// This is used w/ external buffers and causes the utlbuf to switch to reallocatable memory if an overflow happens when Putting.
		CONTAINS_CRLF = 0x4,		// For text buffers only, does this contain \n or \n\r?
		READ_ONLY = 0x8,			// For external buffers; prevents null termination from happening.
		AUTO_TABS_DISABLED = 0x10,	// Used to disable/enable push/pop tabs
	};

	// Overflow functions when a get or put overflows
	typedef bool (CUtlBuffer::*UtlBufferOverflowFunc_t)(int nSize);

	// Constructors for growable + external buffers for serialization/unserialization
	CUtlBuffer(int growSize = 0, int initSize = 0, int nFlags = 0);
	CUtlBuffer(const void* pBuffer, int size, int nFlags = 0);
	// This one isn't actually defined so that we catch contructors that are trying to pass a bool in as the third param.
	CUtlBuffer(const void *pBuffer, int size, bool crap);

	unsigned char	GetFlags() const;

	// NOTE: This will assert if you attempt to recast it in a way that
	// is not compatible. The only valid conversion is binary-> text w/CRLF
	void			SetBufferType(bool bIsText, bool bContainsCRLF);

	// Makes sure we've got at least this much memory
	void			EnsureCapacity(int num);

	// Attaches the buffer to external memory....
	void			SetExternalBuffer(void* pMemory, int nSize, int nInitialPut, int nFlags = 0);
	bool			IsExternallyAllocated() const;
	void			AssumeMemory(void *pMemory, int nSize, int nInitialPut, int nFlags = 0);

	inline void ActivateByteSwappingIfBigEndian(void)
	{

	}


	// Controls endian-ness of binary utlbufs - default matches the current platform
	void			ActivateByteSwapping(bool bActivate);
	void			SetBigEndian(bool bigEndian);
	bool			IsBigEndian(void);

	// Resets the buffer; but doesn't free memory
	void			Clear();

	// Clears out the buffer; frees memory
	void			Purge();

	// Read stuff out.
	// Binary mode: it'll just read the bits directly in, and characters will be
	//		read for strings until a null character is reached.
	// Text mode: it'll parse the file, turning text #s into real numbers.
	//		GetString will read a string until a space is reached
	char			GetChar();
	unsigned char	GetUnsignedChar();
	short			GetShort();
	unsigned short	GetUnsignedShort();
	int				GetInt();
	int				GetIntHex();
	unsigned int	GetUnsignedInt();
	float			GetFloat();
	double			GetDouble();
	void			GetString(char* pString, int nMaxChars = 0);
	void			Get(void* pMem, int size);
	void			GetLine(char* pLine, int nMaxChars = 0);
	int64_t			GetInt64();

	// Used for getting objects that have a byteswap datadesc defined
	template <typename T> void GetObjects(T *dest, int count = 1);

	// This will get at least 1 byte and up to nSize bytes. 
	// It will return the number of bytes actually read.
	int				GetUpTo(void *pMem, int nSize);

	// This version of GetString converts \" to \\ and " to \, etc.
	// It also reads a " at the beginning and end of the string
	void			GetDelimitedString(CUtlCharConversion *pConv, char *pString, int nMaxChars = 0);
	char			GetDelimitedChar(CUtlCharConversion *pConv);

	// This will return the # of characters of the string about to be read out
	// NOTE: The count will *include* the terminating 0!!
	// In binary mode, it's the number of characters until the next 0
	// In text mode, it's the number of characters until the next space.
	int				PeekStringLength();

	// This version of PeekStringLength converts \" to \\ and " to \, etc.
	// It also reads a " at the beginning and end of the string
	// NOTE: The count will *include* the terminating 0!!
	// In binary mode, it's the number of characters until the next 0
	// In text mode, it's the number of characters between "s (checking for \")
	// Specifying false for bActualSize will return the pre-translated number of characters
	// including the delimiters and the escape characters. So, \n counts as 2 characters when bActualSize == false
	// and only 1 character when bActualSize == true
	int				PeekDelimitedStringLength(CUtlCharConversion *pConv, bool bActualSize = true);

	// Just like scanf, but doesn't work in binary mode
	int				Scanf(const char* pFmt, ...);
	int				VaScanf(const char* pFmt, va_list list);

	// Eats white space, advances Get index
	void			EatWhiteSpace();

	// Eats C++ style comments
	bool			EatCPPComment();

	// (For text buffers only)
	// Parse a token from the buffer:
	// Grab all text that lies between a starting delimiter + ending delimiter
	// (skipping whitespace that leads + trails both delimiters).
	// If successful, the get index is advanced and the function returns true,
	// otherwise the index is not advanced and the function returns false.
	bool			ParseToken(const char *pStartingDelim, const char *pEndingDelim, char* pString, int nMaxLen);

	// Advance the get index until after the particular string is found
	// Do not eat whitespace before starting. Return false if it failed
	// String test is case-insensitive.
	bool			GetToken(const char *pToken);

	// Parses the next token, given a set of character breaks to stop at
	// Returns the length of the token parsed in bytes (-1 if none parsed)
	int				ParseToken(characterset_t *pBreaks, char *pTokenBuf, int nMaxLen, bool bParseComments = true);

	// Write stuff in
	// Binary mode: it'll just write the bits directly in, and strings will be
	//		written with a null terminating character
	// Text mode: it'll convert the numbers to text versions
	//		PutString will not write a terminating character
	void			PutChar(char c);
	void			PutUnsignedChar(unsigned char uc);
	void			PutShort(short s);
	void			PutUnsignedShort(unsigned short us);
	void			PutInt(int i);
	void			PutUnsignedInt(unsigned int u);
	void			PutFloat(float f);
	void			PutDouble(double d);
	void			PutString(const char* pString);
	void			Put(const void* pMem, int size);

	// Used for putting objects that have a byteswap datadesc defined
	template <typename T> void PutObjects(T *src, int count = 1);

	// This version of PutString converts \ to \\ and " to \", etc.
	// It also places " at the beginning and end of the string
	void			PutDelimitedString(CUtlCharConversion *pConv, const char *pString);
	void			PutDelimitedChar(CUtlCharConversion *pConv, char c);

	// Just like printf, writes a terminating zero in binary mode
	void			Printf(const char* pFmt, ...);
	void			VaPrintf(const char* pFmt, va_list list);

	// What am I writing (put)/reading (get)?
	void* PeekPut(int offset = 0);
	const void* PeekGet(int offset = 0) const;
	const void* PeekGet(int nMaxSize, int nOffset);

	// Where am I writing (put)/reading (get)?
	int TellPut() const;
	int TellGet() const;

	// What's the most I've ever written?
	int TellMaxPut() const;

	// How many bytes remain to be read?
	// NOTE: This is not accurate for streaming text files; it overshoots
	int GetBytesRemaining() const;

	// Change where I'm writing (put)/reading (get)
	void SeekPut(SeekType_t type, int offset);
	void SeekGet(SeekType_t type, int offset);

	// Buffer base
	const void* Base() const;
	void* Base();

	// memory allocation size, does *not* reflect size written or read,
	//	use TellPut or TellGet for that
	int Size() const;

	// Am I a text buffer?
	bool IsText() const;

	// Can I grow if I'm externally allocated?
	bool IsGrowable() const;

	// Am I valid? (overflow or underflow error), Once invalid it stays invalid
	bool IsValid() const;

	// Do I contain carriage return/linefeeds? 
	bool ContainsCRLF() const;

	// Am I read-only
	bool IsReadOnly() const;

	// Converts a buffer from a CRLF buffer to a CR buffer (and back)
	// Returns false if no conversion was necessary (and outBuf is left untouched)
	// If the conversion occurs, outBuf will be cleared.
	bool ConvertCRLF(CUtlBuffer &outBuf);

	// Push/pop pretty-printing tabs
	void PushTab();
	void PopTab();

	// Temporarily disables pretty print
	void EnableTabs(bool bEnable);

protected:
	// error flags
	enum
	{
		PUT_OVERFLOW = 0x1,
		GET_OVERFLOW = 0x2,
		MAX_ERROR_FLAG = GET_OVERFLOW,
	};

	void SetOverflowFuncs(UtlBufferOverflowFunc_t getFunc, UtlBufferOverflowFunc_t putFunc);

	bool OnPutOverflow(int nSize);
	bool OnGetOverflow(int nSize);

protected:
	// Checks if a get/put is ok
	bool CheckPut(int size);
	bool CheckGet(int size);

	void AddNullTermination();

	// Methods to help with pretty-printing
	bool WasLastCharacterCR();
	void PutTabs();

	// Help with delimited stuff
	char GetDelimitedCharInternal(CUtlCharConversion *pConv);
	void PutDelimitedCharInternal(CUtlCharConversion *pConv, char c);

	// Default overflow funcs
	bool PutOverflow(int nSize);
	bool GetOverflow(int nSize);

	// Does the next bytes of the buffer match a pattern?
	bool PeekStringMatch(int nOffset, const char *pString, int nLen);

	// Peek size of line to come, check memory bound
	int	PeekLineLength();

	// How much whitespace should I skip?
	int PeekWhiteSpace(int nOffset);

	// Checks if a peek get is ok
	bool CheckPeekGet(int nOffset, int nSize);

	// Call this to peek arbitrarily long into memory. It doesn't fail unless
	// it can't read *anything* new
	bool CheckArbitraryPeekGet(int nOffset, int &nIncrement);

	template <typename T> void GetType(T& dest, const char *pszFmt);
	template <typename T> void GetTypeBin(T& dest);
	template <typename T> void GetObject(T *src);

	template <typename T> void PutType(T src, const char *pszFmt);
	template <typename T> void PutTypeBin(T src);
	template <typename T> void PutObject(T *src);

	CUtlMemory<unsigned char> m_Memory;
	int m_Get;
	int m_Put;

	unsigned char m_Error;
	unsigned char m_Flags;
	unsigned char m_Reserved;
#if defined( _X360 )
	unsigned char pad;
#endif

	int m_nTab;
	int m_nMaxPut;
	int m_nOffset;

	UtlBufferOverflowFunc_t m_GetOverflowFunc;
	UtlBufferOverflowFunc_t m_PutOverflowFunc;

	CByteswap	m_Byteswap;
};

//---------------------------------------------------------------------------
// Implementation of CUtlInplaceBuffer
//---------------------------------------------------------------------------
class CUtlInplaceBuffer : public CUtlBuffer
{
public:
	CUtlInplaceBuffer(int growSize = 0, int initSize = 0, int nFlags = 0);

	//
	// Routines returning buffer-inplace-pointers
	//
public:
	//
	// Upon success, determines the line length, fills out the pointer to the
	// beginning of the line and the line length, advances the "get" pointer
	// offset by the line length and returns "true".
	//
	// If end of file is reached or upon error returns "false".
	//
	// Note:	the returned length of the line is at least one character because the
	//			trailing newline characters are also included as part of the line.
	//
	// Note:	the pointer returned points into the local memory of this buffer, in
	//			case the buffer gets relocated or destroyed the pointer becomes invalid.
	//
	// e.g.:	-------------
	//
	//			char *pszLine;
	//			int nLineLen;
	//			while ( pUtlInplaceBuffer->InplaceGetLinePtr( &pszLine, &nLineLen ) )
	//			{
	//				...
	//			}
	//
	//			-------------
	//
	// @param	ppszInBufferPtr		on return points into this buffer at start of line
	// @param	pnLineLength		on return holds num bytes accessible via (*ppszInBufferPtr)
	//
	// @returns	true				if line was successfully read
	//			false				when EOF is reached or error occurs
	//
	bool InplaceGetLinePtr( /* out */ char **ppszInBufferPtr, /* out */ int *pnLineLength);

	//
	// Determines the line length, advances the "get" pointer offset by the line length,
	// replaces the newline character with null-terminator and returns the initial pointer
	// to now null-terminated line.
	//
	// If end of file is reached or upon error returns NULL.
	//
	// Note:	the pointer returned points into the local memory of this buffer, in
	//			case the buffer gets relocated or destroyed the pointer becomes invalid.
	//
	// e.g.:	-------------
	//
	//			while ( char *pszLine = pUtlInplaceBuffer->InplaceGetLinePtr() )
	//			{
	//				...
	//			}
	//
	//			-------------
	//
	// @returns	ptr-to-zero-terminated-line		if line was successfully read and buffer modified
	//			NULL							when EOF is reached or error occurs
	//
	char * InplaceGetLinePtr(void);
};

CUtlCharConversion *GetCStringCharConversion();
CUtlCharConversion *GetNoEscCharConversion();
