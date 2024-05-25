#include "MGS2.framework.h"
#include <random>

namespace MGS2::VRRando {
	const char* const Category = "VRRando";

	const char* const CategoryName = "VR Randomiser";
	bool Active = true;
	bool GroupByCharacter = false;

	int const TOTAL_CHARACTERS = 7;
	unsigned int const CharacterCodes[TOTAL_CHARACTERS]{
		0x9B1527, 0x1C1541, 0x221542, 0x28DCE6, 0x61AD7E, 0x3D04C0, 0x57403E
	};
	short const MissionCounts[TOTAL_CHARACTERS]{
		103, 26, 1, 71, 87, 87, 88
	};

	int const TOTAL_MISSIONS = 464;
	int const MISSION_DATA_SIZE = 5;
	char OriginalData[TOTAL_MISSIONS][MISSION_DATA_SIZE]{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x01, 0x07, 0x00, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x01, 0x09, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x00, 0x00, 0x01, 0x03, 0x03, 0x00, 0x00, 0x01, 0x03, 0x04, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x00, 0x01, 0x04, 0x02, 0x00, 0x00, 0x01, 0x04, 0x03, 0x00, 0x00, 0x01, 0x04, 0x04, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x01, 0x05, 0x01, 0x00, 0x00, 0x01, 0x05, 0x02, 0x00, 0x00, 0x01, 0x05, 0x03, 0x00, 0x00, 0x01, 0x05, 0x04, 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x01, 0x06, 0x01, 0x00, 0x00, 0x01, 0x06, 0x02, 0x00, 0x00, 0x01, 0x06, 0x03, 0x00, 0x00, 0x01, 0x06, 0x04, 0x00, 0x00, 0x01, 0x07, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01, 0x00, 0x00, 0x01, 0x07, 0x02, 0x00, 0x00, 0x01, 0x07, 0x03, 0x00, 0x00, 0x01, 0x07, 0x04, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x01, 0x08, 0x01, 0x00, 0x00, 0x01, 0x08, 0x02, 0x00, 0x00, 0x01, 0x08, 0x03, 0x00, 0x00, 0x01, 0x08, 0x04, 0x00, 0x00, 0x01, 0x09, 0x00, 0x00, 0x00, 0x01, 0x09, 0x01, 0x00, 0x00, 0x01, 0x09, 0x02, 0x00, 0x00, 0x01, 0x09, 0x03, 0x00, 0x00, 0x01, 0x09, 0x04, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x03, 0x00, 0x06, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x01, 0x00, 0x01, 0x05, 0x00, 0x02, 0x00, 0x01, 0x05, 0x00, 0x03, 0x00, 0x01, 0x05, 0x00, 0x04, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x01, 0x06, 0x00, 0x01, 0x00, 0x01, 0x06, 0x00, 0x02, 0x00, 0x01, 0x06, 0x00, 0x03, 0x00, 0x01, 0x06, 0x00, 0x04, 0x00, 0x01, 0x06, 0x00, 0x05, 0x00, 0x01, 0x06, 0x00, 0x06, 0x00, 0x01, 0x06, 0x00, 0x07, 0x00, 0x01, 0x06, 0x00, 0x08, 0x00, 0x01, 0x06, 0x00, 0x09, 0x00, 0x01, 0x07, 0x00, 0x00, 0x00, 0x01, 0x07, 0x00, 0x01, 0x00, 0x01, 0x07, 0x00, 0x02, 0x00, 0x01, 0x07, 0x00, 0x03, 0x00, 0x01, 0x07, 0x00, 0x04, 0x00, 0x01, 0x07, 0x00, 0x05, 0x00, 0x01, 0x07, 0x00, 0x06, 0x00, 0x01, 0x07, 0x00, 0x07, 0x00, 0x01, 0x07, 0x00, 0x08, 0x00, 0x01, 0x07, 0x00, 0x09, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x01, 0x08, 0x00, 0x01, 0x00, 0x01, 0x08, 0x00, 0x02, 0x00, 0x01, 0x08, 0x00, 0x03, 0x00, 0x01, 0x08, 0x00, 0x04, 0x00, 0x01, 0x08, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00, 0x00, 0x00, 0x07, 0x01, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x09, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x00, 0x01, 0x05, 0x01, 0x00, 0x00, 0x01, 0x06, 0x01, 0x00, 0x00, 0x01, 0x07, 0x01, 0x00, 0x00, 0x01, 0x08, 0x01, 0x00, 0x00, 0x01, 0x09, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x00, 0x01, 0x09, 0x01, 0x01, 0x00, 0x01, 0x09, 0x02, 0x01, 0x00, 0x01, 0x09, 0x03, 0x01, 0x00, 0x01, 0x09, 0x04, 0x01, 0x00, 0x03, 0x00, 0x04, 0x02, 0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x04, 0x03, 0x00, 0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x07, 0x03, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00, 0x09, 0x03, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x01, 0x01, 0x03, 0x00, 0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x01, 0x03, 0x03, 0x00, 0x00, 0x01, 0x04, 0x03, 0x00, 0x00, 0x01, 0x05, 0x03, 0x00, 0x00, 0x01, 0x06, 0x03, 0x00, 0x00, 0x01, 0x07, 0x03, 0x00, 0x00, 0x01, 0x08, 0x03, 0x00, 0x00, 0x01, 0x09, 0x03, 0x00, 0x01, 0x02, 0x00, 0x03, 0x00, 0x01, 0x02, 0x01, 0x03, 0x00, 0x01, 0x02, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x03, 0x00, 0x01, 0x02, 0x04, 0x03, 0x00, 0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x03, 0x01, 0x03, 0x00, 0x01, 0x03, 0x02, 0x03, 0x00, 0x01, 0x03, 0x03, 0x03, 0x00, 0x01, 0x03, 0x04, 0x03, 0x00, 0x01, 0x04, 0x00, 0x03, 0x00, 0x01, 0x04, 0x01, 0x03, 0x00, 0x01, 0x04, 0x02, 0x03, 0x00, 0x01, 0x04, 0x03, 0x03, 0x00, 0x01, 0x04, 0x04, 0x03, 0x00, 0x01, 0x05, 0x00, 0x03, 0x00, 0x01, 0x05, 0x01, 0x03, 0x00, 0x01, 0x05, 0x02, 0x03, 0x00, 0x01, 0x05, 0x03, 0x03, 0x00, 0x01, 0x05, 0x04, 0x03, 0x00, 0x01, 0x06, 0x00, 0x03, 0x00, 0x01, 0x06, 0x01, 0x03, 0x00, 0x01, 0x06, 0x02, 0x03, 0x00, 0x01, 0x06, 0x03, 0x03, 0x00, 0x01, 0x06, 0x04, 0x03, 0x00, 0x01, 0x07, 0x00, 0x03, 0x00, 0x01, 0x07, 0x01, 0x03, 0x00, 0x01, 0x07, 0x02, 0x03, 0x00, 0x01, 0x07, 0x03, 0x03, 0x00, 0x01, 0x07, 0x04, 0x03, 0x00, 0x01, 0x08, 0x00, 0x03, 0x00, 0x01, 0x08, 0x01, 0x03, 0x00, 0x01, 0x08, 0x02, 0x03, 0x00, 0x01, 0x08, 0x03, 0x03, 0x00, 0x01, 0x08, 0x04, 0x03, 0x00, 0x01, 0x09, 0x00, 0x03, 0x00, 0x01, 0x09, 0x01, 0x03, 0x00, 0x01, 0x09, 0x02, 0x03, 0x00, 0x01, 0x09, 0x03, 0x03, 0x00, 0x01, 0x09, 0x04, 0x03, 0x00, 0x02, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x03, 0x00, 0x02, 0x00, 0x02, 0x03, 0x00, 0x02, 0x00, 0x03, 0x03, 0x00, 0x02, 0x00, 0x04, 0x03, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x01, 0x03, 0x00, 0x03, 0x00, 0x02, 0x03, 0x00, 0x03, 0x00, 0x03, 0x03, 0x00, 0x03, 0x00, 0x04, 0x03, 0x00, 0x03, 0x00, 0x05, 0x03, 0x00, 0x03, 0x00, 0x06, 0x04, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 0x06, 0x04, 0x00, 0x00, 0x00, 0x07, 0x04, 0x00, 0x00, 0x00, 0x08, 0x04, 0x00, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x01, 0x01, 0x04, 0x00, 0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x01, 0x03, 0x04, 0x00, 0x00, 0x01, 0x04, 0x04, 0x00, 0x00, 0x01, 0x05, 0x04, 0x00, 0x00, 0x01, 0x06, 0x04, 0x00, 0x00, 0x01, 0x07, 0x04, 0x00, 0x00, 0x01, 0x08, 0x04, 0x00, 0x00, 0x01, 0x09, 0x04, 0x00, 0x01, 0x02, 0x00, 0x04, 0x00, 0x01, 0x02, 0x01, 0x04, 0x00, 0x01, 0x02, 0x02, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x04, 0x04, 0x00, 0x01, 0x03, 0x00, 0x04, 0x00, 0x01, 0x03, 0x01, 0x04, 0x00, 0x01, 0x03, 0x02, 0x04, 0x00, 0x01, 0x03, 0x03, 0x04, 0x00, 0x01, 0x03, 0x04, 0x04, 0x00, 0x01, 0x04, 0x00, 0x04, 0x00, 0x01, 0x04, 0x01, 0x04, 0x00, 0x01, 0x04, 0x02, 0x04, 0x00, 0x01, 0x04, 0x03, 0x04, 0x00, 0x01, 0x04, 0x04, 0x04, 0x00, 0x01, 0x05, 0x00, 0x04, 0x00, 0x01, 0x05, 0x01, 0x04, 0x00, 0x01, 0x05, 0x02, 0x04, 0x00, 0x01, 0x05, 0x03, 0x04, 0x00, 0x01, 0x05, 0x04, 0x04, 0x00, 0x01, 0x06, 0x00, 0x04, 0x00, 0x01, 0x06, 0x01, 0x04, 0x00, 0x01, 0x06, 0x02, 0x04, 0x00, 0x01, 0x06, 0x03, 0x04, 0x00, 0x01, 0x06, 0x04, 0x04, 0x00, 0x01, 0x07, 0x00, 0x04, 0x00, 0x01, 0x07, 0x01, 0x04, 0x00, 0x01, 0x07, 0x02, 0x04, 0x00, 0x01, 0x07, 0x03, 0x04, 0x00, 0x01, 0x07, 0x04, 0x04, 0x00, 0x01, 0x08, 0x00, 0x04, 0x00, 0x01, 0x08, 0x01, 0x04, 0x00, 0x01, 0x08, 0x02, 0x04, 0x00, 0x01, 0x08, 0x03, 0x04, 0x00, 0x01, 0x08, 0x04, 0x04, 0x00, 0x01, 0x09, 0x00, 0x04, 0x00, 0x01, 0x09, 0x01, 0x04, 0x00, 0x01, 0x09, 0x02, 0x04, 0x00, 0x01, 0x09, 0x03, 0x04, 0x00, 0x01, 0x09, 0x04, 0x04, 0x00, 0x03, 0x00, 0x03, 0x04, 0x00, 0x03, 0x00, 0x04, 0x04, 0x01, 0x05, 0x00, 0x00, 0x04, 0x01, 0x05, 0x00, 0x01, 0x04, 0x01, 0x05, 0x00, 0x02, 0x04, 0x01, 0x05, 0x00, 0x03, 0x04, 0x01, 0x05, 0x00, 0x04, 0x04, 0x01, 0x06, 0x00, 0x00, 0x04, 0x01, 0x06, 0x00, 0x01, 0x04, 0x01, 0x06, 0x00, 0x02, 0x04, 0x01, 0x06, 0x00, 0x03, 0x04, 0x01, 0x06, 0x00, 0x04, 0x04, 0x01, 0x06, 0x00, 0x05, 0x04, 0x01, 0x06, 0x00, 0x06, 0x04, 0x01, 0x06, 0x00, 0x07, 0x04, 0x01, 0x06, 0x00, 0x08, 0x04, 0x01, 0x06, 0x00, 0x09, 0x04, 0x01, 0x07, 0x00, 0x00, 0x04, 0x01, 0x07, 0x00, 0x01, 0x04, 0x01, 0x07, 0x00, 0x02, 0x04, 0x01, 0x07, 0x00, 0x03, 0x04, 0x01, 0x07, 0x00, 0x04, 0x04, 0x01, 0x07, 0x00, 0x05, 0x04, 0x01, 0x07, 0x00, 0x06, 0x04, 0x01, 0x07, 0x00, 0x07, 0x04, 0x01, 0x07, 0x00, 0x08, 0x04, 0x01, 0x07, 0x00, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x02, 0x05, 0x00, 0x00, 0x00, 0x03, 0x05, 0x00, 0x00, 0x00, 0x04, 0x05, 0x00, 0x00, 0x00, 0x05, 0x05, 0x00, 0x00, 0x00, 0x06, 0x05, 0x00, 0x00, 0x00, 0x07, 0x05, 0x00, 0x00, 0x00, 0x08, 0x05, 0x00, 0x00, 0x00, 0x09, 0x05, 0x00, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x01, 0x01, 0x05, 0x00, 0x00, 0x01, 0x02, 0x05, 0x00, 0x00, 0x01, 0x03, 0x05, 0x00, 0x00, 0x01, 0x04, 0x05, 0x00, 0x00, 0x01, 0x05, 0x05, 0x00, 0x00, 0x01, 0x06, 0x05, 0x00, 0x00, 0x01, 0x07, 0x05, 0x00, 0x00, 0x01, 0x08, 0x05, 0x00, 0x00, 0x01, 0x09, 0x05, 0x00, 0x01, 0x02, 0x00, 0x05, 0x00, 0x01, 0x02, 0x01, 0x05, 0x00, 0x01, 0x02, 0x02, 0x05, 0x00, 0x01, 0x02, 0x03, 0x05, 0x00, 0x01, 0x02, 0x04, 0x05, 0x00, 0x01, 0x03, 0x00, 0x05, 0x00, 0x01, 0x03, 0x01, 0x05, 0x00, 0x01, 0x03, 0x02, 0x05, 0x00, 0x01, 0x03, 0x03, 0x05, 0x00, 0x01, 0x03, 0x04, 0x05, 0x00, 0x01, 0x04, 0x00, 0x05, 0x00, 0x01, 0x04, 0x01, 0x05, 0x00, 0x01, 0x04, 0x02, 0x05, 0x00, 0x01, 0x04, 0x03, 0x05, 0x00, 0x01, 0x04, 0x04, 0x05, 0x00, 0x01, 0x05, 0x00, 0x05, 0x00, 0x01, 0x05, 0x01, 0x05, 0x00, 0x01, 0x05, 0x02, 0x05, 0x00, 0x01, 0x05, 0x03, 0x05, 0x00, 0x01, 0x05, 0x04, 0x05, 0x00, 0x01, 0x06, 0x00, 0x05, 0x00, 0x01, 0x06, 0x01, 0x05, 0x00, 0x01, 0x06, 0x02, 0x05, 0x00, 0x01, 0x06, 0x03, 0x05, 0x00, 0x01, 0x06, 0x04, 0x05, 0x00, 0x01, 0x07, 0x00, 0x05, 0x00, 0x01, 0x07, 0x01, 0x05, 0x00, 0x01, 0x07, 0x02, 0x05, 0x00, 0x01, 0x07, 0x03, 0x05, 0x00, 0x01, 0x07, 0x04, 0x05, 0x00, 0x01, 0x08, 0x00, 0x05, 0x00, 0x01, 0x08, 0x01, 0x05, 0x00, 0x01, 0x08, 0x02, 0x05, 0x00, 0x01, 0x08, 0x03, 0x05, 0x00, 0x01, 0x08, 0x04, 0x05, 0x00, 0x01, 0x09, 0x00, 0x05, 0x00, 0x01, 0x09, 0x01, 0x05, 0x00, 0x01, 0x09, 0x02, 0x05, 0x00, 0x01, 0x09, 0x03, 0x05, 0x00, 0x01, 0x09, 0x04, 0x05, 0x00, 0x03, 0x00, 0x03, 0x05, 0x00, 0x03, 0x00, 0x04, 0x05, 0x01, 0x05, 0x00, 0x00, 0x05, 0x01, 0x05, 0x00, 0x01, 0x05, 0x01, 0x05, 0x00, 0x02, 0x05, 0x01, 0x05, 0x00, 0x03, 0x05, 0x01, 0x05, 0x00, 0x04, 0x05, 0x01, 0x06, 0x00, 0x00, 0x05, 0x01, 0x06, 0x00, 0x01, 0x05, 0x01, 0x06, 0x00, 0x02, 0x05, 0x01, 0x06, 0x00, 0x03, 0x05, 0x01, 0x06, 0x00, 0x04, 0x05, 0x01, 0x06, 0x00, 0x05, 0x05, 0x01, 0x06, 0x00, 0x06, 0x05, 0x01, 0x06, 0x00, 0x07, 0x05, 0x01, 0x06, 0x00, 0x08, 0x05, 0x01, 0x06, 0x00, 0x09, 0x05, 0x01, 0x07, 0x00, 0x00, 0x05, 0x01, 0x07, 0x00, 0x01, 0x05, 0x01, 0x07, 0x00, 0x02, 0x05, 0x01, 0x07, 0x00, 0x03, 0x05, 0x01, 0x07, 0x00, 0x04, 0x05, 0x01, 0x07, 0x00, 0x05, 0x05, 0x01, 0x07, 0x00, 0x06, 0x05, 0x01, 0x07, 0x00, 0x07, 0x05, 0x01, 0x07, 0x00, 0x08, 0x05, 0x01, 0x07, 0x00, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x02, 0x06, 0x00, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x04, 0x06, 0x00, 0x00, 0x00, 0x05, 0x06, 0x00, 0x00, 0x00, 0x06, 0x06, 0x00, 0x00, 0x00, 0x07, 0x06, 0x00, 0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x09, 0x06, 0x00, 0x00, 0x01, 0x00, 0x06, 0x00, 0x00, 0x01, 0x01, 0x06, 0x00, 0x00, 0x01, 0x02, 0x06, 0x00, 0x00, 0x01, 0x03, 0x06, 0x00, 0x00, 0x01, 0x04, 0x06, 0x00, 0x00, 0x01, 0x05, 0x06, 0x00, 0x00, 0x01, 0x06, 0x06, 0x00, 0x00, 0x01, 0x07, 0x06, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x01, 0x09, 0x06, 0x00, 0x01, 0x02, 0x00, 0x06, 0x00, 0x01, 0x02, 0x01, 0x06, 0x00, 0x01, 0x02, 0x02, 0x06, 0x00, 0x01, 0x02, 0x03, 0x06, 0x00, 0x01, 0x02, 0x04, 0x06, 0x00, 0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x03, 0x01, 0x06, 0x00, 0x01, 0x03, 0x02, 0x06, 0x00, 0x01, 0x03, 0x03, 0x06, 0x00, 0x01, 0x03, 0x04, 0x06, 0x00, 0x01, 0x04, 0x00, 0x06, 0x00, 0x01, 0x04, 0x01, 0x06, 0x00, 0x01, 0x04, 0x02, 0x06, 0x00, 0x01, 0x04, 0x03, 0x06, 0x00, 0x01, 0x04, 0x04, 0x06, 0x00, 0x01, 0x05, 0x00, 0x06, 0x00, 0x01, 0x05, 0x01, 0x06, 0x00, 0x01, 0x05, 0x02, 0x06, 0x00, 0x01, 0x05, 0x03, 0x06, 0x00, 0x01, 0x05, 0x04, 0x06, 0x00, 0x01, 0x06, 0x00, 0x06, 0x00, 0x01, 0x06, 0x01, 0x06, 0x00, 0x01, 0x06, 0x02, 0x06, 0x00, 0x01, 0x06, 0x03, 0x06, 0x00, 0x01, 0x06, 0x04, 0x06, 0x00, 0x01, 0x07, 0x00, 0x06, 0x00, 0x01, 0x07, 0x01, 0x06, 0x00, 0x01, 0x07, 0x02, 0x06, 0x00, 0x01, 0x07, 0x03, 0x06, 0x00, 0x01, 0x07, 0x04, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x00, 0x01, 0x08, 0x01, 0x06, 0x00, 0x01, 0x08, 0x02, 0x06, 0x00, 0x01, 0x08, 0x03, 0x06, 0x00, 0x01, 0x08, 0x04, 0x06, 0x00, 0x01, 0x09, 0x00, 0x06, 0x00, 0x01, 0x09, 0x01, 0x06, 0x00, 0x01, 0x09, 0x02, 0x06, 0x00, 0x01, 0x09, 0x03, 0x06, 0x00, 0x01, 0x09, 0x04, 0x06, 0x00, 0x03, 0x00, 0x03, 0x06, 0x00, 0x03, 0x00, 0x04, 0x06, 0x00, 0x03, 0x00, 0x06, 0x06, 0x01, 0x05, 0x00, 0x00, 0x06, 0x01, 0x05, 0x00, 0x01, 0x06, 0x01, 0x05, 0x00, 0x02, 0x06, 0x01, 0x05, 0x00, 0x03, 0x06, 0x01, 0x05, 0x00, 0x04, 0x06, 0x01, 0x06, 0x00, 0x00, 0x06, 0x01, 0x06, 0x00, 0x01, 0x06, 0x01, 0x06, 0x00, 0x02, 0x06, 0x01, 0x06, 0x00, 0x03, 0x06, 0x01, 0x06, 0x00, 0x04, 0x06, 0x01, 0x06, 0x00, 0x05, 0x06, 0x01, 0x06, 0x00, 0x06, 0x06, 0x01, 0x06, 0x00, 0x07, 0x06, 0x01, 0x06, 0x00, 0x08, 0x06, 0x01, 0x06, 0x00, 0x09, 0x06, 0x01, 0x07, 0x00, 0x00, 0x06, 0x01, 0x07, 0x00, 0x01, 0x06, 0x01, 0x07, 0x00, 0x02, 0x06, 0x01, 0x07, 0x00, 0x03, 0x06, 0x01, 0x07, 0x00, 0x04, 0x06, 0x01, 0x07, 0x00, 0x05, 0x06, 0x01, 0x07, 0x00, 0x06, 0x06, 0x01, 0x07, 0x00, 0x07, 0x06, 0x01, 0x07, 0x00, 0x08, 0x06, 0x01, 0x07, 0x00, 0x09
	};
	short RandomIndexes[TOTAL_MISSIONS];


	void Reroll() {
		for (short i = 0; i < TOTAL_MISSIONS; ++i) {
			RandomIndexes[i] = i;
		}

		std::random_device randDevice;
		std::mt19937 rng(randDevice());

		if (GroupByCharacter) {
			short start = 0;
			for (int i = 0; i < TOTAL_CHARACTERS; i++) {
				short end = start + MissionCounts[i];
				std::shuffle(RandomIndexes + start, RandomIndexes + end, rng);
				start = end;
			}
		}
		else {
			std::shuffle(RandomIndexes, RandomIndexes + TOTAL_MISSIONS, rng);
		}
	}


	tFUN_Int_UintUint oFUN_00574220;
	int __cdecl hkFUN_00574220(unsigned int param_1, unsigned int param_2) {
		try_mgs2
			if ((param_2 != 0x8A) || (!Active)) {
				return oFUN_00574220(param_1, param_2);
			}

			void* pData = (void*)(param_1 + 0x4A);

			for (int i = 0; i < TOTAL_MISSIONS; i++) {
				if (memcmp(pData, OriginalData[i], MISSION_DATA_SIZE) != 0) {
					continue;
				}

				int randomIndex = RandomIndexes[i];
				char* replacementData = OriginalData[randomIndex];

				char characterIndex = replacementData[0];
				*(unsigned int*)(*(uintptr_t*)Mem::CurrentData1 + 0x6C) = CharacterCodes[characterIndex];

				memcpy(pData, replacementData, MISSION_DATA_SIZE);
				break;
			}
		catch_mgs2(Category, "574220");

		return oFUN_00574220(param_1, param_2);
	}


	void ToggleAction(Actions::Action action) {
		Active = !Active;
		Log::DisplayToggleMessage(CategoryName, Active);
	}

	void RerollAction(Actions::Action action) {
		if (!Active) {
			ToggleAction(action);
		}
		Reroll();
		Log::DisplayText("Rerolling VR Randomiser");
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		GroupByCharacter = ini.GetBoolValue(Category, "GroupByCharacter", GroupByCharacter);
		Active = ini.GetBoolValue(Category, "Active", Active);

		Reroll();

		oFUN_00574220 = (tFUN_Int_UintUint)mem::TrampHook32((BYTE*)0x574220, (BYTE*)hkFUN_00574220, 7);
		Actions::RegisterAction(ini, "VRRando.Toggle", &ToggleAction);
		Actions::RegisterAction(ini, "VRRando.Reroll", &RerollAction);
	}

}
