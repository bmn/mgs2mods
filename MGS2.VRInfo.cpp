#include "MGS2.framework.h"

namespace MGS2::VRInfo {

	struct LastScoreFormat {
		const char* Key;
		const char* ComparisonFormat;
		const char* StatusFormat;
	};

	const char* CategoryName = "VR Info";
	bool Active = true;
	double LastScoreTimeout = 10;

	const unsigned int COMPLETED = 0x80000000;
	const unsigned int SCORE_COMPLETED = 0x800000;

	TextConfig LastScoreConfig{ true, 12, 2, (TextAlignment)Left, 0xB4B4B4, "{Score} {Status}\n{Comparison}" };
	TextConfig TopScoreConfig{ true, 12, 457, (TextAlignment)Left, 0xB4B4B4, "Top {}" };

	const unsigned int PLAYTYPE_FIRSTPLAY_NOGOLD = 0;
	const unsigned int PLAYTYPE_IMPROVED_NOGOLD = 1;
	const unsigned int PLAYTYPE_NOCHANGE_NOGOLD = 2;
	const unsigned int PLAYTYPE_FIRSTPLAY = 3;
	const unsigned int PLAYTYPE_IMPROVED = 4;
	const unsigned int PLAYTYPE_NOCHANGE = 5;
	const unsigned int PLAYTYPE_IMPROVED_FIRSTGOLD = 6;

	LastScoreFormat LastScoreFormats[7]{
		{ "FirstPlayNoGold", "Gold%2$+d", "First Play" },
		{ "ImprovedNoGold", "PB%1$+d Gold%2$+d", "New Best" },
		{ "NoChangeNoGold", "PB%1$+d Gold%2$+d", "No Change" },
		{ "FirstPlay", "Gold%2$+d", "Gold" },
		{ "Improved", "PB%1$+d", "New Best" },
		{ "NoChange", "PB%1$+d", "No Change" },
		{ "ImprovedFirstGold", "PB%1$+d Gold%2$+d", "Gold" },
	};


	int PrevMission = -1;
	unsigned int PrevTopScore = 0;
	

	// helper functions
	unsigned int* pRawScoreForPlace(int place = 1) {
		return (unsigned int*)(*(uintptr_t*)0xED5268 + ((0x1a0401 + *Mem::VRMissionID) * 0x10) + ((place - 1) * 4));
	}

	unsigned int RawScoreForPlace(int place = 1) {
		return *pRawScoreForPlace(place);
	}

	unsigned int TrueScore(unsigned int score) {
		return score & 0xFFFFF;
	}

	unsigned int TrueScoreForPlace(int place = 1) {
		return TrueScore(RawScoreForPlace(place));
	}

	bool MissionCompleted() {
		return (RawScoreForPlace() & COMPLETED) == COMPLETED;
	}

	bool ScoreCompleted(unsigned int score) {
		return (score & SCORE_COMPLETED) == SCORE_COMPLETED;
	}

	bool MissionGoldCompleted() {
		return ScoreCompleted(RawScoreForPlace());
	}


	unsigned int TopPlayerScore() {
		for (int i = 1; i < 4; i++) {
			unsigned int rawScore = RawScoreForPlace(i);
			if (ScoreCompleted(rawScore)) {
				return TrueScore(rawScore);
			}
		}
		return 0;
	}



	tFUN_Int_IntUint oFUN_00550750;
	int __cdecl hkFUN_00550750(int levelID, unsigned int score) {
		if ((!Active) || (!LastScoreConfig.Enabled)) {
			if (score > PrevTopScore) {
				PrevTopScore = score;
			}
			return oFUN_00550750(levelID, score);
		}

		bool alreadyGold = MissionGoldCompleted();
		int scoreDelta = (int)(score - PrevTopScore);
		int goldDelta = (int)(score - TrueScoreForPlace());
		
		int place = oFUN_00550750(levelID, score);

		// first 0
		size_t format = PLAYTYPE_FIRSTPLAY_NOGOLD;
		// improved 1
		if (PrevTopScore != 0) {
			// no change 2
			format = (scoreDelta > 0) ? PLAYTYPE_IMPROVED_NOGOLD : PLAYTYPE_NOCHANGE_NOGOLD;
		}
		if (MissionGoldCompleted()) {
			// move to the gold formats
			if (alreadyGold) {
				format += 3;
			}
			// improved 1st gold 7
			else {
				format = PLAYTYPE_IMPROVED_FIRSTGOLD;
			}
		}

		try {
			std::string sDelta = fmt::format(fmt::runtime(LastScoreFormats[format].ComparisonFormat),
				"ScoreVsBest"_a = scoreDelta,
				"ScoreVsGold"_a = goldDelta,
				"BestVsScore"_a = -scoreDelta,
				"GoldVsScore"_a = -goldDelta
			);
			try {
				std::string sNewTop = fmt::format(fmt::runtime(LastScoreConfig.Content),
					"Score"_a = score,
					"Comparison"_a = sDelta,
					"Status"_a = LastScoreFormats[format].StatusFormat
				);
				Log::DisplayText(sNewTop, LastScoreConfig, LastScoreTimeout);
			}
			catch (fmt::v10::format_error) {
				std::string errorMsg = std::string("[VRInfo] Invalid format string LastScore.Comparison: ") + LastScoreFormats[format].Key;
				Log::DisplayText(errorMsg, 10);
			}
		}
		catch (fmt::v10::format_error) {
			Log::DisplayText("[VRInfo] Invalid format string LastScore: TextContent", 10);
		}
		
		if (score > PrevTopScore) {
			PrevTopScore = score;
		}

		return place;
	}

	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		if (!Active) {
			return oFUN_00878f70();
		}

		if ((strcmp(Mem::AreaCode, "init") == 0) || (strcmp(Mem::AreaCode, "mselect") == 0) || (*(uintptr_t*)0xED5268 == 0)) {
			return oFUN_00878f70();
		}

		int curMission = *Mem::VRMissionID;
		if (curMission == -1) {
			return oFUN_00878f70();
		}

		// reset the stored data if going to a new mission
		if (PrevMission != curMission) {
			PrevMission = curMission;
			PrevTopScore = TopPlayerScore();
		}

		if (TopScoreConfig.Enabled) {
			unsigned int topScore = TopPlayerScore();
			if (PrevTopScore > topScore) {
				topScore = PrevTopScore;
			}

			if (topScore != 0) {
				std::string sScore = std::to_string(topScore);
				MGS2::TextDraw(TopScoreConfig.Content, sScore.c_str(), TopScoreConfig);
			}
		}


		oFUN_00878f70();
	}


	void ToggleAction(Actions::Action action) {
		Active = !Active;
		Log::DisplayToggleMessage(CategoryName, Active);
	}

	void ToggleTopScoreAction(Actions::Action action) {
		TopScoreConfig.Enabled = !TopScoreConfig.Enabled;
		Log::DisplayToggleMessage("VR Info - Top Score", TopScoreConfig.Enabled);
	}

	void ToggleLastScoreAction(Actions::Action action) {
		LastScoreConfig.Enabled = !LastScoreConfig.Enabled;
		Log::DisplayToggleMessage("VR Info - Last Score", LastScoreConfig.Enabled);
	}


	void Run(CSimpleIniA& ini) {
		const char* category = "VRInfo";

		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		Active = ini.GetBoolValue(category, "Active", Active);

		TopScoreConfig.ParseConfig(ini, "VRInfo.TopScore");
		
		const char* subcategory = "VRInfo.LastScore";
		LastScoreConfig.ParseConfig(ini, subcategory);
		LastScoreTimeout = ini.GetDoubleValue(subcategory, "TextTimeout", LastScoreTimeout);

		for (auto& format : LastScoreFormats) {
			format.ComparisonFormat = ini.GetValue("VRInfo.LastScore.Comparison", format.Key, format.ComparisonFormat);
			format.StatusFormat = ini.GetValue("VRInfo.LastScore.Status", format.Key, format.StatusFormat);
		}

		Actions::RegisterAction(ini, "VRInfo.Toggle", &ToggleAction);
		Actions::RegisterAction(ini, "VRInfo.TopScore", &ToggleTopScoreAction);
		Actions::RegisterAction(ini, "VRInfo.LastScore", &ToggleLastScoreAction);

		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
		oFUN_00550750 = (tFUN_Int_IntUint)mem::TrampHook32((BYTE*)0x550750, (BYTE*)hkFUN_00550750, 10);
	}

}
