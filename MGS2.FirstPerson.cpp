#include "MGS2.framework.h"

namespace MGS2::FirstPerson {
	const char* Category = "FirstPerson";

	int Mode = 1;
	unsigned int Toggle = 0x0002;

	int* CurrentState = (int*)0xA18B00;

	tFUN_Void_Int oFUN_008781e0;
	void __cdecl hkFUN_008781e0(int param_1) {
		try_mgs2
			int frames = *MGS2::Mem::AreaTimeFrames;
			if (frames == 0) {
				*CurrentState = 0;
			}
			else if (frames == 1) {
				*CurrentState = Mode;
			}
		catch_mgs2(Category, "8781E0");

		oFUN_008781e0(param_1);
	}

	void ToggleAction(Actions::Action action) {
		*CurrentState = *CurrentState ? 0 : Mode;
		Log::DisplayToggleMessage("First Person", (*CurrentState != 0));
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		Mode = ConfigParser::ParseInteger(ini, Category, "Mode", 1, 1, 3, true);

		mem::PatchMemory((BYTE*)0x4ECC7F, "\x90\x90\x90\x90\x90\x90");

		NewGameInfo::AddWarning("First Person");
		Actions::RegisterAction(ini, "FirstPerson.Toggle", &ToggleAction);

		oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);

	}

}
