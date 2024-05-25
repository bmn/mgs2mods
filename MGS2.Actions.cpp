#include "MGS2.framework.h"
#include <vector>
#include <Shlwapi.h>
#include <algorithm>
#include <filesystem>

namespace MGS2::Actions {
	const char* Category = "Actions";

	unsigned int GamepadInputHold = 0;
	unsigned int GamepadInputPress = 0;

	std::vector<Action> Actions;
	std::vector<ContextMenuItem*> ContextMenuItems;


	struct StartProcessData {
		const char* Path = NULL;
		const char* Args = NULL;
		long long Affinity = Affinity::MaxAffinity;
		DWORD Priority = Affinity::DefaultPriority;
	};


	std::map<const char*, Shortcut> Shortcuts;
	bool ShortcutActive(const char* key) {
		std::map<const char*, Shortcut>::iterator iterator = Shortcuts.find(key);
		if (iterator == Shortcuts.end()) {
			return false;
		}

		Shortcut& shortcut = iterator->second;
		if (shortcut.Active) {
			shortcut.Active = false;
			return true;
		}

		return false;
	}

	bool GamepadShortcutHeld(const char* key) {
		if (!GamepadInputHold) {
			return false;
		}

		std::map<const char*, Shortcut>::iterator iterator = Shortcuts.find(key);
		if (iterator == Shortcuts.end()) {
			return false;
		}

		Shortcut& shortcut = iterator->second;
		for (const PadCombo& gamepadCombo : shortcut.Gamepad) {
			if (
				((GamepadInputHold & gamepadCombo.Input) == gamepadCombo.Input) &&
				((GamepadInputHold & gamepadCombo.Activation) == gamepadCombo.Activation)
			) {
				return true;
			}
		}

		return false;
	}

	void InvokeCallback(Action action) {
		if (action.Callback.IsSet()) {
			action.Callback.Call(action);
		}

		// old one
		if (action.Func.Method) {
			action.Func.Method(action);
		}
	}

	HMENU ContextMenu = CreatePopupMenu();
	void AddToContextMenu(HMENU menu, const char* name) {
		AppendMenuA(ContextMenu, MF_POPUP, (UINT_PTR)menu, name);
	}

	void AddToContextMenu(unsigned int id, const char* name) {
		AppendMenuA(ContextMenu, MF_STRING, id, name);
	}

	bool SortContextMenuItems(ContextMenuItem* a, ContextMenuItem* b) {
		return a->Position < b->Position;
	}

	void AddContextMenuItem(ContextMenuItem* item) {
		if (item == nullptr) {
			return;
		}
		ContextMenuItems.push_back(item);
	}

	void OpenExplorer(const char* targetPath) {
		char path[MAX_PATH];
		GetWindowsDirectoryA(path, MAX_PATH);
		PathAppendA(path, "explorer.exe");
		Actions::StartProcess(path, targetPath);
	}

	tFUN_00401350 oFUN_00401350;
	LRESULT hkFUN_00401350(HWND hw1_hWnd, unsigned int ui2_uMsg, WPARAM wp3_wParam, LPARAM* plp4_lParam) {
		try_mgs2
			const unsigned int KEYDOWN = WM_KEYDOWN | WM_SYSKEYDOWN;
			const unsigned int KEYUP = WM_KEYUP | WM_SYSKEYUP;

			// Handle action shortcuts
			if ((ui2_uMsg == WM_KEYDOWN) || (ui2_uMsg == WM_SYSKEYDOWN)) {
				//if (ui2_uMsg & (KEYDOWN | KEYUP)) {
				char modifiers = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
				modifiers += (GetAsyncKeyState(VK_CONTROL) & 0x8000) ? 2 : 0;
				modifiers += (GetAsyncKeyState(VK_MENU) & 0x8000) ? 4 : 0;

				for (const Action& action : Actions) {
					for (const KeyCombo& keyCombo : action.Input.Keyboard) {
						if (((modifiers ^ keyCombo.Modifiers) == 0) && (wp3_wParam == keyCombo.Key)) {
							if (ui2_uMsg & KEYDOWN) {
								InvokeCallback(action);
								//keyCombo.Held = true;
							}
							else {
								//keyCombo.Held = false;
							}
						}
					}
				}

				for (auto& [key, shortcut] : Shortcuts) {
					// this occurs 2nd so we have to respect an existing true Active
					if (!shortcut.Active) {
						for (const KeyCombo& keyCombo : shortcut.Keyboard) {
							shortcut.Active = (((modifiers ^ keyCombo.Modifiers) == 0) && (wp3_wParam == keyCombo.Key));
						}
					}
				}
			}


			POINT point;
			int id;
			int menuId = 4000;
			switch (ui2_uMsg) {

				case WM_CREATE:

					for (auto& action : Actions) {
						if ((action.Input.ContextMenu) && (action.Input.ContextMenu->Enabled)) {
							ContextMenuItems.push_back(action.Input.ContextMenu);
						}
					}
					for (auto& shortcut : Shortcuts) {
						if ((shortcut.second.ContextMenu) && (shortcut.second.ContextMenu->Enabled)) {
							ContextMenuItems.push_back(shortcut.second.ContextMenu);
						}
					}
					std::sort(ContextMenuItems.begin(), ContextMenuItems.end(), SortContextMenuItems);

					char* credit;
					asprintf(&credit, "MGS2 ASI %s (%s) by bmn", ASI::Version, __DATE__);
					AppendMenuA(ContextMenu, MF_STRING, menuId, credit);
					AppendMenuA(ContextMenu, MF_SEPARATOR, NULL, NULL);

					for (auto& item : ContextMenuItems) {
						item->MenuId = ++menuId;
						if (item->ContextMenu == nullptr) {
							AppendMenuA(ContextMenu, MF_STRING, menuId, item->FriendlyName);
						}
						else {
							AppendMenuA(ContextMenu, MF_POPUP | MF_STRING, (UINT_PTR)item->ContextMenu, item->FriendlyName);
						}
					}

					break;

				case WM_RBUTTONDOWN:

					point.x = LOWORD(plp4_lParam); // figure out where the mouse was when the user clicked
					point.y = HIWORD(plp4_lParam); //

					ClientToScreen(hw1_hWnd, &point); // convert those coordinates into screen coordinates (relative -> absolute)

					ShowCursor(CURSOR_SHOWING);

					// display the popup menu and inform Windows we want to track the right mouse button
					TrackPopupMenu(ContextMenu, TPM_RIGHTBUTTON | TPM_VERPOSANIMATION, point.x, point.y, 0, hw1_hWnd, NULL);

					ShowCursor(CURSOR_SUPPRESSED);
					break;

				case WM_CONTEXTMENU:
					ShowCursor(CURSOR_SHOWING);
					TrackPopupMenu(ContextMenu, TPM_RIGHTBUTTON | TPM_VERPOSANIMATION, 320, 120, 0, hw1_hWnd, NULL);
					ShowCursor(CURSOR_SUPPRESSED);
					break;

				case WM_COMMAND:
					id = LOWORD(wp3_wParam);

					if (id == 4000) {
						OpenExplorer(std::filesystem::current_path().append("scripts").string().c_str());
						break;
					}

					for (auto& action : Actions) {
						if ((action.Input.ContextMenu) && (action.Input.ContextMenu->MenuId == id)) {
							InvokeCallback(action);
							break;
						}
					}

					for (auto& shortcut : Shortcuts) {
						auto& cut = shortcut.second;
						if ((cut.ContextMenu != NULL) && (cut.ContextMenu->MenuId == id)) {
							cut.Active = true;
							break;
						}
					}

					InvalidateRect(hw1_hWnd, NULL, TRUE);
					break;
				case WM_DESTROY:
					DestroyMenu(ContextMenu);
					PostQuitMessage(0);
					break;
			}
		catch_mgs2(Category, "401350")

		return oFUN_00401350(hw1_hWnd, ui2_uMsg, wp3_wParam, plp4_lParam);
	}


	
	bool GamepadShortcutPressed(unsigned short gamepadCombo, unsigned short activation) {
		bool activationValid;
		if (activation == 0) {
			activationValid = (GamepadInputPress & gamepadCombo);
			// at least one button in the shortcut has just been pressed
		}
		else {
			activationValid = ((GamepadInputPress & activation) == activation);
			// the activation button has just been pressed
		}

		return (
			(GamepadInputPress) && // there's some input
			((GamepadInputHold & gamepadCombo) == gamepadCombo) && // all the buttons in the shortcut are held
			activationValid 
			);
	}

	bool GamepadShortcutPressed(PadCombo& combo) {
		return GamepadShortcutPressed(combo.Input, combo.Activation);
	}

	bool GamepadShortcutHeld(unsigned short gamepadCombo) {
		return ((GamepadInputHold & gamepadCombo) == gamepadCombo);
	}

	bool GamepadShortcutHeld(PadCombo& combo) {
		return GamepadShortcutHeld(combo.Input);
	}


	std::vector<Action const*> NonComplexActionsThisFrame;

	tFUN_Void_Int oFUN_008781e0;
	void __cdecl hkFUN_008781e0(int param_1) {
		try_mgs2
			GamepadInputHold = *Mem::PlayerInput;
			GamepadInputPress = *Mem::PlayerNewInput;

			if (GamepadInputPress) {
				int maxComplexity = 0;
				Action selectedAction;
				bool nonComplexThisFrame = false;

				for (const Action& action : Actions) {
					for (const PadCombo& gamepadCombo : action.Input.Gamepad) {
						if (gamepadCombo.IgnoreComplexity) {
							if (GamepadShortcutPressed(gamepadCombo.Input, gamepadCombo.Activation)) {
								nonComplexThisFrame = true;
								NonComplexActionsThisFrame.push_back(&action);
							}
						}
						else if (gamepadCombo.Complexity > maxComplexity) {
							if (GamepadShortcutPressed(gamepadCombo.Input, gamepadCombo.Activation)) {
								maxComplexity = gamepadCombo.Complexity;
								selectedAction = action;
							}
						}
					}
				}
				if (maxComplexity) {
					// only perform the action that has the highest matched complexity
					InvokeCallback(selectedAction);
				}
				if (nonComplexThisFrame) {
					for (auto* action : NonComplexActionsThisFrame) {
						InvokeCallback(*action);
					}
					NonComplexActionsThisFrame.clear();
				}

				for (auto& [key, shortcut] : Shortcuts) {
					shortcut.Active = false;
					for (const PadCombo& gamepadCombo : shortcut.Gamepad) {
						if (!shortcut.Active) {
							shortcut.Active = GamepadShortcutPressed(gamepadCombo.Input, gamepadCombo.Activation);
							break;
						}
					}
				}
			}
		catch_mgs2(Category, "8781E0")

		oFUN_008781e0(param_1);
	}


	PadCombo ParseInputMask(const char* str) {
		long long llong;
		if (strcmp(str, "") != 0) {
			llong = std::stoll(str, 0, 16);
			if (llong <= 0xFFFF) {
				return PadCombo( RemapL3((unsigned int)llong) );
			}
		}
		return PadCombo();
	}


	Shortcut ShortcutFromConfig(CSimpleIniA& ini, const char* key, const char* keyboard = "Keyboard", const char* gamepad = "Gamepad") {
		std::vector<KeyCombo> keyCombos;
		std::vector<PadCombo> gamepadCombos;
		ContextMenuItem* contextMenu = new ContextMenuItem;

		CSimpleIniA::TNamesDepend entries;
		ini.GetAllValues(key, keyboard, entries);
		for (const CSimpleIniA::Entry& entry : entries) {
			KeyCombo keyCombo = ConfigParser::ParseKeyCombo(entry.pItem);
			if (keyCombo.Key) {
				keyCombos.push_back(keyCombo);
			}
		}

		ini.GetAllValues(key, gamepad, entries);
		for (const CSimpleIniA::Entry& entry : entries) {
			//PadCombo gamepadCombo = ParseInputMask(entry.pItem);
			PadCombo gamepadCombo = ConfigParser::ParsePS2GamepadMask(entry.pItem);
			if (gamepadCombo.Input) {
				gamepadCombos.push_back(gamepadCombo);
			}
		}

		const char* str = ini.GetValue(key, "ContextMenu", "");
		if (strcmp(str, "") != 0) {
			if (char* newStr = (char*)calloc(strlen(str) + 1, 1)) {
				strcpy(newStr, str);
				contextMenu->FriendlyName = newStr;
				contextMenu->Enabled = true;
				contextMenu->Position = ConfigParser::ParseInteger(ini, key, "ContextMenuPosition", 0, INT_MIN, INT_MAX, true);
			}
		}

		return Shortcut{ keyCombos, gamepadCombos, contextMenu };
	}



	Shortcut NewSimpleShortcut(KeyCombo keyCombo, PadCombo gamepadCombo) {
		std::vector<KeyCombo> keyCombos{ keyCombo };
		std::vector<PadCombo> gamepadCombos{ gamepadCombo };

		return Shortcut{ keyCombos, gamepadCombos };
	}

	// new
	bool RegisterAction(Shortcut shortcut, void(*callback)(Action), void* data) {
		if ((shortcut.Keyboard.empty()) && (shortcut.Gamepad.empty()) &&
			((shortcut.ContextMenu == NULL) || (!shortcut.ContextMenu->Enabled))) {
			return false;
		}
		if (!callback) {
			return false;
		}

		Callback func{
			callback
		};

		Actions.push_back(Action { shortcut, func, data });
		return true;
	}

	bool RegisterAction(KeyCombo keyCombo, unsigned short gamepadCombo, void(*callback)(Action), void* data = NULL) {
		return RegisterAction(NewSimpleShortcut(keyCombo, gamepadCombo), callback, data);
	}

	bool RegisterAction(CSimpleIniA& ini, const char* key, void(*callback)(Action), void* data) {
		return RegisterAction(ShortcutFromConfig(ini, key), callback, data);
	}

	// old, not used?
	bool RegisterAction_SimplePatcher(Shortcut shortcut, cb::Callback1<void, Action> callback, void* data = NULL) {
		//static int menuId = 4101;

		if (((shortcut.Keyboard.empty()) && (shortcut.Gamepad.empty())) &&
			((shortcut.ContextMenu == NULL) || (!shortcut.ContextMenu->Enabled))) {
			return false;
		}
		if (!callback.IsSet()) {
			return false;
		}

		Action newAction;
		newAction.Input = shortcut;
		newAction.Callback = callback;
		newAction.Data = data;

		Actions.push_back(newAction);

		return true;
	}

	bool RegisterAction_SimplePatcher(KeyCombo keyCombo, unsigned short gamepadCombo, cb::Callback1<void, Action> callback, void* data = NULL) {
		return RegisterAction_SimplePatcher(NewSimpleShortcut(keyCombo, gamepadCombo), callback, data);
	}

	bool RegisterAction_SimplePatcher(CSimpleIniA& ini, const char* key, cb::Callback1<void, Action> callback, void* data) {
		return RegisterAction_SimplePatcher(ShortcutFromConfig(ini, key), callback, data);
	}


	bool RegisterShortcut(const char* key, Shortcut shortcut) {
		if ( ((shortcut.Keyboard.empty()) && (shortcut.Gamepad.empty())) &&
			((shortcut.ContextMenu == NULL) || (!shortcut.ContextMenu->Enabled)) ) {
			return false;
		}

		Shortcuts[key] = shortcut;
		return true;
	}

	bool RegisterShortcut(const char* key, KeyCombo keyCombo, PadCombo gamepadCombo) {
		return RegisterShortcut(key, NewSimpleShortcut(keyCombo, gamepadCombo));
	}

	bool RegisterShortcut(CSimpleIniA& ini, const char* key) {
		return RegisterShortcut(key, ShortcutFromConfig(ini, key));
	}


	bool StartDll(const char* path) {
		return LoadLibraryA(path);
	}

	PROCESS_INFORMATION StartProcess(const char* path, const char* args, DWORD priority, bool closeHandles) {
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
			args ? cmdLine : NULL, // Command
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			priority,       // No creation flags
			NULL,           // Use parent's environment block
			startDir,       // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
		);

		if (closeHandles) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}

		return pi;
	}

	void ApplicationAction(Action shortcut) {
		StartProcessData* data = (StartProcessData*)shortcut.Data;

		std::string ext(PathFindExtensionA(data->Path));
		transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (ext == ".dll") {
			StartDll(data->Path);
		}
		else if (ext == ".exe") {
			StartProcess(data->Path, data->Args, data->Priority);
		}
	}

	const char* const TrainerNoSoftResetArgs = "conf --no-soft-reset";
	void TrainerAction(Action shortcut) {
		StartProcessData data = *(StartProcessData*)shortcut.Data;
		if (SoftReset::IsEnabled) {
			data.Args = TrainerNoSoftResetArgs;
		}
		ApplicationAction(shortcut);
	}


	void RegisterHooks() {
		if ((!Actions.size()) && (!Shortcuts.size())) {
			return;
		}

		oFUN_00401350 = (tFUN_00401350)mem::TrampHook32((BYTE*)0x401350, (BYTE*)hkFUN_00401350, 7);
		oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);
	}

	void Run(CSimpleIniA& ini) {
		// If Actions is disabled but there are shortcuts for other modules, register the hooks anyway
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			RegisterHooks();
			return;
		}

		const char* profileFormat = "Actions.Application.%d";

		size_t i = 0;
		const char* str;

		for (i = 1; i <= 99; i++) {
			char profile[20];
			snprintf(profile, 20, profileFormat, i);
			if (ini.GetSectionSize(profile) == -1) {
				break;
			}
		}

		FilesystemHelper::SetCurrentPath();

		for (i = 0; i <= 99; i++) {
			Action shortcut;
			StartProcessData* data = new StartProcessData;
			char profileCategory[32];
			if (i == 0) {
				strcpy(profileCategory, "Actions.Trainer");
				shortcut.Func = { &TrainerAction };
			}
			else {
				snprintf(profileCategory, 31, profileFormat, i);
				shortcut.Func = { &ApplicationAction };

				str = ini.GetValue(profileCategory, "Arguments", "");
				if (strcmp(str, "") != 0) {
					if (char* newStr = (char*)calloc(strlen(str) + 1, 1)) {
						strcpy(newStr, str);
						data->Args = newStr;
					}
				}
			}

			if (!ini.GetBoolValue(profileCategory, "Enabled", false)) {
				continue;
			}

			shortcut.Input = ShortcutFromConfig(ini, profileCategory);
				
			str = ini.GetValue(profileCategory, "Path", "");
			if (strcmp(str, "") == 0) {
				continue;
			}

			auto path = std::filesystem::path(str);
			if (!std::filesystem::exists(path))
				continue;
			
			if ((!std::filesystem::is_regular_file(path)) && (std::filesystem::is_symlink(path))) {
				path = std::filesystem::read_symlink(path);
				if (!std::filesystem::exists(path))
					continue;
			}

			data->Path = _strdup(std::filesystem::absolute(path).string().c_str());

			data->Affinity = Affinity::ParseAffinity(ini, profileCategory);
			data->Priority = Affinity::ParsePriority(ini, profileCategory);
			
			shortcut.Data = data;
			Actions.push_back(shortcut);
		}

		FilesystemHelper::RevertCurrentPath();

		RegisterHooks();

	}

}
