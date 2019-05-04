#include "proccess.h"
#include <TlHelp32.h>
#include <exception>
#include <fstream>

DWORD proc::FindProccess(const std::string& procName)
{
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	BOOL haveMore = Process32First(snapshot, &pe);
	while (haveMore)
	{
		if (pe.szExeFile == procName)
			return pe.th32ProcessID;

		haveMore = Process32Next(snapshot, &pe);
	}

	return NULL;
}

DWORD proc::InjectDLL(const std::string& fileName, DWORD pid)
{
	HANDLE proccess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (proccess == INVALID_HANDLE_VALUE)
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't OpenProcess").c_str());

	DWORD bufferSize = fileName.size() + 1;
	LPVOID buffer = VirtualAllocEx(proccess, NULL, bufferSize, MEM_COMMIT, PAGE_READWRITE);
	if (buffer == nullptr)
	{
		CloseHandle(proccess);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't VirtualAllocEx").c_str());
	}

	if (!WriteProcessMemory(proccess, buffer, fileName.c_str(), bufferSize, nullptr))
	{
		VirtualFreeEx(proccess, buffer, bufferSize, MEM_FREE);
		CloseHandle(proccess);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't WriteProcessMemory").c_str());
	}

	HMODULE module = GetModuleHandleA(u8"kernel32.dll");
	if (module == nullptr)
	{
		VirtualFreeEx(proccess, buffer, bufferSize, MEM_FREE);
		CloseHandle(proccess);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't GetModuleHandleA").c_str());
	}

	LPTHREAD_START_ROUTINE fnLoadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(
		GetProcAddress(module, u8"LoadLibraryA"));

	if (fnLoadLibrary == nullptr)
	{
		VirtualFreeEx(proccess, buffer, bufferSize, MEM_FREE);
		CloseHandle(proccess);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't GetProcAddress").c_str());
	}

	HANDLE thread = CreateRemoteThread(proccess, NULL, 0, fnLoadLibrary, buffer, 0, NULL);
	if (thread == nullptr)
	{
		VirtualFreeEx(proccess, buffer, bufferSize, MEM_FREE);
		CloseHandle(proccess);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't CreateRemoteThread").c_str());
	}

	WaitForSingleObject(thread, INFINITE);

	DWORD result = NULL;
	GetExitCodeThread(thread, &result);

	CloseHandle(thread);
	VirtualFreeEx(proccess, buffer, bufferSize, MEM_FREE);
	CloseHandle(proccess);

	return result;
}

std::string proc::GetErrorMessage(DWORD code, const std::string& defaults)
{
	char* message = nullptr;
	DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&message), 0, nullptr);

	if (size == 0)
		return defaults + ":" + std::to_string(code);

	std::string result = message;
	LocalFree(message);
	return result + ":" + std::to_string(code);
}

std::string proc::GetCurrentPath(const std::string& selfPath)
{
	return selfPath.substr(0, selfPath.rfind("\\"));
}

DWORD proc::GetModuleBase(const std::string& moduleName, DWORD pid)
{
	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (!hSnapShot)
		return NULL;

	lpModuleEntry.dwSize = sizeof(lpModuleEntry);
	BOOL __RunModule = Module32First(hSnapShot, &lpModuleEntry);
	while (__RunModule)
	{
		if (lpModuleEntry.szModule == moduleName)
		{
			CloseHandle(hSnapShot);
			return (DWORD)lpModuleEntry.modBaseAddr;
		}
		__RunModule = Module32Next(hSnapShot, &lpModuleEntry);
	}
	CloseHandle(hSnapShot);
	return NULL;
}

proc::DWORDX __stdcall proc::MemLoadLibrary2(PARAMX* X)
{
	LPCVOID lpFileData = X->lpFileData;
	DWORDX DataLength = X->DataLength;

	/****************初始化调用函数********************/
	LdrGetProcedureAddressT LdrGetProcedureAddress = (X->LdrGetProcedureAddress);

	NtAllocateVirtualMemoryT pNtAllocateVirtualMemory = (X->dwNtAllocateVirtualMemory);
	LdrLoadDllT pLdrLoadDll = (X->pLdrLoadDll);
	RtlInitAnsiStringT RtlInitAnsiString = X->RtlInitAnsiString;
	RtlAnsiStringToUnicodeStringT RtlAnsiStringToUnicodeString = X->RtlAnsiStringToUnicodeString;
	RtlFreeUnicodeStringT RtlFreeUnicodeString = X->RtlFreeUnicodeString;

	ProcDllMain pDllMain = NULL;
	void* pMemoryAddress = NULL;



	ANSI_STRING ansiStr;
	UNICODE_STRING UnicodeString;
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_SECTION_HEADER pSectionHeader;
	int ImageSize = 0;

	int nAlign = 0;
	int i = 0;


	//检查数据有效性，并初始化
	/*********************CheckDataValide**************************************/
	//	PIMAGE_DOS_HEADER pDosHeader;
	//检查长度
	if (DataLength > sizeof(IMAGE_DOS_HEADER))
	{
		pDosHeader = (PIMAGE_DOS_HEADER)lpFileData; // DOS头
		//检查dos头的标记
		if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) goto CODEEXIT; //0×5A4D : MZ

		//检查长度
		if ((DWORDX)DataLength < (pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS))) goto CODEEXIT;
		//取得pe头
		pNTHeader = (PIMAGE_NT_HEADERS)((DWORDX)lpFileData + pDosHeader->e_lfanew); // PE头
		//检查pe头的合法性
		if (pNTHeader->Signature != IMAGE_NT_SIGNATURE) goto CODEEXIT; //0×00004550: PE00
		if ((pNTHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) == 0) //0×2000: File is a DLL
			goto CODEEXIT;
		if ((pNTHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == 0) //0×0002: 指出文件可以运行
			goto CODEEXIT;
		if (pNTHeader->FileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER))
			goto CODEEXIT;


		//取得节表（段表）
		pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORDX)pNTHeader + sizeof(IMAGE_NT_HEADERS));
		//验证每个节表的空间
		for (i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
		{
			if ((pSectionHeader[i].PointerToRawData + pSectionHeader[i].SizeOfRawData) > (DWORD)DataLength) goto CODEEXIT;
		}

		/**********************************************************************/
		nAlign = pNTHeader->OptionalHeader.SectionAlignment; //段对齐字节数

		//ImageSize = pNTHeader->OptionalHeader.SizeOfImage;
		//// 计算所有头的尺寸。包括dos, coff, pe头 和 段表的大小
		ImageSize = (pNTHeader->OptionalHeader.SizeOfHeaders + nAlign - 1) / nAlign * nAlign;
		// 计算所有节的大小
		for (i = 0; i < pNTHeader->FileHeader.NumberOfSections; ++i)
		{
			//得到该节的大小
			int CodeSize = pSectionHeader[i].Misc.VirtualSize;
			int LoadSize = pSectionHeader[i].SizeOfRawData;
			int MaxSize = (LoadSize > CodeSize) ? (LoadSize) : (CodeSize);

			int SectionSize = (pSectionHeader[i].VirtualAddress + MaxSize + nAlign - 1) / nAlign * nAlign;
			if (ImageSize < SectionSize)
				ImageSize = SectionSize; //Use the Max;
		}
		if (ImageSize == 0) goto CODEEXIT;

		// 分配虚拟内存
		SIZE_T uSize = ImageSize;
		pNtAllocateVirtualMemory((HANDLE)-1, &pMemoryAddress, 0, &uSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (pMemoryAddress != NULL)
		{

			// 计算需要复制的PE头+段表字节数
			int HeaderSize = pNTHeader->OptionalHeader.SizeOfHeaders;
			int SectionSize = pNTHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
			int MoveSize = HeaderSize + SectionSize;
			//复制头和段信息
			for (i = 0; i < MoveSize; i++)
			{
				*((PCHAR)pMemoryAddress + i) = *((PCHAR)lpFileData + i);
			}
			//memmove(pMemoryAddress, lpFileData, MoveSize);//为了少用一个API,直接用上面的单字节复制数据就行了

			//复制每个节
			for (i = 0; i < pNTHeader->FileHeader.NumberOfSections; ++i)
			{
				if (pSectionHeader[i].VirtualAddress == 0 || pSectionHeader[i].SizeOfRawData == 0)continue;
				// 定位该节在内存中的位置
				void* pSectionAddress = (void*)((DWORDX)pMemoryAddress + pSectionHeader[i].VirtualAddress);
				// 复制段数据到虚拟内存
			//	memmove((void *)pSectionAddress,(void *)((DWORDX)lpFileData + pSectionHeader[i].PointerToRawData),	pSectionHeader[i].SizeOfRawData);
				//为了少用一个API,直接用上面的单字节复制数据就行了
				for (size_t k = 0; k < pSectionHeader[i].SizeOfRawData; k++)
				{
					*((PCHAR)pSectionAddress + k) = *((PCHAR)lpFileData + pSectionHeader[i].PointerToRawData + k);
				}
			}
			/*******************重定位信息****************************************************/

			if (pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress > 0
				&& pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size > 0)
			{

				DWORDX Delta = (DWORDX)pMemoryAddress - pNTHeader->OptionalHeader.ImageBase;
				DWORDX* pAddress;
				//注意重定位表的位置可能和硬盘文件中的偏移地址不同，应该使用加载后的地址
				PIMAGE_BASE_RELOCATION pLoc = (PIMAGE_BASE_RELOCATION)((DWORDX)pMemoryAddress
					+ pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
				while ((pLoc->VirtualAddress + pLoc->SizeOfBlock) != 0) //开始扫描重定位表
				{
					WORD* pLocData = (WORD*)((DWORDX)pLoc + sizeof(IMAGE_BASE_RELOCATION));
					//计算本节需要修正的重定位项（地址）的数目
					int NumberOfReloc = (pLoc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
					for (i = 0; i < NumberOfReloc; i++)
					{
						if ((DWORDX)(pLocData[i] & 0xF000) == 0x00003000 || (DWORDX)(pLocData[i] & 0xF000) == 0x0000A000) //这是一个需要修正的地址
						{
							// 举例：
							// pLoc->VirtualAddress = 0×1000;
							// pLocData[i] = 0×313E; 表示本节偏移地址0×13E处需要修正
							// 因此 pAddress = 基地址 + 0×113E
							// 里面的内容是 A1 ( 0c d4 02 10) 汇编代码是： mov eax , [1002d40c]
							// 需要修正1002d40c这个地址
							pAddress = (DWORDX*)((DWORDX)pMemoryAddress + pLoc->VirtualAddress + (pLocData[i] & 0x0FFF));
							*pAddress += Delta;
						}
					}
					//转移到下一个节进行处理
					pLoc = (PIMAGE_BASE_RELOCATION)((DWORDX)pLoc + pLoc->SizeOfBlock);
				}
				/***********************************************************************/
			}

			/******************* 修正引入地址表**************/
			DWORDX Offset = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
			if (Offset == 0)
				goto CODEEXIT; //No Import Table

			PIMAGE_IMPORT_DESCRIPTOR pID = (PIMAGE_IMPORT_DESCRIPTOR)((DWORDX)pMemoryAddress + Offset);

			PIMAGE_IMPORT_BY_NAME pByName = NULL;
			while (pID->Characteristics != 0)
			{
				PIMAGE_THUNK_DATA pRealIAT = (PIMAGE_THUNK_DATA)((DWORDX)pMemoryAddress + pID->FirstThunk);
				PIMAGE_THUNK_DATA pOriginalIAT = (PIMAGE_THUNK_DATA)((DWORDX)pMemoryAddress + pID->OriginalFirstThunk);
				//获取dll的名字
				char* pName = (char*)((DWORDX)pMemoryAddress + pID->Name);
				HANDLE hDll = 0;

				RtlInitAnsiString(&ansiStr, pName);

				RtlAnsiStringToUnicodeString(&UnicodeString, &ansiStr, true);
				pLdrLoadDll(NULL, NULL, &UnicodeString, &hDll);
				RtlFreeUnicodeString(&UnicodeString);

				if (hDll == NULL) {

					goto CODEEXIT; //NOT FOUND DLL
				}

				//获取DLL中每个导出函数的地址，填入IAT
				//每个IAT结构是 ：
				// union { PBYTE ForwarderString;
				// PDWORDX Function;
				// DWORDX Ordinal;
				// PIMAGE_IMPORT_BY_NAME AddressOfData;
				// } u1;
				// 长度是一个DWORDX ，正好容纳一个地址。
				for (i = 0; ; i++)
				{
					if (pOriginalIAT[i].u1.Function == 0)break;
					FARPROC lpFunction = NULL;
					if (IMAGE_SNAP_BY_ORDINAL(pOriginalIAT[i].u1.Ordinal)) //这里的值给出的是导出序号
					{
						if (IMAGE_ORDINAL(pOriginalIAT[i].u1.Ordinal))
						{

							LdrGetProcedureAddress(hDll, NULL, IMAGE_ORDINAL(pOriginalIAT[i].u1.Ordinal), &lpFunction);
						}
					}
					else//按照名字导入
					{
						//获取此IAT项所描述的函数名称
						pByName = (PIMAGE_IMPORT_BY_NAME)((DWORDX)pMemoryAddress + (DWORDX)(pOriginalIAT[i].u1.AddressOfData));
						if ((char*)pByName->Name)
						{
							RtlInitAnsiString(&ansiStr, (char*)pByName->Name);
							LdrGetProcedureAddress(hDll, &ansiStr, 0, &lpFunction);

						}

					}

					//标记***********

					if (lpFunction != NULL) //找到了！
						pRealIAT[i].u1.Function = (DWORDX)lpFunction;
					else
						goto CODEEXIT;
				}

				//move to next
				pID = (PIMAGE_IMPORT_DESCRIPTOR)((DWORDX)pID + sizeof(IMAGE_IMPORT_DESCRIPTOR));
			}

			/***********************************************************/
			//修正基地址
			pNTHeader->OptionalHeader.ImageBase = (DWORDX)pMemoryAddress;

			//NtProtectVirtualMemory((HANDLE)-1, &pMemoryAddress, (PSIZE_T)&ImageSize, PAGE_EXECUTE_READ, &oldProtect);
			pDllMain = (ProcDllMain)(pNTHeader->OptionalHeader.AddressOfEntryPoint + (DWORDX)pMemoryAddress);

			pDllMain(0, DLL_PROCESS_ATTACH, pMemoryAddress);//这里的参数1本来应该传的是(HMODULE)pMemoryAddress,但是没必要,因为无法使用资源,所以没必要,要使用资源,论坛有其他人说过如何使用

		}
	}

CODEEXIT:

	return (DWORDX)pMemoryAddress;
}

DWORD proc::InjectDLLEx(const std::string& fileName, DWORD pid)
{
	PARAMX params;
	ZeroMemory(&params, sizeof(params));

	HMODULE module = GetModuleHandleA("ntdll");
	params.LdrGetProcedureAddress = reinterpret_cast<LdrGetProcedureAddressT>(GetProcAddress(module, "LdrGetProcedureAddress"));
	params.dwNtAllocateVirtualMemory = reinterpret_cast<NtAllocateVirtualMemoryT>(GetProcAddress(module, "NtAllocateVirtualMemory"));
	params.pLdrLoadDll = reinterpret_cast<LdrLoadDllT>(GetProcAddress(module, "LdrLoadDll"));
	params.RtlInitAnsiString = reinterpret_cast<RtlInitAnsiStringT>(GetProcAddress(module, "RtlInitAnsiString"));
	params.RtlAnsiStringToUnicodeString = reinterpret_cast<RtlAnsiStringToUnicodeStringT>(GetProcAddress(module, "RtlAnsiStringToUnicodeString"));
	params.RtlFreeUnicodeString = reinterpret_cast<RtlFreeUnicodeStringT>(GetProcAddress(module, "RtlFreeUnicodeString"));

	HANDLE proccess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (proccess == INVALID_HANDLE_VALUE)
	{
		delete[] reinterpret_cast<PBYTE>(params.lpFileData);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't OpenProcess").c_str());
	}

	auto [shellCode, shellCodeSize] = GetMemLoadLibrary2ShellCode();
	LPVOID bufShellCode = VirtualAllocEx(proccess, NULL, shellCodeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (bufShellCode == nullptr)
	{
		CloseHandle(proccess);
		delete[] reinterpret_cast<PBYTE>(params.lpFileData);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't VirtualAllocEx").c_str());
	}

	LPVOID bufShellParam = VirtualAllocEx(proccess, NULL, sizeof(params), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (bufShellParam == nullptr)
	{
		VirtualFreeEx(proccess, bufShellCode, shellCodeSize, MEM_DECOMMIT);
		CloseHandle(proccess);
		delete[] reinterpret_cast<PBYTE>(params.lpFileData);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't VirtualAllocEx").c_str());
	}

	{
		HANDLE file = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (file == nullptr)
			throw std::exception(GetErrorMessage(GetLastError(), u8"can't CreateFileA").c_str());

		params.DataLength = GetFileSize(file, NULL);
		params.lpFileData = new BYTE[params.DataLength];
		ReadFile(file, params.lpFileData, params.DataLength, nullptr, NULL);
		CloseHandle(file);
	}

	LPVOID bufDLLCode = VirtualAllocEx(proccess, NULL, params.DataLength, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (bufDLLCode == nullptr)
	{
		VirtualFreeEx(proccess, bufShellCode, shellCodeSize, MEM_FREE);
		VirtualFreeEx(proccess, bufShellParam, sizeof(params), MEM_FREE);
		CloseHandle(proccess);
		delete[] reinterpret_cast<PBYTE>(params.lpFileData);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't VirtualAllocEx").c_str());
	}

	if (!WriteProcessMemory(proccess, bufShellCode, shellCode, shellCodeSize, nullptr) ||
		!WriteProcessMemory(proccess, bufShellParam, &params, sizeof(params), nullptr) ||
		!WriteProcessMemory(proccess, bufDLLCode, params.lpFileData, params.DataLength, nullptr))
	{
		VirtualFreeEx(proccess, bufShellCode, shellCodeSize, MEM_FREE);
		VirtualFreeEx(proccess, bufShellParam, sizeof(params), MEM_FREE);
		VirtualFreeEx(proccess, bufDLLCode, params.DataLength, MEM_FREE);
		CloseHandle(proccess);
		delete[] reinterpret_cast<PBYTE>(params.lpFileData);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't VirtualAllocEx").c_str());
	}

	delete[] reinterpret_cast<PBYTE>(params.lpFileData);
	params.lpFileData = bufDLLCode;

	HANDLE thread = CreateRemoteThread(proccess, nullptr, 0,
		reinterpret_cast<LPTHREAD_START_ROUTINE>(bufShellCode), bufShellParam,
		0, NULL);
	if (thread == nullptr)
	{
		VirtualFreeEx(proccess, bufShellCode, shellCodeSize, MEM_FREE);
		VirtualFreeEx(proccess, bufShellParam, sizeof(params), MEM_FREE);
		VirtualFreeEx(proccess, bufDLLCode, params.DataLength, MEM_FREE);
		CloseHandle(proccess);
		throw std::exception(GetErrorMessage(GetLastError(), u8"can't CreateRemoteThread").c_str());
	}

	WaitForSingleObject(thread, INFINITE);

	DWORD result = EXIT_SUCCESS;
	GetExitCodeThread(thread, &result);

	CloseHandle(thread);
	VirtualFreeEx(proccess, bufShellCode, shellCodeSize, MEM_FREE);
	VirtualFreeEx(proccess, bufShellParam, sizeof(params), MEM_FREE);
	VirtualFreeEx(proccess, bufDLLCode, params.DataLength, MEM_FREE);
	CloseHandle(proccess);

	return result;
}

std::pair<LPVOID, DWORD> proc::GetMemLoadLibrary2ShellCode()
{
	DWORD size = 0;
	WORD* Memx = (WORD*)MemLoadLibrary2;
	while (*Memx != 0xCCCC)
	{
		Memx++;
		size += 2;
	}

	return { Memx, size };
}
