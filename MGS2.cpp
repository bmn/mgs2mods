#pragma once
#include "MGS2.framework.h"
#include "lib/SimpleIni/SimpleIni.h"
#include <vector>
#include <thread>

namespace MGS2 {
	namespace Mem {
		char* Difficulty = (char*)0x118ADD0;
		char* CharacterCode = (char*)0x118C374;
		short* ProgressTanker = (short*)0x118D93C;
		short* ProgressPlant = (short*)0x118D912;
		char* ProgressTalesArray = (char*)0x118D982;
		unsigned int* PlayerInput = (unsigned int*)0xEDADDC;
		unsigned int* PlayerNewInput = (unsigned int*)0xEDADE0;
		int* RenderedFrames = (int*)0x118ADAC;
		int* GameTimeFrames = (int*)0x118AEF8;
		int* AreaTimeFrames = (int*)0x118AEA4;
		char* AreaCode = (char*)0x118ADEC;
		char* SaveGameBasePath = (char*)0xA20AA0;
		uintptr_t* MainGameStats = (uintptr_t*)0xA01F34;
		uintptr_t* MainGameData = (uintptr_t*)0xA01F3C;
		uintptr_t* MainContinueData = (uintptr_t*)0xA01F40;
		uintptr_t* PositionData = (uintptr_t*)0xF609D0;
		unsigned int* Random = (unsigned int*)0x9FCD98;
		int* VRMissionID = (int*)0xF60C1C;

		short* EquippedWeapon = (short*)0x118AEC4;
		short* EquippedItem = (short*)0x118AEC6;
		short* ReserveWeapon = (short*)0x118AED6;
		short* ReserveItem = (short*)0x118AED8;
		uintptr_t* WeaponData = (uintptr_t*)0xA53E08;
		uintptr_t* ItemData = (uintptr_t*)0xA53E10;
		size_t WeaponMaxOffset = 0x48; //0x24;
		size_t ItemMaxOffset = 0x60; //0x30;
		size_t WeaponItemContinueOffset = 0x1596;

		void* CurrentData0 = (void*)0xA01F34;
		void* ContinueData0 = (void*)0xA01F38;
		void* CurrentData1 = (void*)0xA01F3C;
		void* ContinueData1 = (void*)0xA01F40;

		tFUN_Int_Int const GclGetMappedFunction = (tFUN_Int_Int)0x889FF0;
		tFUN_Void_CC const GclWriteDebug = (tFUN_Void_CC)0x6FA180;
		tFUN_Int_Byte const GclParamInit = (tFUN_Int_Byte)0x8DEE10;
		tFUN_Int_Void const GclParamReadNext = (tFUN_Int_Void)0x8DF040;



		MGS2::Stage Stage() {
			char firstChar = *(char*)Mem::AreaCode;
			if ((firstChar == 'd') || (firstChar == 'w')) {
				short* data = (short*)*(uintptr_t*)Mem::CurrentData0;
				return (data[3] & 0x1000) ? Stage::Tanker : Stage::Plant;
			}
			//if (*Mem::VRMissionID != -1) return Stage::Missions; // todo do this properly
			// boss survival?
			if (Mem::ProgressTalesArray[0] > 0) return (MGS2::Stage)(Stage::TalesA + Mem::ProgressTalesArray[0] - 1);
			return Stage::None;
		}

		short Progress() {
			char talesIndex;

			MGS2::Stage stage = Mem::Stage();
			switch (stage) {
				case Stage::Tanker:
					return *Mem::ProgressTanker;
				case Stage::Plant:
					return *Mem::ProgressPlant;
				case Stage::Missions:
					return *Mem::VRMissionID;
				case Stage::BossSurvival:
					return 0;
				case Stage::TalesA:
				case Stage::TalesB:
				case Stage::TalesC:
				case Stage::TalesD:
				case Stage::TalesE:
					talesIndex = Mem::ProgressTalesArray[0];
					return Mem::ProgressTalesArray[talesIndex];
				default:
					return -1;
			}
		}
	}

	unsigned int RNG(unsigned int mod) {
		unsigned int result = *Mem::Random;
		*Mem::Random = (*Mem::Random * 0x5d588b65) + 1;
		return mod ? result % mod : result;
	}

	void TextInit() {
		((void(*)(int))0x8893F0)(1);
	}

	void TextSetPosition(int x, int y, int align) {
		((void(*)(int, int, int, int))0x889440)(0, x, y, align);
	}

	void TextSetColor(int r, int g, int b) {
		((void(*)(int, int, int, int, int))0x889390)(0, r, g, b, 0x80);
	}

	void TextSetColor(int rgb) {
		unsigned char ff = 0xFF;
		TextSetColor((rgb >> 16) & ff, (rgb >> 8) & ff, rgb & ff);
	}

	void TextDraw(const char* format, const char* content) {
		((void(*)(const char*, const char*))0x889B40)(format, content);
	}

	void TextDraw(const char* format, int content) {
		((void(*)(const char*, int))0x889B40)(format, content);
	}

	void TextDraw(const char* content) {
		((void(*)(const char*))0x889B40)(content);
	}

	void TextDraw(int x, int y, int align, int rgb, const char* format, const char* content) {
		TextInit();
		TextSetColor(rgb);
		TextSetPosition(x, y, align);
		
		if (content == nullptr) {
			TextDraw(format);
		}
		else {
			TextDraw(format, content);
		}
	}

	void TextDraw(const char* content, TextConfig config) {
		TextDraw(config.PosX, config.PosY, config.Align, config.Color, content);
	}

	void TextDraw(TextConfig config) {
		TextDraw(config.PosX, config.PosY, config.Align, config.Color, config.Content);
	}

	void TextDraw(const char* format, const char* content, TextConfig config) {
		TextDraw(config.PosX, config.PosY, config.Align, config.Color, format, content);
	}

	void TextSetup(TextConfig config) {
		TextInit();
		TextSetColor(config.Color);
		TextSetPosition(config.PosX, config.PosY, config.Align);
	}

	void TriggerGameOver(unsigned int delay) {
		// Register GO and freeze game
		((tFUN_Void_Int)0x877c80)(0);

		if (delay) {
			// Set the fallback game over timer
			*(int*)0xA539E8 -= (1200 - delay);
		}
		else {
			// Trigger GO manually
			((tFUN_Void)0x877cc0)();
		}
	}

	unsigned int RemapL3(unsigned int mask) {
		if ((mask & 0x200) == 0x200) {
			mask &= 0xFFFFFDFF; // remove 0x200
			mask |= 0x60000; // add 0x60000 (L3)
		}
		return mask;
	}


	void RunDInputBg(CSimpleIniA& ini) {
		const char* category = "DInputBackground";

		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		// change capabilities from 5 (foreground) to 9 (background)
		mem::PatchMemory((void*)0x8D06A7, "\x09");
	}

	void RunNoQuitPrompt(CSimpleIniA& ini) {
		SimplePatcher* patcher = new SimplePatcher(ini, "NoQuitPrompt");

		if (!patcher->Enabled) {
			delete(patcher);
			return;
		}

		patcher->FriendlyName = "No Quit Prompt";

		patcher->Patches = mem::PatchSet{
			// nop the messagebox call...
			mem::Patch((void*)0x878E96, "\x90\x90\x90\x90\x90"),
			// and change the conditional jz to the quit procedure to an unconditional jmp
			mem::Patch((void*)0x878E9F, "\xEB")
		};

		patcher->Run();
	}

	void RunDrebinMode(CSimpleIniA& ini) {
		const char* patch = "\x66\xB8\x0F\x27";

		SimplePatcher* patcher = new SimplePatcher(ini, "DrebinMode");

		if (!patcher->Enabled) {
			delete(patcher);
			return;
		}

		patcher->FriendlyName = "Drebin Mode";
		patcher->NewGameInfoMode = 1; // warning 

		patcher->Patches = mem::PatchSet{
			mem::Patch((void*)0x87E9CA, patch),
			mem::Patch((void*)0x87E9DA, patch),
			mem::Patch((void*)0x87E9EA, patch),
			mem::Patch((void*)0x87E9FA, patch)
		};

		patcher->Run();
	}

	void RunUnlockRadar(CSimpleIniA& ini) {
		SimplePatcher* patcher = new SimplePatcher(ini, "UnlockRadar");

		if (!patcher->Enabled) {
			delete(patcher);
			return;
		}

		patcher->FriendlyName = "Unlock Radar";
		patcher->NewGameInfoMode = 1; // warning 
		patcher->Patches = mem::PatchSet({
			mem::Patch((void*)0x841E4B, "\x66\xB8\x00\x00\x90\x90\x90", 7),
			mem::Patch((void*)0x878BEE, "\x90\x90")
			});

		patcher->Run();
	}

	
	CSimpleIniA Ini;
	void Run() {
		Ini.SetMultiKey();
		Ini.SetMultiLine();

		std::vector<const char*> iniPaths = {
			"MGS2.ini",
			"MGS2.Actions.ini", "MGS2.Affinity.ini", "MGS2.Ames.ini",
			"MGS2.Caution.ini", "MGS2.CutsceneSkip.ini",
			"MGS2.DelayedLoad.ini", "MGS2.DInputBackground.ini", "MGS2.DrebinMode.ini",
			"MGS2.EquipShortcuts.ini",
			"MGS2.FirstPerson.ini",
			"MGS2.GameOver.ini",
			"MGS2.Info.ini", "MGS2.ItemRando.ini",
			"MGS2.NewGameInfo.ini", "MGS2.NoQuitPrompt.ini",
			"MGS2.Options.ini",
			"MGS2.Performance.ini", "MGS2.PS2Controls.ini",
			"MGS2.RNG.ini",
			"MGS2.SaveLocation.ini", "MGS2.SaveMenu.ini", "MGS2.SoftReset.ini", "MGS2.Stats.ini",
			"MGS2.TextChange.ini", "MGS2.Timer.ini", "MGS2.Turbo.ini", "MGS2.TurboDisplay.ini", "MGS2.TwinSnakesInput.ini",
			"MGS2.UnlockRadar.ini",
			"MGS2.VRInfo.ini", "MGS2.VRRando.ini",
			"MGS2.Wet.ini"
		};
		for (const char* iniPath : iniPaths) {
			if (std::filesystem::exists(iniPath)) {
				Ini.LoadFile(iniPath);
			}
		}

		if (!Ini.GetBoolValue("MGS2", "Enabled", true)) {
			return;
		}

		Log::Run(Ini);
		Affinity::Run(Ini);
		Timer::Run(Ini);
		Stats::Run(Ini);
		TwinSnakesInput::Run(Ini);
		Turbo::Run(Ini);
		TurboDisplay::Run(Ini);
		Info::Run(Ini);
		RunDInputBg(Ini);
		RunNoQuitPrompt(Ini);
		PS2Controls::Run(Ini);
		TextChange::Run(Ini);
		RunUnlockRadar(Ini);
		RunDrebinMode(Ini);
		FirstPerson::Run(Ini);
		SaveLocation::Run(Ini);
		SaveMenu::Run(Ini); // after SaveLocation
		Caution::Run(Ini); // after SaveMenu
		GameOver::Run(Ini);
		SoftReset::Run(Ini);
		NewGameInfo::Run(Ini);
		Options::Run(Ini);
		Wet::Run(Ini);
		Performance::Run(Ini);
		Ames::Run(Ini);
		DelayedLoad::Run(Ini);
		VRRando::Run(Ini);
		VRInfo::Run(Ini);
		ItemRando::Run(Ini);
		ItemRando3::Run(Ini);
		EquipShortcuts::Run(Ini);
		CutsceneSkip::Run(Ini);
		Actions::Run(Ini);
	}

}
