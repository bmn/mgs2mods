#include "MGS2.framework.h"

namespace MGS2::Style {
	const char* Category = "Style";

	namespace CodenameSecret {
		const char* Category = Style::Category;
		const size_t START_ID = 1;
		const size_t END_ID = 3531;
		const size_t DEFAULT_ID = 0x547; // "What do you think you're doing!"
		
		std::vector<size_t> VoxTrackIds;

		tFUN_Void_Int oFUN_00751700;
		void __cdecl hkFUN_00751700(int param_1) {
			oFUN_00751700(param_1);

			if (*(int*)(param_1 + 0x130) != 0) return;
			if (*(int*)(param_1 + 300) != 10) return;

			size_t count = VoxTrackIds.size();
			if (!count) return;

			size_t selectedIndex = MGS2::RNG(count);
			size_t selectedId = VoxTrackIds[selectedIndex];
			Vox::Play(selectedId);
		}

		void Run(CSimpleIniA& ini) {
			const char* subOption = "CodenameSecret";

			CSimpleIniA::TNamesDepend entries;
			ini.GetAllValues(Category, subOption, entries);
			for (const CSimpleIniA::Entry& entry : entries) {
				int input = atoi(entry.pItem);
				int voxId = ConfigParser::ParseInteger(input, 0, (int)START_ID, (int)END_ID, true);
				if (voxId) {
					VoxTrackIds.push_back(voxId);
				}
			}

			if ((VoxTrackIds.empty()) && (ini.GetBoolValue(Category, subOption, true))) {
				VoxTrackIds.push_back(DEFAULT_ID);
			}

			if (!VoxTrackIds.empty()) {
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
