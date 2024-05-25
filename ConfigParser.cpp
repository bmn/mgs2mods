#include "framework.h"
#include <Shlwapi.h>
#include <stdexcept>
#include <filesystem>
#include <map>


namespace ConfigParser {

	// ParseKeyCombo takes a string like "CA+O" and converts it to a KeyCombo
	KeyCombo ParseKeyCombo(const char* input) {
		KeyCombo output;

		if (strcmp(input, "") == 0) {
			return output;
		}

		char modifiers = 0; // Shift=1/Ctrl=2/Alt=4

		char keyMode = -1; // -1 = undetermined, 0 = key code, 1 = translated
		char i = 0;

		// modifiers
		while (input[i]) {
			char c = (char)input[i++];

			if ((c == 'S') || (c == 's')) {
				modifiers |= 1;
			}
			else if ((c == 'C') || (c == 'c')) {
				modifiers |= 2;
			}
			else if ((c == 'A') || (c == 'a')) {
				modifiers |= 4;
			}
			else if (c == '+') {
				keyMode = 1;
				break;
			}
			else if (c == '#') {
				keyMode = 0;
				break;
			}
		}

		// if no key-prefix (+/#) was found, throw everything away
		if (keyMode == -1) {
			modifiers = 0;
			keyMode = 1;
			i = 0;
		}
		else if (!input[i]) {
			return output;
		}
		else {
			output.Modifiers = modifiers;
		}

		// key
		const char* key = input + i;
		if (keyMode == 1) {
			// translated
			int length = strlen(key);
			char c = (char)key[0];

			if ((c >= 'a') && (c <= 'z')) {
				c -= ('a' - 'A');
			}

			if (length == 1) {	
				if (
					((c >= '0') && (c <= '9')) ||
					((c >= 'A') && (c <= 'Z'))
				) {
					output.Key = c;
				}
			}
			else {
				if (c == 'N') {
					// numpad
					c = (char)key[1];
					if ((c >= '0') && (c <= '9')) {
						output.Key = (0x30 + c); // c (0x30) + 0x30 = numpad (0x60)
					}
				}
				else if (c == 'F') {
					// F1-F24
					int num = atoi(key);
					if ((num >= 1) && (num <= 24)) {
						output.Key = (0x6F + num);
					}
				}
			}
		}
		else {
			// hex
			long long llong;
			if (strcmp(input, "") != 0) {
				llong = std::stoll(key, 0, 16);
				if (llong <= 0xFF) {
					output.Key = (char)llong;
				}
			}
		}

		return output;
	}

	KeyCombo ParseKeyCombo(CSimpleIniA& ini, const char* category, const char* key) {
		return ParseKeyCombo(ini.GetValue(category, key));
	}


	unsigned int ParseMask(const char* input, std::map<std::string, unsigned int>* inputMap, bool singleButton) {
		if (strcmp(input, "") == 0) {
			return 0;
		}

		unsigned int output = 0;

		char tmp[16];
		int t = 0;
		bool requiresMap = false;

		for (int i = 0; true; i++) {
			char c = (char)input[i];

			if ((c == '|') || (c == '+') || (c == '\0')) {
				if (!t) {
					continue;
				}
				
				if (requiresMap) {
					if (inputMap) {
						tmp[t] = '\0';
						auto it = (*inputMap).find(tmp);
						if (it != (*inputMap).end()) {
							output |= it->second;
						}
					}
				}
				else {
					int num = atoi(tmp);
					if (num <= 32) {
						output |= (1 << num);
					}
				}

				if ((c == '\0') || (c == '|') || (singleButton)) {
					return output;
				}
				else {
					requiresMap = false;
					t = 0;
					tmp[0] = '\0';
				}
			}
			else if (c != ' ') {
				if (t >= 16) {
					return output;
				}
				if ((c < '0') || (c > '9')) {
					requiresMap = true;
				}
				if ((c >= 'A') && (c <= 'Z')) {
					c += 0x20;
				}
				tmp[t++] = c;
			}
		}

		return output;
	}


	std::map<std::string, unsigned int> PS2GamepadButtons{
		{ "l1", 1 << 0 },
		{ "r1", 1 << 1 },
		{ "l2", 1 << 2 },
		{ "r2", 1 << 3 },
		{ "t", 1 << 4 },
		{ "tri", 1 << 4 },
		{ "triangle", 1 << 4 },
		{ "o", 1 << 5 },
		{ "circle", 1 << 5 },
		{ "x", 1 << 6 },
		{ "cross", 1 << 6 },
		{ "sq", 1 << 7 },
		{ "square", 1 << 7 },
		{ "sel", 1 << 8 },
		{ "select", 1 << 8 },
		{ "l3", 1 << 9 },
		{ "r3", 1 << 10 },
		{ "start", 1 << 11 },
		{ "u", 1 << 12 },
		{ "up", 1 << 12 },
		{ "r", 1 << 13 },
		{ "right", 1 << 13 },
		{ "d", 1 << 14 },
		{ "down", 1 << 14 },
		{ "l", 1 << 15 },
		{ "left", 1 << 15 }
	};


	PadCombo ParsePS2GamepadMask(const char* input) {
		if ((input == NULL) || (strcmp(input, "") == 0)) {
			return PadCombo{};
		}

		try {
			unsigned int stoi = std::stoi(input, 0, 16);
			PadCombo combo{ stoi };
			combo.RemapL3();
			return combo;
		}
		catch (const std::invalid_argument&) {
			PadCombo combo{ ParseMask(input, &PS2GamepadButtons) };

			const char* activation = strstr(input, "|");
			if (activation) {
				combo.Activation = ParseMask(activation + 1, &PS2GamepadButtons, true);
			}

			combo.RemapL3();
			return combo;
		}
		return PadCombo{};
	}

	PadCombo ParsePS2GamepadMask(CSimpleIniA& ini, const char* category, const char* key, const char* fallback) {
		return ParsePS2GamepadMask(ini.GetValue(category, key, fallback));
	}


	int ParseHexColor(const char* input, int defaultOutput) {
		char* cInput = (char*)input;
		if (strcmp(input, "") != 0) {
			if (input[0] == '#') {
				cInput++;
			}

			try {
				unsigned int color = std::stoi(cInput, 0, 16);
				if (color <= 0xFFFFFF) {
					return (int)color;
				}
			}
			catch (...) {
			}
		}
		return defaultOutput;
	}

	std::map<char, int> HorizontalAlignmentMap{
		{'L', 0}, {'l', 0}, {'0', 0}, {'R', 1}, {'r', 1}, {'1', 1}, {'C', 2}, {'c', 2}, {'2', 2}
	};
	int ParseHorizontalAlignment(const char* input, int defaultOutput) {
		if (strcmp(input, "") != 0) {
			auto result = HorizontalAlignmentMap.find(input[0]);
			if (result != HorizontalAlignmentMap.end()) {
				return result->second;
			}
		}
		return defaultOutput;
	}

	char ParseRequiredBool(CSimpleIniA& ini, const char* category, const char* key) {
		bool val1 = ini.GetBoolValue(category, key, true);
		bool val2 = ini.GetBoolValue(category, key, false);
		return (val1 == val2) ? val1 : -1;
	}

	std::vector<std::string> SplitString(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);
		std::string token;
		while (std::getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}

}
