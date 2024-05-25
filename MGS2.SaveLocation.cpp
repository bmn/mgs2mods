#include "MGS2.framework.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <locale>
#include <filesystem>
#pragma comment(lib, "shlwapi.lib")

namespace MGS2::SaveLocation {
	const char* Category = "SaveLocation";

	struct Profile {
		char* Location = nullptr;
		char DefaultIndex = -1; // not in use
	};

	char MainSavePath[MAX_PATH];

	size_t CurrentLocationIndex = 0;
	size_t TotalSaveLocations = 0;
	size_t LastListedIndex = 0;
	int TimeoutFrames = INT_MAX;
	std::vector<Profile*> SaveProfiles;
	
	HMENU ContextMenu;


	void SetSaveLocation(char* savePath) {
		memset(Mem::SaveGameBasePath, 0, MAX_PATH);
		memcpy(Mem::SaveGameBasePath, savePath, MAX_PATH - 1);
	}

	tFUN_Void oFUN_0076f8cb;
	void __cdecl hkFUN_0076f8cb() {
		// don't let the game overwrite our location
		if (!TotalSaveLocations) {
			oFUN_0076f8cb();
		}
	}

	void SetSaveLocationIndex(size_t index) {
		if (!TotalSaveLocations) {
			return;
		}
		SetSaveLocation(SaveProfiles[index]->Location);
		CurrentLocationIndex = index;

		std::string path = SaveProfiles[index]->Location;
		size_t i = path.rfind('\\', path.length() - 2);
		if (i != std::string::npos) {
			path = path.substr(i + 1, path.length() - i - 2);
		}

		TimeoutFrames = *Mem::RenderedFrames + 48;

		Log::DisplayText("Save Location [" + std::to_string(index + 1) + '/' + std::to_string(TotalSaveLocations) + ']' + '\n' + path);
	}

	void NextSaveLocation() {
		size_t nextIndex = CurrentLocationIndex + 1;
		if (nextIndex >= TotalSaveLocations) {
			nextIndex = 0;
		}
		SetSaveLocationIndex(nextIndex);
	}

	void PreviousSaveLocation() {
		SetSaveLocationIndex(
			(CurrentLocationIndex == 0) ?
			TotalSaveLocations - 1 :
			CurrentLocationIndex - 1);
	}

	void NextSaveLocation(Actions::Action action) {
		NextSaveLocation();
	}

	void PreviousSaveLocation(Actions::Action action) {
		PreviousSaveLocation();
	}


	void OpenExplorer(Actions::Action action) {
		Actions::OpenExplorer((const char*)Mem::SaveGameBasePath);
	}

	void OpenDirectory(Actions::Action action) {
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
						wcstombs((char*)wPath, *wPath, MAX_PATH - 1);

						SetSaveLocation((char*)wPath);

						CurrentLocationIndex = -1;
						if (LastListedIndex == -1) {
							LastListedIndex = -2;
						}

						TimeoutFrames = 0;
					}
					psi->Release();
				}
			}
			pfd->Release();
		}

		ShowCursor(CURSOR_SUPPRESSED);
	}


	void ReloadSaveGameList(uintptr_t param_1) {
		uintptr_t p2 = *(uintptr_t*)(param_1 + 0x5c);

		int val = 1;
		if (val == 7) {
			*(int*)(param_1 + 0x48) = 0;
		}
		((void(*)(int))0x761fc5)(2);
		((void(*)(int))0x761fc5)(4);
		((void(*)(int))0x761f77)(p2);
		((void(*)(int, int))0x761ef1)(p2, val);

		TimeoutFrames = INT_MAX;
	}


	tFUN_007902a5 oFUN_007902a5;
	void __cdecl hkFUN_007902a5(int param_1, int param_2, unsigned int param_3) {

		try_mgs2
			bool doAction = true;
			if (Actions::ShortcutActive("SaveLocation.PreviousLocation")) {
				PreviousSaveLocation();
			}
			else if (Actions::ShortcutActive("SaveLocation.NextLocation")) {
				NextSaveLocation();
			}

			if ((LastListedIndex != CurrentLocationIndex) && (*Mem::RenderedFrames > TimeoutFrames)) {
				ReloadSaveGameList(param_1);
			}
		catch_mgs2(Category, "7902A5");

		oFUN_007902a5(param_1, param_2, param_3);
	}

	tFUN_Void_Int oFUN_00761e37;
	void __cdecl hkFUN_00761e37(int param_1) {
		LastListedIndex = CurrentLocationIndex;
		oFUN_00761e37(param_1);
	}


	void AddLocationToContextMenu(const char* name) {
		AppendMenuA(ContextMenu, MF_STRING, 5000 + TotalSaveLocations, name);
	}
	

	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		const char* contextMenuText = ini.GetValue(Category, "ContextMenu", "");
		bool useContextMenu = (strcmp(contextMenuText, "") != 0);

		if (useContextMenu) {
			ContextMenu = CreatePopupMenu();
		}


		const char* str = ini.GetValue(Category, "BasePath", "");
		FilesystemHelper::SetCurrentPath();

		char* basePath = nullptr;
		auto dirEntry = std::filesystem::directory_entry(str);
		if (dirEntry.exists() && dirEntry.is_directory()) {
			basePath = _strdup(std::filesystem::absolute(dirEntry.path()).string().c_str());
		}
		

		std::list<CSimpleIniA::Entry> paths;
		bool anyPathsFound = ini.GetAllValues(Category, "Path", paths);
		paths.sort(typename CSimpleIniA::Entry::LoadOrder());

		if (anyPathsFound) {
			CSimpleIniA::TNamesDepend::const_iterator it;
			for (it = paths.begin(); it != paths.end(); ++it) {
				char* path = _strdup(it->pItem);
				PathRemoveBackslashA(path);
				PathAddBackslashA(path);

				if (char* fullSavePath = (char*)calloc(MAX_PATH, 1)) {
					if (PathIsDirectoryA(path)) {
						strncpy(fullSavePath, path, MAX_PATH - 2);
					}
					else if (basePath != nullptr) {
						strncpy(fullSavePath, basePath, MAX_PATH - 2);
						PathAppendA(fullSavePath, path);
						if (!PathIsDirectoryA(fullSavePath)) {
							free(fullSavePath);
							fullSavePath = nullptr;
						}
					}

					if (fullSavePath != nullptr) {
						SaveProfiles.push_back(new Profile{ fullSavePath });
						TotalSaveLocations++;
						if (useContextMenu) {
							AddLocationToContextMenu(fullSavePath);
						}
					}
				}
			}
		}
		else if (basePath != nullptr) {

			using it = std::filesystem::directory_iterator;
			
			for (const auto& path : it(basePath)) {
				if (path.is_directory()) {
					std::filesystem::path canonicalPath = std::filesystem::canonical(path);

					char* fullSavePath = new char[MAX_PATH]{ 0 };
					wcstombs(fullSavePath, canonicalPath.c_str(), MAX_PATH - 2);
					PathAddBackslashA(fullSavePath);

					SaveProfiles.push_back(new Profile{ fullSavePath });
					TotalSaveLocations++;
					if (useContextMenu) {
						AddLocationToContextMenu(fullSavePath);
					}
				}
			}
		}

		FilesystemHelper::RevertCurrentPath();
		free(basePath);

		if (TotalSaveLocations) {
			SetSaveLocation(SaveProfiles[0]->Location);
			if (useContextMenu) {
				ContextMenuItem* contextMenuItem = new ContextMenuItem;
				contextMenuItem->Enabled = true;
				contextMenuItem->FriendlyName = contextMenuText;
				contextMenuItem->ContextMenu = ContextMenu;
				Actions::AddContextMenuItem(contextMenuItem);
			}
		}

		Actions::RegisterAction(ini, "SaveLocation.OpenLocationDialog", &OpenDirectory);
		Actions::RegisterAction(ini, "SaveLocation.OpenExplorer", &OpenExplorer);
		Actions::RegisterAction(ini, "SaveLocation.PreviousLocationGlobal", &PreviousSaveLocation);
		Actions::RegisterAction(ini, "SaveLocation.NextLocationGlobal", &NextSaveLocation);
		Actions::RegisterShortcut(ini, "SaveLocation.PreviousLocation");
		Actions::RegisterShortcut(ini, "SaveLocation.NextLocation");

		oFUN_007902a5 = (tFUN_007902a5)mem::TrampHook32((BYTE*)0x7902A5, (BYTE*)hkFUN_007902a5, 7);
		oFUN_0076f8cb = (tFUN_Void)mem::TrampHook32((BYTE*)0x76F8CB, (BYTE*)hkFUN_0076f8cb, 9);
		oFUN_00761e37 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x761E37, (BYTE*)hkFUN_00761e37, 5);
	}

}
