#include "MGS2.framework.h"

namespace MGS2::Wet {
	const char* Category = "Wet";

	mem::Patch PatchWet;
	mem::Patch PatchCold;

	const char* CategoryName = nullptr;
	bool Active = false;
	int SneezeFrames = -1;

	tFUN_Void_Int oFUN_008781e0;
	void __cdecl hkFUN_008781e0(int param_1) {
		oFUN_008781e0(param_1);

		try_mgs2
			if (Active) {
				*(int*)0x118aec8 = 1;
				*(int*)0x118aeca = (SneezeFrames == -1) ? 600 : SneezeFrames;
			}
		catch_mgs2(Category, "8781E0");
	}

	void ToggleAction(Actions::Action action) {
		if (Active) {
			*(int*)0x118aec8 = 0;
			*(int*)0x118aeca = 600;
			mem::UnpatchMemory(PatchWet);
			mem::UnpatchMemory(PatchCold);
		}
		else {
			mem::PatchMemory(PatchWet);
			mem::PatchMemory(PatchCold);
		}

		Active = !Active;
		Log::DisplayToggleMessage(CategoryName, Active);
	}

	bool NewGameInfoCallback() {
		return Active;
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		char* sneezeMessage = (char*)calloc(1, 32);

		bool isWet = false;
		if (ini.GetBoolValue(Category, "Wet", true)) {
			//PatchWet = mem::PatchMemory((void*)0x7C7D7E, "\xB8\x0B");
			PatchWet = mem::Patch((void*)0x7C7D7E, "\xB8\x0B");
			isWet = true;
		}
		else {
			PatchWet = mem::Patch();
		}

		bool isCold = false;
		if (ini.GetBoolValue(Category, "Cold", false)) {
			isCold = true;

			long sneezeFrames = ini.GetLongValue(Category, "SneezeTimer", 6);
			if ((sneezeFrames < 0) || (sneezeFrames >= SHRT_MAX)) {
				sneezeFrames = -1;
			}
			if (sneezeFrames != -1) {
				SneezeFrames = sneezeFrames;

				snprintf(sneezeMessage, 31, "(%df)", sneezeFrames);
			}

			PatchCold = mem::Patch((void*)0x4C9FDA, (sneezeFrames == -1) ? "\x90\x90" : "\xC3");

			oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);
		}
		else {
			PatchCold = mem::Patch();
		}

		if (isCold) {
			char* message = (char*)calloc(1, 50);
			snprintf(message, 49, isWet ? "Cold + Wet %s" : "Cold %s", sneezeMessage);
			CategoryName = message;
			NewGameInfo::AddWarning(message, &NewGameInfoCallback);
		}
		else if (isWet) {
			CategoryName = "Wet";
			NewGameInfo::AddWarning(CategoryName, &NewGameInfoCallback);
		}

		if (Active = ini.GetBoolValue(Category, "Active", true)) {
			mem::PatchMemory(PatchWet);
			mem::PatchMemory(PatchCold);
		}

		Actions::RegisterAction(ini, "Wet.Toggle", &ToggleAction);
	}

}
