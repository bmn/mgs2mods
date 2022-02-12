#include "MGS2.framework.h"

namespace MGS2::Options {

	unsigned short AndMask = 0xFFFF;
	unsigned short OrMask = 0;
	unsigned short MusicVolume = 15;
	unsigned short SEVolume = 15;
	unsigned short Caption = 1;

	char Applications = 0;

	tFUN_Void oFUN_008787a0;
	void __cdecl hkFUN_008787a0() {
		if (Applications < 2) {
			unsigned short* ppCurrentData = *(unsigned short**)0xA01F34;
			ppCurrentData[3] &= AndMask;
			ppCurrentData[3] |= OrMask;

			// TODO sync the sliders
			*(unsigned short*)0x1212E6C = MusicVolume;
			*(unsigned short*)0x122B6D4 = SEVolume;

			if (Caption > 1) {
				ppCurrentData[0xA] = Caption;
			}

			Applications++;
		}

		oFUN_008787a0();
	}

	void SetMasks(unsigned short mask, bool positive) {
		if (positive) {
			OrMask |= mask;
		}
		else {
			AndMask ^= mask;
		}
	}

	void Run(CSimpleIniA& ini) {
		const char* category = "Options";
		
		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		long long llong;

		if (!ini.GetBoolValue(category, "Enabled", true))
			return;


		SetMasks(1, !ini.GetBoolValue(category, "Vibration", true));
		SetMasks(8, !ini.GetBoolValue(category, "Blood", true));
		SetMasks(0x40, ini.GetBoolValue(category, "OwnViewReverse", false));
		SetMasks(0x80, ini.GetBoolValue(category, "ItemWindowLinear", false));
		SetMasks(0x200, ini.GetBoolValue(category, "QuickChangePrevious", true));

		if ((llong = ini.GetLongValue(category, "SEVolume", LONG_MIN)) != LONG_MIN) {
			if ((llong >= 0) && (llong <= 15)) {
				SEVolume = (unsigned short)llong;
			}
		}
			
		if ((llong = ini.GetLongValue(category, "MusicVolume", LONG_MIN)) != LONG_MIN) {
			if ((llong >= 0) && (llong <= 15)) {
				MusicVolume = (unsigned short)llong;
			}
		}

		if ((llong = ini.GetLongValue(category, "Caption", LONG_MIN)) != LONG_MIN) {
			if ((llong >= 0) && (llong <= 5)) {
				Caption = (unsigned short)llong;
			}
		}

		if ((llong = ini.GetLongValue(category, "RadarType", LONG_MIN)) != LONG_MIN) {
			if (llong == 0) { // off
				// 4
				AndMask ^= 0x20;
				OrMask |= 4;
			}
			else if (llong == 1) { // t1
				// 0
				AndMask ^= 0x24;
			}
			else if (llong == 2) { // t2
				// 20
				AndMask ^= 4;
				OrMask = 0x20;
			}
		}

		oFUN_008787a0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x8787A0, (BYTE*)hkFUN_008787a0, 5);
		return;
	}
}