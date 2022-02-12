#include "MGS2.framework.h"

namespace MGS2::Timer {
	int ExpectedCreditsFrames = ((((9 * 60) + 21) * 60) + 26); // 9:21.43
	int DrawTextFunc = 0x889B40;

	const char* Formats[6]{ "%d'%02d'%02d''%02d", "%d'%02d'%02d''%d", "%d'%02d'%02d", "%d'%02d''%02d", "%d'%02d''%d", "%d'%02d" };

	struct Profile {
		bool Enabled;
		int TextPosX;
		int TextPosY;
		int TextAlign;
		int TextPrecision;
		unsigned int TextColor = 0xB4B4B4;
		const char* Format = "%s";
		const char* Formats[6]{ Timer::Formats[0], Timer::Formats[1], Timer::Formats[2], Timer::Formats[3], Timer::Formats[4], Timer::Formats[5] };
	};
	Profile GameTimeProfile{ true, 0x26D, 2, (TextAlignment)Right, 2 };
	Profile RealTimeProfile{ false, 0x26D, 22, (TextAlignment)Right, 2 };
	Profile ExpectedTimeProfile{ false, 10, 19, (TextAlignment)Left, 2, 0xB4B4B4, "%s Expected" };
	Profile EndingStartTimeProfile{ false, 10, 2, (TextAlignment)Left, 2, 0xB4B4B4, "%s Ending" };

	bool ScaryFeatures = false;

	char TempString[255];



	bool HandleProfileConfig(CSimpleIniA& ini, const char* category, Profile& profile) {
		if ((!profile.Enabled) && (ini.GetSectionSize(category) == -1)) {
			return false;
		}

		profile.Enabled = ini.GetBoolValue(category, "TextVisible", profile.Enabled);

		long long llong;

		if ((llong = ini.GetLongValue(category, "TextPosX", LONG_MIN)) != LONG_MIN)
			profile.TextPosX = (int)llong;

		if ((llong = ini.GetLongValue(category, "TextPosY", LONG_MIN)) != LONG_MIN)
			profile.TextPosY = (int)llong;

		llong = ini.GetLongValue(category, "TextAlign", LONG_MIN);
		if ((llong >= 0) && (llong <= 2))
			profile.TextAlign = (int)llong;

		llong = ini.GetLongValue(category, "Precision", LONG_MIN);
		if ((llong >= 0) && (llong <= 2))
			profile.TextPrecision = (int)llong;

		const char* colorString = ini.GetValue(category, "TextColor", "");
		if (strcmp(colorString, "") != 0) {
			unsigned int color = std::stoi(colorString, 0, 16);
			if (color <= 0xFFFFFF)
				profile.TextColor = color;
		}

		const char* formatString = ini.GetValue(category, "TextFormat", "");
		if (strcmp(formatString, "") != 0) {
			profile.Format = formatString;
		}
		for (int i = 0; i < 6; i++) {
			char* fmt = (char*)malloc(255);
			snprintf(fmt, 255, profile.Format, Formats[i]);
			profile.Formats[i] = fmt;
		}

		return profile.Enabled;
	}



	const char* StringForTime(int totalCsecs, int textPrecision, const char* formats[]) {
		int csecs = totalCsecs % 100;
		int totalSecs = totalCsecs / 100;
		int secs = totalSecs % 60;
		int totalMins = totalSecs / 60;
		int mins = totalMins % 60;
		int hours = totalMins / 60;

		if (hours) {
			switch (textPrecision) {
			case 2:
				snprintf(TempString, 255, formats[0], hours, mins, secs, csecs);
				break;
			case 1:
				snprintf(TempString, 255, formats[1], hours, mins, secs, csecs / 10);
				break;
			case 0:
				snprintf(TempString, 255, formats[2], hours, mins, secs);
				break;
			}

		}
		else {
			switch (textPrecision) {
			case 2:
				snprintf(TempString, 255, formats[3], mins, secs, csecs);
				break;
			case 1:
				snprintf(TempString, 255, formats[4], mins, secs, csecs / 10);
				break;
			case 0:
				snprintf(TempString, 255, formats[5], mins, secs);
				break;
			}
		}

		return TempString;
	}


	void DrawTime(int totalCsecs, int textPrecision, const char* formats[]) {
		StringForTime(totalCsecs, textPrecision, formats);
		strncat(TempString, "\n", 255);
		((void(*)(const char*))DrawTextFunc)(TempString);

		/*
		if (hours) {
			switch (textPrecision) {
			case 2:
				((void(*)(const char*, int, int, int, int))DrawTextFunc)(formats[0], hours, mins, secs, csecs);
				break;
			case 1:
				((void(*)(const char*, int, int, int, int))DrawTextFunc)(formats[1], hours, mins, secs, csecs / 10);
				break;
			case 0:
				((void(*)(const char*, int, int, int))DrawTextFunc)(formats[2], hours, mins, secs);
				break;
			}

		}
		else {
			switch (textPrecision) {
			case 2:
				((void(*)(const char*, int, int, int))DrawTextFunc)(formats[3], mins, secs, csecs);
				break;
			case 1:
				((void(*)(const char*, int, int, int))DrawTextFunc)(formats[4], mins, secs, csecs / 10);
				break;
			case 0:
				((void(*)(const char*, int, int))DrawTextFunc)(formats[5], mins, secs);
				break;
			}
		}
		*/
	}



	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		static int expectedTimeCsecs = 0;
		static unsigned __int64 gtStartTime = (unsigned __int64)0;
		static int gtStartFrames = 0;
		static int gtStartCsecs = 0;
		static bool gtActive = false;
		static short previousPlantProgress = -1;

		char* currentArea = (char*)0x118ADEC;

		if (
			(strcmp(currentArea, "init") == 0) ||
			(strcmp(currentArea, "n_title") == 0) ||
			(strcmp(currentArea, "select") == 0) ||
			(strcmp(currentArea, "sselect") == 0) ||
			(strcmp(currentArea, "") == 0)
			) {
		}
		else {
			// call the unknown function
			TextInit();

			int* currentFrames = (int*)0x118AEF8;
			int csecs = *currentFrames * 100 / 60;

			int* currentAreaFrames = (int*)0x118AEA4;

			// Game Time
			if (GameTimeProfile.Enabled) {
				// set the text colour
				TextSetColor(GameTimeProfile.TextColor);

				// set the position
				TextSetPosition(GameTimeProfile.TextPosX, GameTimeProfile.TextPosY, GameTimeProfile.TextAlign);


				DrawTime(csecs, GameTimeProfile.TextPrecision, GameTimeProfile.Formats);
			}


			// Real Time



			// Expected Time
			if ((EndingStartTimeProfile.Enabled) || (ExpectedTimeProfile.Enabled)) {

				unsigned __int64 ft;
				QueryPerformanceCounter((LARGE_INTEGER*)&ft);

				short plantProgress = *(short*)0x118D912;
				if (plantProgress != previousPlantProgress) {
					previousPlantProgress = plantProgress;

					gtStartFrames = *currentFrames;
					gtActive = false;
				}

				int leadInFrames = 12;
				//if ((!gtActive) && ((gtStartFrames + leadInFrames) == *currentFrames)) {
				if ((!gtActive) && (*currentAreaFrames == leadInFrames)) {
					if (plantProgress == 490) {
						int expectedTimeFrames = *currentFrames - leadInFrames + ExpectedCreditsFrames;
						expectedTimeCsecs = expectedTimeFrames * 100 / 60;
					}
					else {
						expectedTimeCsecs = 0;
					}

					double leadInTicks = ((double)leadInFrames / 60 / Performance::TimerFrequency());
					gtStartTime = ft - leadInTicks;
					//gtStartTime = ft;
					//gtStartFrames = *currentFrames;
					gtStartCsecs = gtStartFrames * 100 / 60;
					gtActive = true;
				}

				if (gtActive) {
					__int64 realDelta = (ft - gtStartTime);
					int gameDelta = (*currentFrames - gtStartFrames);

					double realElapsed = realDelta * Performance::TimerFrequency();
					double gameElapsed = (double)gameDelta / 60;

					double timeVariation = gameElapsed - realElapsed;
					if ((timeVariation > 0) && (timeVariation < 0.01f)) {
						timeVariation = -0.000001;
					}

					TextSetColor(ExpectedTimeProfile.TextColor);

					//snprintf(content, 255, "Credits: %")
					char strVariation[255];
					snprintf(strVariation, 255, "%+1.2f", timeVariation);

					//MGS2::TextSetPosition(10, 200, 0);
					//MGS2::TextDraw(TempString);


					if (plantProgress == 490) { // credits
						if (ExpectedTimeProfile.Enabled) {
							StringForTime(expectedTimeCsecs, ExpectedTimeProfile.TextPrecision, ExpectedTimeProfile.Formats);
							strncat(TempString, " (", 254);
							strncat(TempString, strVariation, 254);
							strncat(TempString, ")\n", 254);
							TextSetColor(ExpectedTimeProfile.TextColor);
							TextSetPosition(ExpectedTimeProfile.TextPosX, ExpectedTimeProfile.TextPosY, ExpectedTimeProfile.TextAlign);
							((void(*)(const char*))DrawTextFunc)(TempString);
							//DrawTime(expectedTimeCsecs, ExpectedTimeProfile.TextPrecision, ExpectedTimeProfile.Formats);
						}

						if (EndingStartTimeProfile.Enabled) {
							TextSetColor(EndingStartTimeProfile.TextColor);
							TextSetPosition(EndingStartTimeProfile.TextPosX, EndingStartTimeProfile.TextPosY, EndingStartTimeProfile.TextAlign);
							DrawTime(gtStartCsecs, EndingStartTimeProfile.TextPrecision, EndingStartTimeProfile.Formats);
						}
					}
					else if (plantProgress == 408) { // bf rays
						if (ExpectedTimeProfile.Enabled) {
							TextSetColor(ExpectedTimeProfile.TextColor);
							TextSetPosition(ExpectedTimeProfile.TextPosX, ExpectedTimeProfile.TextPosY, ExpectedTimeProfile.TextAlign);
							((void(*)(const char*))DrawTextFunc)(strVariation);
						}
					}
				}
			}
		}

		oFUN_00878f70();
	}

	bool ExpectedTimeFunction() {
		// Show the notice if not going into Tanker
		return ((NewGameInfo::Screen() == 0) || (NewGameInfo::Section() != 1));
	}

	void Run(CSimpleIniA& ini) {
		const char* category = "Timer";
		
		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		if (!ini.GetBoolValue(category, "Enabled", true))
			return;

		HandleProfileConfig(ini, category, GameTimeProfile);
		HandleProfileConfig(ini, "Timer.RealTime", RealTimeProfile);

		bool hasExpected = HandleProfileConfig(ini, "Timer.ExpectedTime", ExpectedTimeProfile);
		bool hasEnding = HandleProfileConfig(ini, "Timer.EndingStartTime", EndingStartTimeProfile);

		if (hasExpected) {
			NewGameInfo::AddNotice(hasEnding ? "Ending Start/Expected Clear" : "Expected Clear Time", &ExpectedTimeFunction);
		}
		else if (hasEnding) {
			NewGameInfo::AddNotice("Ending Start Time", &ExpectedTimeFunction);
		}

		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
		return;
	}
}