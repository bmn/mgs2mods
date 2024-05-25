#include "MGS2.framework.h"

namespace MGS2::Timer {
	const char* Category = "Timer";

	bool Active = true;

	int ExpectedCreditsFrames = ((((9 * 60) + 21) * 60) + 26); // 9:21.43
	int DrawTextFunc = 0x889B40;

	int DefaultFormat = 0;
	const int MAX_FORMAT = 1;
	std::string Formats[2][9] {
		{ "%d'%02d'%02d''%02d", "%d'%02d'%02d''%d", "%d'%02d'%02d", "%d'%02d''%02d", "%d'%02d''%d", "%d'%02d", "%02d''%02d", "%02d''%d", "%02d" },
		{ "%d:%02d:%02d.%02d", "%d:%02d:%02d.%d", "%d:%02d:%02d", "%d:%02d.%02d", "%d:%02d.%d", "%d:%02d", "%02d.%02d", "%02d.%d", "%02d" }
	};

	struct Profile : TextConfig {
		const char* Name = nullptr;
		int Precision = 2;
		const char* Format = "{}";
		std::string Formats[9]{ Timer::Formats[0][0], Timer::Formats[0][1], Timer::Formats[0][2], Timer::Formats[0][3], Timer::Formats[0][4], Timer::Formats[0][5], Timer::Formats[0][6], Timer::Formats[0][7], Timer::Formats[0][8] };
		bool SingleTimer = false;
		int FormatIndex = 0;
		bool ShowMinutes = true;
		bool ShowHours = false;
	};
	Profile GameTimeProfile{ { true, 621, 2, (TextAlignment)Right }, "Game Time Timer" };
	Profile AreaGameTimeProfile{ { false, 621, 19, (TextAlignment)Right }, "Area Game Time Timer" };
	Profile RealTimeProfile{ { false, 621, 36, (TextAlignment)Right }, "Real Time Timer" };
	Profile AreaRealTimeProfile{ { false, 621, 53, (TextAlignment)Right }, "Area Real Time Timer" };
	Profile ExpectedTimeProfile{ { false, 10, 19, (TextAlignment)Left, 0xB4B4B4 }, "Predicted Time", 0, "%s Predicted" };
	Profile EndingStartTimeProfile{ { false, 10, 2, (TextAlignment)Left, 0xB4B4B4 }, "Time at Credits", 2, "%s at Credits" };
	Profile AreaDeltaProfile{ { false, 621, 70, (TextAlignment)Right }, "Area Real/Game Time Delta" };
	Profile GameLapProfile{ { false, 621, 460, (TextAlignment)Right }, "Game Lap Time" };
	Profile AreaGameLapProfile{ { false, 621, 460, (TextAlignment)Right }, "Area Game Lap Time" };
	TextConfig CombinedProfile{ false, 621, 2, (TextAlignment)Right, 0xB4B4B4, "GT {GameTime} ({AreaGameTime})\nRT {RealTime} ({AreaRealTime})\nDelta {AreaDelta}" };

	bool ShowEndingDelta = true;
	bool ShowOtherDeltas = false;

	// todo make laps work
	bool ShowLapTime = false;
	bool ShowAreaLapTime = false;
	unsigned __int64 LapTimeTimeout = 0;
	unsigned __int64 AreaLapTimeTimeout = 0;

	std::vector<Profile*> TimeProfiles{ &GameTimeProfile, &AreaGameTimeProfile, &RealTimeProfile, &AreaRealTimeProfile };
	Profile* ReturnProfile = NULL;

	char TempString[255];
	char ProfileStrings[5][MAX_PATH];


	unsigned __int64 GetLapTimeTimeout(double secs) {
		unsigned __int64 ft;
		QueryPerformanceCounter((LARGE_INTEGER*)&ft);
		return static_cast<unsigned __int64>(ft + (secs / Performance::TimerFrequency));
	}

	void TriggerLapTime(double secs) {
		LapTimeTimeout = GetLapTimeTimeout(secs);
	}

	void TriggerAreaLapTime(double secs) {
		AreaLapTimeTimeout = GetLapTimeTimeout(secs);
	}

	void TriggerBothLapTime(double secs) {
		TriggerLapTime(secs);
		TriggerAreaLapTime(secs);
	}

	int FramesToCsecs(int frames) {
		return (frames * 100 / 60);
	}

	void ToggleProfile(Actions::Action action) {
		Profile* profile = (Profile*)(action.Data);

		// if this timer has SingleTimer
		if (profile->SingleTimer) {
			// if hiding this timer, go back to the return timer
			if (profile->Enabled) {
				if (ReturnProfile != NULL) {
					ReturnProfile->Enabled = true;
					ReturnProfile = profile;
				}
			}
			// if we're enabling this timer, hide other timers and register the return timer
			else for (auto& pro : TimeProfiles) {
				if (pro->Enabled) {
					ReturnProfile = pro;
					pro->Enabled = false;
				}
			}
		}

		profile->Enabled = !profile->Enabled;
		Log::DisplayToggleMessage(profile->Name, profile->Enabled);
	}

	void ToggleCombined(Actions::Action action) {
		CombinedProfile.Enabled = !CombinedProfile.Enabled;
		Log::DisplayToggleMessage("Combined Timer", CombinedProfile.Enabled);
	}


	bool HandleProfileConfig(CSimpleIniA& ini, const char* category, Profile* profile) {
		profile->ParseConfig(ini, category);

		profile->Precision = ConfigParser::ParseInteger(ini, category, "Precision", profile->Precision, 0, 3);

		profile->FormatIndex = ConfigParser::ParseInteger(ini, category, "TimeFormat", DefaultFormat, 0, MAX_FORMAT, true);

		profile->ShowMinutes = ini.GetBoolValue(category, "ShowMinutes", profile->ShowMinutes);
		profile->ShowHours = ini.GetBoolValue(category, "ShowHours", profile->ShowHours);

		const char* formatString = ini.GetValue(category, "TextFormat", "");
		if (strcmp(formatString, "") != 0) {
			profile->Format = formatString;
		}

		if (strstr(profile->Format, "%s")) {
			for (int i = 0; i < 9; i++) {
				char* fmt;
				asprintf(&fmt, profile->Format, Formats[profile->FormatIndex][i].c_str());
				profile->Formats[i] = std::string(fmt);
				free(fmt);
			}
		}
		else {
			fmt::runtime_format_string<char> runtime = fmt::runtime(profile->Format);
			for (int i = 0; i < 9; i++) {
				profile->Formats[i] = fmt::format(runtime, Formats[profile->FormatIndex][i]);
			}
		}

		if (Actions::RegisterAction(ini, category, &ToggleProfile, profile)) {
			profile->SingleTimer = ini.GetBoolValue(category, "SingleTimer", false);
		}

		return profile->Enabled;
	}



	const char* StringForTime(int totalCsecs, int textPrecision, std::string formats[], bool alwaysShowMinutes = true, bool alwaysShowHours = false, char buffer[] = TempString) {
		int csecs = totalCsecs % 100;
		int totalSecs = totalCsecs / 100;
		int secs = totalSecs % 60;
		int totalMins = totalSecs / 60;
		int mins = totalMins % 60;
		int hours = totalMins / 60;

		if ((alwaysShowHours) || (hours)) {
			switch (textPrecision) {
			case 2:
				snprintf(buffer, 255, formats[0].c_str(), hours, mins, secs, csecs);
				break;
			case 1:
				snprintf(buffer, 255, formats[1].c_str(), hours, mins, secs, csecs / 10);
				break;
			case 0:
				snprintf(buffer, 255, formats[2].c_str(), hours, mins, secs);
				break;
			}
		}
		else if ((!alwaysShowMinutes) && (!totalMins)) {
			switch (textPrecision) {
			case 2:
				snprintf(buffer, 255, formats[6].c_str(), secs, csecs);
				break;
			case 1:
				snprintf(buffer, 255, formats[7].c_str(), secs, csecs / 10);
				break;
			case 0:
				snprintf(buffer, 255, formats[8].c_str(), secs);
				break;
			}
		}
		else {
			switch (textPrecision) {
			case 2:
				snprintf(buffer, 255, formats[3].c_str(), mins, secs, csecs);
				break;
			case 1:
				snprintf(buffer, 255, formats[4].c_str(), mins, secs, csecs / 10);
				break;
			case 0:
				snprintf(buffer, 255, formats[5].c_str(), mins, secs);
				break;
			}
		}

		return buffer;
	}


	void DrawTime(int totalCsecs, int textPrecision, std::string formats[], bool alwaysShowMinutes = true, bool alwaysShowHours = false, char* buffer = TempString) {
		StringForTime(totalCsecs, textPrecision, formats, alwaysShowMinutes, alwaysShowHours, buffer);
		((void(*)(const char*))DrawTextFunc)(buffer);
	}

	void DrawTimeForProfile(Profile& profile, int totalCsecs, char* buffer = TempString) {
		StringForTime(totalCsecs, profile.Precision, profile.Formats, profile.ShowMinutes, profile.ShowHours, buffer);
		profile.Draw(buffer);
	}
	
	
	void HandleLapTimeCsecs(int& csecs, int &lapCsecs, unsigned __int64 &ft) {
		if (LapTimeTimeout > ft) {
			if (lapCsecs == -1) {
				lapCsecs = csecs;
			}
			else {
				csecs = lapCsecs;
			}
		}
		else if (lapCsecs != -1) {
			LapTimeTimeout = 0;
			lapCsecs = -1;
		}
	}


	tFUN_Void_Int oFUN_008781e0;
	void __cdecl hkFUN_008781e0(int param_1) {
		oFUN_008781e0(param_1);

		static int expectedTimeCsecs = 0;
		static unsigned __int64 gtStartTime = (unsigned __int64)0;
		static unsigned __int64 rtStartTime = 0;
		static unsigned __int64 rtAreaStartTime = 0;
		static int gtStartFrames = 0;
		static int gtStartAreaFrames = 0;
		static int gtStartCsecs = 0;
		static bool gtActive = false;
		static short previousPlantProgress = -1;
		static int gtLapCsecs = -1;
		static int agtLapCsecs = -1;
		static int rtLapCsecs = -1;
		static int artLapCsecs = -1;

		try_mgs2
			if (
				(strcmp(Mem::AreaCode, "init") == 0) ||
				(strcmp(Mem::AreaCode, "n_title") == 0) ||
				(strcmp(Mem::AreaCode, "select") == 0) ||
				(strcmp(Mem::AreaCode, "sselect") == 0) ||
				(strcmp(Mem::AreaCode, "") == 0)
				) {
			}
			else {
				// call the unknown function
				TextInit();

				unsigned __int64 ft;
				QueryPerformanceCounter((LARGE_INTEGER*)&ft);

				// Area Real Time
				if (*Mem::AreaTimeFrames == 0) {
					// Trigger lap
					if (ShowAreaLapTime) {
						artLapCsecs = static_cast<int>((ft - rtAreaStartTime) * Performance::TimerFrequency * 100);
						agtLapCsecs = *Mem::AreaTimeFrames * 100 / 60;
						// TriggerAreaLapTime();
					}
					if (ShowLapTime) {
						rtLapCsecs = static_cast<int>((ft - rtStartTime) * Performance::TimerFrequency * 100);
						gtLapCsecs = *Mem::GameTimeFrames * 100 / 60;
						// TriggerLapTime();
					}
				}
				if (*Mem::AreaTimeFrames <= 2) {
					rtAreaStartTime = static_cast<unsigned __int64>(ft - ((double)*Mem::AreaTimeFrames / 60 / Performance::TimerFrequency));
				}
				if ((AreaRealTimeProfile.Enabled) || (CombinedProfile.Enabled)) {
					int csecs = static_cast<int>((ft - rtAreaStartTime) * Performance::TimerFrequency * 100);
					HandleLapTimeCsecs(csecs, artLapCsecs, ft);

					if (AreaRealTimeProfile.Enabled) {
						DrawTimeForProfile(AreaRealTimeProfile, csecs, ProfileStrings[3]);
					}
					else {
						StringForTime(csecs, AreaRealTimeProfile.Precision, Formats[AreaRealTimeProfile.FormatIndex], AreaRealTimeProfile.ShowMinutes, ProfileStrings[3]);
					}
				}

				// Area Delta
				if ((AreaDeltaProfile.Enabled) || (CombinedProfile.Enabled)) {
					double variation = ((ft - rtAreaStartTime) * Performance::TimerFrequency) - ((double)*Mem::AreaTimeFrames / 60);
					if ((variation > 0) && (variation < 0.001f)) {
						variation = 0;
					}

					snprintf(ProfileStrings[4], 255, "%1.2f", variation);

					if (AreaDeltaProfile.Enabled) {
						AreaDeltaProfile.Draw(ProfileStrings[4]);
					}
				}

				// Game Time
				if ((GameTimeProfile.Enabled) || (CombinedProfile.Enabled)) {
					int csecs = *Mem::GameTimeFrames * 100 / 60;
					HandleLapTimeCsecs(csecs, gtLapCsecs, ft);

					if (GameTimeProfile.Enabled) {
						DrawTimeForProfile(GameTimeProfile, csecs, ProfileStrings[0]);
					}
					else {
						StringForTime(csecs, GameTimeProfile.Precision, Formats[GameTimeProfile.FormatIndex], GameTimeProfile.ShowMinutes, GameTimeProfile.ShowHours, ProfileStrings[0]);
					}
				}

				// Area Game Time
				if ((AreaGameTimeProfile.Enabled) || (CombinedProfile.Enabled)) {
					int csecs = *Mem::AreaTimeFrames * 100 / 60;
					HandleLapTimeCsecs(csecs, agtLapCsecs, ft);

					if (AreaGameTimeProfile.Enabled) {
						DrawTimeForProfile(AreaGameTimeProfile, csecs, ProfileStrings[2]);
					}
					else {
						StringForTime(csecs, AreaGameTimeProfile.Precision, Formats[AreaGameTimeProfile.FormatIndex], AreaGameTimeProfile.ShowMinutes, AreaGameTimeProfile.ShowHours, ProfileStrings[2]);
					}
				}

				// Real Time
				if (*Mem::GameTimeFrames < 2) {
					rtStartTime = ft;
					if (*Mem::GameTimeFrames == 1) {
						rtStartTime -= static_cast<unsigned __int64>(1 / Performance::TimerFrequency / 60);
					}
				}
				if ((RealTimeProfile.Enabled) || (CombinedProfile.Enabled)) {
					int csecs = static_cast<int>((ft - rtStartTime) * Performance::TimerFrequency * 100);
					HandleLapTimeCsecs(csecs, rtLapCsecs, ft);

					if (RealTimeProfile.Enabled) {
						DrawTimeForProfile(RealTimeProfile, csecs, ProfileStrings[1]);
					}
					else {
						StringForTime(csecs, RealTimeProfile.Precision, Formats[RealTimeProfile.FormatIndex], RealTimeProfile.ShowMinutes, RealTimeProfile.ShowHours, ProfileStrings[1]);
					}
				}


				// Expected Time
				if ((EndingStartTimeProfile.Enabled) || (ExpectedTimeProfile.Enabled)) {

					short plantProgress = *(short*)0x118D912;
					if (plantProgress != previousPlantProgress) {
						previousPlantProgress = plantProgress;

						gtStartAreaFrames = *Mem::AreaTimeFrames;
						gtStartFrames = *Mem::GameTimeFrames;
						gtActive = false;
					}

					// 408 = bf rays
					// 490 = ending
					if ((plantProgress == 490) || (plantProgress == 408)) {

						// Attempt to handle the initial video load
						/*
						int leadInFrames = 12;
						//if ((!gtActive) && ((gtStartFrames + leadInFrames) == *currentFrames)) {
						if ((!gtActive) && (*Mem::AreaTimeFrames == leadInFrames)) {
							if (plantProgress == 490) {
								int expectedTimeFrames = *Mem::GameTimeFrames - leadInFrames + ExpectedCreditsFrames;
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
						*/

						// Basic version
						if ((!gtActive) && (*Mem::AreaTimeFrames != gtStartAreaFrames)) {
							if (*Mem::ProgressPlant == 490) {
								int expectedTimeFrames = gtStartFrames + ExpectedCreditsFrames;
								expectedTimeFrames += 4; // the lead-in that we're not using now
								expectedTimeCsecs = expectedTimeFrames * 100 / 60;
							}

							double leadInTicks = (1 / 60 / Performance::TimerFrequency);
							gtStartTime = static_cast<unsigned __int64>(ft - leadInTicks);
							gtStartCsecs = gtStartFrames * 100 / 60;
							gtActive = true;
						}


						if (gtActive) {
							__int64 realDelta = (ft - gtStartTime);
							int gameDelta = (*Mem::GameTimeFrames - gtStartFrames);

							double realElapsed = realDelta * Performance::TimerFrequency;
							double gameElapsed = (double)gameDelta / 60;

							double timeVariation = gameElapsed - realElapsed;
							if ((timeVariation > 0) && (timeVariation < 0.01f)) {
								timeVariation = -0.000001;
							}

							char strVariation[255];
							snprintf(strVariation, 255, "%+1.2f", timeVariation);

							if (plantProgress == 490) { // credits
								if (ExpectedTimeProfile.Enabled) {
									StringForTime(expectedTimeCsecs, ExpectedTimeProfile.Precision, ExpectedTimeProfile.Formats, ExpectedTimeProfile.ShowMinutes);
									if (ShowEndingDelta) {
										strncat(TempString, " (", strlen(TempString) - 1);
										strncat(TempString, strVariation, strlen(TempString) - 1);
										strncat(TempString, ")\n", strlen(TempString) - 1);
									}
									else {
										strncat(TempString, "\n", strlen(TempString) - 1);
									}
									ExpectedTimeProfile.Draw(TempString);
								}

								if (EndingStartTimeProfile.Enabled) {
									DrawTimeForProfile(EndingStartTimeProfile, gtStartCsecs);
								}
							}
							else { // others
								if ((ExpectedTimeProfile.Enabled) && (ShowOtherDeltas)) {
									ExpectedTimeProfile.Draw(strVariation);
								}
							}
						}
					}

				}

				// combined time
				if (CombinedProfile.Enabled) {
					std::string sCombinedTime = fmt::format(fmt::runtime(CombinedProfile.Content),
						"GameTime"_a = ProfileStrings[0],
						"RealTime"_a = ProfileStrings[1],
						"AreaGameTime"_a = ProfileStrings[2],
						"AreaRealTime"_a = ProfileStrings[3],
						"AreaDelta"_a = ProfileStrings[4]
					);
					CombinedProfile.Draw(sCombinedTime.c_str());
				}
			}
		catch_mgs2(Category, "8781E0");
	}

	tFUN_Void oFUN_00884ca0;
	void __cdecl hkFUN_00884ca0() {
		try_mgs2
			if ((!Active) || (*Mem::AreaTimeFrames < 60)) {
				return oFUN_00884ca0();
			}

			if (GameLapProfile.Enabled) {
				int csecs = FramesToCsecs(*Mem::GameTimeFrames);
				const char* txt = StringForTime(csecs, GameLapProfile.Precision, GameLapProfile.Formats, GameLapProfile.ShowMinutes);
				Log::DisplayText(txt, GameLapProfile, 5);
			}

			if (AreaGameLapProfile.Enabled) {
				int csecs = FramesToCsecs(*Mem::AreaTimeFrames);
				const char* txt = StringForTime(csecs, AreaGameLapProfile.Precision, AreaGameLapProfile.Formats, AreaGameLapProfile.ShowMinutes);
				Log::DisplayText(txt, AreaGameLapProfile, 5);
			}
		catch_mgs2(Category, "884CA0");

		oFUN_00884ca0();
	}

	
	// Gameplay timer (not fully functional)
	tFUN_Void_Int oFUN_004e69a0;
	void __cdecl hkFUN_004e69a0(int param_1) {
		oFUN_004e69a0(param_1);

		try_mgs2
			if (*(int*)0xA5397C != 0) { // breaks outside of main game
				return;
			}

			// A53984: 00200000/00000800 normal play/ingame-cutscene, 00100000 attract, 00000000 no game
			// A53988: 00004000 normal play, 00000000 no game
				// (((*(int*)0xA53984 | *(int*)0xA53988) & 0x700) == 0) &&
			// 118ADB0: 00 normal play/no game, 01 codec, 02 paused, 04 equip menu
				// ((*(char*)0x118ADB0 & 0x1F) == 0) &&
			// A18E00: 08 normal play, 00 no game
				// ((*(char*)0xA18E00 & 2) == 0)
			//if (((int(__cdecl*)())0x4FF780)()) {}

			if ((*Mem::ProgressTanker == 18) && (!strcmp(Mem::AreaCode, "w01e"))) {
				return;
			}

			*Mem::GameTimeFrames += 1;
			*Mem::AreaTimeFrames += 1;
		catch_mgs2(Category, "4E69A0");
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		Active = ini.GetBoolValue(Category, "Active", true);

		DefaultFormat = ConfigParser::ParseInteger(ini, Category, "TimeFormat", DefaultFormat, 0, MAX_FORMAT, true);

		HandleProfileConfig(ini, "Timer.GameTime", &GameTimeProfile);
		HandleProfileConfig(ini, "Timer.RealTime", &RealTimeProfile);
		HandleProfileConfig(ini, "Timer.AreaGameTime", &AreaGameTimeProfile);
		HandleProfileConfig(ini, "Timer.AreaRealTime", &AreaRealTimeProfile);
		HandleProfileConfig(ini, "Timer.AreaDelta", &AreaDeltaProfile);
		HandleProfileConfig(ini, "Timer.GameLap", &GameLapProfile);
		HandleProfileConfig(ini, "Timer.AreaGameLap", &AreaGameLapProfile);
		
		CombinedProfile.ParseConfig(ini, "Timer.Combined");
		Actions::RegisterAction(ini, "Timer.Combined", &ToggleCombined);

		const char* expectedCat = "Timer.ExpectedTime";
		const char* endingCat = "Timer.EndingStartTime";

		bool hasExpected = HandleProfileConfig(ini, expectedCat, &ExpectedTimeProfile);
		bool hasEnding = HandleProfileConfig(ini, endingCat, &EndingStartTimeProfile);

		if (hasExpected) {
			hasExpected = ini.GetBoolValue(expectedCat, "NewGameNotice", false);
		}
		if (hasEnding) {
			hasEnding = ini.GetBoolValue(endingCat, "NewGameNotice", false);
		}

		ShowEndingDelta = ini.GetBoolValue(expectedCat, "EndingDelta", ShowEndingDelta);
		ShowOtherDeltas = ini.GetBoolValue(expectedCat, "OtherDeltas", ShowOtherDeltas);

		cb::Callback0<bool> callback(&NewGameInfo::IsTankerNotSelected);

		if (hasExpected) {
			NewGameInfo::AddNotice(hasEnding ? "Credits & Predicted Time" : "Predicted Clear Time", callback);
		}
		else if (hasEnding) {
			NewGameInfo::AddNotice("Time at Credits", callback);
		}

		oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);
		oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);
		
		if (ini.GetBoolValue(Category, "GameplayTimer", false)) {
			NewGameInfo::AddWarning("Gameplay Timer");
			oFUN_004e69a0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x4E69A0, (BYTE*)hkFUN_004e69a0, 6);
			mem::PatchMemory((void*)0x87867E, "\xE9\xE7\x00\x00\x00\x90", 6); // area time disable
			mem::PatchMemory((void*)0x878775, "\xEB"); // game time disable
		}

	}

}
