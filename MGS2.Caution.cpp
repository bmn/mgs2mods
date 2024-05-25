#include "MGS2.framework.h"

namespace MGS2::Caution {
	const char* Category = "Caution";

	mem::PatchSet Patches;

	bool Active = true;
	bool RandomMode = false;
	bool RandomSeeded = true;
	char AlertLevel = 3;

	unsigned int const MAX_SEED = 1000000;
	unsigned int RandomSeed = 0;

	const char* const AlertLevelNames[4]{
		"Infiltration",
		"Alert",
		"Evasion",
		"Caution"
	};
	const char* CurrentLevelName = nullptr;

	
	bool NewGameInfoCallback() {
		return (Active && (!RandomMode));
	}

	bool NewGameInfoRandomCallback() {
		return (Active && RandomMode);
	}

	void RegisterNewGameWarning() {
		NewGameInfo::RemoveWarning(CurrentLevelName);
		CurrentLevelName = AlertLevelNames[AlertLevel];
		NewGameInfo::AddWarning(CurrentLevelName, &NewGameInfoCallback);
	}

	bool ChangeAlertLevel(char newLevel) {
		if (newLevel == AlertLevel) {
			return false;
		}
		if ((newLevel == AlertMode::Caution) || (AlertLevel == AlertMode::Caution)) {
			Patches.Patch(AlertLevel == AlertMode::Caution);
		}
		Active = true;
		AlertLevel = newLevel;
		RegisterNewGameWarning();
		return true;
	}

	void ChangeAlertLevel(Actions::Action action) {
		char newLevel = static_cast<char>((unsigned int)action.Data);
		if (ChangeAlertLevel(newLevel)) {
			Log::DisplayText(std::string("[Caution] Set Alert Level to ") + AlertLevelNames[AlertLevel]);
		}
	}

	void NextAlertLevel(Actions::Action action) {
		unsigned int newLevel = (AlertLevel == AlertMode::Caution) ? 0 : AlertLevel + 1;
		action.Data = (void*)newLevel;
		ChangeAlertLevel(action);
	}


	void SetRandomSeed() {
		RandomSeed = MGS2::RNG(MAX_SEED);
	}

	std::vector<char> const RandomWeights{ 2, 1, 1, 2 };
	char const GameChoosesWeight = 1;
	char const TotalRandomWeight = 6;
	char GetRandomLevel(unsigned int rng) {
		char totalWeight = TotalRandomWeight;
		if (*Mem::Difficulty != Difficulty::VeryEasy) {
			totalWeight += GameChoosesWeight;
		}

		rng >>= 16;
		char choice = rng % totalWeight;

		char i = 0;
		for (char weight : RandomWeights) {
			if (choice < weight) {
				return i; // chosen level
			}
			choice -= weight;
			i++;
		}
		return -1; // game chooses
	}

	void RandomiseAlertLevel() {
		char newLevel;
		bool isNewSeed = false;

		if (RandomSeeded) {
			if (!RandomSeed) {
				SetRandomSeed();
				isNewSeed = true;
			}

			unsigned int areaId = *(unsigned int*)(*(uintptr_t*)Mem::CurrentData0 + 0xBC) & 0x00FFFFFF;
			unsigned int progress = *Mem::ProgressTanker + *Mem::ProgressPlant;
			for (size_t i = 1; i <= 5; i++) {
				progress += Mem::ProgressTalesArray[i];
			}
			
			unsigned int randomId = areaId + progress * *Mem::VRMissionID + RandomSeed;
			unsigned int randomResult = (randomId * 0x5d588b65) + 1;

			newLevel = GetRandomLevel(randomResult);
		}
		else {
			newLevel = GetRandomLevel(*Mem::Random);
		}
		
		//-1 = Keep previous
		// 0 = Infiltration
		// 1 = Alert
		// 2 = Caution
		// 3 = Evasion
		ChangeAlertLevel(newLevel);

#ifdef _DEBUG
		const char* levelName = (newLevel == -1) ? "Game Choice" : AlertLevelNames[newLevel];

		if (isNewSeed) {
			Log::DisplayText(
				fmt::format("[Caution] New Seed {:d} > AlertLevel {:d} ({})", RandomSeed, newLevel, levelName),
				5.0);
		}
		else {
			Log::DisplayText(fmt::format("[Caution] RNG AlertLevel {:d} ({})", newLevel, levelName));
		}
#endif

	}

	// each frame on main menu
	tFUN_Void_IntIntInt oFUN_00766930;
	void __cdecl hkFUN_00766930(int param_1, int param_2, int param_3) {
		try_mgs2
			RandomSeed = 0;
		catch_mgs2(Category, "766930")

		oFUN_00766930(param_1, param_2, param_3);
	}

	int(__cdecl* oFUN_008df0b0)(char param_1, int param_2);
	int __cdecl hkFUN_008df0b0(char param_1, int param_2) {
		int result = oFUN_008df0b0(param_1, param_2);

		try_mgs2
			bool bail = false;

			if (mem::SetCurrentHookParent(_ReturnAddress()) != 0x42DE2F) {
				bail = true;
			}

			mem::ClearCurrentHookParent();

			if (bail || (!Active)) {
				return result;
			}

			const char* areaCode = (char*)0x118ADEC;
			const char* characterCode = (char*)0x118C374;
			short tankerProgress = *(short*)0x118D93C;
			short plantProgress = *(short*)0x118D912;

			// reached guard rush
			if (strcmp(characterCode, "r_tnk0") == 0) {
				if (tankerProgress >= 31) {
					return result;
				}
			}

			// in plant
			else if (plantProgress != 0) {
				if (
					(plantProgress <= 21) || // in dock
					(plantProgress == 379) || // asc colon 1
					(plantProgress == 154) || // ames
					(strcmp(areaCode, "w25a") == 0) || // before harrier
					(strcmp(areaCode, "w25b") == 0) || // after harrier
					(plantProgress >= 382) ||
					false
					) {
					return result;
				}
			}

			//  setting <= 1  =  setting
			//	setting == 3  =  enter(or 3 if enter == 0)
			//	setting == 2  =  2 (or 1 if enter == 1)

			if (RandomMode) {
				RandomiseAlertLevel();
				return (AlertLevel == -1) ? result : AlertLevel;
			}

			if ((AlertLevel == AlertMode::Evasion) && (result == AlertMode::Alert)) {} // keep at 1 if already 1
			else if ((AlertLevel == AlertMode::Caution) && (result > 0)) {} // keep at 1/2/3 if already 1/2/3 (not -1 or 0)
			else if (AlertLevel != -1) {
				// set to AlertLevel if...
				// AL is 0 (VE-like behaviour)
				// AL is 1 (Alerts everywhere)
				// AL is 2 and not already 1
				// AL is 3 and not 1 or 2
				result = AlertLevel;
			}

			return result;
		catch_mgs2(Category, "8DF0B0")

		return result;
	}
	

	void TogglePatches(bool patch = false) {
		Active = patch;
		//if (AlertLevel == 3) {
		bool doPatch = ((AlertLevel == AlertMode::Caution) && Active);
			Patches.Patch(!doPatch);
		//}
	}

	void ToggleAction(Actions::Action action) {
		TogglePatches(!Active);
		Log::DisplayToggleMessage("Caution", Active);
	}

	void ToggleRandomMode(Actions::Action action) {
		RandomMode = !RandomMode;
		if ((RandomMode) && (!Active)) {
			TogglePatches(true);
		}
		Log::DisplayToggleMessage("Random Alerts", RandomMode);
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false)))
		{
			return;
		}

		Actions::RegisterAction(ini, "Caution.Toggle", &ToggleAction);

		const char* randomCategory = "Caution.Random";
		RandomMode = ini.GetBoolValue(randomCategory, "Active", RandomMode);
		RandomSeeded = ini.GetBoolValue(randomCategory, "Seeded", RandomSeeded);
		Actions::RegisterAction(ini, randomCategory, &ToggleRandomMode);

		std::map<std::string, char> alertLevelMap{
			{ "infiltration", AlertMode::Infiltration },
			{ "alert", AlertMode::Alert },
			{ "evasion", AlertMode::Evasion },
			{ "caution", AlertMode::Caution },
		};
		AlertLevel = ConfigParser::ParseValueMap(ini, Category, "AlertLevel", alertLevelMap, (char)-1);
		if (AlertLevel == -1) {
			AlertLevel = ConfigParser::ParseInteger(ini, Category, "AlertLevel", 3, 0, 3, true);
		}

		RegisterNewGameWarning();
		NewGameInfo::AddWarning("Random Alerts", &NewGameInfoRandomCallback);

		Patches = mem::PatchSet({
			// stop the timer during caution
			mem::Patch((void*)0x42CFF9, "\x90\x90\x90\x90\x90\x90"),
			// replace the caution timer restore with a flat 60 secs
			mem::Patch((void*)0x42B1A0, "\x50\x66\xB8\x10\x0E\x90\xA3\xC8\x60\xA1\x00\x58", 12)
			});
		
		if (Active = ini.GetBoolValue(Category, "Active", Active))
		{
			TogglePatches(true);
		} 

		char* actionCategory = new char[21] { "Caution.AlertLevel.0" };
		for (char i = 0; i <= 3; i++) {
			actionCategory[19] = i + '0';
			Actions::RegisterAction(ini, actionCategory, &ChangeAlertLevel, (void*)i);
		}
		Actions::RegisterAction(ini, "Caution.AlertLevel.Next", &NextAlertLevel);

		oFUN_008df0b0 = (int(__cdecl*)(char param_1, int param_2))mem::TrampHook32((BYTE*)0x8DF0B0, (BYTE*)hkFUN_008df0b0, 5);
		oFUN_00766930 = (tFUN_Void_IntIntInt)mem::TrampHook32((BYTE*)0x766930, (BYTE*)hkFUN_00766930, 6);

	}

}