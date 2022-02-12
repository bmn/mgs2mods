#include "MGS2.framework.h"
#include <bitset>

namespace MGS2::InputDisplay {

	bool TurboSpeed = false;
	int InputDisplayFrames = 0;
	int InputDisplayFramesPrevSecond = 0;

	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		try {
			if (strcmp((char*)0x118ADEC, "init") == 0)
				throw "Game not initialised yet";
			
			unsigned int* currentInput = (unsigned int*)0xEDADDC;
			//unsigned int* currentNewInput = (unsigned int*)0xEDADE0; // for menus?
			//unsigned int* currentNewInput = (unsigned int*)0xEDADE4;

			// call the unknown function
			TextInit();

			// set the text colour
			TextSetColor(0xB4B4B4);

			// set the position
			TextSetPosition(10, 0x1B9, 0);
			((void(*)(const char*))0x889B40)("");

			std::bitset<32> bits((unsigned long long) * currentInput);
			std::string bitz = bits.to_string();
			const char* bitString = bitz.data();
			bitString += 14; // remove unused digits

			// draw the text
			((void(*)(int, int, int, int))0x889440)(0, 10, 0x1C9, 0);
			((void(*)(const char*))0x889B40)(bitString);

			
			if (TurboSpeed) {
				if (*(unsigned int*)0xEDADE0 != 0)
					InputDisplayFrames++;

				if ((*(int*)0x0118ADAC % 60) == 0) {
					InputDisplayFramesPrevSecond = InputDisplayFrames;
					InputDisplayFrames = 0;
				}
				((void(*)(int, int, int, int))0x889440)(0, 10, 0x1B9, 0);
				((void(*)(const char*, int))0x889B40)("%02d /sec", InputDisplayFramesPrevSecond);
			}
		}
		catch (const char* message) {
		}

		oFUN_00878f70();
	}

	void Run(CSimpleIniA& ini) {
		const char* category = "InputDisplay";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		TurboSpeed = ini.GetBoolValue(category, "InputsPerSecond", false);

		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
	}

}