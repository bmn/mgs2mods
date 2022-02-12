#include "MGS2.framework.h"
#include <vector>
#include <filesystem>
#include <mmsystem.h>

#pragma comment(lib, "Winmm.lib")

namespace MGS2::TurboDisplay {

	struct Profile {
		bool TextVisible = false;
		int TextPosX = 622;
		int TextPosY = 457;
		int TextAlign = (TextAlignment)Right;
		int TextColor = 0xB4B4B4;
		const char* TextContent = "TURBO\n";
		bool PlaySoundFile = false;
		char* SoundFile = 0;
	};
	std::vector<Profile> Profiles;
	Profile nullProfile;
	uintptr_t XInputBaseAddr = 0;

	bool ShowXInput = true;
	bool SystemSound = false;
	DWORD SoundFlags = SND_ASYNC;
	size_t CurrentProfileIndex = 0;
	size_t TotalProfiles = 0;
	bool XInputTurboActive = false;

	void ProfileChangedCallback(int newProfile) {
		if (newProfile > TotalProfiles) {
			return;
		}

		auto profile = Profiles[newProfile];
		if (profile.PlaySoundFile) {
			PlaySoundA(NULL, NULL, SoundFlags);
			PlaySoundA((LPCSTR)profile.SoundFile, NULL, SoundFlags);
		}
	}

	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		try {
			if (strcmp((char*)0x118ADEC, "init") == 0)
				throw "Game not initialised yet";

			// built-in turbo
			if (int currentInternalProfile = Turbo::CurrentProfile()) {
				CurrentProfileIndex = currentInternalProfile;
			}
			else if (ShowXInput) {
				if (!XInputBaseAddr) {
					DWORD currentProcess = GetCurrentProcessId();
					XInputBaseAddr = mem::GetModuleBaseAddress(currentProcess, L"XInput1_3.dll");
				}
				if (XInputBaseAddr) {
					// default xinputplus turbo location from v's fix
					BYTE* currentTurboState = (BYTE*)(XInputBaseAddr + 0x9C0BC);

					if (currentTurboState[0] != 1)
						// turbo location on bmn's xinputplus
						currentTurboState = (BYTE*)(XInputBaseAddr + 0xA5478);

					int active = (currentTurboState[0] == 1);

					if (CurrentProfileIndex != active) {
						CurrentProfileIndex = active;
						ProfileChangedCallback(active);
					}
				}
			}
			else {
				CurrentProfileIndex = 0;
			}

			Profile& profile = (CurrentProfileIndex <= TotalProfiles) ? Profiles[CurrentProfileIndex] : Profiles[TotalProfiles];

			if (profile.TextVisible)
				TextDraw(profile.TextPosX, profile.TextPosY, profile.TextAlign, profile.TextColor, profile.TextContent);
		}
		catch (const char* message) {
		}

		oFUN_00878f70();
	}

	void Run(CSimpleIniA& ini) {
		const char* category = "TurboDisplay";
		
		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		size_t i;
		long long llong;
		const char* str;
			
		ShowXInput = ini.GetBoolValue(category, "ShowXInput", true);
		
		if (ini.GetBoolValue(category, "SystemSound", false)) {
			SoundFlags += SND_SYSTEM;
		}

		const char* profileFormat = "TurboDisplay.Profile.%d";
		for (i = 1; i <= 99; i++) {
			char profile[24];
			snprintf(profile, 24, profileFormat, i);
			if (ini.GetSectionSize(profile) == -1) {
				break;
			}
		}

		TotalProfiles = i - 1;
		for (i = 0; i <= TotalProfiles; i++) {
			Profile profile;

			char profileCategory[24];
			Profile& previousProfile = nullProfile;
			bool valid = false;

			if (i == 0) {
				strcpy(profileCategory, category);
				previousProfile = profile;
			}
			else {
				snprintf(profileCategory, 24, profileFormat, i);
				previousProfile = Profiles[i - 1];
			}

			profile.TextVisible = ini.GetBoolValue(profileCategory, "TextVisible", previousProfile.TextVisible);

			if ((llong = ini.GetLongValue(profileCategory, "TextPosX", LONG_MIN)) != LONG_MIN) {
				profile.TextPosX = (int)llong;
			}
			else {
				profile.TextPosX = previousProfile.TextPosX;
			}

			if ((llong = ini.GetLongValue(profileCategory, "TextPosY", LONG_MIN)) != LONG_MIN) {
				profile.TextPosY = (int)llong;
			}
			else {
				profile.TextPosY = previousProfile.TextPosY;
			}

			llong = (int)ini.GetLongValue(profileCategory, "TextAlign", LONG_MIN);
			if ((llong >= (TextAlignment)Left) && (llong <= (TextAlignment)Center)) {
				profile.TextAlign = (int)llong;
			}
			else {
				profile.TextAlign = previousProfile.TextAlign;
			}

			str = ini.GetValue(profileCategory, "TextColor", "");
			if (strcmp(str, "") != 0) {
				unsigned int color = std::stoi(str, 0, 16);
				if (color <= 0xFFFFFF) {
					profile.TextColor = color;
					valid = true;
				}
			}
			if (!valid) {
				profile.TextColor = previousProfile.TextColor;
			}
			valid = false;

			str = ini.GetValue(profileCategory, "TextContent", "");
			if ((strlen(str) > 0) && (strlen(str) < 250)) {
				char* catString = (char*)malloc(strlen(str) + 2);
				strcpy(catString, str);
				strcat(catString, "\n");
				profile.TextContent = catString;
			}
			else {
				profile.TextContent = previousProfile.TextContent;
			}

			str = ini.GetValue(profileCategory, "PlaySound", "");
			if (strcmp(str, "") != 0) {
				std::filesystem::path soundFile{ str };
				if (std::filesystem::exists(soundFile)) {
					int len = strlen(str) + 1;
					char* cpy = (char*)malloc(len);
					strncpy(cpy, str, len);
					profile.SoundFile = cpy;

					profile.PlaySoundFile = true;
					valid = true;
				}
			}
			if (!valid) {
				profile.PlaySoundFile = previousProfile.PlaySoundFile;
				profile.SoundFile = previousProfile.SoundFile;
			}



			Profiles.push_back(profile);
		}

		Turbo::RegisterProfileChangedCallback(ProfileChangedCallback);

		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
	}

}