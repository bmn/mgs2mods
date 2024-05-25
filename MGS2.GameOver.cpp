#include "MGS2.framework.h"

namespace MGS2::GameOver {
	const char* Category = "GameOver";

	bool Active = true;
	bool TriggerOnActivate = true;
	bool TriggerFromAlert = true;
	
	
	// Main SetCaution function
	tFUN_Void_Int oFUN_0042b1a0;
	void __cdecl hkFUN_0042b1a0(int param_1) {
		oFUN_0042b1a0(param_1);

		try_mgs2
			if (!Active) return;
			if (!TriggerOnActivate) return;
			if (*Mem::AreaTimeFrames == 0) return; // allow any held-over Cautions

			MGS2::TriggerGameOver();
		catch_mgs2(Category, "42B1A0");
	}

	// Add Caution coming out of Alert
	tFUN_Void_Int oFUN_0042cc50;
	void __cdecl hkFUN_0042cc50(int param_1) {
		bool hookCalled = false;

		try_mgs2
			short* pAlertState = (short*)(*(uintptr_t*)Mem::CurrentData0 + 0x11a);
			short originalAlertState = *pAlertState;

			hookCalled = true;
			oFUN_0042cc50(param_1);

			if (!Active) return;
			if (!TriggerFromAlert) return;
			if ((originalAlertState != AlertMode::Evasion) || (*pAlertState != AlertMode::Caution)) return;

			MGS2::TriggerGameOver(90);
			return;
		
		catch_mgs2(Category, "42CC50");

		// fallback
		if (!hookCalled) {
			oFUN_0042cc50(param_1);
		}
	}

	// Add Evasion timer to the radar display
	tFUN_Void_Int oFUN_008420c0;
	void __cdecl hkFUN_008420c0(int param_1) {
		oFUN_008420c0(param_1);

		try_mgs2
			if (!Active) return;
			if (!TriggerFromAlert) return;
			if ((*(int*)(param_1 + 0x74C)) != 2) return;

			//int evasionLevel = *(int*)0xA160DC; // 100% to 0
			int evasionLevel = 1200 - *(int*)0xA160DC; // 0 to 100%

			uintptr_t pGauge = *(uintptr_t*)(param_1 + 0x718);
			*(float*)(pGauge + 0x84) = (float)(evasionLevel / 10.0);
			*(int*)(pGauge + 0x88) = 0x41333333;
		catch_mgs2(Category, "8420C0");
	}


	void ToggleAction(Actions::Action action) {
		Active = !Active;
		Log::DisplayToggleMessage("Game Over If Caution", Active);
	}

	bool NewGameInfoCallback() {
		return Active;
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false)))
			return;

		NewGameInfo::AddWarning("Game Over If Caution", &NewGameInfoCallback);
		Actions::RegisterAction(ini, "GameOver.Caution.Toggle", &ToggleAction);

		const char* cautionCategory = "GameOver.Caution";

		Active = ini.GetBoolValue(cautionCategory, "Active", Active);
		TriggerOnActivate = ini.GetBoolValue(cautionCategory, "TriggerOnCaution", TriggerOnActivate);
		TriggerFromAlert = ini.GetBoolValue(cautionCategory, "TriggerFromAlert", TriggerFromAlert);
		
		oFUN_0042b1a0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x42B1A0, (BYTE*)hkFUN_0042b1a0, 6);
		oFUN_0042cc50 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x42CC50, (BYTE*)hkFUN_0042cc50, 9);
		oFUN_008420c0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8420C0, (BYTE*)hkFUN_008420c0, 5);
	}

}
