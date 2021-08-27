#include <Windows.h>
#include <iostream>

#include "proccess.h"

int main(int argc, char** argv)
{
	try
	{
		proc::EnableDebugPrivilege();
	}
	catch (const std::exception& error)
	{
		std::cout << error.what() << std::endl;
		return EXIT_FAILURE;
	}

	std::string procName = u8"left4dead2.exe";

	if (argc >= 2)
		procName = argv[1];

	std::string fileName = proc::GetCurrentPath(argv[0]) + u8"\\l4d2Simple2.dll";

	if (argc >= 3)
		fileName = argv[2];

	DWORD pid = proc::FindProccess(procName);

	if (pid == NULL)
	{
		std::cout << procName << " not found" << std::endl;
		return EXIT_FAILURE;
	}

	DWORD result = EXIT_SUCCESS;

	try
	{
		result = proc::InjectDLL(fileName, pid);
	}
	catch (const std::exception& error)
	{
		std::cout << error.what() << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "proccess: " << procName << std::endl;
	std::cout << "module: " << fileName << std::endl;
	std::cout << "pid: " << pid << std::endl;

	std::cout << "address: " << std::ios::uppercase << std::ios::hex <<
		proc::GetModuleBase(fileName.substr(fileName.rfind("\\")), pid);

	return result;
}