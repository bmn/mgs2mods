#include "MGS2.framework.h"
#include <chrono>

namespace MGS2::NewGameInfo {
	const char* Category = "NewGameInfo";

	int CurrentScreen = 0;
	int CurrentSection = 0;

	bool ShowNewGame = false;
	bool ShowNGOID = false;
	bool ShowHostages = false;
	bool WarnUnequip = true;
	bool WarnNewGame = true;

	bool ShowWarnings = true;
	std::map<const char*, cb::Callback0<bool>> Warnings;

	char const SHOWVERSION_NO = 0;
	char const SHOWVERSION_WARNING = 1;
	char const SHOWVERSION_NOTICE = 2;
	char const SHOWVERSION_YES = 3;
	char ShowVersionLevel = SHOWVERSION_NOTICE;

	TextConfig VersionTextConfig{ true, 606, 40, Right, 0x335947 };


	void AddWarning(const char* warning) {
		AddWarning(warning, cb::Callback0<bool>());
	}

	void AddWarning(const char* warning, cb::Callback0<bool> require) {
		Warnings.insert(std::pair<const char*, cb::Callback0<bool>>(warning, require));
	}

	void RemoveWarning(const char* warning) {
		Warnings.erase(warning);
	}

	bool ShowNotices = true;
	std::map<const char*, cb::Callback0<bool>> Notices;

	void AddNotice(const char* notice) {
		AddNotice(notice, cb::Callback0<bool>());
	}

	void AddNotice(const char* notice, cb::Callback0<bool> require) {
		Notices.insert(std::pair<const char*, cb::Callback0<bool>>(notice, require));
	}

	void RemoveNotice(const char* notice) {
		Notices.erase(notice);
	}

	bool IsTankerSelected() {
		return ((CurrentScreen != 0) && (CurrentSection == 1));
	}

	bool IsTankerNotSelected() {
		// Show the notice if not going into Tanker
		return !IsTankerSelected();
	}


	bool ApplyDefaultSettings = false;
	bool AutoloadSave = true;
	bool SettingsApplied = false;
	std::map<int, unsigned short> DefaultSettings = { { 0x3C, 0 }, { 0x3D, 2 }, { 0x40, 1 }, { 0x41, 0 } };
	TextConfig DefaultSavedTextConfig{ true, 605, 8, Right, 0x64B46E };
	TextConfig DefaultClearedTextConfig{ true, 605, 8, Right, 0xB4646E };


	const char* Difficulties[6]{
		"VERY EASY", "EASY", "NORMAL", "HARD", "EXTREME", "EUROPEAN EXTREME"
	};

	void SetPosition(int line) {
		MGS2::TextSetPosition(605, 390 - (18 * line), 1);
	}

	void SetWarningPosition(int line) {
		MGS2::TextSetPosition(67, 390 - (18 * line), 0);
	}

	void SetNoticePosition(int line) {
		MGS2::TextSetPosition(256, 390 - (18 * line), 0);
	}

	void SetTextColor(bool warning = false) {
		if ((warning) && ((*Mem::RenderedFrames / 30) % 2)) {
			MGS2::TextSetColor(0x84, 0, 0);
			return;
		}

		MGS2::TextSetColor(0x64, 0x80, 0x6E);
	}

	void SaveDefaults(int param_1, bool showMessage = false) {
		for (std::pair<int, unsigned short> setting : DefaultSettings) {
			DefaultSettings[setting.first] = *(unsigned short*)(param_1 + (setting.first * 2));
		}

		ApplyDefaultSettings = true;
		SettingsApplied = true;
		Log::DisplayText("Defaults Saved", DefaultSavedTextConfig);
	}

	void ClearDefaults(int param_1, bool showMessage = false) {
		ApplyDefaultSettings = false;
		SettingsApplied = false;
		Log::DisplayText("Defaults Cleared", DefaultClearedTextConfig);
	}

	void CheckSaveDefaults(int param_1) {
		if (ApplyDefaultSettings && Actions::ShortcutActive("NewGameInfo.ClearChoices")) {
			ClearDefaults(param_1, true);
		}
		else if (Actions::ShortcutActive("NewGameInfo.SaveChoices")) {
			SaveDefaults(param_1, true);
		}
	}

	struct HostageStatus{
		const char* CurrentType = "NORMAL";
		const char* NextType = "Nml";
		int NextHour = 0;
		int NextMins = 0;
		time_t LastUpdate = 0;

		void Update() {
			time_t LastUpdate = time(0);
			tm* curTime = localtime(&LastUpdate);
			int hour = curTime->tm_hour;
			int min = curTime->tm_min;

			const char* curHostages = "NORMAL";
			const char* nextHostages = "Nml";
			int nextHostageHour = 0;
			int nextHostageMins = 60 - min;
			switch (hour) {
			case 0:
				curHostages = "BEAUTIES";
				nextHostageHour = 1;
				break;
			case 13:
				curHostages = "BEASTS";
				nextHostageHour = 14;
				break;
			case 22:
				curHostages = "OLD BEAUTIES";
				nextHostageHour = 23;
				break;
			}
			if (!nextHostageHour) {
				if (hour > 22) {
					nextHostages = "Jen";
					nextHostageHour = 24;
				}
				else if (hour > 13) {
					nextHostages = "Old";
					nextHostageHour = 22;
				}
				else {
					nextHostages = "Men";
					nextHostageHour = 13;
				}
				nextHostageMins += 60 * (nextHostageHour - hour - 1);
			}

			CurrentType = curHostages;
			NextType = nextHostages;
			NextHour = nextHostageHour;
			NextMins = nextHostageMins;
		}
	};


	void DrawInfo(int param_1, bool newGamePlus, int screen) {
		CurrentScreen = screen;

		MGS2::TextInit();
		int currentLine = 0;

		if (ShowHostages && IsTankerNotSelected()) {
			static HostageStatus hostages;
			if ((!hostages.LastUpdate) || (!(*Mem::GameTimeFrames % 60))) {
				hostages.Update();
			}

			SetPosition(currentLine++);
			SetTextColor();

			char content[255];
			snprintf(content, 254, "Hostages: %s (%s %dm)",
				hostages.CurrentType, hostages.NextType, hostages.NextMins);
			MGS2::TextDraw(content);
		}

		unsigned short* ppCurrentData = *(unsigned short**)0xA01F34;
		unsigned short optionStatus = ppCurrentData[0xAC5]; // +158a

		unsigned short options;
		if ((!newGamePlus) || (optionStatus & 0x100)) {
			// new game, or...
			// options menu has been used, use current settings
			options = ppCurrentData[3];
		}
		else {
			// options menu has not been used, use continue settings from loaded game
			unsigned short* ppContinueData = *(unsigned short**)0xA01F38;
			options = ppContinueData[3];
		}
		bool isPrevious = options & 0x200;
		SetPosition(currentLine++);
		SetTextColor(!isPrevious);
		MGS2::TextDraw((const char*)"Quick Change: %s", isPrevious ? "PREVIOUS" : "UNEQUIP");

		unsigned short* ppNGSettings = *(unsigned short**)0xED4FD0;

		// GOID screen (showing radar)
		if (screen >= 3) {
			CheckSaveDefaults(param_1);

			unsigned short radar = ppNGSettings[0x40];
			SetPosition(currentLine++);
			SetTextColor();
			if (radar <= 1) {
				MGS2::TextDraw((const char*)"Radar: TYPE%d", radar + 1);
			}
			else {
				MGS2::TextDraw((const char*)"Radar: OFF");
			}
		}

		// Radar screen (showing difficulty)
		if (screen >= 2) {
			unsigned short difficulty = ppNGSettings[0x3D];
			if (difficulty <= 5) {
				
				if (difficulty <= 2) {
					if (ShowNGOID) {
						SetPosition(currentLine++);
						SetTextColor();
						MGS2::TextDraw((const char*)"Not Game Over If Discovered");
					}
					if (screen == 2) {
						CheckSaveDefaults(param_1);
					}
				}
				SetPosition(currentLine++);
				SetTextColor();
				MGS2::TextDraw((const char*)"Difficulty: %s", Difficulties[difficulty]);
			}
		}

		// All screens (show NG status)
		char* ngExtra = (char*)"++";
		int ngExtraOffset = 0;
		bool show = true;
		if (newGamePlus) {
			unsigned short numClears = *(unsigned short*)0x118C358;
			if (numClears % 2) {
				ngExtraOffset = 1; // NG+
			} // else NG++
		}
		else {
			ngExtraOffset = 2; // NG
			if (!ShowNewGame && !WarnNewGame) {
				show = false;
			}
		}
		if (show) {
			SetPosition(currentLine++);
			SetTextColor(!newGamePlus && WarnNewGame);
			ngExtra += ngExtraOffset;
			MGS2::TextDraw("NEW GAME%s", ngExtra);
		}

		// Difficulty screen (show TP)
		if (screen >= 1) {
			unsigned short section = ppNGSettings[0x3C];
			CurrentSection = section;
			char* tpText = (char*)"TANKER-PLANT";
			if (section == 1) {
				tpText = (char*)"TANKER";
			}
			else if (section == 2) {
				tpText += 7;
			}
			SetPosition(currentLine++);
			SetTextColor();
			MGS2::TextDraw(tpText);
		}


		bool drawVersion = (ShowVersionLevel == SHOWVERSION_YES);
		currentLine = 0;

		// Warnings
		if (ShowWarnings) {
			SetTextColor(true);

			for (std::pair<const char*, cb::Callback0<bool>> warning : Warnings) {
				if ((warning.second.IsSet()) && (!warning.second.Call())) {
					continue;
				}
				SetWarningPosition(currentLine++);
				MGS2::TextDraw(warning.first);
				drawVersion |= (ShowVersionLevel >= SHOWVERSION_WARNING);
			}
		}

		// Notices
		if (ShowNotices) {
			MGS2::TextSetColor(0x84, 0x55, 0);

			for (std::pair<const char*, cb::Callback0<bool>> notice : Notices) {
				if ((notice.second.IsSet()) && (!notice.second.Call())) {
					continue;
				}
				SetWarningPosition(currentLine++);
				MGS2::TextDraw(notice.first);
				drawVersion |= (ShowVersionLevel >= SHOWVERSION_NOTICE);
			}
		}

		// Version
		if (drawVersion) {
			VersionTextConfig.Draw();
		}
		
	}


	void ApplyDefaultsForNewGame(int param_1, int targetScreen = 0) {
		int* currentScreen = (int*)(param_1 + 0x5c);
		if ((*currentScreen == targetScreen) && (ApplyDefaultSettings) && (!SettingsApplied)) {
			for (std::pair<int, unsigned short> setting : DefaultSettings) {
				*(unsigned short*)(param_1 + (setting.first * 2)) = setting.second;
			}

			*currentScreen = (DefaultSettings[0x3D] > 2) ? 5 : 4;
			SettingsApplied = true;
		}
		else if (*currentScreen == 0x10) {
			SettingsApplied = false;
		}
	}

	// Apply default settings at start of New Game
	tFUN_Void_Int oFUN_00756a40;
	void __cdecl hkFUN_00756a40(int param_1) {
		try_mgs2
			ApplyDefaultsForNewGame(param_1);
		catch_mgs2(Category, "756A40");

		oFUN_00756a40(param_1);
	}

	// Same for Load Game
	tFUN_Void_Int oFUN_00758330;
	void __cdecl hkFUN_00758330(int param_1) {
		try_mgs2
			ApplyDefaultsForNewGame(param_1, 1);
		catch_mgs2(Category, "758330");

		oFUN_00758330(param_1);
	}
	

	tFUN_Int_Int oFUN_007585f0;
	int __cdecl hkFUN_007585f0(int param_1) {
		try_mgs2
			DrawInfo(param_1, true, 0);
		catch_mgs2(Category, "7585F0");

		return oFUN_007585f0(param_1);
	}

	tFUN_Int_Int oFUN_007589c0;
	int __cdecl hkFUN_007589c0(int param_1) {
		try_mgs2
			DrawInfo(param_1, true, 1);
		catch_mgs2(Category, "7589C0");

		return oFUN_007589c0(param_1);
	}

	tFUN_Int_Int oFUN_00758d50;
	int __cdecl hkFUN_00758d50(int param_1) {
		try_mgs2
			DrawInfo(param_1, true, 2);
		catch_mgs2(Category, "758D50");

		return oFUN_00758d50(param_1);
	}

	tFUN_Int_Int oFUN_00759030;
	int __cdecl hkFUN_00759030(int param_1) {
		try_mgs2
			DrawInfo(param_1, true, 3);
		catch_mgs2(Category, "759030");

		return oFUN_00759030(param_1);
	}

	tFUN_Int_Int oFUN_00756da0;
	int __cdecl hkFUN_00756da0(int param_1) {
		try_mgs2
			DrawInfo(param_1, false, 0);
		catch_mgs2(Category, "756DA0");

		return oFUN_00756da0(param_1);
	}

	tFUN_Int_Int oFUN_007571f0;
	int __cdecl hkFUN_007571f0(int param_1) {
		try_mgs2
			DrawInfo(param_1, false, 1);
		catch_mgs2(Category, "7571F0");

		return oFUN_007571f0(param_1);
	}

	tFUN_Int_Int oFUN_007576e0;
	int __cdecl hkFUN_007576e0(int param_1) {
		try_mgs2
			DrawInfo(param_1, false, 2);
		catch_mgs2(Category, "7576E0");

		return oFUN_007576e0(param_1);
	}

	tFUN_Int_Int oFUN_00757b30;
	int __cdecl hkFUN_00757b30(int param_1) {
		try_mgs2
			DrawInfo(param_1, false, 3);
		catch_mgs2(Category, "757B30");

		return oFUN_00757b30(param_1);
	}

	tFUN_Int_Int oFUN_0058fca0;
	int __cdecl hkFUN_0058fca0(int param_1) {	
		try_mgs2
			if (*(char*)0xA53A00) {
				DrawInfo(param_1, false, 3);
			}
		catch_mgs2(Category, "58FCA0");

		return oFUN_0058fca0(param_1);
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		ShowWarnings = ini.GetBoolValue(Category, "ShowWarnings", ShowWarnings);
		ShowNotices = ini.GetBoolValue(Category, "ShowNotices", ShowNotices);
		ShowNewGame = ini.GetBoolValue(Category, "ShowNewGame", ShowNewGame);
		ShowNGOID = ini.GetBoolValue(Category, "ShowNGOID", ShowNGOID);
		ShowHostages = ini.GetBoolValue(Category, "ShowHostages", ShowHostages);

		std::map<std::string, char> showVersionMap{
			{ "warning", SHOWVERSION_WARNING }, { "notice", SHOWVERSION_NOTICE }
		};
		const char* showVersionKey = "ShowVersion";
		char showVersionLevel = ConfigParser::ParseValueMap<char>(ini, Category, showVersionKey, showVersionMap, -1);
		if (showVersionLevel == -1) {
			showVersionLevel = ConfigParser::ParseRequiredBool(ini, Category, showVersionKey);
			if (showVersionLevel == 1) {
				ShowVersionLevel = SHOWVERSION_YES;
			}
			else if (showVersionLevel == -1) {
				ShowVersionLevel = ConfigParser::ParseInteger<char>(ini, Category, showVersionKey, ShowVersionLevel, SHOWVERSION_NO, SHOWVERSION_YES, true);
			}
			else {
				ShowVersionLevel = SHOWVERSION_NO;
			}
		}
		else {
			ShowVersionLevel = showVersionLevel;
		}

		WarnUnequip = ini.GetBoolValue(Category, "WarnUnequip", WarnUnequip);
		WarnNewGame = ini.GetBoolValue(Category, "WarnNewGame", WarnNewGame);

		oFUN_007585f0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7585F0, (BYTE*)hkFUN_007585f0, 6);
		oFUN_007589c0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7589C0, (BYTE*)hkFUN_007589c0, 5);
		oFUN_00758d50 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x758D50, (BYTE*)hkFUN_00758d50, 5);
		oFUN_00759030 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x759030, (BYTE*)hkFUN_00759030, 5);
		oFUN_00756da0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x756DA0, (BYTE*)hkFUN_00756da0, 6);
		oFUN_007571f0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7571F0, (BYTE*)hkFUN_007571f0, 5);
		oFUN_007576e0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7576E0, (BYTE*)hkFUN_007576e0, 5);
		oFUN_00757b30 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x757B30, (BYTE*)hkFUN_00757b30, 5);

		// apply defaults
		if (Actions::RegisterShortcut(ini, "NewGameInfo.SaveChoices")) {
			Actions::RegisterShortcut(ini, "NewGameInfo.ClearChoices");
			oFUN_00756a40 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x756A40, (BYTE*)hkFUN_00756a40, 5);
			oFUN_00758330 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x758330, (BYTE*)hkFUN_00758330, 5);
		}

		// GOID patch (maybe turn this into a "lowest difficulty" setting later?
		if (ini.GetBoolValue(Category, "AskGOID", false)) {
			mem::PatchSet goidPatches{
				// New Game
				mem::Patch((void*)0x756B2D, "\x90\x90"),
				mem::Patch((void*)0x7577F9, "\xEB\x09"),
				// Load Game
				mem::Patch((void*)0x758438, "\x90\x90"),
				mem::Patch((void*)0x758E5F, "\xEB\x09"),
			};
			goidPatches.Patch();
		}

		VersionTextConfig.Content = Log::VersionConfig.Content;
		VersionTextConfig.OutlineColor = -1;

		return;
	}
}
