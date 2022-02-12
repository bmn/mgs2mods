#include "MGS2.framework.h"
#include <vector>
#include <Shlwapi.h>

namespace MGS2::Actions {

	std::vector<Shortcut> Profiles;
	tFUN_00401350 oFUN_00401350;
	LRESULT hkFUN_00401350(HWND hWnd_param_1, unsigned int uMsg_param_2, WPARAM wParam_param_3, LPARAM* lParam_param_4) {
		if (uMsg_param_2 == WM_KEYDOWN) {
			char modifiers = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
			modifiers += (GetAsyncKeyState(VK_CONTROL) & 0x8000) ? 2 : 0;
			modifiers += (GetAsyncKeyState(VK_MENU) & 0x8000) ? 4 : 0;

			for (Shortcut shortcut : Profiles) {
				if ((modifiers & shortcut.modifierMask) && (wParam_param_3 == shortcut.keyCode)) {
					shortcut.callback(shortcut);
				}
			}
		}

		return oFUN_00401350(hWnd_param_1, uMsg_param_2, wParam_param_3, lParam_param_4);
	}

	char ParseKeyCode(const char* str) {
		long long llong;
		if (strcmp(str, "") != 0) {
			llong = std::stoll(str, 0, 16);
			if (llong > 0xFF) {
				return 0;
			}
			return (char)llong;
		}
		return 0;
	}

	char ParseModifierMask(long long llong) {
		return ((llong >= 0) && (llong <= 7)) ?
			(char)llong : 0;
	}

	void RegisterShortcut(WPARAM keyCode, char modifierMask, void(*callback)(Shortcut)) {
		if (keyCode == 0) {
			return;
		}

		auto shortcut = new Shortcut{
			keyCode, modifierMask, callback };

		if ((keyCode) && (callback)) {
			Profiles.push_back(*shortcut);
		}
	}

	void RegisterShortcut(const char* keyCode, long long modifierMask, void(*callback)(Shortcut)) {
		RegisterShortcut(ParseKeyCode(keyCode), ParseModifierMask(modifierMask), callback);
	}

	void StartProcess(const char* path, const char* args) {
		// additional information
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		// set the size of the structures
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		char startDir[MAX_PATH];
		strcpy(startDir, path);
		PathRemoveFileSpecA(startDir);

		char cmdLine[MAX_PATH];
		if (args) {
			// combine the path and args
			strcpy(cmdLine, "\"");
			strcat(cmdLine, path);
			strcat(cmdLine, "\" ");
			strcat(cmdLine, args);
			path = NULL;
		}

		// start the program up
		CreateProcessA(path,// the path
			args ? cmdLine : NULL, // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			startDir,       // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
		);
		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	void ApplicationAction(Shortcut shortcut) {
		StartProcess(shortcut.Path, shortcut.Args);
	}

	const char* TrainerNoSoftResetArgs = "conf --no-soft-reset";
	void TrainerAction(Shortcut shortcut) {
		if (SoftReset::IsEnabled()) {
			shortcut.Args = TrainerNoSoftResetArgs;
		}
		ApplicationAction(shortcut);
	}



	


	void Run(CSimpleIniA& ini) {
		oFUN_00401350 = (tFUN_00401350)mem::TrampHook32((BYTE*)0x401350, (BYTE*)hkFUN_00401350, 7);

		const char* category = "Actions";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		const char* profileFormat = "Actions.Application.%d";

		size_t i = 0;
		long long llong;
		const char* str;

		

		for (i = 1; i <= 99; i++) {
			char profile[20];
			snprintf(profile, 20, profileFormat, i);
			if (ini.GetSectionSize(profile) == -1) {
				break;
			}
		}

		//size_t totalProfiles = i - 1;
		for (i = 0; i <= 99; i++) {
			Shortcut shortcut;
			char profileCategory[32];
			if (i == 0) {
				strcpy(profileCategory, "Actions.Trainer");
				shortcut.callback = &TrainerAction;
			}
			else {
				snprintf(profileCategory, 31, profileFormat, i);
				shortcut.callback = &ApplicationAction;

				str = ini.GetValue(profileCategory, "Arguments", "");
				if (strcmp(str, "") != 0) {
					char* newStr = (char*)malloc(strlen(str) + 1);
					strcpy(newStr, str);
					shortcut.Args = newStr;
				}
			}

			if (!ini.GetBoolValue(profileCategory, "Enabled", false)) {
				continue;
			}

			shortcut.keyCode = ParseKeyCode(ini.GetValue(profileCategory, "KeyCode", ""));
			/*
			str = ;
			if (strcmp(str, "") != 0) {
				llong = std::stoll(str, 0, 16);
				if (llong > 0xFF) {
					continue;
				}
				shortcut.keyCode = (char)llong;
			}*/

			shortcut.modifierMask = ParseModifierMask(ini.GetLongValue(profileCategory, "ModifierKeys", 0));
			/*
			shortcut.modifierMask = 0;
			if ((llong = ini.GetLongValue(profileCategory, "ModifierKeys", LONG_MIN)) != LONG_MIN) {
				if ((llong >= 0) && (llong <= 7)) {
					shortcut.modifierMask = (char)llong;
				}
			}
			*/
				
			str = ini.GetValue(profileCategory, "Path", "");
			if (strcmp(str, "") == 0) {
				continue;
			}
			char* newStr = (char*)malloc(strlen(str) + 1);
			strcpy(newStr, str);
			shortcut.Path = newStr;
			
			Profiles.push_back(shortcut);
		}




	}

}