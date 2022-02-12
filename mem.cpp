#include "framework.h"

namespace mem {

	bool DetourPres32(BYTE* src, BYTE* dst, const uintptr_t len, bool preserve) {
		uintptr_t inLength = 5;
		uintptr_t outLength = 0;
		if (preserve) {
			inLength += 2;
			outLength += 2;
		}
		uintptr_t totalLength = inLength + outLength;

		if (len < totalLength)
			return false;

		DWORD curProtection;
		VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

		uintptr_t relativeAddress = dst - src - inLength;

		uintptr_t i = 0;
		if (preserve) {
			*(uintptr_t*)(src + i++) = 0x60; // pushad
			*(uintptr_t*)(src + i++) = 0x9C; // pushfd
		}
		*(uintptr_t*)(src + i++) = 0xE9; // jmp
		*(uintptr_t*)(src + i) = relativeAddress;

		for (i = inLength; len > i; i++)
			*(uintptr_t*)(src + i) = 0x90;

		if (preserve) {
			*(uintptr_t*)(src + i++) = 0x9D; // popfd
			*(uintptr_t*)(src + i) = 0x61; // popad
		}

		VirtualProtect(src, len, curProtection, &curProtection);
		
		return true;
	}

	unsigned char* TrampHookBak32(BYTE* src, BYTE* dst, const uintptr_t len) {
		if (len < 5)
			return 0;

		BYTE* gateway = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		memcpy_s(gateway, len, src, len);

		uintptr_t* newSrc;
		if (src[0] == 0xE9) {
			newSrc = (uintptr_t*)(gateway + 1);
			*newSrc = *newSrc - (gateway - src);
		}

		uintptr_t gatewayRelativeAddr = src - gateway - 5;

		*(gateway + len) = 0xE9;
		*(uintptr_t*)((uintptr_t)gateway + len + 1) = gatewayRelativeAddr;

		Detour32(src, dst, len);

		return gateway;
	}

	uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
	{
		uintptr_t modBaseAddr = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
		if (hSnap != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 modEntry;
			modEntry.dwSize = sizeof(modEntry);
			if (Module32First(hSnap, &modEntry))
			{
				do
				{
					if (!_wcsicmp(modEntry.szModule, modName))
					{
						modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
						break;
					}
				} while (Module32Next(hSnap, &modEntry));
			}
		}
		CloseHandle(hSnap);
		return modBaseAddr;
	}

	bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len) {
		if (len < 5)
			return false;

		DWORD curProtection;
		VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

		uintptr_t relativeAddress = dst - src - 5;

		*src = 0xE9;
		*(uintptr_t*)(src + 1) = relativeAddress;

		for (uintptr_t i = 5; len > i; i++)
			*(unsigned char*)(src + i) = 0x90;

		VirtualProtect(src, len, curProtection, &curProtection);

		return true;
	}

	unsigned char* TrampHook32(BYTE* src, BYTE* dst, uintptr_t len, bool preserve, bool cut) {
		if (len < 5)
			return 0;

		BYTE* gateway = (BYTE*)VirtualAlloc(0, len + 9, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		BYTE* newGateway = gateway;
		if (preserve) {
			*gateway = 0x90;//0x60;
			*(gateway + 1) = 0x90;//0x9C;
			// todo add call to dst and push newGateway back 5b
			*(gateway + 2) = 0xE8; // call

			uintptr_t newCodeRelativeAddr = dst - gateway - 7;
			*(uintptr_t*)(gateway + 3) = newCodeRelativeAddr;

			*(gateway + 7) = 0x90;//0x9D;
			*(gateway + 8) = 0x90;//0x61;

			newGateway += 9;
		}

		if (cut) {
			len = 0;
		}
		else {
			memcpy_s(newGateway, len, src, len);
		}

		uintptr_t* newSrc;
		if ( (src[0] == 0xE9) || (src[0] == 0xE8) ) { // jmp or call
			newSrc = (uintptr_t*)(gateway + 1);
			*newSrc = *newSrc - (gateway - src);
		}
		else {
			uintptr_t lenOffset = len - 5;
			if ((src[lenOffset] == 0xE9) || (src[lenOffset] == 0xE8)) { // jmp or call
				newSrc = (uintptr_t*)(gateway + 1 + lenOffset);
				*newSrc = *newSrc - (gateway + lenOffset - src);
			}
		}

		uintptr_t gatewayRelativeAddr = src - newGateway - 5;

		BYTE* i = newGateway + len;
		*(i++) = 0xE9;
		*(uintptr_t*)i = gatewayRelativeAddr;

		Detour32(src, preserve ? gateway : dst, len);

		return gateway;
	}

	unsigned char* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len) {
		return TrampHook32(src, dst, len, false, false);
	}

};