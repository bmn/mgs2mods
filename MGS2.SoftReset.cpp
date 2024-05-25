#include "MGS2.framework.h"

namespace MGS2::SoftReset {
	const char* Category = "SoftReset";

	bool IsEnabled;

	tFUN_Void_PshortPint oFUN_008cfe60;
	void __cdecl hkFUN_008cfe60(short* param_1, int* param_2) {

		try_mgs2
			if (*(BYTE*)0x118ADB0 != 1) {
				if (Actions::ShortcutActive("SoftReset.Reset")) {
					((void(*)(int))0x877DE0)(1);
				}
			}
		catch_mgs2(Category, "8CFE60");

		oFUN_008cfe60(param_1, param_2);
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		Actions::RegisterShortcut(ini, "SoftReset.Reset");

		IsEnabled = true;

		oFUN_008cfe60 = (tFUN_Void_PshortPint)mem::TrampHook32((BYTE*)0x8CFE60, (BYTE*)hkFUN_008cfe60, 8);
	}

}
