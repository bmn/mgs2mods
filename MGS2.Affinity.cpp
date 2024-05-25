#include "MGS2.framework.h"
#include <thread>

namespace MGS2::Affinity {

	const char* Category = "Affinity";

	unsigned int ThreadCount = std::thread::hardware_concurrency();
	long long MaxAffinity = ((long long)1 << ThreadCount) - 1;

	DWORD const Priorities[5]{ IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, HIGH_PRIORITY_CLASS }; // REALTIME_PRIORITY_CLASS but no thx
	

	long long ParseAffinity(CSimpleIniA& ini, const char* category, const char* key) {
		return ConfigParser::ParseInteger(ini, category, key, MaxAffinity, (long long)1, MaxAffinity, true);
	}

	bool SetAffinity(HANDLE process, long long affinity) {
		return SetProcessAffinityMask(process, static_cast<DWORD_PTR>(affinity));
	}

	DWORD ParsePriority(CSimpleIniA& ini, const char* category, const char* key) {
		int priority = ConfigParser::ParseInteger(ini, category, key, 2, 0, 4, true);
		return Priorities[priority];
	}

	bool SetPriority(HANDLE process, DWORD priority) {
		return SetPriorityClass(process, priority);
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		HANDLE process = GetCurrentProcess();
		SetAffinity(process, ParseAffinity(ini, Category, "Affinity"));
		SetPriority(process, ParsePriority(ini, Category, "Priority"));
	}

}
