#include "MGS2.framework.h"
#include <unordered_set>

namespace MGS2::PS2Controls {
	const char* Category = "PS2Controls";
	
	SimplePatcher* Patcher = nullptr;
	int* FUN575f00_Param2 = nullptr;
	int* FUN575f00_Param3 = nullptr;
	std::unordered_set<int*> AlreadySwapped;

	tFUN_Int_IntIntInt oFUN_00575a80;
	int _cdecl hkFUN_00575a80(int param_1, int param_2, int param_3) {
		int result = oFUN_00575a80(param_1, param_2, param_3);
		
		try_mgs2
			if (!Patcher->Active || !FUN575f00_Param2 || !FUN575f00_Param3) {
				return result;
			}

			int* value = (int*)(FUN575f00_Param2 + *FUN575f00_Param3);
			if ((value < (int*)0x400000) || !(value[2] & 0x60) || (AlreadySwapped.contains(value))) {
				return result;
			}

			if (value[2] & 0x40) {
				value[2] &= (0xFFFFFFFF - 0x40);
				value[2] |= 0x20;
			}
			else { // if (value[2] & 0x20) {
				// 0x20 implied by 0x40 check
				value[2] &= (0xFFFFFFFF - 0x20);
				value[2] |= 0x40;
			}

			AlreadySwapped.insert(value);
		catch_mgs2(Category, "575A80");

		return result;
	}

	tFUN_Int_IntIntInt oFUN_00575f00;
	int _cdecl hkFUN_00575f00(int param_1, int param_2, int param_3) {
		try_mgs2
			if (!Patcher->Active) {
				return oFUN_00575f00(param_1, param_2, param_3);
			}

			FUN575f00_Param2 = (int*)param_2;
			FUN575f00_Param3 = (int*)param_3;
			int result = oFUN_00575f00(param_1, param_2, param_3);
			FUN575f00_Param2 = nullptr;
			FUN575f00_Param3 = nullptr;
			return result;
		catch_mgs2(Category, "575F00");

		// fallback
		return oFUN_00575f00(param_1, param_2, param_3);
	}

	void __declspec(naked) hkJMP_00800722() {
		__asm {
			test dx, 0x890
			jz btnNotPressed

			// btnPressed:
			mov ecx, 0x80072B
			jmp ecx

			btnNotPressed:
			mov ecx, 0x8008AA
			jmp ecx
		}
	}


	void Run(CSimpleIniA& ini) {
		Patcher = new SimplePatcher(ini, Category);

		if (!Patcher->Enabled) {
			delete(Patcher);
			return;
		}

		const char* nullb = "\x00";
		const char* confirm;
		const char* cancel;
		const char* eleConfirm;
		const char* prevStory;

		bool gcMode = ini.GetBoolValue(Category, "GameCubeMenu", false);
		if (gcMode) {
			Patcher->FriendlyName = "GC Menu Controls";
			confirm = "\x80";
			cancel = "\x20";
			eleConfirm = "\x90";
			prevStory = "\x8C";
		}
		else {
			Patcher->FriendlyName = "PS2 Controls";
			confirm = "\x20";
			cancel = "\x40";
			eleConfirm = "\x30";
			prevStory = "\x2C";
		}

		Patcher->Patches = mem::PatchSet{
			mem::Patch((void*)0x407F9A, cancel),
			mem::Patch((void*)0x407FB3, confirm),
			mem::Patch((void*)0x40825B, confirm),
			mem::Patch((void*)0x4082C4, cancel),
			mem::Patch((void*)0x409583, confirm),
			mem::Patch((void*)0x4095C5, cancel),
			mem::Patch((void*)0x410D01, cancel), // photo computer skip
			mem::Patch((void*)0x4274C3, confirm), // ration
			mem::Patch((void*)0x4275A0, confirm), // medicine
			mem::Patch((void*)0x4275F8, confirm), // bandage
			mem::Patch((void*)0x42765E, confirm), // pentazemin
			mem::Patch((void*)0x4C1E32, eleConfirm), // elevator confirm (originally \x50\x08, but start would pause then go?)
			mem::Patch((void*)0x4C1E33, nullb, 1),
			mem::Patch((void*)0x4C1E42, cancel), // elevator cancel (originally \x20\x01, select doesn't cause issues)
			mem::Patch((void*)0x581767, confirm),
			mem::Patch((void*)0x581776, cancel),
			mem::Patch((void*)0x584B81, confirm), // snake tales game over continue
			mem::Patch((void*)0x584CCF, confirm), // snake tales game over exit
			mem::Patch((void*)0x584EEF, confirm), // retirement% confirm exit
			mem::Patch((void*)0x58AEB0, confirm), // end game confirm
			mem::Patch((void*)0x58AEF1, cancel), // end game cancel
			mem::Patch((void*)0x59DA18, confirm), // basic actions 1 confirm
			mem::Patch((void*)0x59DA60, cancel), // basic actions 1 cancel
			mem::Patch((void*)0x59E207, confirm), // basic actions 2 confirm
			mem::Patch((void*)0x59E21E, cancel), // basic actions 2 cancel
			mem::Patch((void*)0x59F102, confirm), // basic actions page switch
			mem::Patch((void*)0x59F194, cancel), // basic actions last page exit
			mem::Patch((void*)0x5A5BC8, confirm), // ??? basic actions video exit
			mem::Patch((void*)0x5A5C10, cancel),
			mem::Patch((void*)0x5A89A9, confirm), // tanker game over confirm
			mem::Patch((void*)0x747244, cancel), // boss survival main menu cancel
			mem::Patch((void*)0x74727D, confirm), // boss survival main menu confirm
			mem::Patch((void*)0x747552, cancel), // boss survival difficulty cancel
			mem::Patch((void*)0x74758B, confirm), // boss survival difficulty confirm
			mem::Patch((void*)0x7477F0, cancel), // boss survival options cancel
			mem::Patch((void*)0x747830, confirm), // boss survival options confirm
			mem::Patch((void*)0x74784A, confirm), // boss survival options exit confirm
			mem::Patch((void*)0x74E397, confirm), // missions save reminder confirm
			mem::Patch((void*)0x752A99, cancel), // options cancel
			mem::Patch((void*)0x752AC9, confirm), // options confirm
			mem::Patch((void*)0x752CB7, confirm), // options open brightness adjustment
			mem::Patch((void*)0x752FFF, confirm), // options exit confirm
			mem::Patch((void*)0x7547D9, cancel), // special cancel
			mem::Patch((void*)0x754805, confirm), // special confirm
			mem::Patch((void*)0x756E7F, cancel),
			mem::Patch((void*)0x756EB1, confirm),
			mem::Patch((void*)0x757356, cancel), // difficulty cancel
			mem::Patch((void*)0x7573A5, confirm), // difficulty confirm 
			mem::Patch((void*)0x7577BA, cancel), // radar cancel
			mem::Patch((void*)0x7577EE, confirm), // radar confirm
			mem::Patch((void*)0x757C0A, cancel), // goid cancel
			mem::Patch((void*)0x757C3E, confirm), // goid confirm
			mem::Patch((void*)0x7586D7, cancel),
			mem::Patch((void*)0x75870F, confirm),
			mem::Patch((void*)0x758B2D, cancel),
			mem::Patch((void*)0x758B61, confirm),
			mem::Patch((void*)0x758E20, cancel),
			mem::Patch((void*)0x758E54, confirm),
			mem::Patch((void*)0x75910A, cancel),
			mem::Patch((void*)0x75913E, confirm),
			mem::Patch((void*)0x75982B, cancel), // casting theater cancel
			mem::Patch((void*)0x759841, confirm), // casting theater confirm
			mem::Patch((void*)0x759DFD, cancel), // casting theater cast change cancel
			mem::Patch((void*)0x759E18, confirm), // casting theater cast change confirm
			mem::Patch((void*)0x76B54B, cancel), // main menu cancel
			mem::Patch((void*)0x76B582, confirm), // main menu confirm
			mem::Patch((void*)0x79014F, confirm), // load game choice confirm
			mem::Patch((void*)0x7903BA, confirm), // load game confirm
			mem::Patch((void*)0x7904B4, cancel), // load game cancel
			mem::Patch((void*)0x7E44EC, confirm),
			mem::Patch((void*)0x7F0E18, confirm), // previous story confirm
			mem::Patch((void*)0x7F1093, cancel), // previous story cancel
			mem::Patch((void*)0x7F136B, confirm), // previous story no forward page change
			mem::Patch((void*)0x7F166D, confirm), // previous story no forward page change
			mem::Patch((void*)0x7F1674, confirm),
			mem::Patch((void*)0x7F19A3, cancel), // previous story page cancel
			mem::Patch((void*)0x7F1A97, prevStory), // previous story no page change
			mem::Patch((void*)0x7F316B, confirm),
			mem::Patch((void*)0x831BDF, cancel), // dog tag viewer cancel
			mem::Patch((void*)0x831E3E, confirm), // dog tag viewer mission select confirm
			mem::Patch((void*)0x831EF3, cancel), // dog tag viewer mission select cancel
		};

		if (ini.GetBoolValue(Category, "NameEntry", true)) {
			std::vector<mem::Patch> nameEntryPatches{
				mem::Patch((void*)0x8004ED, confirm), // plant name confirm
				mem::Patch((void*)0x8004EE, nullb, 1), // plant name confirm (remove start)
				mem::Patch((void*)0x8008AC, cancel), // plant name cancel
				mem::Patch((void*)0x80111D, cancel), // plant profile cancel
				mem::Patch((void*)0x80111E, nullb, 1), // plant profile cancel (remove select)
				mem::Patch((void*)0x801176, confirm), // plant profile confirm
				mem::Patch((void*)0x801177, nullb, 1), // plant profile confirm (remove start)
				// mem::Patch((void*)0x80111D, cancel), // plant ??? cancel
				// mem::Patch((void*)0x801176, confirm), // plant ??? confirm
				mem::Patch((void*)0x8012E0, confirm), // plant sex confirm
				mem::Patch((void*)0x8012E7, cancel), // plant sex cancel
				mem::Patch((void*)0x801658, confirm), // plant birthday confirm
				mem::Patch((void*)0x801679, cancel), // plant birthday cancel
				mem::Patch((void*)0x801956, confirm), // plant blood confirm
				mem::Patch((void*)0x80195D, cancel), // plant blood cancel
				mem::Patch((void*)0x801A58, confirm), // plant naitonality confirm
				mem::Patch((void*)0x801A8C, cancel), // plant nationality cancel
				mem::Patch((void*)0x801BC7, confirm), // plant exit confirm
				mem::Patch((void*)0x801BCE, cancel), // plant exit cancel
				// mem::Patch((void*)0x800723, "\xC6\x08"), // plant name exit (start, would ideally be st/sq/tri but no space to check 2B)
			};
			Patcher->Patches.Patches.insert(Patcher->Patches.Patches.end(), nameEntryPatches.begin(), nameEntryPatches.end());
			mem::Detour32((BYTE*)0x800722, (BYTE*)hkJMP_00800722, 9); // plant name exit
		}

		if (ini.GetBoolValue(Category, "Missions", true)) {
			oFUN_00575f00 = (tFUN_Int_IntIntInt)mem::TrampHook32((BYTE*)0x575f00, (BYTE*)hkFUN_00575f00, 6); // input remapper
			oFUN_00575a80 = (tFUN_Int_IntIntInt)mem::TrampHook32((BYTE*)0x575a80, (BYTE*)hkFUN_00575a80, 8); // input remapper find loc
		}

		Patcher->Run();

	}

}
