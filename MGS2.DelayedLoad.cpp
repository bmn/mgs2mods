#include "MGS2.framework.h"
#include <thread>
#include <chrono>

namespace MGS2::DelayedLoad {

	std::chrono::microseconds DelayUsecs;

	tFUN_Void oFUN_00884ca0;
	void __cdecl hkFUN_00884ca0() {
		std::this_thread::sleep_for(DelayUsecs);
		oFUN_00884ca0();
	}

	void Run(CSimpleIniA& ini) {
		const char* category = "DelayedLoad";

		OverrideIni(category, ini);
		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		long long llong;
		llong = ini.GetLongValue(category, "Delay", 1000);
		if (llong < 0) {
			llong = 0;
		}
		else if (llong > INT_MAX) {
			llong = INT_MAX;
		}
		DelayUsecs = std::chrono::microseconds(llong * 1000);

		char* ngMessage = (char*)malloc(32);
		snprintf(ngMessage, 32, "Delayed Loads (%dms)", (int)llong);
		NewGameInfo::AddWarning(ngMessage);

		oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);
	}

}