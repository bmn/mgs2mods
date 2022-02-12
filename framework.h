#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <stddef.h>
#include <TlHelp32.h>
#include "SimpleIni.h"


bool PatchMemory(const char* find, const int offset, const char* replace);
bool PatchMemory(void* pointer, const char* replace);
bool PatchMemory(void* pointer, const char* replace, const int length);
void OverrideIni(const char* category, CSimpleIniA& oldIni);

namespace mem {
	bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len);
	unsigned char* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len, bool preserve, bool cut);
	unsigned char* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len);
	uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
}

VOID DoPropertySheet(HWND hwndOwner);