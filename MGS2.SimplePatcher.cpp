#include "MGS2.framework.h"

namespace MGS2 {

	SimplePatcher::SimplePatcher(CSimpleIniA& ini, const char* category) : Ini(ini), Category(category) {
		if (!ini.IsEmpty()) {
			Enabled = ini.GetBoolValue(Category, "Enabled", false);
		}
	}

	void SimplePatcher::TogglePatches() {
		Patches.Toggle();
		Active = Patches.Patched;
	}

	void SimplePatcher::ToggleAction(Actions::Action action) {
		TogglePatches();
		Log::DisplayToggleMessage(FriendlyName, Active);
	}

	void SimplePatcher::Run() {
		if (!Enabled) {
			return;
		}

		int strLength = strlen(Category) + 8;
		
		if (Toggleable) {
			char* toggleCategory;
			if (asprintf(&toggleCategory, "%s.Toggle", Category) > -1) {
				cb::Callback1<void, Actions::Action> callback(this, &SimplePatcher::ToggleAction);
				Actions::RegisterAction_SimplePatcher(Ini, toggleCategory, callback);
			}
		}

		if (NewGameInfoMode != 0) {
			cb::Callback0<bool> ngiCallback(this, &SimplePatcher::NewGameInfoCallback);
			if (NewGameInfoMode == 1) {
				NewGameInfo::AddWarning(FriendlyName, ngiCallback);
			}
			else {
				NewGameInfo::AddNotice(FriendlyName, ngiCallback);
			}
		}

		Active = Ini.GetBoolValue(Category, "Active", true);
		if (Active) {
			TogglePatches();
		}
	}

}
