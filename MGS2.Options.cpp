#include "MGS2.framework.h"

namespace MGS2::Options {
	const char* Category = "Options";

	unsigned short AndMask = 0xFFFF;
	unsigned short OrMask = 0;
	unsigned short MusicVolume = 15;
	unsigned short SEVolume = 15;
	unsigned short Caption = 1;

	char Applications = 0;

	tFUN_Void oFUN_008787a0;
	void __cdecl hkFUN_008787a0() {
		try_mgs2
			if (Applications < 2) {
				unsigned short* ppCurrentData = *(unsigned short**)Mem::CurrentData0;
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
		catch_mgs2(Category, "8787A0");

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
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		long long llong;

		if (!ini.GetBoolValue(Category, "Enabled", true))
			return;

		SetMasks(1, !ini.GetBoolValue(Category, "Vibration", true));
		SetMasks(8, !ini.GetBoolValue(Category, "Blood", true));
		SetMasks(0x40, ini.GetBoolValue(Category, "OwnViewReverse", false));
		SetMasks(0x80, ini.GetBoolValue(Category, "ItemWindowLinear", false));

		std::map<std::string, char> mapQuickChange{
			{ "previous", 1 }, { "unequip", 0 }
		};
		char chrTmp = ConfigParser::ParseValueMap(ini, Category, "QuickChange", mapQuickChange, (char)-1);
		SetMasks(0x200, (chrTmp == -1) ? ini.GetBoolValue(Category, "QuickChangePrevious", false) : (bool)chrTmp);
		
		SEVolume = ConfigParser::ParseInteger(ini, Category, "SEVolume", SEVolume, (unsigned short)0, (unsigned short)15, true);
		MusicVolume = ConfigParser::ParseInteger(ini, Category, "MusicVolume", MusicVolume, (unsigned short)0, (unsigned short)15, true);

		std::map<std::string, short> mapCaption{
			{ "off", 0 }, { "no", 0 }, { "n", 0 },
			{ "english", 1 }, { "en", 1 },
			{ "french", 2 }, { "francais", 2 }, { "fr", 2 },
			{ "german", 3 }, { "deutsch", 3 }, { "de", 3 },
			{ "italian", 4 }, { "italiano", 4 }, { "it", 4 },
			{ "spanish", 5 }, { "espanol", 5 }, { "es", 5 }
		};
		short sTmp = ConfigParser::ParseValueMap(ini, Category, "Caption", mapCaption, (short)-1);
		Caption = (sTmp == -1) ?
			ConfigParser::ParseInteger(ini, Category, "Caption", Caption, (unsigned short)0, (unsigned short)5, true)
			: (unsigned short)sTmp;

		if ((llong = ini.GetLongValue(Category, "RadarType", LONG_MIN)) != LONG_MIN) {
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
