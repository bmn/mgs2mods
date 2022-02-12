#pragma once
#include "framework.h"

namespace MGS2 {
	enum TextAlignment {
		Left = 0,
		Right = 1,
		Center = 2
	};

	void TextInit();
	void TextSetColor(int r, int g, int b);
	void TextSetColor(int rgb);
	void TextSetPosition(int x, int y, int align);
	void TextDraw(const char* content);
	void TextDraw(const char* format, const char* content);
	void TextDraw(const char* format, int content);
	void TextDraw(int x, int y, int align, int rgb, const char* content);
	void TextDraw(int x, int y, int align, int rgb, const char* format, const char* content);
	//void TextDraw(int x, int y, int align, int rgb, const char* format, int content);
	unsigned int RemapL3(unsigned int mask);

	typedef void(__cdecl* tFUN_007902a5)(int param_1, int param_2, unsigned int param_3);
	typedef LRESULT(__stdcall* tFUN_00401350)(HWND hWnd_param_1, unsigned int uMsg_param_2, WPARAM wParam_param_3, LPARAM* lParam_param_4);

	typedef void(__cdecl* tFUN_Void)();
	typedef int(__cdecl* tFUN_Int_Int)(int param_1);
	typedef void(__cdecl* tFUN_Void_Int)(int param_1);
	typedef void(__cdecl* tFUN_Void_IntInt)(int param_1, int param_2);
	typedef void(__cdecl* tFUN_Void_PshortPint)(short* param_1, int* param_2);
	typedef void(__cdecl* tFUN_Void_IntIntUint)(int param_1, int param_2, unsigned int param_3);




	namespace Turbo {
		void Run(CSimpleIniA& ini);
		int CurrentProfile();
		void RegisterProfileChangedCallback(void(*callback)(int));
	}

	namespace TextChange {
		void Run(CSimpleIniA& ini);
	}

	namespace TurboDisplay {
		void __cdecl hkFUN_00878f70();
		void Run(CSimpleIniA& ini);
	}

	namespace Timer {
		void __cdecl hkFUN_00878f70();
		void Run(CSimpleIniA& ini);
	}

	namespace Caution {
		void __cdecl hkFUN_0042de2f();
		void Run(CSimpleIniA& ini);
	}

	namespace FirstPerson {
		void Run(CSimpleIniA& ini);
	}

	namespace SoftReset {
		void Run(CSimpleIniA& ini);
		bool IsEnabled();
	}

	namespace SaveLocation {
		void Run(CSimpleIniA& ini);
	}

	namespace SaveGame {
		void Run(CSimpleIniA& ini);
	}

	namespace InputDisplay {
		void Run(CSimpleIniA& ini);
	}

	namespace NewGameInfo {
		int Screen();
		int Section();
		void Run(CSimpleIniA& ini);
		void AddWarning(const char* warning);
		void AddWarning(const char* warning, bool(*require)(void));
		void AddNotice(const char* notice);
		void AddNotice(const char* notice, bool(*require)(void));
	}

	namespace Options {
		void Run(CSimpleIniA& ini);
	}

	namespace Performance {
		void Run(CSimpleIniA& ini);
		double TimerFrequency();
	}

	void RunDInputBg(CSimpleIniA& ini);
	void RunNoQuitPrompt(CSimpleIniA& ini);
	void RunConsole(CSimpleIniA& ini);
	void RunPS2Controls(CSimpleIniA& ini);
	void RunDrebinMode(CSimpleIniA& ini);
	void RunUnlockRadar(CSimpleIniA& ini);
	void RunAffinity(CSimpleIniA& ini);

	namespace Actions {
		struct Shortcut {
			WPARAM keyCode = 0;
			char modifierMask = 0;
			void(*callback)(Shortcut) = NULL;
			const char* Path = NULL;
			const char* Args = NULL;
		};

		void Run(CSimpleIniA& ini);
		void RegisterShortcut(WPARAM keyCode, char modifierMask, void(*callback)(Shortcut));
		void RegisterShortcut(const char* keyCode, long long modifierMask, void(*callback)(Shortcut));
	}

	namespace DelayedLoad {
		void Run(CSimpleIniA& ini);
	}

	namespace Wet {
		void Run(CSimpleIniA& ini);
	}
};