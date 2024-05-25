template<typename T>
T ParseValueMap(std::string input, std::map<std::string, T>& inputMap, T defaultValue) {
	//char* lcInput = _strlwr(const_cast<char*>(input));
	std::string lcInput = input;
	transform(lcInput.begin(), lcInput.end(), lcInput.begin(), ::tolower);
	auto it = inputMap.find(lcInput);
	return (it == inputMap.end()) ? defaultValue : it->second;
}

template<typename T>
T ParseValueMap(const char* input, std::map<std::string, T>& inputMap, T defaultValue) {
	std::string strInput = input;
	return ParseValueMap(strInput, inputMap, defaultValue);
}

template<typename T>
T ParseValueMap(CSimpleIniA& ini, const char* category, const char* key, std::map<std::string, T>& inputMap, T defaultValue) {
	const char* input = ini.GetValue(category, key, "");
	return (strcmp(input, "")) ? ParseValueMap(input, inputMap, defaultValue) : defaultValue;
}


template<typename T>
T ParseInteger(CSimpleIniA& ini, const char* category, const char* key, T defaultValue, T minValue, T maxValue, bool useDefaultValue = false) {
	long long input = ini.GetLongValue(category, key, (long)defaultValue);
	if (input < minValue) {
		input = useDefaultValue ? defaultValue : minValue;
	}
	else if (input > maxValue) {
		input = useDefaultValue ? defaultValue : maxValue;
	}
	return (T)input;
}
