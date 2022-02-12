/********************************************
	Caution% does not work in debug mode,
	due to changed registers
********************************************/

#include "MGS2.framework.h"

namespace MGS2::Caution {

	unsigned char AlertLevel = 3;

	BYTE* oFUN_0042de2f;
	void __cdecl hkFUN_0042de2f() {

		unsigned char alertStatus;

		__asm {
			mov alertStatus, al

			//pushad
			//pushfd
		}

		//unsigned char alertStatus = *(unsigned char*)0x118AEDA;
		char* characterCode = (char*)0x118C374;
		short tankerProgress = *(short*)0x118D93C;
		short plantProgress = *(short*)0x118D912;

		bool nope = false;

		// reached guard rush
		if (strcmp(characterCode, "r_tnk0") == 0) {
			if (tankerProgress >= 31)
				nope = true;
		}

		// in plant
		else if (plantProgress != 0) {
			if (
				(plantProgress <= 21) || // in dock
				(plantProgress == 379) || // asc colon 1
				(plantProgress == 154) || // ames
				(strcmp((char*)0x118ADEC, "w25b") == 0) || // after harrier
				false
				)
				nope = true;
		}

		// todo after harrier


		//setting <= 1 = setting
		//	setting == 3 = enter(or 3 if enter == 0)
		//	setting == 2 = 2 (or 1 if enter == 1)



		if (nope) {
			alertStatus = -1;
		}
		else {
			if ((AlertLevel == 2) && (alertStatus == 1)) {} // keep at 1 if already 1
			else if ((AlertLevel == 3) && (alertStatus != 0)) {} // keep at 1/2/3 if already 1/2/3
			else {
				// set to AlertLevel if...
				// AL is 0 (VE-like behaviour)
				// AL is 1 (Alerts everywhere)
				// AL is 2 and not already 1
				// AL is 3 and not 1 or 2
				alertStatus = AlertLevel;
			}
		}
			
		__asm {
			//popfd
			//popad
			xor eax,eax
			mov al,alertStatus
		}
	}
	void Run(CSimpleIniA& ini) {
		const char* category = "Caution";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		long long llong;
		llong = (int)ini.GetLongValue(category, "AlertLevel", LONG_MIN);
		if ((llong >= 0) && (llong <= 3)) {
			AlertLevel = (unsigned char)llong;
		}

		// ????
		// replace the caution timer restore with flat 60 secs if in forced Caution
		//PatchMemory((void*)0x42B1A6, "\x31\xC0\x90\x90\x90");


		if (AlertLevel == 3) {
			NewGameInfo::AddWarning("Caution");
			// stop the timer during caution
			PatchMemory((void*)0x42CFF9, "\x90\x90\x90\x90\x90\x90");
			// replace the caution timer restore with a flat 60 secs
			PatchMemory((void*)0x42B1A0, "\x50\x66\xB8\x10\x0E\x90\xA3\xC8\x60\xA1\x00\x58", 12);
		}
		else if (AlertLevel != 0) {
			NewGameInfo::AddWarning((AlertLevel == 1) ? "Alert" : "Evasion");
		}

		oFUN_0042de2f = mem::TrampHook32((BYTE*)0x42DE2F, (BYTE*)hkFUN_0042de2f, 5, true, false);
	}

}