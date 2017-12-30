#ifndef __SPLICE_H__
#define __SPLICE_H__


//////////////////////////////////////////////////////////////////////////
// splice internals

#define JMP_SIZE	5

typedef struct _splice_entry{
	
	LPVOID	original_addr;
	LPVOID	trampoline;
	DWORD	repair_code_size; // without JMP_SIZE

#ifdef __cplusplus
	_splice_entry();
	_splice_entry(LPVOID function, LPVOID handler);
	~_splice_entry();
#endif

}SPLICE_ENTRY, *PSPLICE_ENTRY;

//////////////////////////////////////////////////////////////////////////
// prototypes
SPLICE_ENTRY*	SpliceHookFunction(LPVOID function, LPVOID handler);
void			SpliceUnHookFunction(SPLICE_ENTRY* pse);

//////////////////////////////////////////////////////////////////////////

#define		utilsVAlloc(x)	VirtualAlloc(0, x, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE)
#define		utilsVFree(x)	VirtualFree(x, 0, MEM_RELEASE)

void	utilsGetInstructionLength(LPVOID address, DWORD* osizeptr);
DWORD	utilsGetInstructionLength(LPVOID addr);
DWORD	utilsGetWholeCodeSize(LPVOID start);

BOOL	utilsCompareData(const BYTE* pData, const BYTE* bMask, const char* pszMask);
DWORD	utilsFindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * pszMask);
void	utilsCreateJMP(DWORD at, DWORD to);
void	utilsCreateCall(DWORD at, DWORD to);
DWORD	utilsRedirectCall(DWORD at, DWORD to);
DWORD	utilsReconstructJMP(DWORD dwAddress);

DWORD	utilsCopyRange(DWORD dwFrom, DWORD dwTo);
void	utilsWrapMemory(DWORD dwAddress, DWORD dwNew, DWORD dwMinLen);

void	utilsDPrint(char *fmt, ...);

#endif