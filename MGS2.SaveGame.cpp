#include "MGS2.framework.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <map>
#include <locale>
#include "Hooking.Patterns.h"
#include <fstream>
#pragma comment(lib, "shlwapi.lib")


namespace MGS2::SaveGame {

	bool ShoulderButtons = true;
	bool ShowInfo = true;

	int TextPosY = 116;
	unsigned int TextColor = 0x64806E;
	static char* AbsoluteSavePath = (char*)0xA20AA0;

#define TOTAL_SAVE_GAMES 50
	char SaveGameInfo[TOTAL_SAVE_GAMES][255];


	tFUN_007902a5 oFUN_007902a5;
	void __cdecl hkFUN_007902a5(int param_1, int param_2, unsigned int param_3) {
		

		//TextDraw(30, TextPosY, 0, TextColor, "< L1");
		//TextDraw(320, TextPosY, 2, TextColor, (char*)AbsoluteSavePath);
		//TextDraw(610, TextPosY, 1, TextColor, "R1 >");


		int currentFile = *(int*)(param_2 + 0x20);
		if (currentFile < 50) {
			char* saveInfo = SaveGameInfo[currentFile];
			if (strcmp(saveInfo, "") != 0) {
				TextDraw(604, TextPosY, 1, TextColor, saveInfo);
			}
		}


		oFUN_007902a5(param_1, param_2, param_3);
	}


	tFUN_Void_IntInt oFUN_0076f3d0;
	void __cdecl hkFUN_0076f3d0(int param_1, int param_2) {
		oFUN_0076f3d0(param_1, param_2);

		int counter = 0;

		char infoPath[MAX_PATH];
		strncpy(infoPath, AbsoluteSavePath, MAX_PATH - 13);
		PathAddBackslashA(infoPath);
		PathAppendA(infoPath, (LPSTR)"Game Data.txt");

		// Open our file
		std::ifstream inFile(infoPath, std::ifstream::in);

		// If we can read/write great
		if (inFile.good()) {

			// Read throuthe file and load into array
			while (!inFile.eof() && (counter < TOTAL_SAVE_GAMES)) {
				inFile.getline(SaveGameInfo[counter], 255);
				counter++;
			}
		}

		inFile.close();

	}





	void Run(CSimpleIniA& ini) {
		const char* category = "SaveGame";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		ShoulderButtons = ini.GetBoolValue(category, "ShoulderButtons", true);
		ShowInfo = ini.GetBoolValue(category, "ShowInfo", true);


		oFUN_0076f3d0 = (tFUN_Void_IntInt)mem::TrampHook32((BYTE*)0x76F3D0, (BYTE*)hkFUN_0076f3d0, 9);
		oFUN_007902a5 = (tFUN_007902a5)mem::TrampHook32((BYTE*)0x7902A5, (BYTE*)hkFUN_007902a5, 7);
	}

}