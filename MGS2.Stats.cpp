#include "MGS2.framework.h"
#include "lib/fmt/chrono.h"

namespace MGS2::Stats {
	const char* Category = "Stats";

	struct Profile {
		bool Enabled = true;
		bool Codename = true;
		bool BigBossComparison = false;
		TextConfig NamesConfig;
		TextConfig ValuesConfig;
		TextConfig BigBossConfig;
		std::vector<std::string> ExcludedFields;
		fmt::runtime_format_string<> ValuesFormat;
		fmt::runtime_format_string<> NamesFormat;
		size_t Lines = 0;
	};

	char TempStore[0x100];
	bool PauseActive = false;
	Profile* LastUsedProfile = nullptr;
	
	const int yPos = 436;
	Profile PausedProfile{ true, true, false, { true, 145, yPos, Right }, { true, 185, yPos, Center }, { true, 255, yPos, Center } };
	Profile InGameProfile{ false, false, false, 
		{ true, 145, yPos, Right }, { true, 185, yPos, Center }, { true, 255, yPos, Center },
		{ "GameTime", "Saves", "MechKills", "ClearingEscapes" } };


	void DrawStats(Profile& profile, bool refresh = false) {
		static const char* codename = nullptr;
		static std::string namesText = "";
		static std::string valuesText = "";
		static std::string bigBossText = "";
		static bool seaLouse, specialItems;
		static int codenameId, areaTime, gameTime;
		static short level, shotsFired, alerts, continues, rations, kills, damageTaken, saves, mechsDestroyed, clearings;

		bool showCodename = profile.Codename && (*Mem::ProgressTanker || *Mem::ProgressPlant);

		static ValueTracker<int> Tracker;
		Tracker.Reset();

		uintptr_t pStats = *Mem::MainGameStats;
		gameTime = Tracker.Update(12, *(int*)(pStats + 0x138));

		bool updated = false;
		if (refresh || Tracker.IsUpdated(true)) {
			level = Tracker.Update(0, *(int*)(pStats + 0x158A));
			areaTime = *(int*)(pStats + 0xE4);
			shotsFired = Tracker.Update(1, *(short*)(pStats + 0x140));
			alerts = Tracker.Update(2, *(short*)(pStats + 0x142));
			continues = Tracker.Update(3, *(short*)(pStats + 0x132));
			rations = Tracker.Update(4, *(short*)(pStats + 0x1590));
			kills = Tracker.Update(5, *(short*)(pStats + 0x144));
			damageTaken = Tracker.Update(6, *(short*)(pStats + 0x146));
			saves = Tracker.Update(7, *(short*)(pStats + 0x136));
			mechsDestroyed = Tracker.Update(8, *(short*)(pStats + 0x158));
			clearings = Tracker.Update(9, *(short*)(pStats + 0x1592));
			seaLouse = Tracker.Update(10, *(short*)(pStats + 0x158C));
			specialItems = Tracker.Update(11, *(char*)(pStats + 0x1596) & 0x20);

			char modFrames = gameTime % 3600;
			if ((!modFrames) || (modFrames == 60)) { // if gametime is xx:00 or xx:01 secs
				updated = true;
			}
		}		

		if (refresh || Tracker.IsUpdated() || updated) {

			if (showCodename) {
				// handle mandatory alerts
				short alertAdjustment = 0;
				short progressPlant = *Mem::ProgressPlant;
				if ((level & 0xF) != 0xD) { // not tanker-only
					if (progressPlant >= 403) {
						alertAdjustment = 0;
					}
					else if (progressPlant >= 397) {
						alertAdjustment = 1;
					}
					else if (progressPlant >= 313) {
						alertAdjustment = 2;
					}
					else {
						alertAdjustment = 3;
					}

					if (alertAdjustment) {
						*(short*)(pStats + 0x142) += alertAdjustment;
					}
				}

				codenameId = ((int(__cdecl*)(uintptr_t))0x945930)((uintptr_t)TempStore);
				codename = (const char*)*(int*)(0xA13D98 + (codenameId * 4));

				if (alertAdjustment) {
					*(short*)(pStats + 0x142) -= alertAdjustment;
				}
			}

			std::chrono::seconds dGameTime(gameTime / 60);

			namesText = fmt::format(
				profile.NamesFormat,
				//"Codename"_a = codename ? codename : "",
				"GameTime"_a = dGameTime,
				"Alerts"_a = alerts,
				"Continues"_a = continues,
				"Rations"_a = rations,
				"Kills"_a = kills,
				"DamageTaken"_a = damageTaken,
				"Saves"_a = saves,
				"ClearingEscapes"_a = clearings,
				"MechKills"_a = mechsDestroyed,
				"AmmoUsed"_a = shotsFired
			);

			valuesText = fmt::format(
				profile.ValuesFormat,
				//"Indicator"_a = (*Mem::Difficulty >= 50) ? "YOU" : "",
				//"Codename"_a = codename ? codename : "",
				"GameTime"_a = dGameTime,
				"Alerts"_a = alerts,
				"Continues"_a = continues,
				"Rations"_a = rations,
				"Kills"_a = kills,
				"DamageTaken"_a = damageTaken,
				"Saves"_a = saves,
				"ClearingEscapes"_a = clearings,
				"MechKills"_a = mechsDestroyed,
				"AmmoUsed"_a = shotsFired
			);

			if (profile.BigBossComparison && (*Mem::Difficulty >= Difficulty::Extreme)) {
				const std::chrono::minutes dBigBossTime(179);

				bigBossText = fmt::format(
					profile.ValuesFormat,
					//"Indicator"_a = "BB",
					"GameTime"_a = dBigBossTime,
					//"Codename"_a = "",
					"Alerts"_a = 3,
					"Continues"_a = 0,
					"Rations"_a = 0,
					"Kills"_a = 0,
					"DamageTaken"_a = (*Mem::Difficulty == Difficulty::EuroExtreme) ? 279 : 499,
					"Saves"_a = 8,
					"ClearingEscapes"_a = "",
					"MechKills"_a = "",
					"AmmoUsed"_a = 700
				);
			}
			else if (!bigBossText.empty()) {
				bigBossText.clear();
			}

#ifdef _DEBUG
			Log::DisplayText(fmt::format("Stat display updated at RF {}", *Mem::RenderedFrames));
#endif
		}

		if (showCodename) {
			TextConfig codenameConfig{ true, profile.NamesConfig.PosX - 25, profile.NamesConfig.PosY - 20, Center };
			codenameConfig.Draw(codename);
		}

		profile.NamesConfig.Draw(namesText.c_str());
		profile.ValuesConfig.Draw(valuesText.c_str());


		if ((*Mem::Difficulty >= 50) && (!bigBossText.empty())) {
			profile.BigBossConfig.Draw(bigBossText.c_str());
		}
	}


	short const PAUSEFLAG_NONE = 0;
	short const PAUSEFLAG_FADEIN = 1;
	short const PAUSEFLAG_PAUSED = 2;
	short const PAUSEFLAG_FADEOUT = 3;
	
	tFUN_Void_Int oFUN_0083b2e0;
	void __cdecl hkFUN_0083b2e0(int param_1) {
		oFUN_0083b2e0(param_1);

		static bool refreshNeeded = true;

		try_mgs2
			if (!PausedProfile.Enabled) {
				refreshNeeded = true;
				return;
			}

			short pauseFlag = *(short*)(param_1 + 0x54); // 0 = no pause, 1 = fadein, 2 = pause, 3 = fadeout
			if ((pauseFlag != PAUSEFLAG_FADEIN) && (pauseFlag != PAUSEFLAG_PAUSED)) {
				refreshNeeded = true;
				return;
			}

			DrawStats(PausedProfile, refreshNeeded);

			refreshNeeded = false;
			PauseActive = true;
		catch_mgs2(Category, "83B2E0");
	}


	tFUN_Int_Int oFUN_0058fca5;
	int __cdecl hkFUN_0058fca5(int param_1) {
		int result = oFUN_0058fca5(param_1);

		static bool refreshNeeded = true;

		try_mgs2
			if (!PausedProfile.Enabled) {
				refreshNeeded = true;
				return result;
			}

			short pauseFlag = *(short*)(param_1 + 0x4e); // 0 = no pause, 1 = fadein, 2 = pause, 3 = fadeout
			if ((pauseFlag != PAUSEFLAG_FADEIN) && (pauseFlag != PAUSEFLAG_PAUSED)) {
				refreshNeeded = true;
				return result;
			}

			if (*(short*)(*(uintptr_t*)0xA01F34 + 0xFC) == 0) {
				return result;
			}

			DrawStats(PausedProfile, refreshNeeded);

			refreshNeeded = false;
			PauseActive = true;
		catch_mgs2(Category, "58FCA5");

		return result;
	}

	tFUN_Void_Int oFUN_008781e0;
	void __cdecl hkFUN_008781e0(int param_1) {
		oFUN_008781e0(param_1);

		static bool refreshNeeded = true;

		try_mgs2
			if (!InGameProfile.Enabled) {
				refreshNeeded = true;
				return;
			}

			if (PauseActive) {
				PauseActive = false;
				refreshNeeded = true;
				return;
			}

			if (*(short*)(*(uintptr_t*)0xA01F34 + 0xFC) == 0) {
				return;
			}

			DrawStats(InGameProfile, refreshNeeded);

			refreshNeeded = false;
		catch_mgs2(Category, "8781E0");
	}

	void ToggleProfile(Actions::Action action) {
		Profile* profile = (Profile*)(action.Data);
		profile->Enabled = !profile->Enabled;
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		std::vector<std::pair<std::string, std::string>> parts{
			{ "GameTime", "Elapsed Time" },
			{ "Saves", "Saves" },
			{ "Continues", "Continues" },
			{ "Alerts", "Alerts" },
			{ "Kills", "Enemy Kills" },
			{ "Rations", "Rations Used" },
			{ "DamageTaken", "Damage Taken" },
			{ "AmmoUsed", "Ammo Used" },
			{ "MechKills", "Mech Kills" },
			{ "ClearingEscapes", "Clearing Escs" },
		};

		std::vector<std::pair<const char*, Profile&>> profiles{
			{ "Stats.Paused", PausedProfile },
			{ "Stats.InGame", InGameProfile },
		};
		for (auto& p : profiles) {
			const char* subCategory = p.first;
			std::string sFieldsCategory = std::string(subCategory) + ".Fields";
			const char* fieldsCategory = sFieldsCategory.c_str();
			Profile& profile = p.second;

			std::string* sKeyFormat = new std::string();
			std::string* sValueFormat = new std::string();
			sKeyFormat->reserve(0x70);
			sValueFormat->reserve(0x70);

			profile.Codename = ini.GetBoolValue(fieldsCategory, "Codename", profile.Codename);

			size_t i = 0;
			for (auto& part : parts) {
				std::string& key = part.first;

				auto result = std::find(profile.ExcludedFields.begin(), profile.ExcludedFields.end(), key);
				if (ini.GetBoolValue(fieldsCategory, key.c_str(), (result == profile.ExcludedFields.end()))) {
					if (i++) {
						*sKeyFormat += "\n";
						*sValueFormat += "\n";
					}

					if (key == "GameTime") {
						*sKeyFormat += "{GameTime:%H:%M}";
						*sValueFormat += part.second.c_str();
					}
					else {
						*sKeyFormat += "{";
						*sKeyFormat += key.c_str();
						*sKeyFormat += "}";
						*sValueFormat += part.second.c_str();
					}
				}
			}
			profile.ValuesFormat = fmt::runtime(*sKeyFormat);
			profile.NamesFormat = fmt::runtime(*sValueFormat);

			profile.Enabled = ini.GetBoolValue(subCategory, "Active", profile.Enabled);
			profile.BigBossComparison = ini.GetBoolValue(subCategory, "CompareBigBoss", profile.BigBossComparison);

			profile.Lines = i;
			profile.NamesConfig.PosY -= i * TextConfig::LineHeight;
			profile.ValuesConfig.PosY = profile.BigBossConfig.PosY = profile.NamesConfig.PosY;

			Actions::RegisterAction(ini, subCategory, &ToggleProfile, &profile);
		}

		oFUN_0058fca5 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x58FCA5, (BYTE*)hkFUN_0058fca5, 8);
		oFUN_0083b2e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x83B2E0, (BYTE*)hkFUN_0083b2e0, 7);
		oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);
	}

}
