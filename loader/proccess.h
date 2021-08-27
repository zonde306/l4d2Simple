#pragma once

#include <Windows.h>
#include <Winternl.h>
#include <string>
#include <tuple>

namespace proc
{
	DWORD FindProccess(const std::string& procName);
	DWORD InjectDLL(const std::string& fileName, DWORD pid);
	std::string GetErrorMessage(DWORD code, const std::string& defaults = "");
	std::string GetCurrentPath(const std::string& selfPath);
	DWORD GetModuleBase(const std::string& moduleName, DWORD pid);

    #ifdef _WIN64
	typedef  DWORD64 DWORDX;
    #else
	typedef  DWORD32 DWORDX;
    #endif

	typedef NTSTATUS(WINAPI* LdrGetProcedureAddressT)(IN PVOID DllHandle, IN PANSI_STRING ProcedureName OPTIONAL, IN ULONG ProcedureNumber OPTIONAL, OUT FARPROC* ProcedureAddress);
	typedef VOID(WINAPI* RtlFreeUnicodeStringT)(_Inout_ PUNICODE_STRING UnicodeString);
	typedef  VOID(WINAPI* RtlInitAnsiStringT)(_Out_    PANSI_STRING DestinationString, _In_opt_ PCSZ         SourceString);
	typedef NTSTATUS(WINAPI* RtlAnsiStringToUnicodeStringT)(_Inout_ PUNICODE_STRING DestinationString, _In_ PCANSI_STRING SourceString, _In_ BOOLEAN AllocateDestinationString);
	typedef NTSTATUS(WINAPI* LdrLoadDllT)(PWCHAR, PULONG, PUNICODE_STRING, PHANDLE);
	typedef BOOL(APIENTRY* ProcDllMain)(LPVOID, DWORD, LPVOID);
	typedef NTSTATUS(WINAPI* NtAllocateVirtualMemoryT)(IN HANDLE ProcessHandle, IN OUT PVOID* BaseAddress, IN ULONG ZeroBits, IN OUT PSIZE_T RegionSize, IN ULONG AllocationType, IN ULONG Protect);

	struct PARAMX
	{
		PVOID lpFileData;
		DWORD DataLength;
		LdrGetProcedureAddressT LdrGetProcedureAddress;
		NtAllocateVirtualMemoryT dwNtAllocateVirtualMemory;
		LdrLoadDllT pLdrLoadDll;
		RtlInitAnsiStringT RtlInitAnsiString;
		RtlAnsiStringToUnicodeStringT RtlAnsiStringToUnicodeString;
		RtlFreeUnicodeStringT RtlFreeUnicodeString;
	};

	DWORDX WINAPI MemLoadLibrary2(PARAMX* X);
	DWORD InjectDLLEx(const std::string& fileName, DWORD pid);

	std::pair<LPVOID, DWORD> GetMemLoadLibrary2ShellCode();
	bool EnableDebugPrivilege();
}