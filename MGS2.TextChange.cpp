#include "MGS2.framework.h"

namespace MGS2::TextChange {
	char* LifeName;
	char* OlgaName;
	char* MerylName;
	char* FortuneName;
	char* FatmanName;
	char* HarrierName;
	char* KasatkaName;
	char* VampO2;
	char* VampName;
	char* Vamp2Name;
	char* EmmaO2;
	char* EmmaName;
	char* SnakeName;
	char* PrezName;
	char* RayNames[25];
	char* SolidusName;
	char* GurlugonName;
	char* GenolaName;
	char* MechGenolaName;
	char* Grip1Name;
	char* Grip2Name;
	char* Grip3Name;
	char* O2Name;

	mem::PatchSet Patches;


	void HandleConfigItem(CSimpleIniA& ini, const char* category, const char* itemName, char** pName, std::vector<uintptr_t> pointers) {
		unsigned int pDefaultName = *(unsigned int*)pointers[0];
		const char* defaultName = (const char*)pDefaultName;
		
		const char* name = ini.GetValue(category, itemName, defaultName);
		if (strcmp(name, defaultName) == 0) {
			return;
		}

		if (char* cName = (char*)malloc(strlen(name) + 1)) {
			strcpy(cName, name);
			*pName = cName;

			for (uintptr_t ptr : pointers) {
				mem::Patch patch = mem::PatchPointer((BYTE*)ptr, (uintptr_t)pName);
				Patches.Patches.push_back(patch);
			}
		}
	}

	void HandleConfigItem(CSimpleIniA& ini, const char* category, const char* itemName, char** pName, uintptr_t pointer) {
		std::vector<uintptr_t> pointers{ pointer };
		HandleConfigItem(ini, category, itemName, pName, pointers);
	}

	void ToggleAction(Actions::Action action) {
		Patches.Toggle();
		Log::DisplayToggleMessage("Text Change", Patches.Patched);
	}


	void Run(CSimpleIniA& ini) {
		const char* category = "TextChange";

		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		// fallback for old config
		HandleConfigItem(ini, category, "LifeName", &LifeName, std::vector<uintptr_t>{ 0x4b9c70, 0x4ee654 });

		// life gauges
		const char* subcat = "TextChange.LifeGauge";
		HandleConfigItem(ini, subcat, "Life", &LifeName, std::vector<uintptr_t>{ 0x4b9c70, 0x4ee654 });
		HandleConfigItem(ini, subcat, "Olga", &OlgaName, 0x710302);
		HandleConfigItem(ini, subcat, "Meryl", &MerylName, std::vector<uintptr_t>{ 0x4fbeab, 0x7102fb });
		HandleConfigItem(ini, subcat, "Fortune", &FortuneName, 0x727c73);
		HandleConfigItem(ini, subcat, "Fatman", &FatmanName, 0x63c083);
		HandleConfigItem(ini, subcat, "Harrier", &HarrierName, 0x6013ab);
		HandleConfigItem(ini, subcat, "Kasatka", &KasatkaName, std::vector<uintptr_t>{ 0x5f0992, 0x5f7fe2 });
		HandleConfigItem(ini, subcat, "VampO2", &VampO2, 0x5ac646);
		HandleConfigItem(ini, subcat, "Vamp", &VampName, 0x5ac5c8);
		HandleConfigItem(ini, subcat, "Vamp2", &Vamp2Name, 0x705bd2);
		HandleConfigItem(ini, subcat, "EmmaO2", &EmmaO2, 0x4b8797);
		HandleConfigItem(ini, subcat, "Emma", &EmmaName, std::vector<uintptr_t>{ 0x4b8741, 0x4fbe87, 0x70387e });
		HandleConfigItem(ini, subcat, "Snake", &SnakeName, 0x78ce45);
		HandleConfigItem(ini, subcat, "Prez", &PrezName, 0x61b765);
		HandleConfigItem(ini, subcat, "Solidus", &SolidusName, 0x4a482f);
		HandleConfigItem(ini, subcat, "Gurlugon", &GurlugonName, 0x43d107);
		HandleConfigItem(ini, subcat, "Genola", &GenolaName, 0x43d13b);
		HandleConfigItem(ini, subcat, "MechGenola", &MechGenolaName, 0x43d134);
		HandleConfigItem(ini, subcat, "GripLv1", &Grip1Name, 0x4ee6bb);
		HandleConfigItem(ini, subcat, "GripLv2", &Grip2Name, std::vector<uintptr_t>{ 0x4eac06, 0x4ee716 });
		HandleConfigItem(ini, subcat, "GripLv3", &Grip3Name, std::vector<uintptr_t>{ 0x4eab63, 0x4ee79c });
		HandleConfigItem(ini, subcat, "O2", &O2Name, 0x4bf820);

		for (char i = 0; i < 25; i++) {
			std::string rayName = fmt::format("Ray{}", i + 1);
			uintptr_t pRayName = 0x9fa5d0 + (i * 4);
			HandleConfigItem(ini, subcat, rayName.c_str(), &RayNames[i], pRayName);
		}

		Patches.Patched = true;

		Actions::RegisterAction(ini, "TextChange.Toggle", &ToggleAction);
	}

}
