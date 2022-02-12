#include "MGS2.framework.h"

namespace MGS2::TextChange {

	void Run(CSimpleIniA& ini) {
		const char* category = "TextChange";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		const char* str;

		str = ini.GetValue(category, "LifeName", "");
		if ((strlen(str) > 0) && (strlen(str) <= 13)) {
			char catString[14];
			strcpy(catString, str);
			PatchMemory((BYTE*)0x9A84DC, catString, 14);
		}


	}

}