#include "MGS2.framework.h"
#include <thread>
#include <chrono>

namespace MGS2::DelayedLoad {
	const char* Category = "DelayedLoad";

	bool Active = false;
	char* NewGameWarning = (char*)calloc(32, 1);
	std::chrono::microseconds DelayUsecs;

	tFUN_Void oFUN_00884ca0;
	void __cdecl hkFUN_00884ca0() {
		try_mgs2
			if (Active) {
				std::this_thread::sleep_for(DelayUsecs);
			}
		catch_mgs2(Category, "884CA0");

		oFUN_00884ca0();
	}

	void ToggleAction(Actions::Action action) {
		Active = !Active;
		Log::DisplayToggleMessage("Delayed Load", Active);
	}

	bool NewGameInfoCallback() {
		return Active;
	}

	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		long long msecs = ConfigParser::ParseInteger(ini, Category, "Delay", 1000, 0, INT_MAX);
		DelayUsecs = std::chrono::microseconds(msecs * 1000);

		Active = ini.GetBoolValue(Category, "Active", true);

		snprintf(NewGameWarning, 32, "Delayed Loads (%dms)", (int)msecs);
		NewGameInfo::AddWarning(NewGameWarning, &NewGameInfoCallback);

		Actions::RegisterAction(ini, "DelayedLoad.Toggle", &ToggleAction);

		oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);
	}

}
