#include "MGS2.framework.h"

namespace MGS2::Ames {

	const char* Category = "Ames";

	short const TargetProgress = 154;

	char* const Position = (char*)0x118FB9F;
	char HostageHour = -1;

	namespace YoureAmes {

		struct YoureAmesPublicData {
			char h1[16] = "BMNDATA.URAMES.";
			int Attempts = 0;
			int FoundTotal = 0;
		} PD;

		bool Active = false;
		bool ShowStats = false;
		bool ResetTimer = false;
		bool ResetStats = true;

		unsigned short PositionCount[20];
		unsigned short RegionCount[5];
		char const Regions[20]{ 0,0,0,0,0,0, 1,1,1,1, 2,2,2, 3,3, 4,4,4,4,4 }; // NW, NE, SW, W, SE
		const char* const RegionAcronyms[5]{ "NW", "NE", "SW", "W", "SE" };

		TextConfig StatsTextConfig = Log::DefaultTextConfig;

		void IncrementCounts() {
			PD.FoundTotal++;

			if (*Position < 20) {
				PositionCount[*Position]++;
				RegionCount[Regions[*Position]]++;
			}
		}

		void Reroll(bool restart = false) {
			*Position = RNG(20);

			if (restart) {
				((void(*)(bool))0x877DE0)(false);
			}
		}

		void CheckEachFrame() {
			short static progress = 0;
			// Reroll and go to next Ames
			if ((progress == TargetProgress) && (*Mem::ProgressPlant > TargetProgress)) {
				IncrementCounts();
				if (ShowStats) {
					char* msg = NULL;
					char curRegion = 0;
					const char* regionAcronym = "Unknown";
					short curRegionCount = 0;
					short curPositionCount = 0;
					if (*Position < 20) {
						curRegion = Regions[*Position];
						regionAcronym = RegionAcronyms[curRegion];
						curRegionCount = RegionCount[curRegion];
						curPositionCount = PositionCount[*Position];
					}
					asprintf(&msg, "%d (%d %s)", PD.FoundTotal, curRegionCount, regionAcronym);
					Log::DisplayText(msg, StatsTextConfig, 5);
					free(msg);
				}
				*Mem::ProgressPlant = TargetProgress;
				Reroll(true);
			}
			progress = *Mem::ProgressPlant;
		}

		void ToggleAction(Actions::Action action) {
			Active = !Active;
			Log::DisplayToggleMessage("Ames Practice Mode", Active);
		}

		void RestartAction(Actions::Action action) {
			if (*Mem::ProgressPlant != TargetProgress) {
				return;
			}

			if (ResetTimer) {
				*Mem::GameTimeFrames = -3;
			}
			if (ResetStats) {
				PD.FoundTotal = 0;
				memset(PositionCount, 0, sizeof(PositionCount));
				memset(RegionCount, 0, sizeof(RegionCount));
			}

			Active = true;
			PD.Attempts++;
			Log::DisplayText("Starting New Attempt", StatsTextConfig);
			Reroll(true);
		}

		bool NewGameInfoCallback() {
			return Active;
		}

	}

		
	tFUN_Void oFUN_00884ca0;
	void __cdecl hkFUN_00884ca0() {
		try_mgs2
			if (HostageHour != -1) {
				*(char*)(*Mem::MainGameData + 0x6F9) = HostageHour;
				*(char*)(*Mem::MainContinueData + 0x6F9) = HostageHour;
			}
		catch_mgs2(Category, "884CA0")

		oFUN_00884ca0();
	}

	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		short static progress;

		try_mgs2
			if (YoureAmes::Active) {
				YoureAmes::CheckEachFrame();
			}
		catch_mgs2(Category, "878F70")

		oFUN_00878f70();
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		HostageHour = ConfigParser::ParseInteger(ini, Category, "HostageHour", -1, 0, 23, true);

		const char* hostageText = nullptr;
		if (HostageHour == 0) {
			hostageText = "Hostages: Beauties";
		}
		else if (HostageHour == 13) {
			hostageText = "Hostages: Beasts";
		}
		else if (HostageHour == 22) {
			hostageText = "Hostages: Old Beauties";
		}
		else {
			hostageText = "Hostages: Mixed";
		}

		if (HostageHour != -1) {
			cb::Callback0<bool> callback(&NewGameInfo::IsTankerNotSelected);
			NewGameInfo::AddNotice(hostageText, callback);
		}

		const char* youreAmesCategory = "Ames.Practice";
		const char* youreAmesRestartCategory = "Ames.Practice.Restart";
		YoureAmes::Active = ini.GetBoolValue(youreAmesCategory, "Active", YoureAmes::Active);
		YoureAmes::ShowStats = ini.GetBoolValue(youreAmesCategory, "ShowStats", YoureAmes::ShowStats);
		YoureAmes::ResetTimer = ini.GetBoolValue(youreAmesRestartCategory, "ResetTimer", YoureAmes::ResetTimer);
		YoureAmes::ResetStats = ini.GetBoolValue(youreAmesRestartCategory, "ResetStats", YoureAmes::ResetStats);
		Actions::RegisterAction(ini, "Ames.Practice.Toggle", &YoureAmes::ToggleAction);
		Actions::RegisterAction(ini, youreAmesRestartCategory, &YoureAmes::RestartAction);
		NewGameInfo::AddWarning("Ames Practice Mode", &YoureAmes::NewGameInfoCallback);

		YoureAmes::StatsTextConfig.PosY = 460;

		oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);
		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
	}

}
