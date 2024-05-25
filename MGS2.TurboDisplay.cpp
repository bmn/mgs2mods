#include "MGS2.framework.h"
#include <vector>
#include <filesystem>
#include <mmsystem.h>
#include "resource.h"

#pragma comment(lib, "Winmm.lib")

namespace MGS2::TurboDisplay {
	const char* Category = "TurboDisplay";

	struct Profile {
		TextConfig Text = {
			.Enabled = true,
			.PosX = 622,
			.PosY = 457,
			.Align = (TextAlignment)Right,
			.Color = 0xB4B4B4,
			.Content = "TURBO\n"
		};
		SoundConfig Sound;
	};
	std::vector<Profile*> Profiles;
	uintptr_t XInputBaseAddr = 0;

	bool ShowXInput = true;
	bool SystemSound = false;
	DWORD SoundFlags = SND_ASYNC;
	size_t CurrentProfileIndex = 0;
	size_t TotalProfiles = 0;
	bool XInputTurboActive = false;

	void ProfileChangedCallback(unsigned int newProfile) {
		if (newProfile > TotalProfiles) {
			return;
		}

		Profiles[newProfile]->Sound.Play();
	}

	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		try_mgs2
			if (strcmp(Mem::AreaCode, "init") == 0) {
				oFUN_00878f70();
				return;
			}

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

			Profile* profile = (CurrentProfileIndex <= TotalProfiles) ? Profiles[CurrentProfileIndex] : Profiles[TotalProfiles];

			if (profile->Text.Enabled) {
				profile->Text.Draw();
			}
		catch_mgs2(Category, "878F70");

		oFUN_00878f70();
	}

	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		ShowXInput = ini.GetBoolValue(Category, "ShowXInput", true);
		
		char profileCategory[25]{ 0 };
		for (size_t i = 0; i <= 99; i++) {
			Profile* profile = new Profile;

			if (i == 0) {
				strncpy(profileCategory, "TurboDisplay.Profile.Off", 25);
				profile->Text.Enabled = false;
			}
			else {
				snprintf(profileCategory, 24, "TurboDisplay.Profile.%d", i);
				if (!ini.GetSection(profileCategory)) {
					break;
				}
			}

			profile->Text.ParseConfig(ini, profileCategory);
			profile->Sound.ParseConfig(ini, profileCategory);

			Profiles.push_back(profile);
			TotalProfiles = i;
		}

		Turbo::RegisterProfileChangedCallback(ProfileChangedCallback);

		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
	}

}
