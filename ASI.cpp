namespace ASI {

	unsigned int GetRawVersion(char major, char minor, char version, char build = 0) {
		return build + (version << 8) + (minor << 16) + (major << 24);
	}

	unsigned int GetRawVersionFromChars(char major, char minor, char version, char build = 0) {
		char charMinor = minor - 'a';
		char charBuild = build ? (build - 'a' - 1) : 0;
		return GetRawVersion(major, charMinor, version, charBuild);
	}

	char Version[] = "b3r";
	unsigned int RawVersion = GetRawVersionFromChars(0, 'b', 3, 'r');

}
