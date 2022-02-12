#include "MGS2.framework.h"

namespace MGS2::FirstPerson {

	int Mode = 1;
	unsigned int Toggle = 0x0002;

	int* CurrentState = (int*)0xA18B00;

	/*
	tFUN_Void_Int oFUN_008d09f0;
	void __cdecl hkFUN_008d09f0(int param_1) {
		int state = Mode;
		
		short progressTanker = *(short*)0x118D93C;
		if ((progressTanker >= 16) && (progressTanker <= 25)) {
			state = 0;
		}
		
		*CurrentState = state;

		oFUN_008d09f0(param_1);
	}
	*/

	tFUN_Void_Int oFUN_008781e0;
	void __cdecl hkFUN_008781e0(int param_1) {
		int frames = *(int*)(*(int*)0xA01F34 + 0xE4);
		if (frames == 0) {
			*CurrentState = 0;
		}
		else if (frames == 1) {
			*CurrentState = Mode;
		}

		// if paused
		if (*(bool*)0xA53A00) {
			// check for a new input
			if ((*(unsigned int*)0xEDADE0 & Toggle) == Toggle) {
				*CurrentState = *CurrentState ? 0 : Mode;
			}
		}

		oFUN_008781e0(param_1);
	}


	void Run(CSimpleIniA& ini) {
		const char* category = "FirstPerson";
		
		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		long long llong;
		if ((llong = ini.GetLongValue(category, "Mode", LONG_MIN)) != LONG_MIN) {
			if ((llong >= 1) && (llong <= 3)) {
				Mode = llong;
			}
		}

		const char* str;
		str = ini.GetValue(category, "Toggle", "");
		if (strcmp(str, "") != 0) {
			llong = std::stoll(str, 0, 16);
			if (llong <= 0xFFFF) {
				Toggle = RemapL3((unsigned int)llong);
			}
		}


		//PatchMemory((BYTE*)0x8D09FB, "\x75"); // swap control scheme
		PatchMemory((BYTE*)0x4ECC7F, "\x90\x90\x90\x90\x90\x90");

		NewGameInfo::AddWarning("First Person");

		//oFUN_008d09f0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8D09F0, (BYTE*)hkFUN_008d09f0, 6);
		oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);

	}

}