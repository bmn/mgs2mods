#include "MGS2.framework.h"

namespace MGS2::SoftReset {

	bool _IsEnabled;
	bool IsEnabled() {
		return _IsEnabled;
	}

	tFUN_Void_PshortPint oFUN_008cfe60;
	void __cdecl hkFUN_008cfe60(short* param_1, int* param_2) {

		if (*(unsigned char*)0x118ADB0 != 1) {
			unsigned int currentFrameInput = *(unsigned int*)(param_2 + 1);

			unsigned int resetMask = 0x80F;
			if ((currentFrameInput & resetMask) == resetMask) {
				((void(*)(int))0x877DE0)(1);
			}
		}

		oFUN_008cfe60(param_1, param_2);
	}

	void Run(CSimpleIniA& ini) {
		const char* category = "SoftReset";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		_IsEnabled = true;

		oFUN_008cfe60 = (tFUN_Void_PshortPint)mem::TrampHook32((BYTE*)0x8CFE60, (BYTE*)hkFUN_008cfe60, 8);
	}

}