#include "MGS2.framework.h"
#include <unordered_set>

namespace MGS2::CutsceneSkip {
	const char* Category = "CutsceneSkip";

	bool SkipCutscenes = true;
	bool SkipAttract = true;
	bool SkipBladeDemo = false;
	bool AllowSkipBladeDemo = true;
	bool AllowSkipSolidusSpeech = true;
	bool AllowSkipChokeDialogue = true;
	bool SkipMainGameCredits = false;
	bool AllowSkipMainGameCredits = true;
	bool SkipSnakeTalesCredits = false;
	bool FastSkip = true;
	bool AddGameTime = false;

	bool SkipBridgeSensors = false;
	bool SkipKLBridgeGuards = false;
	bool SkipStrutBBomb = false;
	bool SkipStrutBCamera = false;
	bool SkipCDBridgeCamera = false;
	bool SkipStrutDBomb = false;

	int const COMPENSATION_BLADEDEMO = 2700;
	int const COMPENSATION_SOLIDUSSPEECH = 5214;
	int const COMPENSATION_MAINGAMECREDITS = 33220;

	short const CODEC_SECTION_EMMA = 0x3748;
	short const CODEC_SECTION_PREZ = 0x3739;
	short const CODEC_SECTION_AMES = 0x36F8;
	short const CODEC_SECTION_SNAKE = 0x3764;
	short const CODEC_SECTION_NINJA = 0x36E0;
	short const CODEC_SECTION_ROSE = 0x3710;
	short const CODEC_SECTION_STILLMAN = 0x36C9;
	short const CODEC_SECTION_TANKER_OTACON = 0x3720;
	short const CODEC_SECTION_COWBELL = 0x3705;
	char const CODEC_PRIORITY_OPTIONAL = 2;
	char const CODEC_PRIORITY_FORCED = 3;
	char const CODEC_PRIORITY_CUTSCENE = 4;

	std::unordered_set<short> const ValidCodecSections{
		CODEC_SECTION_AMES, CODEC_SECTION_COWBELL, CODEC_SECTION_EMMA, CODEC_SECTION_NINJA, CODEC_SECTION_PREZ,
		CODEC_SECTION_ROSE, CODEC_SECTION_SNAKE, CODEC_SECTION_STILLMAN, CODEC_SECTION_TANKER_OTACON
	};

	tFUN_Int_Byte const ParamInitFunc = (tFUN_Int_Byte)0x8DEE10;
	tFUN_Int_Void const ParamFunc = (tFUN_Int_Void)0x8DF040;
	tFUN_Void_IntInt const ProcFunc = (tFUN_Void_IntInt)0x8DF240;
	auto const NewDelayFunc = (int(*)(int, short, int))0x57E840;


	int ResetLogMessageSetAt;
	std::string LogMessageSet;
	TextConfig LogMessageConfig = Log::DefaultTextConfig;
	void AddToLog(std::string msgFormat, uintptr_t nextProc) {
		std::string newMessage = fmt::format(fmt::runtime(msgFormat), nextProc);
		OutputDebugStringA(newMessage.c_str());

		if (*Mem::RenderedFrames < ResetLogMessageSetAt) {
			LogMessageSet += "\n" + newMessage;
		}
		else {
			LogMessageSet = newMessage;
		}

		ResetLogMessageSetAt = *Mem::RenderedFrames + 180;
		Log::DisplayText(LogMessageSet, LogMessageConfig);
	}

	void AddFramesToGameTimeConditional(int frames) {
		if (!AddGameTime) return;

		// avoid keyboard double/triple effect by only allowing once a second
		static int lastActionRF = 0;
		if ((*Mem::RenderedFrames >= lastActionRF) && (*Mem::RenderedFrames < (lastActionRF + 60))) {
			return;
		}
		lastActionRF = *Mem::RenderedFrames;

		*Mem::GameTimeFrames += frames;
		*Mem::AreaTimeFrames += frames;
#ifdef _DEBUG
		AddToLog("Added {}F to Game Time", frames);
#endif
	}


	// Avoid codec sequences and calls
	tFUN_Int_Void oFUN_0040ab70;
	int __cdecl hkFUN_0040ab70() {
		try_mgs2
			if (*(int*)0xA154DC == 0) {
				return 0;
			}

			if ((BYTE*)((tFUN_Int_Byte)0x8DEE10)('c') == nullptr) {
				return oFUN_0040ab70();
			}

			int paramC[4];
			tFUN_Int_Void currentParamGetNextValue = (tFUN_Int_Void)0x8DF040;
			for (size_t i = 0; i < 3; i++) {
				paramC[i] = currentParamGetNextValue();
			}

			// Codec section [0]
			// Codec priority [2] = 2 (optional), 3 (forced), 4 (cutscene)

			// Codec type
			if (ValidCodecSections.contains(paramC[0])) {
				// Allow optional codecs only
				if (paramC[2] == CODEC_PRIORITY_OPTIONAL) {
					return oFUN_0040ab70();
				}
			}
			// Not codec type
			else {
				return oFUN_0040ab70();
			}

			// Default behaviour if no callback proc provided
			if (((tFUN_Int_Void)0x8DF020)() == 0) {
				return oFUN_0040ab70();
			}
			paramC[3] = currentParamGetNextValue();

			// issue: tanker cutscene camera not being removed, true camera not being added (caused by end of codec triggering progress set)
			// todo do more cleanly?
			if (*Mem::ProgressTanker == 12) {
				*Mem::ProgressTanker = 14;
				EquipShortcuts::ChangeWeapon(1);
			}

			uintptr_t nextProc = paramC[3];

	#ifdef _DEBUG
			AddToLog("Codec SKIP {:06X}", nextProc);
	#endif

			if (nextProc == 0xE5781F) {
				NewDelayFunc(nextProc, (short)0, 2); // newdelay for 2f before skipping (for camera changes)
			}
			else {
				ProcFunc(nextProc, 0);
			}

			return 0;
		catch_mgs2(Category, "40AB70")

		// let game decide if there was an error
		return oFUN_0040ab70();
	}


	std::unordered_set<int> CancelNoSkipProcs;
	int const FastSkipSafeFrames = 2;
	
	// automatic cancel
	tFUN_Void_Int oFUN_0057e5b0;
	void __cdecl hkFUN_0057e5b0(int param_1) {
		static int currentNoSkipProc = 0;

		try_mgs2
			int* param = (int*)param_1;
			int nextProc = param[0xE];

			// TODO avoid skipping on casting theatre

			if (CancelNoSkipProcs.contains(nextProc)) {
	#ifdef _DEBUG
				if (currentNoSkipProc != nextProc) {
					AddToLog("Cancel ALLOW {:06X}", nextProc); // crash!
					currentNoSkipProc = nextProc;
				}
	#endif
				return oFUN_0057e5b0(param_1);
			}
			currentNoSkipProc = 0;

			if (nextProc == 0xBA5837) {
				bool noLetterboxSkip = false;
				if (
					((!SkipBridgeSensors) && (!strcmp(Mem::AreaCode, "w25a")))
					|| ((!SkipKLBridgeGuards) && (!strcmp(Mem::AreaCode, "w25b")))
					) {
					noLetterboxSkip = true;
				}
				else if (!strcmp(Mem::AreaCode, "w14a")) { // strut b
					bool nearBomb = (*(int*)((*(uintptr_t*)Mem::CurrentData0) + 0xE8) > -52800); // x pos
					if (
						((!SkipStrutBBomb) && nearBomb)
						|| ((!SkipStrutBCamera) && (!nearBomb))
						) {
						noLetterboxSkip = true;
					}
				}

				if (noLetterboxSkip) {
					return oFUN_0057e5b0(param_1);
				}
			}

			if (FastSkip) {
				if (
					(nextProc != 0xCFB02E) // deck e
					&& (nextProc != 0xBA5837) // hori bars (bomb reveals, harrier sensors complete)
					&& (nextProc != 0x157E3D) // raven
					&& (false) // todo bring back when bugs are fixed
					) {
					param[0xC] = 0;
				}
				else if (param[0xC] > FastSkipSafeFrames) {
					param[0xC] = FastSkipSafeFrames; // no 10-frame dead period
				}
			}

	#ifdef _DEBUG
			if (((int*)param_1)[0xC] == 0) {
				AddToLog("Cancel SKIP {:06X}", ((int*)param_1)[0xE]);
			}
	#endif

			unsigned int* puInput = (unsigned int*)0xEDADE4;
			unsigned int inputBak = *puInput;
			*puInput |= *(unsigned short*)0x57E5C8; // add 0x840 to the input
			oFUN_0057e5b0(param_1);
			*puInput = inputBak;
			return;
		catch_mgs2(Category, "57E5B0")

		// fallback
		return oFUN_0057e5b0(param_1);
	}

	
	// autocancel mpegstrx
	tFUN_Int_Int oFUN_0057d480;
	int __cdecl hkFUN_0057d480(int param_1) {
		try_mgs2
#ifdef _DEBUG
			static int lastSkipTime = 0;
			if (lastSkipTime > *Mem::RenderedFrames) {
				AddToLog("MPEGSTRX SKIP", 0);
				lastSkipTime = *Mem::RenderedFrames + 1; // don't do it on successive frames
			}
#endif

			unsigned int* puInput = (unsigned int*)0xEDADE4;
			unsigned int inputBak = *puInput;
			*puInput |= *(unsigned short*)0x57E5C8; // add 0x840 to the input
			int result = oFUN_0057d480(param_1);
			*puInput = inputBak;
			return result;
		catch_mgs2(Category, "57D480")

		// fallback
		return oFUN_0057d480(param_1);
	}
	

	int* piBladeDemoTimer = nullptr;
	// new delay
	tFUN_Int_Int oFUN_0057e940;
	int __cdecl hkFUN_0057e940(int param_1) {
		int* puVar = (int*)oFUN_0057e940(param_1);

		try_mgs2
			// asc colon sword use
			if (param_1 == 0xC04998) {
				piBladeDemoTimer = &puVar[0x10];
				if (SkipBladeDemo) {
					AddFramesToGameTimeConditional(*piBladeDemoTimer);
					*piBladeDemoTimer = 0; // set timer to 0
				}
			}
		catch_mgs2(Category, "57E940")

		return (int)puVar;
	}



	int RedirectIntVoid(tFUN_Int_Void& callback, std::string message, int delay = 0, char paramName = 'p') {
		if (!ParamInitFunc(paramName)) {
			return callback();
		}
		uintptr_t nextProc = ParamFunc();
#ifdef _DEBUG
		AddToLog(message, nextProc);
#endif
		if (delay) {
			NewDelayFunc(nextProc, (short)0, delay);
		}
		else {
			ProcFunc(nextProc, 0);
		}
		return 0;
	}

	int RedirectIntInt(tFUN_Int_Int& callback, int param_1, std::string message, int delay = 0, char paramName = 'p') {
		if (!ParamInitFunc(paramName)) {
			return callback(param_1);
		}
		uintptr_t nextProc = ParamFunc();
#ifdef _DEBUG
		AddToLog(message, nextProc);
#endif
		if (delay) {
			NewDelayFunc(nextProc, (short)0, delay); // crash risk?
		}
		else {
			ProcFunc(nextProc, 0);
		}
		return 0;
	}


	// vr book init
	tFUN_Int_Void oFUN_00581230;
	int __cdecl hkFUN_00581230() {
		try_mgs2
			return RedirectIntVoid(oFUN_00581230, "VR Book SKIP {:06X}");
		catch_mgs2(Category, "581230");

		// fallback
		return oFUN_00581230();
	}


	// konami logo init (i.e. plant start)
	tFUN_Int_Int oFUN_00751240;
	int __cdecl hkFUN_00751240(int param_1) {
		try_mgs2
			if (!ParamInitFunc('p')) {
				return oFUN_00751240(param_1);
			}

			int nextProc = ParamFunc();

			bool isAttract = ((nextProc == 0x7DE45E) || (nextProc == 0x537D94));
			bool doSkipAttract = (SkipAttract && isAttract);
			bool doSkipCutscene = (SkipCutscenes && (!isAttract));

			return (doSkipAttract || doSkipCutscene) ?
				RedirectIntInt(oFUN_00751240, param_1, "Logo SKIP {:06X}") :
				oFUN_00751240(param_1);
		catch_mgs2(Category, "751240");

		// fallback
		return oFUN_00751240(param_1);
	}


	// confirm init (save prompt)
	tFUN_Int_Int oFUN_0074e1e0;
	int __cdecl hkFUN_0074e1e0(int param_1) {
		try_mgs2
			// todo only for main game save prompts
#ifdef _DEBUG
			if (false) {
				std::string msg = "";
				bool go = false;
				if (ParamInitFunc('p')) {
					msg = fmt::format(fmt::runtime("p={:02X}"), ParamFunc());
					go = true;
				}
				if (ParamInitFunc('s')) {
					msg = msg + fmt::format(fmt::runtime(" s={:02X}"), ParamFunc());
					go = true;
				}
				if (go) {
					Log::DisplayText(msg, 15.0);
				}
			}
#endif

			if (
				(!strcmp(Mem::AreaCode, "d13t")) // Tanker complete
				|| (!strcmp(Mem::AreaCode, "d080p01")) // Completed Rays
				) {
				return RedirectIntInt(oFUN_0074e1e0, param_1, "Confirm SKIP {:06X}");
			}
		catch_mgs2(Category, "74E1E0");

		return oFUN_0074e1e0(param_1);
	}


	// ending init
	tFUN_Int_Void oFUN_00580830;
	int __cdecl hkFUN_00580830() {
		try_mgs2
			std::string endingSkipMsg = "Ending SKIP {:06X}";
#ifdef _DEBUG
			endingSkipMsg = endingSkipMsg + std::format(" / GT={}", *Mem::GameTimeFrames);
#endif

			// todo fix for snake tales
			if (!strcmp(Mem::AreaCode, "sselect")) {
				if (!SkipSnakeTalesCredits) {
					return oFUN_00580830();
				}
				ProcFunc(0x6B1372, 0);
				return 0;
			}

			if (!SkipMainGameCredits) {
				return oFUN_00580830();
			}

			AddFramesToGameTimeConditional(COMPENSATION_MAINGAMECREDITS);
			return RedirectIntVoid(oFUN_00580830, endingSkipMsg);
		catch_mgs2(Category, "580830");

		// fallback
		return oFUN_00580830();
	}

	
	// web site init
	tFUN_Int_Int oFUN_007db100;
	int __cdecl hkFUN_007db100(int param_1) {
		try_mgs2
			return RedirectIntInt(oFUN_007db100, param_1, "Website SKIP {:06X}", 0, 'c');
		catch_mgs2(Category, "7DB100");

		// fallback
		return oFUN_007db100(param_1);
	}
	
	// show pic init 0 (newspaper in tales e)
	tFUN_Int_Void oFUN_00583b50;
	int __cdecl hkFUN_00583b50() {
		try_mgs2
			return RedirectIntVoid(oFUN_00583b50, "ShowPic SKIP {:06X}");
		catch_mgs2(Category, "583B50");

		// fallback
		return oFUN_00583b50();
	}

	// photo view init (gurlugon photo)
	tFUN_Int_Void oFUN_007e6db0;
	int __cdecl hkFUN_007e6db0() {
		try_mgs2
			return RedirectIntVoid(oFUN_007e6db0, "PhotoView SKIP {:06X}");
		catch_mgs2(Category, "7E6DB0");

		// fallback
		return oFUN_007e6db0();
	}


	void SkipAction(Actions::Action action) {
		if (AllowSkipChokeDialogue && (!strcmp(Mem::AreaCode, "w41a")) && ((*Mem::ProgressPlant == 351))) {
			ProcFunc(0xD7FF46, 0);
		}
		else if (AllowSkipBladeDemo && (!strcmp(Mem::AreaCode, "w43a")) && (*Mem::ProgressPlant == 389)
			&& (piBladeDemoTimer) && (*piBladeDemoTimer > 0)) {
			AddFramesToGameTimeConditional(*piBladeDemoTimer);
			*piBladeDemoTimer = 0;
		}
		else if (AllowSkipSolidusSpeech && (!strcmp(Mem::AreaCode, "w46a"))) {
			if (*Mem::ProgressPlant != 408) return;
			AddFramesToGameTimeConditional(COMPENSATION_SOLIDUSSPEECH - *Mem::AreaTimeFrames);
			ProcFunc(0x592B35, 0);
		}
		else if (AllowSkipChokeDialogue && (!strcmp(Mem::AreaCode, "w51a"))) {
			ProcFunc(0xC42C25, 0);
		}
		else if (AllowSkipMainGameCredits && (!strcmp(Mem::AreaCode, "wmovie")) && (*Mem::ProgressPlant == 490)) {
			AddFramesToGameTimeConditional(COMPENSATION_MAINGAMECREDITS - *Mem::AreaTimeFrames);
			ProcFunc(0xD2DD13, 0);
		}
		else {
			return;
		}
	}

	tFUN_Void_Int oFUN_00408ac0;
	void __cdecl hkFUN_00408ac0(int param_1) {
		static char step = 0; // 0=start, 1=autopaused, 2=autoskip activated

		try_mgs2
			int* codecPhase = (int*)(param_1 + 0x88);
			BYTE* newInput = (BYTE*)(*(int*)(param_1 + 0x44) + 8);

			if (!*codecPhase) {
				step = 0;
			}
			// 5=normal, 0x18=video
			else if ((step == 0) && ((*codecPhase == 5) || (*codecPhase == 0x18))) {
				// character is talking
				step = 1;
				*newInput |= 0xF0; // add cancel input
			}
			// 4=paused
			else if ((step == 1) && (*codecPhase == 4)) {
				// in cancelled or fast-skip state
				step = 2;
				*newInput |= 0x90; // add fast skip input
			}
		catch_mgs2(Category, "408AC0");

		oFUN_00408ac0(param_1);
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		LogMessageConfig.PosY = 78;
		LogMessageConfig.Timeout = 10;

		std::map<std::string, bool> modesMap{
			{ "safe", false }, { "full", true },
			{ "basic", false }, { "custom", true }
		};
		bool fullModeActive = ConfigParser::ParseValueMap(ini, Category, "Mode", modesMap, false);
		
		const char* modeCategory = "CutsceneSkip.Custom";
		if (fullModeActive) {
			// Full Mode
			NewGameInfo::AddWarning("Cutscene Skip (Custom)");

			if (ini.GetBoolValue(modeCategory, "SkipCodecs", true)) {
				oFUN_0040ab70 = (tFUN_Int_Void)mem::TrampHook32((BYTE*)0x40AB70, (BYTE*)hkFUN_0040ab70, 5); // codecs
			}

			if (ini.GetBoolValue(modeCategory, "SkipSavePrompts", true)) {
				oFUN_0074e1e0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x74E1E0, (BYTE*)hkFUN_0074e1e0, 5); // confirm
			}

			if (SkipCutscenes = ini.GetBoolValue(modeCategory, "SkipCutscenes", SkipCutscenes)) {
				oFUN_0057e5b0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x57E5B0, (BYTE*)hkFUN_0057e5b0, 5); // cancel
				oFUN_0057d480 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x57D480, (BYTE*)hkFUN_0057d480, 6); // mpegstrx autoskip
				oFUN_007db100 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7DB100, (BYTE*)hkFUN_007db100, 6); // websiteinit

				FastSkip = ini.GetBoolValue(modeCategory, "FastSkip", FastSkip);

				const char* cutscenesCategory = "CutsceneSkip.Custom.Cutscenes";
				bool cutscenesDefault = ini.GetBoolValue(cutscenesCategory, "Default", false);

				std::map<const char*, int> settingsMap{
					{ "AftDeckSpotted", 0x9E4972 }, // Aft Deck - spotted by top floor guard
					{ "DeckB", 0xBE0980 }, // Deck B
					{ "DeckDMess", 0x674437 }, // Deck D Mess Hall
					{ "DeckDPantry", 0x2FFE73 }, // Deck D pantry guard
					{ "EngineRoomRaven", 0x157E3D }, // engine room - raven
					{ "EngineRoomDoor", 0x86F46A }, // engine room - door repair
					{ "HoldsGameOver", 0x4B4AD7 }, // holds - game over
					{ "Hold2Projector", 0x4D6AA1 }, // hold 2 projector demo
					{ "DockElevator", 0x06EB2B }, // strut a dock elevator
					{ "CDBridgeCamera", 0x833530 }, // cd bridge camera
					{ "StrutDBomb", 0x293D08 }, // strut d bombs
					{ "Shell1Elevator", 0xF5F0BF }, // shell 1 core 1f elevator demo
					{ "Shell1RunningMan", 0x6D8EDB }, // shell 1 core b2 running man
					{ "Shell1Biometrics", 0xEE74BD }, // shell 1 core b1 biometrics demo
					{ "Shell1Microphone", 0xEAEF35 }, // ames - king
					{ "Harrier", 0x8E0C75 }, // harrier (harrier actions)
					{ "HarrierSnake", 0x570AE5 }, // harrier (snake actions)
					{ "PerimeterFloor", 0x293D08 }, // 1-2 and kl walkway floor
					{ "KLPerimeterGuard", 0xCF0264 }, // 1-2 - gonna go in pantz
					{ "Shell2Microphone", 0x871697 }, // shell 2 core 1f - olga mic
				};
				for (std::pair<const char*, int> item : settingsMap) {
					const char* key = item.first; //std::get<0>(item);
					int hash = item.second; //std::get<1>(item);
					if (!ini.GetBoolValue(cutscenesCategory, key, cutscenesDefault)) {
						CancelNoSkipProcs.insert(hash);
					}
				}
				SkipBridgeSensors = ini.GetBoolValue(cutscenesCategory, "BridgeSensors", cutscenesDefault);
				SkipKLBridgeGuards = ini.GetBoolValue(cutscenesCategory, "KLBridgeGuards", cutscenesDefault);
				SkipStrutBBomb = ini.GetBoolValue(cutscenesCategory, "StrutBBomb", cutscenesDefault);
				SkipStrutBCamera = ini.GetBoolValue(cutscenesCategory, "StrutBCamera", cutscenesDefault);
				SkipCDBridgeCamera = ini.GetBoolValue(cutscenesCategory, "CDBridgeCamera", cutscenesDefault);
				SkipStrutDBomb = ini.GetBoolValue(cutscenesCategory, "StrutDBomb", cutscenesDefault);
			}

			if (ini.GetBoolValue(modeCategory, "SkipSnakeTalesStory", false)) {
				oFUN_00581230 = (tFUN_Int_Void)mem::TrampHook32((BYTE*)0x581230, (BYTE*)hkFUN_00581230, 6); // vrbookinit
				oFUN_00583b50 = (tFUN_Int_Void)mem::TrampHook32((BYTE*)0x583B50, (BYTE*)hkFUN_00583b50, 7); // showpicinit0
				oFUN_007e6db0 = (tFUN_Int_Void)mem::TrampHook32((BYTE*)0x7E6DB0, (BYTE*)hkFUN_007e6db0, 6); // photoviewinit
			}

			if (ini.GetBoolValue(modeCategory, "FastCodecs", true)) {
				oFUN_00408ac0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x408AC0, (BYTE*)hkFUN_00408ac0, 7); // codec input handler
			}

			SkipSnakeTalesCredits = ini.GetBoolValue(modeCategory, "SkipSnakeTalesCredits", SkipSnakeTalesCredits);
			AddGameTime = ini.GetBoolValue(modeCategory, "AddGameTime", AddGameTime);
		}
		else {
			// Safe Mode
			NewGameInfo::AddWarning("Cutscene Skips (Basic)");

			SkipCutscenes = false;
			SkipSnakeTalesCredits = false;
			AllowSkipChokeDialogue = false;
			FastSkip = false; // not used, but...
			AddGameTime = true;

			modeCategory = "CutsceneSkip.Basic";
		}

		// Stuff available in both modes

		SkipAttract = ini.GetBoolValue(Category, "SkipAttract", SkipAttract);
		if (SkipAttract || SkipCutscenes) {
			oFUN_00751240 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x751240, (BYTE*)hkFUN_00751240, 5); // logo
		}

		SkipMainGameCredits = ini.GetBoolValue(modeCategory, "SkipCredits", SkipMainGameCredits);
		if (SkipMainGameCredits || SkipSnakeTalesCredits) {
			oFUN_00580830 = (tFUN_Int_Void)mem::TrampHook32((BYTE*)0x580830, (BYTE*)hkFUN_00580830, 8); // endingx
		}


		SkipBladeDemo = ini.GetBoolValue(modeCategory, "SkipBladeDemo", SkipBladeDemo);
		AllowSkipBladeDemo = ini.GetBoolValue(modeCategory, "AllowSkipBladeDemo", AllowSkipBladeDemo);
		
		if (SkipBladeDemo || AllowSkipBladeDemo) {
			oFUN_0057e940 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x57E940, (BYTE*)hkFUN_0057e940, 5); // newdelay
		}

		AllowSkipSolidusSpeech = ini.GetBoolValue(modeCategory, "AllowSkipSolidusSpeech", AllowSkipSolidusSpeech);
		AllowSkipMainGameCredits = ini.GetBoolValue(modeCategory, "AllowSkipMainGameCredits", AllowSkipMainGameCredits);
		
		KeyCombo emptyKeyCombo;
		// PadCombo startButtonCombo = PadCombo(0x800);
		PadCombo triangleButtonCombo = PadCombo(0x10);
		if (AllowSkipBladeDemo || AllowSkipSolidusSpeech || AllowSkipChokeDialogue || AllowSkipMainGameCredits) {
			Actions::Shortcut skipShortcut = Actions::NewSimpleShortcut(emptyKeyCombo, triangleButtonCombo);
			skipShortcut.Gamepad[0].IgnoreComplexity = true;
			Actions::RegisterAction(skipShortcut, &SkipAction);
		}
		
	}

}
