#include "MGS2.framework.h"
//#include <vector>
#include <map>

namespace MGS2::NewGameInfo {

	int CurrentScreen = 0;
	int CurrentSection = 0;
	int Screen() { return CurrentScreen; }
	int Section() { return CurrentSection; }
	
	bool ShowNewGame = false;
	bool ShowNGOID = false;

	bool ShowWarnings = true;
	//std::vector<const char*> Warnings;
	std::map<const char*, bool(*)(void)> Warnings;
	void AddWarning(const char* warning) {
		AddWarning(warning, nullptr);
	}
	void AddWarning(const char* warning, bool(*require)(void)) {
		Warnings.insert(std::pair<const char*, bool(*)(void)>(warning, require));
	}

	bool ShowNotices = true;
	//std::vector<const char*> Notices;
	std::map<const char*, bool(*)(void)> Notices;
	void AddNotice(const char* notice) {
		AddNotice(notice, nullptr);
	}
	void AddNotice(const char* notice, bool(*require)(void)) {
		Notices.insert(std::pair<const char*, bool(*)(void)>(notice, require));
	}


	const char* Difficulties[6] {
		"Very Easy", "Easy", "Normal", "Hard", "Extreme", "European Extreme"
	};

	void SetPosition(int line) {
		MGS2::TextSetPosition(0x25D, 0x186 - (0x12 * line), 1);
	}

	void SetWarningPosition(int line) {
		MGS2::TextSetPosition(0x43, 0x186 - (0x12 * line), 0);
	}

	void SetNoticePosition(int line) {
		MGS2::TextSetPosition(0x100, 0x186 - (0x12 * line), 0);
	}

	void DrawInfo(bool newGamePlus, int screen) {
		CurrentScreen = screen;

		MGS2::TextInit();
		MGS2::TextSetColor(0x64, 0x80, 0x6E);
		int currentLine = 0;

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
		SetPosition(currentLine++);
		MGS2::TextDraw((const char*)"Quick Change: %s", (options & 0x200) ? "PREVIOUS" : "UNEQUIP");

		unsigned short* ppNGSettings = *(unsigned short**)0xED4FD0;

		// GOID screen (showing radar)
		if (screen >= 3) {
			unsigned short radar = ppNGSettings[0x40];
			SetPosition(currentLine++);
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
				if ((difficulty <= 2) && (ShowNGOID)) {
					SetPosition(currentLine++);
					MGS2::TextDraw((const char*)"Not Game Over If Discovered");
				}
				SetPosition(currentLine++);
				MGS2::TextDraw((const char*)"Difficulty: %s", Difficulties[difficulty]);
			}
		}

		// All screens (show NG status)
		char* ngExtra = (char*)"++";
		bool show = true;
		if (newGamePlus) {
			unsigned short numClears = *(unsigned short*)0x118C358;
			if (numClears % 2) {
				ngExtra += 1; // NG+
			} // else NG++
		}
		else {
			ngExtra += 2; // NG
			if (!ShowNewGame) {
				show = false;
			}
		}
		if (show) {
			SetPosition(currentLine++);
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
			MGS2::TextDraw(tpText);
		}


		currentLine = 0;

		// Warnings
		if (ShowWarnings) {
			MGS2::TextSetColor(0x84, 0, 0);

			//for (const char* warning : Warnings) {
			for (std::pair<const char*, bool(*)(void)> warning : Warnings) {
				if ((warning.second) && (!warning.second())) {
					continue;
				}
				SetWarningPosition(currentLine++);
				//MGS2::TextDraw(warning);
				MGS2::TextDraw(warning.first);
			}
		}


		// Notices
		if (ShowNotices) {
			MGS2::TextSetColor(0x84, 0x55, 0);

			//for (const char* notice : Notices) {
			for (std::pair<const char*, bool(*)(void)> notice : Notices) {
				if ((notice.second) && (!notice.second())) {
					continue;
				}
				SetWarningPosition(currentLine++);
//				MGS2::TextDraw(notice);
				MGS2::TextDraw(notice.first);
			}
		}
		
	}
	

	tFUN_Int_Int oFUN_007585f0;
	int __cdecl hkFUN_007585f0(int param_1) {
		DrawInfo(true, 0);
		return oFUN_007585f0(param_1);
	}

	tFUN_Int_Int oFUN_007589c0;
	int __cdecl hkFUN_007589c0(int param_1) {
		DrawInfo(true, 1);
		return oFUN_007589c0(param_1);
	}

	tFUN_Int_Int oFUN_00758d50;
	int __cdecl hkFUN_00758d50(int param_1) {
		DrawInfo(true, 2);
		return oFUN_00758d50(param_1);
	}

	tFUN_Int_Int oFUN_00759030;
	int __cdecl hkFUN_00759030(int param_1) {
		DrawInfo(true, 3);
		return oFUN_00759030(param_1);
	}

	tFUN_Int_Int oFUN_00756da0;
	int __cdecl hkFUN_00756da0(int param_1) {
		DrawInfo(false, 0);
		return oFUN_00756da0(param_1);
	}

	tFUN_Int_Int oFUN_007571f0;
	int __cdecl hkFUN_007571f0(int param_1) {
		DrawInfo(false, 1);
		return oFUN_007571f0(param_1);
	}

	tFUN_Int_Int oFUN_007576e0;
	int __cdecl hkFUN_007576e0(int param_1) {
		DrawInfo(false, 2);
		return oFUN_007576e0(param_1);
	}

	tFUN_Int_Int oFUN_00757b30;
	int __cdecl hkFUN_00757b30(int param_1) {
		DrawInfo(false, 3);
		return oFUN_00757b30(param_1);
	}

	tFUN_Int_Int oFUN_0058fca0;
	int __cdecl hkFUN_0058fca0(int param_1) {
		
		if (*(char*)0xA53A00) {
			DrawInfo(false, 3);
		}

		return oFUN_0058fca0(param_1);
	}


	void Run(CSimpleIniA& ini) {
		const char* category = "NewGameInfo";
		
		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		ShowWarnings = ini.GetBoolValue(category, "ShowWarnings", true);
		ShowNotices = ini.GetBoolValue(category, "ShowNotices", true);
		ShowNewGame = ini.GetBoolValue(category, "ShowNewGame", false);
		ShowNGOID = ini.GetBoolValue(category, "ShowNGOID", false);

		oFUN_007585f0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7585F0, (BYTE*)hkFUN_007585f0, 6);
		oFUN_007589c0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7589C0, (BYTE*)hkFUN_007589c0, 5);
		oFUN_00758d50 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x758D50, (BYTE*)hkFUN_00758d50, 5);
		oFUN_00759030 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x759030, (BYTE*)hkFUN_00759030, 5);
		oFUN_00756da0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x756DA0, (BYTE*)hkFUN_00756da0, 6);
		oFUN_007571f0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7571F0, (BYTE*)hkFUN_007571f0, 5);
		oFUN_007576e0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7576E0, (BYTE*)hkFUN_007576e0, 5);
		oFUN_00757b30 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x757B30, (BYTE*)hkFUN_00757b30, 5);

		// pause menu
		//oFUN_0058fca0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x58FCA0, (BYTE*)hkFUN_0058fca0, 5);
		return;
	}
}