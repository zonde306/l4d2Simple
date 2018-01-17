#include "utlstring.h"
#include "../definitions.h"
#include <cstdio>
#include <direct.h>

//-----------------------------------------------------------------------------
// Simple string class. 
//-----------------------------------------------------------------------------

#ifndef min
#define min(_l,_r)		(_l < _r ? _l : _r)
#endif

//-----------------------------------------------------------------------------
// Purpose: Changes all '/' or '\' characters into separator
// Input  : *pname - 
//			separator - 
//-----------------------------------------------------------------------------
void V_FixSlashes(char *pname, char separator /* = CORRECT_PATH_SEPARATOR */)
{
	while (*pname)
	{
		if (*pname == INCORRECT_PATH_SEPARATOR || *pname == CORRECT_PATH_SEPARATOR)
		{
			*pname = separator;
		}
		pname++;
	}
}



//-----------------------------------------------------------------------------
// Either allocates or reallocates memory to the length
//
// Allocated space for length characters.  It automatically adds space for the 
// nul and the cached length at the start of the memory block.  Will adjust
// m_pString and explicitly set the nul at the end before returning.
void *CUtlString::AllocMemory(uint32_t length)
{
	void *pMemoryBlock;
	if (m_pString)
	{
		pMemoryBlock = realloc(m_pString, length + 1);
	}
	else
	{
		pMemoryBlock = malloc(length + 1);
	}
	m_pString = (char*)pMemoryBlock;
	m_pString[length] = 0;

	return pMemoryBlock;
}

//-----------------------------------------------------------------------------
void CUtlString::SetDirect(const char *pValue, int nChars)
{
	if (pValue && nChars > 0)
	{
		if (pValue == m_pString)
		{
			// AssertMsg(nChars == Q_strlen(m_pString), "CUtlString::SetDirect does not support resizing strings in place.");
			return; // Do nothing. Realloc in AllocMemory might move pValue's location resulting in a bad memcpy.
		}

		// Assert(nChars <= Min<int>(strnlen(pValue, nChars) + 1, nChars));
		AllocMemory(nChars);
		memcpy(m_pString, pValue, nChars);
	}
	else
	{
		Purge();
	}

}


void CUtlString::Set(const char *pValue)
{
	int length = pValue ? strlen(pValue) : 0;
	SetDirect(pValue, length);
}

// Sets the length (used to serialize into the buffer )
void CUtlString::SetLength(int nLen)
{
	if (nLen > 0)
	{
#ifdef _DEBUG_OUTPUT
		int prevLen = m_pString ? Length() : 0;
#endif
		AllocMemory(nLen);
#ifdef _DEBUG_OUTPUT
		if (nLen > prevLen)
		{
			memset(m_pString + prevLen, 0xEB, nLen - prevLen);
		}
#endif
	}
	else
	{
		Purge();
	}
}

const char *CUtlString::Get() const
{
	if (!m_pString)
	{
		return "";
	}
	return m_pString;
}

char *CUtlString::GetForModify()
{
	if (!m_pString)
	{
		// In general, we optimise away small mallocs for empty strings
		// but if you ask for the non-const bytes, they must be writable
		// so we can't return "" here, like we do for the const version - jd
		void *pMemoryBlock = malloc(1);
		m_pString = (char *)pMemoryBlock;
		*m_pString = 0;
	}

	return m_pString;
}

char CUtlString::operator[](int i) const
{
	if (!m_pString)
		return '\0';

	if (i >= Length())
	{
		return '\0';
	}

	return m_pString[i];
}

void CUtlString::Clear()
{
	Purge();
}

void CUtlString::Purge()
{
	free(m_pString);
	m_pString = NULL;
}

bool CUtlString::IsEqual_CaseSensitive(const char *src) const
{
	if (!src)
	{
		return (Length() == 0);
	}
	return (strcmp(Get(), src) == 0);
}

bool CUtlString::IsEqual_CaseInsensitive(const char *src) const
{
	if (!src)
	{
		return (Length() == 0);
	}
	return (_stricmp(Get(), src) == 0);
}


void CUtlString::ToLower()
{
	if (!m_pString)
	{
		return;
	}

	_strlwr(m_pString);
}

void CUtlString::ToUpper()
{
	if (!m_pString)
	{
		return;
	}

	_strupr(m_pString);
}

CUtlString &CUtlString::operator=(const CUtlString &src)
{
	SetDirect(src.Get(), src.Length());
	return *this;
}

CUtlString &CUtlString::operator=(const char *src)
{
	Set(src);
	return *this;
}

bool CUtlString::operator==(const CUtlString &src) const
{
	if (IsEmpty())
	{
		if (src.IsEmpty())
		{
			return true;
		}

		return false;
	}
	else
	{
		if (src.IsEmpty())
		{
			return false;
		}

		return strcmp(m_pString, src.m_pString) == 0;
	}
}

CUtlString &CUtlString::operator+=(const CUtlString &rhs)
{
	const int lhsLength(Length());
	const int rhsLength(rhs.Length());

	if (!rhsLength)
	{
		return *this;
	}

	const int requestedLength(lhsLength + rhsLength);

	AllocMemory(requestedLength);
	memcpy(m_pString + lhsLength, rhs.m_pString, rhsLength);

	return *this;
}

CUtlString &CUtlString::operator+=(const char *rhs)
{
	const int lhsLength(Length());
	const int rhsLength(strlen(rhs));
	const int requestedLength(lhsLength + rhsLength);

	if (!requestedLength)
	{
		return *this;
	}

	AllocMemory(requestedLength);
	memcpy(m_pString + lhsLength, rhs, rhsLength);

	return *this;
}

CUtlString &CUtlString::operator+=(char c)
{
	const int lhsLength(Length());

	AllocMemory(lhsLength + 1);
	m_pString[lhsLength] = c;

	return *this;
}

CUtlString &CUtlString::operator+=(int rhs)
{
	// Assert(sizeof(rhs) == 4);

	char tmpBuf[12];	// Sufficient for a signed 32 bit integer [ -2147483648 to +2147483647 ]
	snprintf(tmpBuf, sizeof(tmpBuf), "%d", rhs);
	tmpBuf[sizeof(tmpBuf) - 1] = '\0';

	return operator+=(tmpBuf);
}

CUtlString &CUtlString::operator+=(double rhs)
{
	char tmpBuf[256];	// How big can doubles be???  Dunno.
	snprintf(tmpBuf, sizeof(tmpBuf), "%lg", rhs);
	tmpBuf[sizeof(tmpBuf) - 1] = '\0';

	return operator+=(tmpBuf);
}

bool CUtlString::MatchesPattern(const CUtlString &Pattern, int nFlags) const
{
	const char *pszSource = String();
	const char *pszPattern = Pattern.String();
	bool	bExact = true;

	while (1)
	{
		if ((*pszPattern) == 0)
		{
			return ((*pszSource) == 0);
		}

		if ((*pszPattern) == '*')
		{
			pszPattern++;

			if ((*pszPattern) == 0)
			{
				return true;
			}

			bExact = false;
			continue;
		}

		int nLength = 0;

		while ((*pszPattern) != '*' && (*pszPattern) != 0)
		{
			nLength++;
			pszPattern++;
		}

		while (1)
		{
			const char *pszStartPattern = pszPattern - nLength;
			const char *pszSearch = pszSource;

			for (int i = 0; i < nLength; i++, pszSearch++, pszStartPattern++)
			{
				if ((*pszSearch) == 0)
				{
					return false;
				}

				if ((*pszSearch) != (*pszStartPattern))
				{
					break;
				}
			}

			if (pszSearch - pszSource == nLength)
			{
				break;
			}

			if (bExact == true)
			{
				return false;
			}

			if ((nFlags & PATTERN_DIRECTORY) != 0)
			{
				if ((*pszPattern) != '/' && (*pszSource) == '/')
				{
					return false;
				}
			}

			pszSource++;
		}

		pszSource += nLength;
	}
}


int CUtlString::Format(const char *pFormat, ...)
{
	va_list marker;

	va_start(marker, pFormat);
	int len = FormatV(pFormat, marker);
	va_end(marker);

	return len;
}

//--------------------------------------------------------------------------------------------------
// This can be called from functions that take varargs.
//--------------------------------------------------------------------------------------------------

int CUtlString::FormatV(const char *pFormat, va_list marker)
{
	char tmpBuf[4096];	//< Nice big 4k buffer, as much memory as my first computer had, a Radio Shack Color Computer

						//va_start( marker, pFormat );
	int len = vsprintf_s(tmpBuf, pFormat, marker);
	//va_end( marker );
	Set(tmpBuf);
	return len;
}

//-----------------------------------------------------------------------------
// Strips the trailing slash
//-----------------------------------------------------------------------------
void CUtlString::StripTrailingSlash()
{
	if (IsEmpty())
		return;

	int nLastChar = Length() - 1;
	char c = m_pString[nLastChar];
	if (c == '\\' || c == '/')
	{
		SetLength(nLastChar);
	}
}

void CUtlString::FixSlashes(char cSeparator/*=CORRECT_PATH_SEPARATOR*/)
{
	if (m_pString)
	{
		V_FixSlashes(m_pString, cSeparator);
	}
}

//-----------------------------------------------------------------------------
// Trim functions
//-----------------------------------------------------------------------------
void CUtlString::TrimLeft(char cTarget)
{
	int nIndex = 0;

	if (IsEmpty())
	{
		return;
	}

	while (m_pString[nIndex] == cTarget)
	{
		++nIndex;
	}

	// We have some whitespace to remove
	if (nIndex > 0)
	{
		memcpy(m_pString, &m_pString[nIndex], Length() - nIndex);
		SetLength(Length() - nIndex);
	}
}


void CUtlString::TrimLeft(const char *szTargets)
{
	int i;

	if (IsEmpty())
	{
		return;
	}

	for (i = 0; m_pString[i] != 0; i++)
	{
		bool bWhitespace = false;

		for (int j = 0; szTargets[j] != 0; j++)
		{
			if (m_pString[i] == szTargets[j])
			{
				bWhitespace = true;
				break;
			}
		}

		if (!bWhitespace)
		{
			break;
		}
	}

	// We have some whitespace to remove
	if (i > 0)
	{
		memcpy(m_pString, &m_pString[i], Length() - i);
		SetLength(Length() - i);
	}
}


void CUtlString::TrimRight(char cTarget)
{
	const int nLastCharIndex = Length() - 1;
	int nIndex = nLastCharIndex;

	while (nIndex >= 0 && m_pString[nIndex] == cTarget)
	{
		--nIndex;
	}

	// We have some whitespace to remove
	if (nIndex < nLastCharIndex)
	{
		m_pString[nIndex + 1] = 0;
		SetLength(nIndex + 2);
	}
}


void CUtlString::TrimRight(const char *szTargets)
{
	const int nLastCharIndex = Length() - 1;
	int i;

	for (i = nLastCharIndex; i > 0; i--)
	{
		bool bWhitespace = false;

		for (int j = 0; szTargets[j] != 0; j++)
		{
			if (m_pString[i] == szTargets[j])
			{
				bWhitespace = true;
				break;
			}
		}

		if (!bWhitespace)
		{
			break;
		}
	}

	// We have some whitespace to remove
	if (i < nLastCharIndex)
	{
		m_pString[i + 1] = 0;
		SetLength(i + 2);
	}
}


void CUtlString::Trim(char cTarget)
{
	TrimLeft(cTarget);
	TrimRight(cTarget);
}


void CUtlString::Trim(const char *szTargets)
{
	TrimLeft(szTargets);
	TrimRight(szTargets);
}


CUtlString CUtlString::Slice(int32_t nStart, int32_t nEnd) const
{
	int length = Length();
	if (length == 0)
	{
		return CUtlString();
	}

	if (nStart < 0)
		nStart = length - (-nStart % length);
	else if (nStart >= length)
		nStart = length;

	if (nEnd == INT32_MAX)
		nEnd = length;
	else if (nEnd < 0)
		nEnd = length - (-nEnd % length);
	else if (nEnd >= length)
		nEnd = length;

	if (nStart >= nEnd)
		return CUtlString();

	const char *pIn = String();

	CUtlString ret;
	ret.SetDirect(pIn + nStart, nEnd - nStart);
	return ret;
}

// Grab a substring starting from the left or the right side.
CUtlString CUtlString::Left(int32_t nChars) const
{
	return Slice(0, nChars);
}

CUtlString CUtlString::Right(int32_t nChars) const
{
	return Slice(-nChars);
}

CUtlString CUtlString::Replace(char cFrom, char cTo) const
{
	if (!m_pString)
	{
		return CUtlString();
	}

	CUtlString ret = *this;
	int len = ret.Length();
	for (int i = 0; i < len; i++)
	{
		if (ret.m_pString[i] == cFrom)
			ret.m_pString[i] = cTo;
	}

	return ret;
}

CUtlString CUtlString::Replace(const char *pszFrom, const char *pszTo) const
{
	// Assert(pszTo); // Can be 0 length, but not null
	// Assert(pszFrom && *pszFrom); // Must be valid and have one character.

	const char *pos = strstr(String(), pszFrom);
	if (!pos)
	{
		return *this;
	}

	const char *pFirstFound = pos;

	// count number of search string
	int nSearchCount = 0;
	int nSearchLength = strlen(pszFrom);
	while (pos)
	{
		nSearchCount++;
		int nSrcOffset = (pos - String()) + nSearchLength;
		pos = strstr(String() + nSrcOffset, pszFrom);
	}

	// allocate the new string
	int nReplaceLength = strlen(pszTo);
	int nAllocOffset = nSearchCount * (nReplaceLength - nSearchLength);
	size_t srcLength = Length();
	CUtlString strDest;
	size_t destLength = srcLength + nAllocOffset;
	strDest.SetLength(destLength);

	// find and replace the search string
	pos = pFirstFound;
	int nDestOffset = 0;
	int nSrcOffset = 0;
	while (pos)
	{
		// Found an instance
		int nCurrentSearchOffset = pos - String();
		int nCopyLength = nCurrentSearchOffset - nSrcOffset;
		strncpy(strDest.GetForModify() + nDestOffset, String() + nSrcOffset, nCopyLength + 1);
		nDestOffset += nCopyLength;
		strncpy(strDest.GetForModify() + nDestOffset, pszTo, nReplaceLength + 1);
		nDestOffset += nReplaceLength;

		nSrcOffset = nCurrentSearchOffset + nSearchLength;
		pos = strstr(String() + nSrcOffset, pszFrom);
	}

	// making sure that the left over string from the source is the same size as the left over dest buffer
	// Assert(destLength - nDestOffset == srcLength - nSrcOffset);
	if (destLength - nDestOffset > 0)
	{
		strncpy(strDest.GetForModify() + nDestOffset, String() + nSrcOffset, destLength - nDestOffset + 1);
	}

	return strDest;
}

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

//-----------------------------------------------------------------------------
// small helper function shared by lots of modules
//-----------------------------------------------------------------------------
bool V_IsAbsolutePath(const char *pStr)
{
	return (pStr[0] && pStr[1] == ':') || pStr[0] == '/' || pStr[0] == '\\';
}

// Even though \ on Posix (Linux&Mac) isn't techincally a path separator we are
// now counting it as one even Posix since so many times our filepaths aren't actual
// paths but rather text strings passed in from data files, treating \ as a pathseparator
// covers the full range of cases
bool PATHSEPARATOR(char c)
{
	return c == '\\' || c == '/';
}

bool V_RemoveDotSlashes(char *pFilename, char separator, bool bRemoveDoubleSlashes /* = true */)
{
	char *pIn = pFilename;
	char *pOut = pFilename;
	bool bRetVal = true;

	bool bBoundary = true;
	while (*pIn)
	{
		if (bBoundary && pIn[0] == '.' && pIn[1] == '.' && (PATHSEPARATOR(pIn[2]) || !pIn[2]))
		{
			// Get rid of /../ or trailing /.. by backing pOut up to previous separator

			// Eat the last separator (or repeated separators) we wrote out
			while (pOut != pFilename && pOut[-1] == separator)
			{
				--pOut;
			}

			while (true)
			{
				if (pOut == pFilename)
				{
					bRetVal = false; // backwards compat. return value, even though we continue handling
					break;
				}
				--pOut;
				if (*pOut == separator)
				{
					break;
				}
			}

			// Skip the '..' but not the slash, next loop iteration will handle separator
			pIn += 2;
			bBoundary = (pOut == pFilename);
		}
		else if (bBoundary && pIn[0] == '.' && (PATHSEPARATOR(pIn[1]) || !pIn[1]))
		{
			// Handle "./" by simply skipping this sequence. bBoundary is unchanged.
			if (PATHSEPARATOR(pIn[1]))
			{
				pIn += 2;
			}
			else
			{
				// Special case: if trailing "." is preceded by separator, eg "path/.",
				// then the final separator should also be stripped. bBoundary may then
				// be in an incorrect state, but we are at the end of processing anyway
				// so we don't really care (the processing loop is about to terminate).
				if (pOut != pFilename && pOut[-1] == separator)
				{
					--pOut;
				}
				pIn += 1;
			}
		}
		else if (PATHSEPARATOR(pIn[0]))
		{
			*pOut = separator;
			pOut += 1 - (bBoundary & bRemoveDoubleSlashes & (pOut != pFilename));
			pIn += 1;
			bBoundary = true;
		}
		else
		{
			if (pOut != pIn)
			{
				*pOut = *pIn;
			}
			pOut += 1;
			pIn += 1;
			bBoundary = false;
		}
	}
	*pOut = 0;

	return bRetVal;
}

void V_AppendSlash(char *pStr, int strSize)
{
	int len = strlen(pStr);
	if (len > 0 && !PATHSEPARATOR(pStr[len - 1]))
	{
		if (len + 1 >= strSize)
			throw("V_AppendSlash: ran out of space on %s.", pStr);

		pStr[len] = CORRECT_PATH_SEPARATOR;
		pStr[len + 1] = 0;
	}
}

void V_MakeAbsolutePath(char *pOut, int outLen, const char *pPath, const char *pStartingDir)
{
	if (V_IsAbsolutePath(pPath))
	{
		// pPath is not relative.. just copy it.
		strncpy(pOut, pPath, outLen);
	}
	else
	{
		// Make sure the starting directory is absolute..
		if (pStartingDir && V_IsAbsolutePath(pStartingDir))
		{
			strncpy(pOut, pStartingDir, outLen);
		}
		else
		{
			if (!_getcwd(pOut, outLen))
				throw("V_MakeAbsolutePath: _getcwd failed.");

			if (pStartingDir)
			{
				V_AppendSlash(pOut, outLen);
				strncat(pOut, pStartingDir, outLen);
			}
		}

		// Concatenate the paths.
		V_AppendSlash(pOut, outLen);
		strncat(pOut, pPath, outLen);
	}

	if (!V_RemoveDotSlashes(pOut, ' ', true))
		throw("V_MakeAbsolutePath: tried to \"..\" past the root.");

	//V_FixSlashes( pOut ); - handled by V_RemoveDotSlashes
}

//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to the beginning of the unqualified file name 
//			(no path information)
// Input:	in - file name (may be unqualified, relative or absolute path)
// Output:	pointer to unqualified file name
//-----------------------------------------------------------------------------
const char * V_UnqualifiedFileName(const char * in)
{
	// back up until the character after the first path separator we find,
	// or the beginning of the string
	const char * out = in + strlen(in) - 1;
	while ((out > in) && (!PATHSEPARATOR(*(out - 1))))
		out--;
	return out;
}


//-----------------------------------------------------------------------------
// Purpose: Strip off the last directory from dirName
// Input  : *dirName - 
//			maxlen - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool V_StripLastDir(char *dirName, int maxlen)
{
	if (dirName[0] == 0 ||
		!_stricmp(dirName, "./") ||
		!_stricmp(dirName, ".\\"))
		return false;

	int len = strlen(dirName);

	Assert(len < maxlen);

	// skip trailing slash
	if (PATHSEPARATOR(dirName[len - 1]))
	{
		len--;
	}

	while (len > 0)
	{
		if (PATHSEPARATOR(dirName[len - 1]))
		{
			dirName[len] = 0;
			V_FixSlashes(dirName, CORRECT_PATH_SEPARATOR);
			return true;
		}
		len--;
	}

	// Allow it to return an empty string and true. This can happen if something like "tf2/" is passed in.
	// The correct behavior is to strip off the last directory ("tf2") and return true.
	if (len == 0)
	{
		snprintf(dirName, maxlen, ".%c", CORRECT_PATH_SEPARATOR);
		return true;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *ppath - 
//-----------------------------------------------------------------------------
void V_StripTrailingSlash(char *ppath)
{
	Assert(ppath);

	int len = strlen(ppath);
	if (len > 0)
	{
		if (PATHSEPARATOR(ppath[len - 1]))
		{
			ppath[len - 1] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *in - 
//			*out - 
//			outSize - 
//-----------------------------------------------------------------------------
void V_StripExtension(const char *in, char *out, int outSize)
{
	// Find the last dot. If it's followed by a dot or a slash, then it's part of a 
	// directory specifier like ../../somedir/./blah.

	// scan backward for '.'
	int end = strlen(in) - 1;
	while (end > 0 && in[end] != '.' && !PATHSEPARATOR(in[end]))
	{
		--end;
	}

	if (end > 0 && !PATHSEPARATOR(in[end]) && end < outSize)
	{
		int nChars = min(end, outSize - 1);
		if (out != in)
		{
			memcpy(out, in, nChars);
		}
		out[nChars] = 0;
	}
	else
	{
		// nothing found
		if (out != in)
		{
			strncpy(out, in, outSize);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Extracts the base name of a file (no path, no extension, assumes '/' or '\' as path separator)
// Input  : *in - 
//			*out - 
//			maxlen - 
//-----------------------------------------------------------------------------
void V_FileBase(const char *in, char *out, int maxlen)
{
	Assert(maxlen >= 1);
	Assert(in);
	Assert(out);

	if (!in || !in[0])
	{
		*out = 0;
		return;
	}

	int len, start, end;

	len = strlen(in);

	// scan backward for '.'
	end = len - 1;
	while (end&& in[end] != '.' && !PATHSEPARATOR(in[end]))
	{
		end--;
	}

	if (in[end] != '.')		// no '.', copy to end
	{
		end = len - 1;
	}
	else
	{
		end--;					// Found ',', copy to left of '.'
	}

	// Scan backward for '/'
	start = len - 1;
	while (start >= 0 && !PATHSEPARATOR(in[start]))
	{
		start--;
	}

	if (start < 0 || !PATHSEPARATOR(in[start]))
	{
		start = 0;
	}
	else
	{
		start++;
	}

	// Length of new sting
	len = end - start + 1;

	int maxcopy = min(len + 1, maxlen);

	// Copy partial string
	strncpy(out, &in[start], maxcopy);
}

//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to the file extension within a file name string
// Input:	in - file name 
// Output:	pointer to beginning of extension (after the "."), or NULL
//				if there is no extension
//-----------------------------------------------------------------------------
const char * V_GetFileExtension(const char * path)
{
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a . or the start
	//
	while (src != path && *(src - 1) != '.')
		src--;

	// check to see if the '.' is part of a pathname
	if (src == path || PATHSEPARATOR(*src))
	{
		return NULL;  // no extension
	}

	return src;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *path - 
//			*dest - 
//			destSize - 
// Output : void V_ExtractFileExtension
//-----------------------------------------------------------------------------
void V_ExtractFileExtension(const char *path, char *dest, int destSize)
{
	*dest = NULL;
	const char * extension = V_GetFileExtension(path);
	if (NULL != extension)
		strncpy(dest, extension, destSize);
}

//-----------------------------------------------------------------------------
// Purpose: Composes a path and filename together, inserting a path separator
//			if need be
// Input:	path - path to use
//			filename - filename to use
//			dest - buffer to compose result in
//			destSize - size of destination buffer
//-----------------------------------------------------------------------------
void V_ComposeFileName(const char *path, const char *filename, char *dest, int destSize)
{
	strncpy(dest, path, destSize);
	V_FixSlashes(dest, '/');
	V_AppendSlash(dest, destSize);
	strncat(dest, filename, destSize);
	V_FixSlashes(dest, '/');
}

CUtlString CUtlString::AbsPath(const char *pStartingDir) const
{
	char szNew[MAX_PATH];
	V_MakeAbsolutePath(szNew, sizeof(szNew), this->String(), pStartingDir);
	return CUtlString(szNew);
}

CUtlString CUtlString::UnqualifiedFilename() const
{
	const char *pFilename = V_UnqualifiedFileName(this->String());
	return CUtlString(pFilename);
}

CUtlString CUtlString::DirName() const
{
	CUtlString ret(this->String());
	V_StripLastDir((char*)ret.Get(), ret.Length() + 1);
	V_StripTrailingSlash((char*)ret.Get());
	return ret;
}

CUtlString CUtlString::StripExtension() const
{
	char szTemp[MAX_PATH];
	V_StripExtension(String(), szTemp, sizeof(szTemp));
	return CUtlString(szTemp);
}

CUtlString CUtlString::StripFilename() const
{
	const char *pFilename = V_UnqualifiedFileName(Get()); // NOTE: returns 'Get()' on failure, never NULL
	int nCharsToCopy = pFilename - Get();
	CUtlString result;
	result.SetDirect(Get(), nCharsToCopy);
	result.StripTrailingSlash();
	return result;
}

CUtlString CUtlString::GetBaseFilename() const
{
	char szTemp[MAX_PATH];
	V_FileBase(String(), szTemp, sizeof(szTemp));
	return CUtlString(szTemp);
}

CUtlString CUtlString::GetExtension() const
{
	char szTemp[MAX_PATH];
	V_ExtractFileExtension(String(), szTemp, sizeof(szTemp));
	return CUtlString(szTemp);
}


CUtlString CUtlString::PathJoin(const char *pStr1, const char *pStr2)
{
	char szPath[MAX_PATH];
	V_ComposeFileName(pStr1, pStr2, szPath, sizeof(szPath));
	return CUtlString(szPath);
}

CUtlString CUtlString::operator+(const char *pOther) const
{
	CUtlString s = *this;
	s += pOther;
	return s;
}

CUtlString CUtlString::operator+(const CUtlString &other) const
{
	CUtlString s = *this;
	s += other;
	return s;
}

CUtlString CUtlString::operator+(int rhs) const
{
	CUtlString ret = *this;
	ret += rhs;
	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: concatenate the provided string to our current content
//-----------------------------------------------------------------------------
void CUtlString::Append(const char *pchAddition)
{
	(*this) += pchAddition;
}

void CUtlString::Append(const char *pchAddition, int nChars)
{
	nChars = min(nChars, (int)strlen(pchAddition));

	const int lhsLength(Length());
	const int rhsLength(nChars);
	const int requestedLength(lhsLength + rhsLength);

	AllocMemory(requestedLength);
	const int allocatedLength(requestedLength);
	const int copyLength(allocatedLength - lhsLength < rhsLength ? allocatedLength - lhsLength : rhsLength);
	memcpy(GetForModify() + lhsLength, pchAddition, copyLength);
	m_pString[allocatedLength] = '\0';
}

// Shared static empty string.
const CUtlString &CUtlString::GetEmptyString()
{
	static const CUtlString s_emptyString;

	return s_emptyString;
}

