#pragma once
#include "IAppSystem.h"
#include <Windows.h>

// direct references to localized strings
typedef unsigned long StringIndex_t;
const unsigned long INVALID_LOCALIZE_STRING_INDEX = (StringIndex_t)-1;
#define ILOCALIZE_CLIENT_INTERFACE_VERSION	"Localize_001"

class KeyValues;

//-----------------------------------------------------------------------------
// Purpose: Handles localization of text
//			looks up string names and returns the localized unicode text
//-----------------------------------------------------------------------------
class ILocalize
{
public:
	// adds the contents of a file to the localization table
	virtual bool AddFile(const char *fileName, const char *pPathID = NULL, bool bIncludeFallbackSearchPaths = false) = 0;

	// Remove all strings from the table
	virtual void RemoveAll() = 0;

	// Finds the localized text for tokenName
	virtual wchar_t *Find(char const *tokenName) = 0;

	// finds the index of a token by token name, INVALID_STRING_INDEX if not found
	virtual StringIndex_t FindIndex(const char *tokenName) = 0;

	// gets the values by the string index
	virtual const char *GetNameByIndex(StringIndex_t index) = 0;
	virtual wchar_t *GetValueByIndex(StringIndex_t index) = 0;

	///////////////////////////////////////////////////////////////////
	// the following functions should only be used by localization editors

	// iteration functions
	virtual StringIndex_t GetFirstStringIndex() = 0;
	// returns the next index, or INVALID_STRING_INDEX if no more strings available
	virtual StringIndex_t GetNextStringIndex(StringIndex_t index) = 0;

	// adds a single name/unicode string pair to the table
	virtual void AddString(const char *tokenName, wchar_t *unicodeString, const char *fileName) = 0;

	// changes the value of a string
	virtual void SetValueByIndex(StringIndex_t index, wchar_t *newValue) = 0;

	// saves the entire contents of the token tree to the file
	virtual bool SaveToFile(const char *fileName) = 0;

	// iterates the filenames
	virtual int GetLocalizationFileCount() = 0;
	virtual const char *GetLocalizationFileName(int index) = 0;

	// returns the name of the file the specified localized string is stored in
	virtual const char *GetFileNameByIndex(StringIndex_t index) = 0;

	// for development only, reloads localization files
	virtual void ReloadLocalizationFiles() = 0;

	virtual const char *FindAsUTF8(const char *pchTokenName) = 0;

	// need to replace the existing ConstructString with this
	virtual void ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const char *tokenName, KeyValues *localizationVariables) = 0;
	virtual void ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, StringIndex_t unlocalizedTextSymbol, KeyValues *localizationVariables) = 0;

	///////////////////////////////////////////////////////////////////
	// static interface

	// converts an english string to unicode
	// returns the number of wchar_t in resulting string, including null terminator
	static int ConvertANSIToUnicode(const char *ansi, wchar_t *unicode, int unicodeBufferSizeInBytes);

	// converts an unicode string to an english string
	// unrepresentable characters are converted to system default
	// returns the number of characters in resulting string, including null terminator
	static int ConvertUnicodeToANSI(const wchar_t *unicode, char *ansi, int ansiBufferSize);

	// builds a localized formatted string
	// uses the format strings first: %s1, %s2, ...  unicode strings (wchar_t *)
	template < typename T >
	static void ConstructString(T *unicodeOuput, int unicodeBufferSizeInBytes, const T *formatString, int numFormatParameters, ...)
	{
		va_list argList;
		va_start(argList, numFormatParameters);

		ConstructStringVArgsInternal(unicodeOuput, unicodeBufferSizeInBytes, formatString, numFormatParameters, argList);

		va_end(argList);
	}

	template < typename T >
	static void ConstructStringVArgs(T *unicodeOuput, int unicodeBufferSizeInBytes, const T *formatString, int numFormatParameters, va_list argList)
	{
		ConstructStringVArgsInternal(unicodeOuput, unicodeBufferSizeInBytes, formatString, numFormatParameters, argList);
	}

	template < typename T >
	static void ConstructString(T *unicodeOutput, int unicodeBufferSizeInBytes, const T *formatString, KeyValues *localizationVariables)
	{
		ConstructStringKeyValuesInternal(unicodeOutput, unicodeBufferSizeInBytes, formatString, localizationVariables);
	}

private:
	// internal "interface"
	static void ConstructStringVArgsInternal(char *unicodeOutput, int unicodeBufferSizeInBytes, const char *formatString, int numFormatParameters, va_list argList);
	static void ConstructStringVArgsInternal(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const wchar_t *formatString, int numFormatParameters, va_list argList);

	static void ConstructStringKeyValuesInternal(char *unicodeOutput, int unicodeBufferSizeInBytes, const char *formatString, KeyValues *localizationVariables);
	static void ConstructStringKeyValuesInternal(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const wchar_t *formatString, KeyValues *localizationVariables);
};