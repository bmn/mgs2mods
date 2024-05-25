#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include "lib/asprintf/asprintf.h"
#define FMT_HEADER_ONLY
#include "lib/fmt/format.h"
#include "lib/SimpleIni/SimpleIni.h"
using namespace fmt::literals;

// Windows Header Files
#include <windows.h>
#include <stddef.h>
#include <TlHelp32.h>
#include <map>
#include <vector>
#include <mmsystem.h>
#include "resource.h"
#include <filesystem>
#include <cstdint>
#include <stdexcept>

#pragma comment(lib, "Winmm.lib")

#ifdef _DEBUG
#define try_mgs2
#define catch_mgs2(category, subCategory)
#else
#define try_mgs2 \
try {
#define catch_mgs2(category, subCategory) \
} \
catch (const std::exception& e) { \
	Log::UncaughtException(category, subCategory, e.what()); \
} \
catch (...) { \
	Log::UncaughtException(category, subCategory); \
}
#endif


extern HMODULE ModuleHandle;

namespace ASI {
	extern char Version[];
}


namespace FilesystemHelper {
	void SetCurrentPath(size_t levels = 2);
	void SetCurrentPath(std::filesystem::path path);
	void RevertCurrentPath();
}


namespace mem {
	uintptr_t SetCurrentHookParent(void* parent);
	void ClearCurrentHookParent();

	struct Patch {
		void* Pointer = nullptr;
		void* NewBytes = nullptr;
		int Length = 0;
		void* OldBytes = nullptr;
		bool Success = false;

		Patch() {}
		Patch(void* pointer, const char* replace, int length = 0);
		Patch(const char* find, int offset, const char* replace, int length = 0);
	};

	struct PatchSet {
		std::vector<Patch> Patches;
		bool Patched = false;
		int Failures = 0;

		PatchSet() {}
		PatchSet(std::initializer_list<mem::Patch> patches);
		bool Patch(bool undo = false);
		bool Unpatch();
		bool Toggle();
	};


	bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len);
	unsigned char* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len, bool preserve, bool cut);
	unsigned char* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len);
	uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
	Patch PatchMemory(const char* find, int offset, const char* replace, int length = 0);
	//bool PatchMemory(const char* find, int offset, const char* replace);
	Patch PatchMemory(void* pointer, const char* replace, int length = 0);
	//bool PatchMemory(void* pointer, const char* replace);
	Patch PatchMemory(Patch patch, bool undo = false);
	Patch PatchPointer(void* pointer, uintptr_t replace);
	Patch UnpatchMemory(Patch patch);
}


struct ContextMenuItem {
	const char* FriendlyName = "";
	bool Enabled = false;
	int Position = 0;
	int MenuId = 0;
	HMENU ContextMenu = nullptr;
};

struct KeyCombo {
	WPARAM Key = 0;
	char Modifiers = 0;
	const char* FriendlyName = "";
};

struct PadCombo {
	unsigned int Input = 0;
	unsigned int Activation = 0;
	int Complexity = 0;
	bool IgnoreComplexity = false;

	PadCombo(unsigned int input) {
		Input = input;

		// kernighan bit counting algo :o
		for (Complexity = 0; input; Complexity++)
		{
			input &= input - 1;
		}
	}

	void RemapL3() {
		if ((Input & 0x200) == 0x200) {
			Input &= 0xFFFFFDFF; // remove 0x200
			Input |= 0x60000; // add 0x60000 (L3)
		}
		if ((Activation & 0x200) == 0x200) {
			Activation &= 0xFFFFFDFF; // remove 0x200
			Activation |= 0x60000; // add 0x60000 (L3)
		}
	}

	PadCombo() {}
};


namespace ConfigParser {
	KeyCombo ParseKeyCombo(const char* input);
	PadCombo ParsePS2GamepadMask(const char* input);
	PadCombo ParsePS2GamepadMask(CSimpleIniA& ini, const char* category, const char* key, const char* fallback = "");
	unsigned int ParseMask(const char* input, std::map<std::string, unsigned int>* inputMap = nullptr, bool singleButton = false);
	int ParseHexColor(const char* input, int defaultOutput = -1);
	int ParseHorizontalAlignment(const char* input, int defaultOutput = -1);
	extern std::map<std::string, unsigned int> PS2GamepadButtons;
	char ParseRequiredBool(CSimpleIniA& ini, const char* category, const char* key);
	std::vector<std::string> SplitString(const std::string& str, char delimiter);
#include "ConfigParser.tpp"
}


struct SoundConfig {
	bool Enabled = false;
	bool UseResource = false;
	LPSTR ResourceName = 0;
	const char* SoundFile = 0;
	bool SystemSound = false;

	bool Play() const {
		if (!Enabled) {
			return false;
		}

		long systemSound = SystemSound ? SND_SYSTEM : 0;

		PlaySoundA(NULL, NULL, SND_ASYNC);
		if (UseResource) {
			return PlaySoundA(ResourceName, ModuleHandle, SND_RESOURCE | SND_ASYNC | systemSound);
		}
		return PlaySoundA((LPCSTR)SoundFile, NULL, SND_ASYNC | systemSound);
	};

	
	const std::map<std::string_view, unsigned short> ResourcesMap{
		{ "TonyHawkOFF", IDR_WAVE1 },
		{ "TonyHawkON", IDR_WAVE2 },
		{ "VatsOFF", IDR_WAVE3 },
		{ "VatsON", IDR_WAVE4 },
		{ "DreamcastOFF", IDR_WAVE5 },
		{ "DreamcastON", IDR_WAVE6 },
		//{ "SolidusOFF", IDR_WAVE7 },
		//{ "SolidusON", IDR_WAVE8 }
	};

	void ParseConfig(CSimpleIniA& ini, const char* category) {
		if (ini.GetSectionSize(category) == -1) {
			return;
		}

		const char* text = ini.GetValue(category, "PlaySound", "");
		if (strcmp(text, "") == 0) {
			return;
		}

		SystemSound = ini.GetBoolValue(category, "SystemSound", SystemSound);

		auto iterator = ResourcesMap.find(text);
		if (iterator != ResourcesMap.end()) {
			Enabled = true;
			UseResource = true;
			ResourceName = MAKEINTRESOURCEA(iterator->second);
			return;
		}

		std::filesystem::path soundFile{ text };
		if (!std::filesystem::exists(soundFile)) {
			return;
		}

		Enabled = true;
		SoundFile = text;
	}
};


template <typename T>
class ValueTracker {
private:
	std::map<size_t, T> ValueMap;
	bool _IsUpdated = true;

public:
	ValueTracker() = default;

	void Reset() {
		_IsUpdated = false;
	}

	bool IsUpdated(bool reset = false) {
		bool result = _IsUpdated;
		if (reset) {
			Reset();
		}
		return result;
	}

	T Update(size_t index, T value) {
		if (auto search = ValueMap.find(index); search != ValueMap.end()) {
			if (search->second != value) {
				_IsUpdated = true;
			}
		}
		else {
			_IsUpdated = true;
		}
		ValueMap[index] = value;
		return value;
	}
};
