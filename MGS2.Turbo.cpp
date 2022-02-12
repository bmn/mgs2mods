#include "MGS2.framework.h"
#include <queue>
#include <vector>

namespace MGS2::Turbo {

	struct Profile {
		int FramesOn = 2;
		//int FramesOff = 1;
		//unsigned int InputMask = 0x000000F0; // 4 face buttons only
		//unsigned char DpadMask = 0x00;
		unsigned int Mask = 0x000000F0;
		bool HasDpad = false;
	};

	std::vector<Profile> Profiles;
	std::vector<void(*)(int)> ProfileChangedCallback;
	
	//std::deque<int> HeldInput1;
	//std::deque<int> HeldInput2;
	std::deque<int> ComputedInput1;
	std::deque<int> ComputedInput2;
	unsigned int LastFrameInput = 0;
	size_t ActiveProfileIndex = 0;
	int BitShift = 24;
	
	Profile nullProfile;
	Profile& ActiveProfile = nullProfile;

	bool ScaryFeatures = false;

	unsigned int Toggle = 0x00000401; // L1+R3


	int CurrentProfile() {
		return ActiveProfileIndex;
	}

	void RegisterProfileChangedCallback(void(*callback)(int)) {
		ProfileChangedCallback.push_back(callback);
	}

	tFUN_Void_PshortPint oFUN_008cfe60;
	void __cdecl hkFUN_008cfe60(short* param_1, int* param_2) {

		bool one = ((int)param_1 == 0xEDAD38);
		unsigned int* currentInput = (unsigned int*)(param_2 + 1);
		//unsigned char* currentDpadInput = (unsigned char*)((unsigned char*)param_2 + 9);

		unsigned int currentFrameInput = *currentInput;
		unsigned int currentFrameAnalogInput = *(unsigned int*)(param_2 + 3); // bytes: L(U/D) L(L/R) R(U/D) R(L/R)
		//unsigned char currentFrameDpadInput = *currentDpadInput;
		//unsigned char currentFrameDpadMask = (currentFrameDpadInput == 0xFF) ? 0x00 : (1 << (currentFrameDpadInput >> 1));

		if (ActiveProfileIndex != 0) {
			//std::deque<int>(&heldInput) = one ? HeldInput1 : HeldInput2;
			std::deque<int>(&computedInput) = one ? ComputedInput1 : ComputedInput2;

			unsigned int tempMask = ActiveProfile.Mask;
			if (ActiveProfile.HasDpad) {
				unsigned char upDown = currentFrameAnalogInput >> 24; // 0xFF000000
				unsigned char leftRight = (currentFrameAnalogInput >> 16) & 0xFF; // 0x00FF0000
				unsigned int unmask = 0;
				if (leftRight < 0x50) {
					unmask |= 0x8000;
				}
				if (leftRight > 0xB0) {
					unmask |= 0x2000;
				}
				if (upDown < 0x50) {
					unmask |= 0x1000;
				}
				if (upDown > 0xB0) {
					unmask |= 0x4000;
				}
				tempMask &= (0xFFFFFFFF - unmask);
			}

		    unsigned int nonTurboInput = currentFrameInput & ~tempMask;
			unsigned int turboInput = currentFrameInput & tempMask;

			//unsigned char nonTurboDpadMask = currentFrameDpadMask & ~ActiveProfile.DpadMask;
			//unsigned char turboDpadMask = currentFrameDpadMask & ActiveProfile.DpadMask;

			// put the dpad in the highest byte (it's free real estate)
			// // atmp1
			//turboInput |= ((unsigned int)turboDpadMask << BitShift);
			//nonTurboInput |= ((unsigned int)nonTurboDpadMask << BitShift);

			// attmp2
			//unsigned int combinedInput = (one) ? currentFrameInput : (currentFrameInput | ((unsigned int)currentFrameDpadMask << BitShift));
			//unsigned int nonTurboInput = combinedInput & ~ActiveProfile.Mask;
			//unsigned int turboInput = combinedInput & ActiveProfile.Mask;

			unsigned int autoInput = turboInput;

			// original
			/*
			unsigned int newPressInput = 0;

			int end = heldInput.size();
			if (end > 0) {
				newPressInput = (turboInput & ~heldInput[0]); // true input from the previous frame

				for (int i = 0; i < end; i++) {
					autoInput &= heldInput[i];
				}
				autoInput &= computedInput[0]; // final test for auto: was the last-stored frame a turbo input?

				if (end > ActiveProfile.FramesOff) {
					while (heldInput.size() > ActiveProfile.FramesOff) {
						heldInput.pop_back();
						computedInput.pop_front();
					}
				}
			}
			*/


			unsigned int testInput = 0xFFFFFFFF;
			int end = computedInput.size();
			for (int i = 0; i < end; i++) {
				testInput &= computedInput[i]; // switch bit to 0 for any off-input in the FramesOn window
			}
			autoInput &= ~testInput; // remove any turbo input that didn't have an off-input in the window

			while (computedInput.size() >= ActiveProfile.FramesOn) {
				computedInput.pop_back();
			}
			


			unsigned int finalInput = autoInput | nonTurboInput;// | newPressInput;

			computedInput.push_front(finalInput);
			//heldInput.push_front(turboInput); // true input (turbo only)
			//computedInput.push_back(finalInput); // computed input

			*currentInput = (finalInput & 0x00FFFFFF);

			/*
			if (!one) {
				int count = 0;
				unsigned char finalInputDpad = (unsigned char)(finalInput >> BitShift);
				if (!finalInput) {
					finalInput = 0xFF;
				}
				else {
					while (finalInputDpad >>= 1) {
						count++;
					}
					finalInputDpad <<= 1;
				}

				*currentDpadInput = finalInputDpad;
			}
			*/
		}
		
		// if last frame was the toggle input and this frame isn't, toggle it
		if (!one) {
			if (((LastFrameInput & Toggle) == Toggle) && ((currentFrameInput & Toggle) != Toggle)) {
				if (Profiles.size() <= ++ActiveProfileIndex) {
					ActiveProfileIndex = 0;
				}
				ActiveProfile = Profiles[ActiveProfileIndex];

				for(void(*callback)(int) : ProfileChangedCallback) {
					callback(ActiveProfileIndex);
				}
			}
			LastFrameInput = currentFrameInput;
		}

		oFUN_008cfe60(param_1, param_2);
	}

	void Run(CSimpleIniA& ini) {
		const char* category = "Turbo";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		ScaryFeatures = ini.GetBoolValue(category, "ScaryFeatures", false);

		const char* profileFormat = "Turbo.Profile.%d";

		size_t i = 0;
		long long llong;
		const char* str;

		str = ini.GetValue(category, "Toggle", "");
		if (strcmp(str, "") != 0) {
			llong = std::stoll(str, 0, 16);
			if (llong <= 0xFFFF) {
				Toggle = RemapL3((unsigned int)llong);
			}
		}

		for (i = 1; i <= 99; i++) {
			char profile[17];
			snprintf(profile, 17, profileFormat, i);
			if (ini.GetSectionSize(profile) == -1) {
				break;
			}
		}
		
		size_t totalProfiles = i - 1;
		for (i = 0; i <= totalProfiles; i++) {
			Profile profile;

			char profileCategory[17];
			Profile& previousProfile = nullProfile;
			bool valid = false;

			if (i == 0) {
				strcpy(profileCategory, category);
				previousProfile = profile;
			}
			else {
				snprintf(profileCategory, 17, profileFormat, i);
				if (ini.GetBoolValue(profileCategory, "EnabledAtStart", false)) {
					ActiveProfileIndex = i;
				}
				previousProfile = Profiles[i - 1];
			}

			if ((llong = ini.GetLongValue(profileCategory, "FramesOn", LONG_MIN)) != LONG_MIN) {
				if ((llong > 0) && (llong < 65536)) {
					profile.FramesOn = (int)llong;
					valid = true;
				}
			}
			if (!valid) {
				profile.FramesOn = previousProfile.FramesOn;
			}
			valid = false;

			str = ini.GetValue(profileCategory, "InputMask", "");
			if (strcmp(str, "") != 0) {
				llong = std::stoll(str, 0, 16);
				if (llong <= 0xFFFF) {
					unsigned int mask = RemapL3((unsigned int)llong);
					profile.HasDpad = ((mask & 0xF000) != 0);
					profile.Mask = mask;
					valid = true;
				}
			}
			if (!valid) {
				profile.Mask = previousProfile.Mask;
			}
			valid = false;


			Profiles.push_back(profile);
		}

		if (ActiveProfileIndex == 0) {
			ActiveProfile = Profiles[0];
		}
		else {
			ActiveProfile = Profiles[ActiveProfileIndex];
		}

		oFUN_008cfe60 = (tFUN_Void_PshortPint)mem::TrampHook32((BYTE*)0x8CFE60, (BYTE*)hkFUN_008cfe60, 8);
	}

}