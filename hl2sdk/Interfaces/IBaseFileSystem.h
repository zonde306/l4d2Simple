#pragma once
#include "IAppSystem.h"
#include "../Utils/utlbuffer.h"
#include <cstdio>
#include <cstdint>

//-----------------------------------------------------------------------------
// Base file system interface
//-----------------------------------------------------------------------------

// This is the minimal interface that can be implemented to provide access to
// a named set of files.
#define BASEFILESYSTEM_INTERFACE_VERSION		"VBaseFileSystem011"

typedef void * FileHandle_t;
typedef void * FileCacheHandle_t;
typedef int FileFindHandle_t;
typedef void(*FileSystemLoggingFunc_t)(const char *fileName, const char *accessType);
typedef int WaitForResourcesHandle_t;


//-----------------------------------------------------------------------------
// Enums used by the interface
//-----------------------------------------------------------------------------

#define FILESYSTEM_MAX_SEARCH_PATHS 128

enum FileSystemSeek_t
{
	FILESYSTEM_SEEK_HEAD = SEEK_SET,
	FILESYSTEM_SEEK_CURRENT = SEEK_CUR,
	FILESYSTEM_SEEK_TAIL = SEEK_END,
};

enum
{
	FILESYSTEM_INVALID_FIND_HANDLE = -1
};

enum FileWarningLevel_t
{
	// A problem!
	FILESYSTEM_WARNING = -1,

	// Don't print anything
	FILESYSTEM_WARNING_QUIET = 0,

	// On shutdown, report names of files left unclosed
	FILESYSTEM_WARNING_REPORTUNCLOSED,

	// Report number of times a file was opened, closed
	FILESYSTEM_WARNING_REPORTUSAGE,

	// Report all open/close events to console ( !slow! )
	FILESYSTEM_WARNING_REPORTALLACCESSES,

	// Report all open/close/read events to the console ( !slower! )
	FILESYSTEM_WARNING_REPORTALLACCESSES_READ,

	// Report all open/close/read/write events to the console ( !slower! )
	FILESYSTEM_WARNING_REPORTALLACCESSES_READWRITE,

	// Report all open/close/read/write events and all async I/O file events to the console ( !slower(est)! )
	FILESYSTEM_WARNING_REPORTALLACCESSES_ASYNC,

};

// search path filtering
enum PathTypeFilter_t
{
	FILTER_NONE = 0,	// no filtering, all search path types match
	FILTER_CULLPACK = 1,	// pack based search paths are culled (maps and zips)
	FILTER_CULLNONPACK = 2,	// non-pack based search paths are culled
};

// search path querying (bit flags)
enum
{
	PATH_IS_NORMAL = 0x00, // normal path, not pack based
	PATH_IS_PACKFILE = 0x01, // path is a pack file
	PATH_IS_MAPPACKFILE = 0x02, // path is a map pack file
	PATH_IS_REMOTE = 0x04, // path is the remote filesystem
};

//-----------------------------------------------------------------------------
// File system allocation functions. Client must free on failure
//-----------------------------------------------------------------------------
typedef void *(*FSAllocFunc_t)(const char *pszFilename, unsigned nBytes);

class IBaseFileSystem
{
public:
	virtual int				Read(void* pOutput, int size, FileHandle_t file) = 0;
	virtual int				Write(void const* pInput, int size, FileHandle_t file) = 0;

	// if pathID is NULL, all paths will be searched for the file
	virtual FileHandle_t	Open(const char *pFileName, const char *pOptions, const char *pathID = 0) = 0;
	virtual void			Close(FileHandle_t file) = 0;


	virtual void			Seek(FileHandle_t file, int pos, FileSystemSeek_t seekType) = 0;
	virtual unsigned int	Tell(FileHandle_t file) = 0;
	virtual unsigned int	Size(FileHandle_t file) = 0;
	virtual unsigned int	Size(const char *pFileName, const char *pPathID = 0) = 0;

	virtual void			Flush(FileHandle_t file) = 0;
	virtual bool			Precache(const char *pFileName, const char *pPathID = 0) = 0;

	virtual bool			FileExists(const char *pFileName, const char *pPathID = 0) = 0;
	virtual bool			IsFileWritable(char const *pFileName, const char *pPathID = 0) = 0;
	virtual bool			SetFileWritable(char const *pFileName, bool writable, const char *pPathID = 0) = 0;

	virtual long			GetFileTime(const char *pFileName, const char *pPathID = 0) = 0;

	//--------------------------------------------------------
	// Reads/writes files to utlbuffers. Use this for optimal read performance when doing open/read/close
	//--------------------------------------------------------
	virtual bool			ReadFile(const char *pFileName, const char *pPath, CUtlBuffer &buf, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = NULL) = 0;
	virtual bool			WriteFile(const char *pFileName, const char *pPath, CUtlBuffer &buf) = 0;
	virtual bool			UnzipFile(const char *pFileName, const char *pPath, const char *pDestination) = 0;
};

// This class represents a group of files. Internally, it can represent whole folders of files
// that are in or out of the group. So you can't iterate the list, but you can ask the
// class if a particular filename is in the list.
class IFileList
{
public:
	virtual bool	IsFileInList(const char *pFilename) = 0;
	virtual void	Release() = 0;
};
