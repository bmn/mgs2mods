#include "MGS2.framework.h"
#include <regex>
#include "intrin.h"

namespace MGS2::SaveMenu {
	const char* Category = "SaveMenu";

	int const MAX_SAVE_FILES = 100;

	char const DATAMOD_OP_SET = 0;
	char const DATAMOD_OP_AND = 1;
	char const DATAMOD_OP_OR = 2;
	char const DATAMOD_OP_NOT = 3;

	namespace Customisation {
		struct DataMod {
			char Section = 0;
			uintptr_t Offset = 0;
			char Operator = DATAMOD_OP_SET;
			char* Data = nullptr;
			size_t Length = 0;
		};

		struct Entry {
			const char* Directory = nullptr;
			const char* Title = nullptr;
			char AlertLevel = 0;
			short CautionTimer = 0;
			bool RestoreOnContinue;
			std::vector<DataMod> DataMods;
		};
		Entry Entries[MAX_SAVE_FILES];
		Entry* StandbyEntry = nullptr;
		bool UseStandby = true;
		BYTE Data0[0x566 * 4];
		BYTE Data1[0x700 * 4];

		bool EnableTitle = true;
		bool EnableAlert = false;
		bool EnableData = false;
		bool RestoreOnContinue = false;
		

		void HandleIniEntry(CSimpleIniA& ini, Entry& entry, const char* category = "Custom") {
			if (!ini.GetSection(category)) {
				goto HandleData;
			}

			if (EnableTitle) {
				const char* str = ini.GetValue(category, "Title", "");
				if (strcmp(str, "")) {
					if (char* title = (char*)calloc(1, strlen(str) + 1)) {
						strncpy(title, str, strlen(str));
						entry.Title = title;
					}
				}
			}

			if (EnableAlert) {
				entry.AlertLevel = ini.GetBoolValue(category, "Alert", false) ? 2 : 0;
				entry.CautionTimer = ConfigParser::ParseInteger<short>(ini, category, "Caution", entry.CautionTimer, 0, SHRT_MAX, true);
				if ((entry.CautionTimer > 0) && (entry.AlertLevel == 0)) {
					entry.AlertLevel = 3;
				}
			}

			entry.RestoreOnContinue = ini.GetBoolValue(category, "RestoreOnContinue", RestoreOnContinue);

			HandleData:
			if (!EnableData) {
				return;
			}

			std::string dataCategory = std::string(category) + ".Data";
			auto values = ini.GetSection(dataCategory.c_str());

			if (!values) {
				return;
			}

			for (auto& value : *values) {
				const char* rOffset = value.first.pItem;
				const char* rValue = value.second;

				char section = 0;
				uintptr_t offset = 0;
				size_t invalidOffset = 0x566 * 4;

				if (rOffset[1] == ':') {
					char rSection = rOffset[0];
					if (rSection == '1') {
						section = 1;
						invalidOffset = 0x700 * 4;
					}
					else if (rSection != '0') {
						continue;
					}
					
					rOffset = (const char*)(rOffset + 2);
				}

				try {
					offset = std::stoi(rOffset, nullptr, 16);
					if ((offset < 0) || (offset >= invalidOffset)) {
						continue;
					}
				}
				catch (std::invalid_argument) {
					continue;
				}
				catch (std::out_of_range) {
					continue;
				}
				

				char* data = nullptr;
				char op = DATAMOD_OP_SET;
				size_t length = 4;
				if (rValue[1] == ':') {
					switch (rValue[0]) {
					case 'S':
					case 's':
						length = 0;
						break;
					case '4':
						length = 4;
						break;
					case '2':
						length = 2;
						break;
					case '1':
						length = 1;
						break;
					default:
						continue;
					}
					rValue = (const char*)(rValue + 2);
				}

				if (length > 0) {
					switch (rValue[0]) {
					case '&':
						op = DATAMOD_OP_AND;
						rValue = (const char*)(rValue + 1);
						break;
					case '|':
						op = DATAMOD_OP_OR;
						rValue = (const char*)(rValue + 1);
						break;
					case '~':
						op = DATAMOD_OP_NOT;
						rValue = (const char*)(rValue + 1);
						break;
					default:
						continue;
					}

					try {
						int val = std::stoi(rValue, nullptr, 0);
						if (data = (char*)calloc(1, length)) {
							switch (length) {
							case 4:
								*(int*)data = val;
								break;
							case 2:
								if ((val < SHRT_MIN) || (val > SHRT_MAX)) {
									continue;
								}
								*(short*)data = (short)val;
								break;
							case 1:
								if ((val < CHAR_MIN) || (val > CHAR_MAX)) {
									continue;
								}
								*(char*)data = (char)val;
								break;
							}
						}
					}
					catch (std::invalid_argument) {
						length = 0;
						op = DATAMOD_OP_SET;
					}
					catch (std::out_of_range) {
						continue;
					}
				}
				
				if (length == 0) {
					length = strlen(rValue) + 1;
					if (data = (char*)calloc(1, length)) {
						strncpy(data, rValue, length - 1);
					}
					else {
						continue;
					}
				}
				
				entry.DataMods.push_back({ section, offset, op, data, length });
			}
		}

		void HandleCustomIni(std::filesystem::path path, bool tales, Entry* entry = nullptr) {
			auto iniPath = path.c_str();
			if (!std::filesystem::exists(iniPath) && std::filesystem::is_regular_file(iniPath)) {
				return;
			}

			CSimpleIniA ini;
			ini.LoadFile(iniPath);

			if (entry == nullptr) {
				for (auto& e : Entries) {
					if (e.Directory == nullptr) {
						continue;
					}
					HandleIniEntry(ini, e, e.Directory);
				}
			}
			else {
				HandleIniEntry(ini, *entry);
			}
		}

		void ReloadEntries(int param_2, char saveModeFlags = 0x20) {

			memset(Entries, 0, sizeof(Entries));

			for (size_t i = 0; i < MAX_SAVE_FILES; i++) {
				uintptr_t pSaveInfo = (i * 0x2a4 + *(int*)(param_2 + 0x128));
				const char* dirName = (const char*)(pSaveInfo + 0x245);
				
				if (strcmp(dirName, "") == 0) {
					continue;
				}

				Entries[i].Directory = dirName;
			}

			const char* baseDirPath = (const char*)Mem::SaveGameBasePath;
			if (!std::filesystem::exists(baseDirPath) && std::filesystem::is_directory(baseDirPath)) {
				return;
			}
			
			bool tales = ((saveModeFlags & 0x20) == 0);

			std::filesystem::path customBasePath = std::string(baseDirPath);;
			customBasePath.append("custom.ini");
			HandleCustomIni(customBasePath, tales);

			for (auto& entry : Entries) {
				if (entry.Directory == nullptr) {
					continue;
				}
				std::filesystem::path customSavePath = std::string(baseDirPath);
				customSavePath.append(entry.Directory);
				customSavePath.append("custom.ini");

				HandleCustomIni(customSavePath, tales, &entry);
			}
		}

		tFUN_Void_Int oFUN_00761e37;
		void __cdecl hkFUN_00761e37(int param_1) {
			oFUN_00761e37(param_1);

			try_mgs2
				uintptr_t ptr = *(uintptr_t*)0xA20604;
				char saveModeFlags = *(char*)(ptr + 0x44); // 0x10=Photo, 0x20=Main, 0x40=Missions, 0x80=Tales
				if ((saveModeFlags & 0xA0) != 0) {
					ReloadEntries(ptr, saveModeFlags);
				}
			catch_mgs2(Category, "761E37");
		}

		tFUN_Void_IntInt oFUN_00770a7a;
		void __cdecl hkFUN_00770a7a(int param_1, int param_2) {
			try_mgs2
				int index = *(int*)(param_2 + 0x20);
				StandbyEntry = &Entries[index];
				UseStandby = true;
			catch_mgs2(Category, "770A7A");

			oFUN_00770a7a(param_1, param_2);
		}

		int(__cdecl* oFUN_008df0b0)(char param_1, int param_2);
		int __cdecl hkFUN_008df0b0(char param_1, int param_2) {
			// TODO how to do the catch macro here properly?

			if (mem::SetCurrentHookParent(_ReturnAddress()) != 0x42DE2F) {
				goto bail;
			}
			
			// alert/caution check not required, as this hook wasn't registered if they're not enabled
			// if ((!EnableAlert && !EnableCaution) || (StandbyEntry == nullptr)) { goto bail; }

			if (!UseStandby) {
				StandbyEntry = nullptr;
			}

			UseStandby = false; // for next load

			if ((!EnableAlert) || (StandbyEntry == nullptr)) {
				goto bail;
			}
			
			if (StandbyEntry->AlertLevel != 0) {
				oFUN_008df0b0(param_1, param_2);
				*(int*)0xF6DE10 = StandbyEntry->CautionTimer;

				char result = StandbyEntry->AlertLevel;
						
				return (int)result;
			}
		bail:
			int result = oFUN_008df0b0(param_1, param_2);
			mem::ClearCurrentHookParent();
			return result;
		}

		
		tFUN_Void_Int oFUN_00877de0;
		void __cdecl hkFUN_00877de0(int param_1) {
			try_mgs2
				if ((param_1 == 0) && (StandbyEntry != nullptr)) {
					UseStandby = true;

					if (EnableData && StandbyEntry->RestoreOnContinue) {
						memcpy_s(*(void**)Mem::ContinueData0, sizeof(Data0), Data0, sizeof(Data0));
						memcpy_s(*(void**)Mem::ContinueData1, sizeof(Data1), Data1, sizeof(Data1));
					}
				}
			catch_mgs2(Category, "877DE0");

			oFUN_00877de0(param_1);
		}
		

		tFUN_Void_IntInt oFUN_00770443;
		void __cdecl hkFUN_00770443(int param_1, int param_2) {
			oFUN_00770443(param_1, param_2);

			try_mgs2
				int index = *(int*)(param_2 + 0x20);

				if ((index < 0) || (index >= MAX_SAVE_FILES)) {
					return;
				}

				Entry* entry = &Entries[index];
				if ((entry == nullptr) || (entry->Title == nullptr)) {
					return;
				}

				const char* title = entry->Title;

				uintptr_t ptr = param_2 + 0x7b8 + *(int*)(param_2 + 0x808) * 0x28;

				((void(__cdecl*)(int))0x76fd76)(ptr);
				auto titleFunc = (void(__cdecl*)(int, const char*, const char*))0x76fd09;
				titleFunc(ptr, "%s", "");
				titleFunc(ptr, "%s", title);

			catch_mgs2(Category, "770443");
		}

		tFUN_Int_IntInt oFUN_00745460;
		int __cdecl hkFUN_00745460(int param_1, int param_2) {
			uintptr_t data0, data1, section;
			int result = oFUN_00745460(param_1, param_2);

			if (StandbyEntry == nullptr) {
				return result;
			}

			data0 = *(uintptr_t*)Mem::ContinueData0;
			data1 = *(uintptr_t*)Mem::ContinueData1;

			if (StandbyEntry->DataMods.empty()) {
				goto bail;
			}

			if ((data0 == 0) || (data1 == 0)) {
				goto bail;
			}

			for (auto& mod : StandbyEntry->DataMods) {
				section = mod.Section ? data1 : data0;

				if (mod.Operator == DATAMOD_OP_SET) {
					memcpy_s((void*)(section + mod.Offset), mod.Length, mod.Data, mod.Length);
					break;
				}

				unsigned int data = *(unsigned int*)(section + mod.Offset);
				unsigned int opData = *(unsigned int*)mod.Data & (0xFFFFFFF >> 8*(4 - mod.Length));

				switch (mod.Operator) {
				case DATAMOD_OP_AND:
					data &= opData;
					break;
				case DATAMOD_OP_OR:
					data |= opData;
					break;
				case DATAMOD_OP_NOT:
					data &= (0xFFFFFFFF - opData);
					break;
				}

				memcpy_s((void*)(section + mod.Offset), mod.Length, &data, mod.Length);
			}

		bail:

			memcpy_s(Data0, sizeof(Data0), (void*)data0, sizeof(Data0));
			memcpy_s(Data1, sizeof(Data1), (void*)data1, sizeof(Data1));

			return result;
		}

		

		void Run(CSimpleIniA& ini, const char* category) {
			EnableTitle = ini.GetBoolValue(category, "CustomTitle", EnableTitle);
			EnableAlert = ini.GetBoolValue(category, "CustomAlert", EnableAlert);
			RestoreOnContinue = ini.GetBoolValue(category, "RestoreOnContinue", RestoreOnContinue);

			if (EnableTitle || EnableAlert || EnableData) {
				oFUN_00761e37 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x761E37, (BYTE*)hkFUN_00761e37, 5); // atpt 2
			}

			if (EnableAlert || EnableData) {
				oFUN_00877de0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x877DE0, (BYTE*)hkFUN_00877de0, 6);
				oFUN_00745460 = (tFUN_Int_IntInt)mem::TrampHook32((BYTE*)0x745460, (BYTE*)hkFUN_00745460, 5);
			}

			if (EnableTitle) {
				oFUN_00770443 = (tFUN_Void_IntInt)mem::TrampHook32((BYTE*)0x770443, (BYTE*)hkFUN_00770443, 6);
			}

			if (EnableAlert) {
				oFUN_008df0b0 = (int (__cdecl*)(char param_1, int param_2))mem::TrampHook32((BYTE*)0x8DF0B0, (BYTE*)hkFUN_008df0b0, 5);
				oFUN_00770a7a = (tFUN_Void_IntInt)mem::TrampHook32((BYTE*)0x770A7A, (BYTE*)hkFUN_00770a7a, 5);
			}
		}
	}


	tFUN_007902a5 oFUN_007902a5;
	void __cdecl hkFUN_007902a5(int i1_pMain, int i2_pSaveMenu, unsigned int ui3_controllerInput) {
		try_mgs2
			int* pCurrent = (int*)(i2_pSaveMenu + 0x20);
			int current = *pCurrent;
			int max = *(int*)(i2_pSaveMenu + 0x24);

			// L1/L2
			if (Actions::ShortcutActive("SaveMenu.FirstPage")) {
				if (current > 9) {
					*pCurrent = 10;
					ui3_controllerInput = 0x8000;
				}
			}
			// R1/R2
			else if (Actions::ShortcutActive("SaveMenu.LastPage")) {
				int lastPage = ((max - 1) / 10);
				if ((current / 10) < lastPage) {
					*pCurrent = (lastPage * 10) - 1;
					ui3_controllerInput = 0x2000;
				}
			}
		catch_mgs2(Category, "7902A5");

		oFUN_007902a5(i1_pMain, i2_pSaveMenu, ui3_controllerInput);
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		Actions::RegisterShortcut(ini, "SaveMenu.FirstPage");
		Actions::RegisterShortcut(ini, "SaveMenu.LastPage");

		if (ini.GetBoolValue(Category, "IncreaseMaxSaves", true)) {
			const char* hunner = "\x64";
			mem::PatchMemory((void*)0x7619A8, hunner); // Main game limit for viewing
			mem::PatchMemory((void*)0x7619DC, hunner); // Snake Tales limit for viewing
			mem::PatchMemory((void*)0x77109F, hunner); // Main game limit for new save
			mem::PatchMemory((void*)0x7710A8, hunner); // Snake Tales limit for new save
			
			//mem::PatchMemory((void*)0x7619C2, "\x64"); // Missions limit for viewing
			//mem::PatchMemory((void*)0x771091, "\x90\x90\x90\x90\x90\x90\x90\x90\x90"); // Disable forced new save limit
		}

		oFUN_007902a5 = (tFUN_007902a5)mem::TrampHook32((BYTE*)0x7902A5, (BYTE*)hkFUN_007902a5, 7);

		Customisation::Run(ini, Category);
	}

}
