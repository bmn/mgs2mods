#include "MGS2.framework.h"

namespace MGS2::Wet {

	int SneezeFrames = -1;

	tFUN_Void_Int oFUN_008781e0;
	void __cdecl hkFUN_008781e0(int param_1) {
		oFUN_008781e0(param_1);

		*(int*)0x118aec8 = 1;
		*(int*)0x118aeca = (SneezeFrames == -1) ? 600 : SneezeFrames;
	}

	void Run(CSimpleIniA& ini) {
		const char* category = "Wet";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		char* sneezeMessage = (char*)calloc(1, 32);

		bool isWet = false;
		if (ini.GetBoolValue(category, "Wet", true)) {
			PatchMemory((void*)0x7C7D7E, "\xB8\x0B");
			isWet = true;
		}

		bool isCold = false;
		if (ini.GetBoolValue(category, "Cold", false)) {
			isCold = true;

			long sneezeFrames = ini.GetLongValue(category, "SneezeTimer", 6);
			if ((sneezeFrames < 0) || (sneezeFrames >= SHRT_MAX)) {
				sneezeFrames = -1;
			}
			if (sneezeFrames != -1) {
				SneezeFrames = sneezeFrames;

				snprintf(sneezeMessage, 31, "(%df)", sneezeFrames);
			}

			PatchMemory((void*)0x4C9FDA, (sneezeFrames == -1) ? "\x90\x90" : "\xC3");

			oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);
		}

		if (isCold) {
			char* message = (char*)calloc(1, 50);
			snprintf(message, 49, isWet ? "Cold + Wet %s" : "Cold %s", sneezeMessage);
			NewGameInfo::AddWarning(message);
		}
		else if (isWet) {
			NewGameInfo::AddWarning("Wet");
		}
	}

}