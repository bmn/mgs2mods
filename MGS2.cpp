#pragma once

#include "MGS2.framework.h"
//#include <Windows.h>
#include "SimpleIni.h"
//#include <iostream>
#include <vector>
#include <thread>



namespace MGS2 {
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

	void TextDraw(int x, int y, int align, int rgb, const char* content) {
		TextInit();
		TextSetColor(rgb);
		TextSetPosition(x, y, align);
		TextDraw(content);
	}

	void TextDraw(int x, int y, int align, int rgb, const char* format, const char* content) {
		TextInit();
		TextSetColor(rgb);
		TextSetPosition(x, y, align);
		TextDraw(format, content);
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
		OverrideIni(category, ini);

		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		// change capabilities from 5 (foreground) to 9 (background)
		PatchMemory((void*)0x8D06A7, "\x09");
	}

	void RunNoQuitPrompt(CSimpleIniA& ini) {
		const char* category = "NoQuitPrompt";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		// nop the messagebox call...
		PatchMemory((void*)0x878E96, "\x90\x90\x90\x90\x90");
		// and change the conditional jz to the quit procedure to an unconditional jmp
		PatchMemory((void*)0x878E9F, "\xEB");
	}

	void RunPS2Controls(CSimpleIniA& ini) {
		const char* category = "PS2Controls";
		
		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		const char* nullb = "\x00";
		const char* circle = "\x20";
		const char* cross = "\x40";

		PatchMemory((void*)0x407F9A, cross);
		PatchMemory((void*)0x407FB3, circle);
		PatchMemory((void*)0x40825B, circle);
		PatchMemory((void*)0x4082C4, cross);
		PatchMemory((void*)0x409583, circle);
		PatchMemory((void*)0x4095C5, cross);
		PatchMemory((void*)0x410D01, cross); // photo computer skip
		PatchMemory((void*)0x4274C3, nullb); // ration
		PatchMemory((void*)0x4275A0, nullb); // medicine
		PatchMemory((void*)0x4275F8, nullb); // bandage
		PatchMemory((void*)0x42765E, nullb); // pentazemin
		PatchMemory((void*)0x4C1E32, "\x30"); // elevator confirm (originally \x50\x80, but start would pause then go?)
		PatchMemory((void*)0x4C1E33, nullb);
		PatchMemory((void*)0x4C1E42, cross); // elevator cancel (originally \x20\x10, select doesn't cause issues)
		PatchMemory((void*)0x581767, circle);
		PatchMemory((void*)0x581776, cross);
		PatchMemory((void*)0x584B81, circle); // snake tales game over continue
		PatchMemory((void*)0x584CCF, circle); // snake tales game over exit
		PatchMemory((void*)0x584EEF, circle); // retirement% confirm exit
		PatchMemory((void*)0x58AEB0, circle); // end game confirm
		PatchMemory((void*)0x58AEF1, cross); // end game cancel
		PatchMemory((void*)0x59DA18, circle); // basic actions 1 confirm
		PatchMemory((void*)0x59DA60, cross); // basic actions 1 cancel
		PatchMemory((void*)0x59E207, circle); // basic actions 2 confirm
		PatchMemory((void*)0x59E21E, cross); // basic actions 2 cancel
		PatchMemory((void*)0x59F102, circle); // basic actions page switch
		PatchMemory((void*)0x59F194, cross); // basic actions last page exit
		PatchMemory((void*)0x5A5BC8, circle); // ??? basic actions video exit
		PatchMemory((void*)0x5A5C10, cross);
		PatchMemory((void*)0x5A89A9, circle); // tanker game over confirm
		PatchMemory((void*)0x747244, cross); // boss survival main menu cancel
		PatchMemory((void*)0x74727D, circle); // boss survival main menu confirm
		PatchMemory((void*)0x747552, cross); // boss survival difficulty cancel
		PatchMemory((void*)0x74758B, circle); // boss survival difficulty confirm
		PatchMemory((void*)0x7477F0, cross); // boss survival options cancel
		PatchMemory((void*)0x747830, circle); // boss survival options confirm
		PatchMemory((void*)0x74784A, circle); // boss survival options exit confirm
		PatchMemory((void*)0x74E397, circle); // missions save reminder confirm
		PatchMemory((void*)0x752A99, cross); // options cancel
		PatchMemory((void*)0x752AC9, circle); // options confirm
		PatchMemory((void*)0x752CB7, circle); // options open brightness adjustment
		PatchMemory((void*)0x752FFF, circle); // options exit confirm
		PatchMemory((void*)0x7547D9, cross); // special cancel
		PatchMemory((void*)0x754805, circle); // special confirm
		PatchMemory((void*)0x756E7F, cross);
		PatchMemory((void*)0x756EB1, circle);
		PatchMemory((void*)0x757356, cross); // difficulty cancel
		PatchMemory((void*)0x7573A5, circle); // difficulty confirm 
		PatchMemory((void*)0x7577BA, cross); // radar cancel
		PatchMemory((void*)0x7577EE, circle); // radar confirm
		PatchMemory((void*)0x757C0A, cross); // goid cancel
		PatchMemory((void*)0x757C3E, circle); // goid confirm
		PatchMemory((void*)0x7586D7, cross); 
		PatchMemory((void*)0x75870F, circle);
		PatchMemory((void*)0x758B2D, cross);
		PatchMemory((void*)0x758B61, circle);
		PatchMemory((void*)0x758E20, cross);
		PatchMemory((void*)0x758E54, circle);
		PatchMemory((void*)0x75910A, cross);
		PatchMemory((void*)0x75913E, circle);
		PatchMemory((void*)0x75982B, cross); // casting theater cancel
		PatchMemory((void*)0x759841, circle); // casting theater confirm
		PatchMemory((void*)0x759DFD, cross); // casting theater cast change cancel
		PatchMemory((void*)0x759E18, circle); // casting theater cast change confirm
		PatchMemory((void*)0x76B54B, cross); // main menu cancel
		PatchMemory((void*)0x76B582, circle); // main menu confirm
		PatchMemory((void*)0x79014F, circle); // load game choice confirm
		PatchMemory((void*)0x7903BA, circle); // load game confirm
		PatchMemory((void*)0x7904B4, cross); // load game cancel
		PatchMemory((void*)0x7E44EC, circle); 
		PatchMemory((void*)0x7F0E18, circle); // previous story confirm
		PatchMemory((void*)0x7F1093, cross); // previous story cancel
		PatchMemory((void*)0x7F136B, circle); // previous story no forward page change
		PatchMemory((void*)0x7F166D, circle); // previous story no forward page change
		PatchMemory((void*)0x7F1674, circle);
		PatchMemory((void*)0x7F19A3, cross); // previous story page cancel
		PatchMemory((void*)0x7F1A97, "\x2C"); // previous story no page change
		PatchMemory((void*)0x7F316B, circle);
		PatchMemory((void*)0x831BDF, cross); // dog tag viewer cancel
		PatchMemory((void*)0x831E3E, circle); // dog tag viewer mission select confirm
		PatchMemory((void*)0x831EF3, cross); // dog tag viewer mission select cancel
	}

	void RunConsole(CSimpleIniA& ini) {
	}


	void RunUnlockRadar(CSimpleIniA& ini) {
		const char* category = "UnlockRadar";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}
		PatchMemory((void*)0x841E4B, "\x66\xB8\x00\x00\x90\x90\x90", 7);
		PatchMemory((void*)0x878BEE, "\x90\x90");

		NewGameInfo::AddWarning("Unlock Radar");
	}

	void RunDrebinMode(CSimpleIniA& ini) {
		const char* category = "DrebinMode";
		
		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		const char* patch = "\x66\xB8\x0F\x27";
		PatchMemory((void*)0x87E9CA, patch);
		PatchMemory((void*)0x87E9DA, patch);
		PatchMemory((void*)0x87E9EA, patch);
		PatchMemory((void*)0x87E9FA, patch);

		NewGameInfo::AddWarning("Drebin Mode");
	}

	void RunAffinity(CSimpleIniA& ini) {
		const char* category = "Affinity";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		HANDLE process = GetCurrentProcess();

		long long llong;
		if ((llong = ini.GetLongValue(category, "Affinity", -1)) >= 0) {
			unsigned int threadCount = std::thread::hardware_concurrency();
			long long maxAffinity = (1 << threadCount) - 1;
			llong = min(llong, maxAffinity);
			SetProcessAffinityMask(process, llong);
 		}

		if ((llong = ini.GetLongValue(category, "Priority", -1)) >= 0) {
			DWORD classes[5]{ IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS,
				HIGH_PRIORITY_CLASS }; // REALTIME_PRIORITY_CLASS but no thx
			
			llong = min(llong, 4); // todo check #
			DWORD priority = classes[llong];
			SetPriorityClass(process, priority);
		}

	}

}
