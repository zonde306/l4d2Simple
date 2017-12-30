#ifndef CRIANOSFERA_PEB
#define CRIANOSFERA_PEB
extern BOOL WINAPI HideDll(HINSTANCE);
extern bool HideThread(HANDLE hThread);
extern bool RemoveHeader(HINSTANCE hinstDLL);
extern void UnlinkModuleFromPEB(HMODULE hModule);
#endif
