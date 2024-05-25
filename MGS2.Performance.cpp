#include "MGS2.framework.h"
#include <vector>
#include <deque>

namespace MGS2::Performance {
	const char* Category = "Performance";

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
	std::vector<Profile> Actions;
	Profile nullProfile;

	unsigned int UpdatePeriod = 1;
	unsigned int TotalSamples = 1;

	size_t CurrentProfileIndex = 0;
	size_t TotalProfiles = 0;

	unsigned int Iteration = 0;
	double TimerFrequency = 0;
	std::deque<unsigned __int64> Samples;
	Profile* FrameTimeProfile;


	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		try_mgs2
			if (strcmp(Mem::AreaCode, "init") == 0) {
				oFUN_00878f70();
				return;
			}

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

			if (Actions[0].TextVisible) {
				double frameDelta = (Samples[0] - Samples[1]) * TimerFrequency * 1000 / UpdatePeriod;

				char content[255];
				snprintf(content, 255, Actions[0].TextContent, frameDelta);
				MGS2::TextSetPosition(Actions[0].TextPosX, Actions[0].TextPosY, Actions[0].TextAlign);
				MGS2::TextSetColor(Actions[0].TextColor);
				MGS2::TextDraw(content);
			}

			int sampleCount = Samples.size();
			for (unsigned int i = 1; i <= TotalProfiles; i++) {
				Profile p = Actions[i];

				if (!p.TextVisible) {
					continue;
				}

				if (sampleCount <= (p.SamplePeriodFrames + 2)) {
					continue;
				}
				
				double numSecs = (Samples[0] - Samples[p.SamplePeriodFrames]) * TimerFrequency;
				double fps = (double)p.SamplePeriodFrames * UpdatePeriod / numSecs;

				char content[255];
				snprintf(content, 255, p.TextContent, fps);
				MGS2::TextSetPosition(p.TextPosX, p.TextPosY, p.TextAlign);
				MGS2::TextSetColor(p.TextColor);
				MGS2::TextDraw(content);
			}
		catch_mgs2(Category, "878F70");

		oFUN_00878f70();
	}


	void Run(CSimpleIniA& ini) {
		unsigned __int64 freq;
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		TimerFrequency = (1.0 / freq);

		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		unsigned int i;
		long long llong;

		if ((llong = ini.GetLongValue(Category, "UpdatePeriod", LONG_MIN)) != LONG_MIN) {
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
				previousProfile = Actions[i - 1];
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
				if (char* catString = (char*)calloc(strlen(str) + 2, 1)) {
					strcpy(catString, str);
					strcat(catString, "\n");
					profile.TextContent = catString;
				}
			}
			else {
				profile.TextContent = previousProfile.TextContent;
			}

			Actions.push_back(profile);
		}

		TotalSamples = (int)(maxSamplePeriod * 60 / UpdatePeriod) + 10; // to be safe
		
		unsigned __int64 ft;
		QueryPerformanceCounter((LARGE_INTEGER*)&ft);
		Samples.push_front(ft);

		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
	}

}
