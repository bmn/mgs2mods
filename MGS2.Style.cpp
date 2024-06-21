#include "MGS2.framework.h"

namespace MGS2::Style {
	const char* Category = "Style";

	namespace CodenameSecret {
		const char* Category = Style::Category;
		const size_t START_ID = 0;
		const size_t END_ID = 3531;

		size_t VoxTrackId = 0x547; // default "What do you think you're doing!"

		tFUN_Void_Int oFUN_00751700;
		void __cdecl hkFUN_00751700(int param_1) {
			oFUN_00751700(param_1);

			if (*(int*)(param_1 + 0x130) != 0) return;
			if (*(int*)(param_1 + 300) != 10) return;

			Vox::Play(VoxTrackId);
		}

		void Run(CSimpleIniA& ini) {
			const char* subOption = "CodenameSecret";
			int codenameSecret = ConfigParser::ParseInteger(ini, Category, subOption, -1, (int)START_ID, (int)END_ID, true);
			if (codenameSecret == -1) {
				codenameSecret = ini.GetBoolValue(Category, subOption, true) ? VoxTrackId : -1;
			}
			else {
				VoxTrackId = codenameSecret;
			}
			if (codenameSecret != -1) {
				oFUN_00751700 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x751700, (BYTE*)hkFUN_00751700, 5);
			}
		}
	}

	
	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		CodenameSecret::Run(ini);

		if (ini.GetBoolValue(Category, "PhoneRingtone", true)) {
			mem::PatchMemory((void*)0x40CE68, "\xA0");
		}

	}

}
