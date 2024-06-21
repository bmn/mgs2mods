#pragma once
#include "framework.h"
#include <vector>
#include <functional>
#include "lib/Callback/Callback.hpp"

namespace MGS2 {
	struct TextConfig;

	typedef void(__cdecl* tFUN_007902a5)(int param_1, int param_2, unsigned int param_3);
	typedef LRESULT(__stdcall* tFUN_00401350)(HWND hWnd_param_1, unsigned int uMsg_param_2, WPARAM wParam_param_3, LPARAM* lParam_param_4);

	typedef void(__cdecl* tFUN_Void)();
	typedef void(__cdecl* tFUN_Void_CC)(const char* param_1);
	typedef int(__cdecl* tFUN_Int_Void)();
	typedef int(__cdecl* tFUN_Int_Byte)(BYTE param_1);
	typedef int(__cdecl* tFUN_Int_Int)(int param_1);
	typedef int(__cdecl* tFUN_Int_UintUint)(unsigned int param_1, unsigned int param_2);
	typedef int(__cdecl* tFUN_Int_IntInt)(int param_1, int param_2);
	typedef int(__cdecl* tFUN_Int_IntIntInt)(int param_1, int param_2, int param_3);
	typedef int(__cdecl* tFUN_Int_IntIntIntInt)(int param_1, int param_2, int param_3, int param_4);
	typedef int(__cdecl* tFUN_Int_IntUint)(int param_1, unsigned int param_2);
	typedef void(__cdecl* tFUN_Void_Int)(int param_1);
	typedef void(__cdecl* tFUN_Void_IntInt)(int param_1, int param_2);
	typedef void(__cdecl* tFUN_Void_IntIntInt)(int param_1, int param_2, int param_3);
	typedef void(__cdecl* tFUN_Void_IntIntIntInt)(int param_1, int param_2, int param_3, int param_4);
	typedef void(__cdecl* tFUN_Void_PshortPint)(short* param_1, int* param_2);
	typedef void(__cdecl* tFUN_Void_IntIntUint)(int param_1, int param_2, unsigned int param_3);
	typedef BYTE* (__cdecl* tFUN_PByte_Char)(char param_1);


	enum Stage : char {
		None = 0,
		Tanker = 1,
		Plant = 2,
		Missions = 3,
		BossSurvival = 4,
		TalesA = 5,
		TalesB = 6,
		TalesC = 7,
		TalesD = 8,
		TalesE = 9,
	};

	enum TextAlignment : char {
		Left = 0,
		Right = 1,
		Center = 2
	};

	enum Difficulty : char {
		VeryEasy = 10,
		Easy = 20,
		Normal = 30,
		Hard = 40,
		Extreme = 50,
		EuroExtreme = 60
	};

	struct AlertMode {
		enum Enum : char {
			Infiltration = 0,
			Alert = 1,
			Evasion = 2,
			Caution = 3
		};
	};

	/*
	const char* WeaponNames[]{
		"No Weapon", "M9", "USP", "SOCOM", "PSG-1", "RGB6", "Nikita", "Stinger", "Claymore", "C4",
		"Chaff G", "Stun G", "D.Mic", "HF.Blade", "Coolant", "AK-74u", "Magazine", "Grenade", "M4", "PSG-1T",
		"D.Mic?", "Book"
	};

	const char* ItemNames[]{
		"No Item", "Ration", "Scope?", "Medicine", "Bandage", "Pentazemin", "BDU", "B.Armor", "Stealth", "Mine.D",
		"Sensor A", "Sensor B", "NVG", "Therm.G", "Scope", "D.Camera", "Box 1", "Cigs", "Card", "Shaver",
		"Phone", "Camera", "Box 2", "Box 3", "Wet Box", "AP Sensor", "Box 4", "Box 5", "???", "SOCOM Supp.",
		"AK Supp.", "Camera ? ", "Bandana", "Dog Tags", "MO Disc", "USP Suppressor", "Inf.Wig", "Blue Wig", "Orange B", "Wig C",
		"Wig D",
	};
	*/

	namespace Mem {
		extern char* Difficulty;
		extern char* CharacterCode;
		extern short* ProgressTanker;
		extern short* ProgressPlant;
		extern short* ProgressTales_Deprecated;
		extern char* ProgressTalesArray;
		extern unsigned int* PlayerInput;
		extern unsigned int* PlayerNewInput;
		extern int* RenderedFrames;
		extern int* GameTimeFrames;
		extern int* AreaTimeFrames;
		extern char* AreaCode;
		extern char* SaveGameBasePath;
		extern uintptr_t* MainGameStats;
		extern uintptr_t* MainGameData;
		extern uintptr_t* MainContinueData;
		extern uintptr_t* PositionData;
		extern unsigned int* Random;
		extern short* Stats;
		extern short* EquippedWeapon;
		extern short* EquippedItem;
		extern short* ReserveWeapon;
		extern short* ReserveItem;
		extern uintptr_t* WeaponData;
		extern uintptr_t* ItemData;
		extern size_t WeaponMaxOffset;
		extern size_t ItemMaxOffset;
		extern size_t WeaponItemContinueOffset;
		extern void* CurrentData0;
		extern void* ContinueData0;
		extern void* CurrentData1;
		extern void* ContinueData1;
		extern int* VRMissionID;
		extern tFUN_Int_Int const GclGetMappedFunction;
		extern tFUN_Void_CC const GclWriteDebug;
		extern tFUN_Int_Byte const GclParamInit;
		extern tFUN_Int_Void const GclParamReadNext;

		short Progress();
		MGS2::Stage Stage();
	}

	namespace Vox {
		int PlayBlock(size_t block);
		int PlayOffset(size_t offset);
		int Play(size_t index);
	}



	unsigned int RNG(unsigned int mod);

	extern CSimpleIniA Ini;
	extern HMODULE ModuleHandleA;

	void Run();

	void TextInit();
	void TextSetColor(int r, int g, int b);
	void TextSetColor(int rgb);
	void TextSetPosition(int x, int y, int align);
	void TextDraw(const char* content);
	void TextDraw(const char* format, const char* content);
	void TextDraw(const char* format, int content);
	void TextDraw(int x, int y, int align, int rgb, const char* format, const char* content = nullptr);
	void TextDraw(const char* content, TextConfig config);
	void TextDraw(TextConfig config);
	void TextDraw(const char* format, const char* content, TextConfig config);
	void TextSetup(TextConfig config);
	void TriggerGameOver(unsigned int delay = 0);
	unsigned int RemapL3(unsigned int mask);


	struct TextConfig {
		bool Enabled = true; // todo check
		int PosX = 0;
		int PosY = 0;
		int Align = 0;
		unsigned int Color = 0xB4B4B4;
		const char* Content = 0;
		int TextMaxLength = -1;
		const char* Text = 0;
		int OutlineColor = 0;
		TextConfig* ShadowTextConfig = nullptr;
		double Timeout = 0;

		static int const LineHeight = 17;

		size_t FormatArgs() {
			std::string_view str = (std::string_view)Content;
			return std::count(str.begin(), str.end(), '%');
		}

		void ParseConfig(CSimpleIniA& ini, const char* category) {
			if (ini.GetSectionSize(category) == -1) {
				return;
			}

			Enabled = ini.GetBoolValue(category, "TextVisible", Enabled);
			Align = ConfigParser::ParseHorizontalAlignment(ini.GetValue(category, "TextAlign", ""), Align);
			Color = ConfigParser::ParseHexColor(ini.GetValue(category, "TextColor", ""), Color);
			Content = ini.GetValue(category, "TextContent", Content);
			
			Timeout = ini.GetDoubleValue(category, "TextTimeout", Timeout);
			if (Timeout < 0) {
				Timeout = 0;
			}

			int outlineColor = ConfigParser::ParseHexColor(ini.GetValue(category, "TextOutline", ""), -1);
			if ((outlineColor != -1) || (!ini.GetBoolValue(category, "TextOutline", true))) {
				OutlineColor = outlineColor;
			}

			PosX = ConfigParser::ParseInteger(ini, category, "TextPosX", PosX, INT_MIN, INT_MAX);
			PosY = ConfigParser::ParseInteger(ini, category, "TextPosY", PosY, INT_MIN, INT_MAX);
		}

		TextConfig ShadowConfig() {
			TextConfig output = *this;
			output.PosX += 2;
			output.PosY += 2;
			output.Color = OutlineColor & 0xFFFFFF;
			return output;
		}

		TextConfig OutlineConfig(bool x = false, bool y = false) {
			const char mul = 1;
			TextConfig output = *this;
			output.PosX += x ? mul : -mul;
			output.PosY += y ? mul : -mul;
			output.Color = OutlineColor;
			return output;
		}

		void Draw(const char* content) {
			if ((OutlineColor & 0xFF000000) == 0) {
				TextDraw(content, OutlineConfig(true, true));
				TextDraw(content, OutlineConfig(true, false));
				TextDraw(content, OutlineConfig(false, true));
				TextDraw(content, OutlineConfig(false, false));
			}
			TextDraw(content, *this);
		}

		void Draw(const char* format, const char* content) {
			if ((OutlineColor & 0xFF000000) == 0) {
				TextDraw(format, content, OutlineConfig(true, true));
				TextDraw(format, content, OutlineConfig(true, false));
				TextDraw(format, content, OutlineConfig(false, true));
				TextDraw(format, content, OutlineConfig(false, false));
			}
			TextDraw(format, content, *this);
		}

		void Draw() {
			if (this->Content) {
				Draw(this->Content);
			}
			else if (this->Text) {
				Draw(this->Text);
			}
		}
	};



	namespace Turbo {
		void Run(CSimpleIniA& ini);
		int CurrentProfile();
		void RegisterProfileChangedCallback(cb::Callback1<void, unsigned int> callback);
	}

	namespace TextChange {
		void Run(CSimpleIniA& ini);
	}

	namespace TurboDisplay {
		void Run(CSimpleIniA& ini);
	}

	namespace Timer {
		void Run(CSimpleIniA& ini);
		void TriggerLapTime(double secs = 5);
		void TriggerAreaLapTime(double secs = 5);
		void TriggerBothLapTime(double secs = 5);
	}

	namespace Caution {
		void Run(CSimpleIniA& ini);
	}

	namespace FirstPerson {
		void Run(CSimpleIniA& ini);
	}

	namespace SoftReset {
		void Run(CSimpleIniA& ini);
		extern bool IsEnabled;
	}

	namespace SaveLocation {
		void Run(CSimpleIniA& ini);
	}

	namespace SaveGame {
		void Run(CSimpleIniA& ini);
	}

	namespace SaveMenu {
		void Run(CSimpleIniA& ini);
	}

	namespace Info {
		void Run(CSimpleIniA& ini);
	}

	namespace NewGameInfo {
		extern int CurrentScreen;
		extern int CurrentSection;
		void Run(CSimpleIniA& ini);
		void AddWarning(const char* warning);
		void AddWarning(const char* warning, cb::Callback0<bool> require);
		void RemoveWarning(const char* warning);
		void AddNotice(const char* notice);
		void AddNotice(const char* notice, cb::Callback0<bool> require);
		bool IsTankerSelected();
		bool IsTankerNotSelected();
	}

	namespace Options {
		void Run(CSimpleIniA& ini);
	}

	namespace Performance {
		extern double TimerFrequency;
		void Run(CSimpleIniA& ini);
	}

	namespace Actions {
		struct Shortcut {
			std::vector<KeyCombo> Keyboard;
			std::vector<PadCombo> Gamepad;
			ContextMenuItem* ContextMenu = NULL;
			bool Active = false;
		};

		struct Action;
		struct Callback {
			void(*Method)(Action) = nullptr;
		};

		struct Action {
			Shortcut Input;
			Callback Func;
			void* Data = NULL;
			cb::Callback1<void, Actions::Action> Callback;

			template<typename T>
			T CastData() {
				return static_cast<T>((unsigned int)Data);
			}
		};

		extern unsigned int GamepadInputHold;
		extern unsigned int GamepadInputPress;

		void Run(CSimpleIniA& ini);
		bool RegisterAction(CSimpleIniA& ini, const char* key, void(*callback)(Action), void* data = NULL);
		bool RegisterShortcut(const char* key, KeyCombo keyCombo, PadCombo gamepadCombo);
		bool RegisterShortcut(CSimpleIniA& ini, const char* key);
		bool ShortcutActive(const char* key);
		Shortcut NewSimpleShortcut(KeyCombo keyCombo, PadCombo gamepadCombo);

		bool RegisterAction(Shortcut shortcut, void(*callback)(Action), void* data = NULL);
		bool RegisterAction_SimplePatcher(CSimpleIniA& ini, const char* key, cb::Callback1<void, Actions::Action> callback, void* data = NULL);
		void AddToContextMenu(HMENU menu, const char* name);
		void AddToContextMenu(unsigned int id, const char* name);
		void AddContextMenuItem(ContextMenuItem* item);
		
		void OpenExplorer(const char* targetPath);
		PROCESS_INFORMATION StartProcess(const char* path, const char* args, DWORD priority = NORMAL_PRIORITY_CLASS, bool closeHandles = true);
	}

	namespace DelayedLoad {
		void Run(CSimpleIniA& ini);
	}

	namespace Wet {
		void Run(CSimpleIniA& ini);
	}

	struct SimplePatcher {
		enum NewGameInfoModes {
			None,
			Warning,
			Notice
		};

		const char* Category = nullptr;
		mem::PatchSet Patches;
		char NewGameInfoMode = NewGameInfoModes::None;
		const char* FriendlyName = nullptr;
		CSimpleIniA& Ini;
		bool Enabled = false;
		bool Active = false;
		bool Toggleable = true;

		void Run();
		void TogglePatches();

		bool NewGameInfoCallback() {
			return Active;
		}

		void ToggleAction(Actions::Action action);

		SimplePatcher(CSimpleIniA& ini, const char* category);
	};

	namespace Log {
		enum Level {
			Error,
			Warning,
			Info
		};

		extern TextConfig DefaultTextConfig;
		extern TextConfig VersionConfig;

		void SetCurrentCategory(std::string category = "");
		void Log(std::string message, Level level = Info);
		void Log(std::string message, std::string category = "", Level level = Info);

		void DisplayText(std::string message, TextConfig config, double timeout = 3);
		void DisplayText(std::string message, double timeout = 3);

		void DisplayToggleMessage(std::string message, bool active, TextConfig config, double timeout = 3);
		void DisplayToggleMessage(std::string message, bool active, double timeout = 3);

		void UncaughtException(const char* category = nullptr, const char* subCategory = nullptr, const char* what = nullptr);

		void Run(CSimpleIniA& ini);
	}

	namespace Affinity {
		extern long long MaxAffinity;
		const int DefaultPriority = NORMAL_PRIORITY_CLASS;

		long long ParseAffinity(CSimpleIniA& ini, const char* category, const char* key = "Affinity");
		bool SetAffinity(HANDLE process, long long affinity);
		DWORD ParsePriority(CSimpleIniA& ini, const char* category, const char* key = "Priority");
		bool SetPriority(HANDLE process, DWORD priority);

		void Run(CSimpleIniA& ini);
	}

	namespace Ames {
		void Run(CSimpleIniA& ini);
	}

	namespace VRRando {
		void Run(CSimpleIniA& ini);
	}

	namespace VRInfo {
		void Run(CSimpleIniA& ini);
	}

	namespace EquipShortcuts {
		extern const size_t WEAPON_COUNT;
		extern const size_t ITEM_COUNT;

		bool ChangeWeapon(short weapon, bool reserve = false);
		bool ChangeItem(short item, bool reserve = false);

		void Run(CSimpleIniA& ini);
	}

	namespace TwinSnakesInput {
		void Run(CSimpleIniA& ini);
	}

	namespace Stats {
		void Run(CSimpleIniA& ini);
	}
	
	namespace GameOver {
		void Run(CSimpleIniA& ini);
	}

	namespace CutsceneSkip {
		void Run(CSimpleIniA& ini);
	}

	namespace PS2Controls {
		void Run(CSimpleIniA& ini);
	}

	namespace ItemRando {
		extern bool Enabled;
		void Run(CSimpleIniA& ini);
	}
	
	namespace ItemRando3 {
		void Run(CSimpleIniA& ini);
	}

	namespace Style {
		void Run(CSimpleIniA& ini);
	}

};
