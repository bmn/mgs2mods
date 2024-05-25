#include "MGS2.framework.h"
#include <queue>
#include <vector>

namespace MGS2::Turbo {
	const char* Category = "Turbo";

	unsigned char DefaultDeadzone = 0x30;
	unsigned char Deadzone = DefaultDeadzone;
	bool UseDeadzone = false;

	struct Profile {
		unsigned int FramesOn = 2;
		unsigned int Mask = 0x000000F0;
		bool HasDpad = false;
	};

	struct SwitchToProfileData {
		unsigned int ProfileIndex;
	};

	std::vector<Profile> Profiles;
	std::vector<cb::Callback1<void, unsigned int>> ProfileChangedCallback;

	
	//std::deque<int> HeldInput1;
	//std::deque<int> HeldInput2;
	std::deque<int> ComputedInput1;
	std::deque<int> ComputedInput2;
	unsigned int LastFrameInput = 0;
	size_t ActiveProfileIndex = 0;
	size_t ReturnProfileIndex = 1;
	int BitShift = 24;
	
	Profile nullProfile;
	Profile& ActiveProfile = nullProfile;

	//unsigned int Toggle = 0x00000401; // L1+R3
	unsigned int Toggle = 0;


	int CurrentProfile() {
		return ActiveProfileIndex;
	}

	
	void RegisterProfileChangedCallback(cb::Callback1<void, unsigned int> callback) {
		ProfileChangedCallback.push_back(callback);
	}


	void SwitchToProfile(unsigned int index) {
		unsigned int newIndex = ((Profiles.size() <= index) || (index == ActiveProfileIndex)) ? 0 : index;

		ReturnProfileIndex = newIndex ? 0 : 1;
		ActiveProfileIndex = newIndex;
		ActiveProfile = Profiles[newIndex];

		for (auto& callback : ProfileChangedCallback) {
			callback(ActiveProfileIndex);
		}
	}

	void SwitchToProfile(Actions::Action action) {
		SwitchToProfileData data = *(SwitchToProfileData*)(action.Data);
		SwitchToProfile(data.ProfileIndex);
	}

	void NextProfile() {
		SwitchToProfile(ActiveProfileIndex + 1);
	}

	void NextProfile(Actions::Action action) {
		NextProfile();
	}

	void ToggleOnOff() {
		unsigned int returnIndex = ReturnProfileIndex;
		ReturnProfileIndex = ActiveProfileIndex;
		SwitchToProfile(returnIndex);
	}

	void ToggleOnOff(Actions::Action action) {
		ToggleOnOff();
	}


	tFUN_Void_PshortPint oFUN_008cfe60;
	void __cdecl hkFUN_008cfe60(short* param_1, int* param_2) {

		try_mgs2
			bool one = ((int)param_1 == 0xEDAD38);
			unsigned int* currentInput = (unsigned int*)(param_2 + 1);

			unsigned int currentFrameInput = *currentInput;
			unsigned char* currentFrameAnalogInput = (unsigned char*)(param_2 + 3);
			unsigned char upDown = currentFrameAnalogInput[3];
			unsigned char leftRight = currentFrameAnalogInput[2];

			// process any custom deadzone
			if (Deadzone != DefaultDeadzone) {
				if ((leftRight < (0x80 - DefaultDeadzone)) && (leftRight >= (0x80 - Deadzone))) {
					currentFrameAnalogInput[2] = 0x80 - DefaultDeadzone + 1;
				}
				if ((leftRight > (0x80 + DefaultDeadzone)) && (leftRight <= (0x80 + Deadzone))) {
					currentFrameAnalogInput[2] = 0x80 + DefaultDeadzone - 1;
				}
				if ((upDown < (0x80 - DefaultDeadzone)) && (upDown >= (0x80 - Deadzone))) {
					currentFrameAnalogInput[3] = 0x80 - DefaultDeadzone + 1;
				}
				if ((upDown > (0x80 + DefaultDeadzone)) && (upDown <= (0x80 + Deadzone))) {
					currentFrameAnalogInput[3] = 0x80 + DefaultDeadzone - 1;
				}
			}

			if (ActiveProfileIndex != 0) {
				std::deque<int>(&computedInput) = one ? ComputedInput1 : ComputedInput2;

				unsigned int tempMask = ActiveProfile.Mask;
				if (ActiveProfile.HasDpad) {
					unsigned int unmask = 0;
					if (leftRight < (0x80 - Deadzone)) {
						unmask |= 0x8000;
					}
					if (leftRight > (0x80 + Deadzone)) {
						unmask |= 0x2000;
					}
					if (upDown < (0x80 - Deadzone)) {
						unmask |= 0x1000;
					}
					if (upDown > (0x80 + Deadzone)) {
						unmask |= 0x4000;
					}
					tempMask &= (0xFFFFFFFF - unmask);
				}

				unsigned int nonTurboInput = currentFrameInput & ~tempMask;
				unsigned int turboInput = currentFrameInput & tempMask;

				unsigned int autoInput = turboInput;
				unsigned int testInput = 0xFFFFFFFF;

				int end = computedInput.size();
				for (int i = 0; i < end; i++) {
					testInput &= computedInput[i]; // switch bit to 0 for any off-input in the FramesOn window
				}
				autoInput &= ~testInput; // remove any turbo input that didn't have an off-input in the window

				while (computedInput.size() >= ActiveProfile.FramesOn) {
					computedInput.pop_back();
				}

				unsigned int finalInput = autoInput | nonTurboInput;
				computedInput.push_front(finalInput);
				*currentInput = (finalInput & 0x00FFFFFF);

			}
		catch_mgs2(Category, "8CFE60");
		
		oFUN_008cfe60(param_1, param_2);
	}

	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		Deadzone = ConfigParser::ParseInteger(ini, Category, "Deadzone", Deadzone, Deadzone, (unsigned char)0x80, true);

		Actions::RegisterAction(ini, "Turbo.Toggle", &ToggleOnOff);
		Actions::RegisterAction(ini, "Turbo.NextProfile", &NextProfile);

		size_t i = 0;
		for (i = 1; i <= 99; i++) {
			char profile[17];
			snprintf(profile, 17, "Turbo.Profile.%d", i);
			if (ini.GetSectionSize(profile) == -1) {
				break;
			}
		}

		Profile offProfile{ 0, 0, false };
		Profiles.push_back(offProfile);
		
		size_t totalProfiles = i - 1;
		for (i = 1; i <= totalProfiles; i++) {
			Profile profile;

			char profileCategory[17];
			snprintf(profileCategory, 17, "Turbo.Profile.%d", i);

			if (ini.GetBoolValue(profileCategory, "EnabledAtStart", false)) {
				ActiveProfileIndex = i;
			}

			profile.FramesOn = ConfigParser::ParseInteger<unsigned int>(ini, profileCategory, "FramesOn", 2, 1, UINT_MAX, true);
			profile.Mask = ConfigParser::ParsePS2GamepadMask(ini, profileCategory, "InputMask", "O+Tri+X+Sq").Input;
			profile.HasDpad = ((profile.Mask & 0xF000) != 0);
			
			SwitchToProfileData* actionData = new SwitchToProfileData{ i };
			Actions::RegisterAction(ini, profileCategory, &SwitchToProfile, actionData);

			Profiles.push_back(profile);
		}

		if (ActiveProfileIndex == 0) {
			ActiveProfile = Profiles[0];
			ReturnProfileIndex = 1;
		}
		else {
			ActiveProfile = Profiles[ActiveProfileIndex];
			ReturnProfileIndex = 0;
		}

		oFUN_008cfe60 = (tFUN_Void_PshortPint)mem::TrampHook32((BYTE*)0x8CFE60, (BYTE*)hkFUN_008cfe60, 8);
	}

}
