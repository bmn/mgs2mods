#include "MGS2.framework.h"
#include <vector>
#include <deque>

namespace MGS2::Performance {

	struct Profile {
		bool TextVisible = false;
		int SamplePeriod = 1;
		int SamplePeriodFrames = 60;
		int TextPosX = 18;
		int TextPosY = 457;
		int TextAlign = (TextAlignment)Right;
		int TextColor = 0xB4B4B4;
		const char* TextContent = "%d\n";
	};
	std::vector<Profile> Profiles;
	Profile nullProfile;

	int UpdatePeriod = 1;
	int TotalSamples = 1;

	size_t CurrentProfileIndex = 0;
	size_t TotalProfiles = 0;

	unsigned int Iteration = 0;
	double TimerFreq = 0;
	std::deque<unsigned __int64> Samples;
	Profile* FrameTimeProfile;


	double TimerFrequency() {
		return TimerFreq;
	}


	//int* CurrentState = (int*)0xA18B00;


	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {

		try {
			if (strcmp((char*)0x118ADEC, "init") == 0)
				throw "Game not initialised yet";

			MGS2::TextInit();

			bool updateThisFrame = ((Iteration++ % UpdatePeriod) == 0);
			if (updateThisFrame) {
				unsigned __int64 ft;
				QueryPerformanceCounter((LARGE_INTEGER*)&ft);
				Samples.push_front(ft);
				if (Samples.size() > TotalSamples) {
					Samples.pop_back();
				}
			}

			if (Profiles[0].TextVisible) {
				double frameDelta = (Samples[0] - Samples[1]) * TimerFreq * 1000 / UpdatePeriod;

				char content[255];
				snprintf(content, 255, Profiles[0].TextContent, frameDelta);
				MGS2::TextSetPosition(Profiles[0].TextPosX, Profiles[0].TextPosY, Profiles[0].TextAlign);
				MGS2::TextSetColor(Profiles[0].TextColor);
				MGS2::TextDraw(content);
				//MGS2::TextDraw(Profiles[0].TextPosX, Profiles[0].TextPosY, Profiles[0].TextAlign, Profiles[0].TextColor, content);
			}

			int sampleCount = Samples.size();
			for (int i = 1; i <= TotalProfiles; i++) {
				Profile p = Profiles[i];

				if (!p.TextVisible) {
					continue;
				}

				if (sampleCount <= (p.SamplePeriodFrames + 2)) {
					continue;
				}
				
				double numSecs = (Samples[0] - Samples[p.SamplePeriodFrames]) * TimerFreq;
				double fps = p.SamplePeriodFrames * UpdatePeriod / numSecs;

				char content[255];
				snprintf(content, 255, p.TextContent, fps);
				MGS2::TextSetPosition(p.TextPosX, p.TextPosY, p.TextAlign);
				MGS2::TextSetColor(p.TextColor);
				MGS2::TextDraw(content);
			}
		}
		catch (const char* message) {
		}

		oFUN_00878f70();
	}


	void Run(CSimpleIniA& ini) {
		unsigned __int64 freq;
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		TimerFreq = (1.0 / freq);

		const char* category = "Performance";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		int i;
		long long llong;

		if ((llong = ini.GetLongValue(category, "UpdatePeriod", LONG_MIN)) != LONG_MIN) {
			if ((llong > 0) && (llong < INT_MAX)) {
				UpdatePeriod = (int)llong;
			}
		}

		const char* profileFormat = "Performance.Counter.%d";
		for (i = 1; i <= 99; i++) {
			char profile[24];
			snprintf(profile, 24, profileFormat, i);
			if (ini.GetSectionSize(profile) == -1) {
				break;
			}
		}

		TotalProfiles = i - 1;
		int maxSamplePeriod = 1;
		for (i = 0; i <= TotalProfiles; i++) {
			Profile profile;

			char profileCategory[24];
			Profile& previousProfile = nullProfile;
			bool valid = false;

			if (i == 0) {
				strcpy(profileCategory, "Performance.FrameTime\x00");
				previousProfile = profile;
			}
			else {
				snprintf(profileCategory, 24, profileFormat, i);
				previousProfile = Profiles[i - 1];
			}

			if ((llong = ini.GetLongValue(profileCategory, "SamplePeriod", LONG_MIN)) != LONG_MIN) {
				if ((llong > 0) && (llong < INT_MAX)) {
					profile.SamplePeriod = (int)llong;
					if (profile.SamplePeriod > maxSamplePeriod) {
						maxSamplePeriod = profile.SamplePeriod;
					}
				}
			}
			profile.SamplePeriodFrames = profile.SamplePeriod * 60 / UpdatePeriod;

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

			const char* str = ini.GetValue(profileCategory, "TextColor", "");
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

			str = ini.GetValue(profileCategory, "TextFormat", "");
			if ((strlen(str) > 0) && (strlen(str) < 250)) {
				char* catString = (char*)malloc(strlen(str) + 2);
				strcpy(catString, str);
				strcat(catString, "\n");
				profile.TextContent = catString;
			}
			else {
				profile.TextContent = previousProfile.TextContent;
			}

			Profiles.push_back(profile);
		}

		TotalSamples = (int)(maxSamplePeriod * 60 / UpdatePeriod) + 10; // to be safe
		
		unsigned __int64 ft;
		QueryPerformanceCounter((LARGE_INTEGER*)&ft);
		Samples.push_front(ft);

		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
	}

}