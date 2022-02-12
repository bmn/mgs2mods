#include "MGS2.framework.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <map>
#include <locale>
#include "Hooking.Patterns.h"
#pragma comment(lib, "shlwapi.lib")

namespace MGS2::SaveLocation {

	bool ChangeDirectory = false;
	int TextPosY = 438;
	unsigned int TextColor = 0x64806E;
	static char* AbsoluteSavePath = (char*)0xA20AA0;
	CHAR MainSavePath[MAX_PATH];



	void OpenDirectory(Actions::Shortcut shortcut) {
		ShowCursor(CURSOR_SHOWING);

		IFileDialog* pfd;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
			{
				pfd->SetTitle(L"Select MGS2 Saves Directory");
				pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
			}
			if (SUCCEEDED(pfd->Show(NULL)))
			{
				IShellItem* psi;
				if (SUCCEEDED(pfd->GetResult(&psi)))
				{
					LPWSTR wPath[MAX_PATH * 2];

					if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, wPath)))
					{
						PathAddBackslashW(*wPath);
						wcstombs((char*)wPath, *wPath, MAX_PATH);

						memcpy(MainSavePath, wPath, MAX_PATH);
						memcpy(AbsoluteSavePath, MainSavePath, MAX_PATH);

						ChangeDirectory = true;
					}
					psi->Release();
				}
			}
			pfd->Release();
		}

		ShowCursor(CURSOR_SUPPRESSED);
	}

	/*
	tFUN_00401350 oFUN_00401350;
	LRESULT hkFUN_00401350(HWND hWnd_param_1, unsigned int uMsg_param_2, WPARAM wParam_param_3, LPARAM* lParam_param_4) {
		if (uMsg_param_2 == WM_KEYDOWN) {
			// O
			if (wParam_param_3 == 0x4F) {
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
					OpenDirectory();
				}
			}

			// config? P
			if (wParam_param_3 == 0x50) {
				DoPropertySheet(NULL);
			}
		}

		return oFUN_00401350(hWnd_param_1, uMsg_param_2, wParam_param_3, lParam_param_4);
	}
	*/


	tFUN_007902a5 oFUN_007902a5;
	void __cdecl hkFUN_007902a5(int param_1, int param_2, unsigned int param_3) {
		int* pCurrent = (int*)(param_2 + 0x20);
		int current = *pCurrent;
		int max = *(int*)(param_2 + 0x24);

		// L1/L2
		if ((param_3 & 0x5) != 0) {
			if (current > 9) {
				*pCurrent = 10;
				param_3 = 0x8000;
			}
			/*
			else if (current != 0) {
				*pCurrent = 1;
				param_3 = 0x1000;
			}
			*/
		}
		// R1/R2
		else if ((param_3 & 0xA) != 0) {
			int lastPage = ((max - 1) / 10);
			if ((current / 10) < lastPage) {
				*pCurrent = (lastPage * 10) - 1;
				param_3 = 0x2000;
			}
			/*
			else if (current != max) {
				*pCurrent = max - 1;
				param_3 = 0x4000;
			}
			*/
		}

		//TextDraw(30, TextPosY, 0, TextColor, "< L1");
		//TextDraw(320, TextPosY, 2, TextColor, (char*)AbsoluteSavePath);
		//TextDraw(610, TextPosY, 1, TextColor, "R1 >");

		oFUN_007902a5(param_1, param_2, param_3);
	}


	tFUN_Void oFUN_0076f8cb;
	void __cdecl hkFUN_0076f8cb() {
		if (ChangeDirectory) {
			memcpy(AbsoluteSavePath, MainSavePath, MAX_PATH);
		}
		else {
			oFUN_0076f8cb();
		}
	}




	void Run(CSimpleIniA& ini) {
		const char* category = "SaveLocation";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		ChangeDirectory = ini.GetBoolValue(category, "ChangeDirectory", false);
		if (ChangeDirectory) {

			/*
			std::map<std::string, GUID> baseDirs = {
				{ "desktop", FOLDERID_Desktop },
				{ "documents", FOLDERID_Documents },
				{ "downloads", FOLDERID_Downloads },
				{ "localappdata", FOLDERID_LocalAppData },
				{ "localappdatalow", FOLDERID_LocalAppDataLow },
				{ "profile", FOLDERID_Profile },
				{ "roamingappdata", FOLDERID_RoamingAppData },
				{ "savedgames", FOLDERID_SavedGames },
				{ "skydrive", FOLDERID_SkyDrive }
			};
			*/

			CHAR fullPath[MAX_PATH];

			std::map<std::string, unsigned short> baseDirs = {
				{ "desktop", CSIDL_DESKTOPDIRECTORY },
				{ "documents", CSIDL_MYDOCUMENTS },
				{ "localappdata", CSIDL_LOCAL_APPDATA },
				{ "profile", CSIDL_PROFILE },
				{ "roamingappdata", CSIDL_APPDATA },
			};

			std::string str = ini.GetValue(category, "BaseDirectory", "");
			transform(str.begin(), str.end(), str.begin(), ::tolower);
			const char* base = str.c_str();

			if (strcmp(base, "none") == 0) {
			}
			else if (strcmp(base, "mgs2") == 0) {
				GetModuleFileNameA(nullptr, fullPath, sizeof(*fullPath) / sizeof(fullPath[0]));
				PathRemoveFileSpecA(fullPath);
			}
			else {
				if (strcmp(base, "") == 0) {
					base = "documents";
				}
				if (baseDirs.count(base)) {
					//if (FAILED(SHGetKnownFolderPath(baseDirs[base], KF_FLAG_DEFAULT, NULL, *fullPath))) {
					if (FAILED(SHGetFolderPathA(nullptr, baseDirs[base], nullptr, SHGFP_TYPE_CURRENT, fullPath))) {
						return;
					}
				}
				else {
					return;
				}
			}

			LPSTR path;

			str = ini.GetValue(category, "SavePath", "");
			base = str.c_str();
			if (strcmp(base, "") == 0) {
				path = (LPSTR)"My Games\\METAL GEAR SOLID 2 SUBSTANCE";
			}
			else {
				path = (LPSTR)base;
			}
			PathAppendA(fullPath, path);

			/*
			PathRemoveBackslashA(fullPath);
			if (false && (!PathIsDirectoryA(fullPath))) {
				MessageBoxA(NULL, "Target save directory does not exist, attempting to create it.", fullPath, MB_ICONEXCLAMATION);
				CreateDirectoryA(fullPath, nullptr);
				if (PathIsDirectoryA(fullPath)) {
					MessageBoxA(NULL, "Save directory created successfully.", fullPath, MB_ICONINFORMATION);
				}
				else {
					MessageBoxA(NULL, "Save directory could not be created, reverting.", fullPath, MB_ICONERROR);
					return;
				}
			}
			*/
			PathAddBackslashA(fullPath);

			memcpy(MainSavePath, fullPath, MAX_PATH);
		}

		/*
		WPARAM openKeyCode = ini.GetLongValue(category, "KeyCode", 0x4F); // O
		char openModifierKeys = ini.GetLongValue(category, "ModifierKeys", 2); // Ctrl
		Actions::RegisterShortcut(openKeyCode, openModifierKeys, &OpenDirectory);
		*/
		Actions::RegisterShortcut(
			ini.GetValue(category, "KeyCode", "4F"),
			ini.GetLongValue(category, "ModifierKeys", 2),
			&OpenDirectory
		);

		/*
		auto getSavesDir = hook::pattern::pattern("53 56 57 68 ? ? ? ? BF").count(1);
		if (getSavesDir.size() == 1)
		{
			AbsoluteSavePath = *getSavesDir.get_first<char*>(0x19 + 1);
		}
		else {
			return;
		}
		*/

		oFUN_007902a5 = (tFUN_007902a5)mem::TrampHook32((BYTE*)0x7902A5, (BYTE*)hkFUN_007902a5, 7);
		oFUN_0076f8cb = (tFUN_Void)mem::TrampHook32((BYTE*)0x76F8CB, (BYTE*)hkFUN_0076f8cb, 9);
		//oFUN_00401350 = (tFUN_00401350)mem::TrampHook32((BYTE*)0x401350, (BYTE*)hkFUN_00401350, 7);
	}

}