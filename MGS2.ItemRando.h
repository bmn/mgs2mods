#pragma once
#include "MGS2.framework.h"

#ifdef _DEBUG
#include "lib/sqlite/sqlite3.h"
#endif
#include <iostream>
#include <functional>
#include <unordered_set>


namespace MGS2::ItemRando {
	bool Enabled = false;

	// Main modes
	char const MODE_NONE = 0;
	char const MODE_DEFAULT = 1;
	char const MODE_TAGSUPPLY = 2;
	char const MODE_TAGSUPPLY_NORANDO = 3;

	// Fallback modes
	char const FALLBACK_NONE = 0;
	char const FALLBACK_SAMESET = 1;
	char const FALLBACK_ANYSET = 2;

	// item sets (additive - use with bitwise ops)
	char const SET_MANY = 0;
	char const SET_SINGLE = 1;
	char const SET_NOT_EXTREME = 2;

	// item categories
	char const CAT_ITEM = 0;
	char const CAT_WEAPON_AMMO = 1;
	char const CAT_STANDALONE_AMMO = 2;

	// difficulties
	char const DIFF_VERYEASY = 1;
	char const DIFF_EASY = 2;
	char const DIFF_NORMAL = 4;
	char const DIFF_HARD = 8;
	char const DIFF_EXTREME = 16;
	char const DIFF_EUROEXTREME = 32;
	char const DIFF_ALL = 63;

	// card behaviour (not used yet)
	char const CARD_ADD = 0;
	char const CARD_OVERWRITE = 1;

	// spawn
	char const DEBUGSPAWN_NONE = 0;
	char const DEBUGSPAWN_WEAPONS = 1;
	char const DEBUGSPAWN_AMMO = 2;

	// seed
	int const MAX_SEED = 0xFFFFFF;

	bool SaveReport = false;
	std::string ReportDirectory;
	std::string ReportFilename = "ItemRando-Report.csv";


	struct Seed;
	struct Item;
	struct KeyItem;
	struct Position;
	struct Location;
	struct LocationGroup;


	short WeaponAmmo(char index, short current = SHRT_MIN, short max = SHRT_MIN, bool addContinue = false);
	short ToolAmmo(char index, short current = SHRT_MIN, short max = SHRT_MIN, bool addContinue = false);
	short ItemAmmo(Item* item, short current = SHRT_MIN, short max = SHRT_MIN, bool addContinue = false);


	extern Seed DefaultSeed;
	struct Seed {
		// User-controllable
		int UserSeed = 0;
		bool Enabled = true;
		char MainMode = MODE_DEFAULT;
		bool Logic = true;
		bool RandomLoadout = true;
		bool CardInLoadout = false;
		bool DigicamInLoadout = false;
		bool SpecialsInLoadout = false;
		bool ChangeModels = false;
		bool SaveSeed = true;
		char FallbackType = FALLBACK_ANYSET;
		char CardType = CARD_OVERWRITE;

		// Not user-controllable, set by MainMode
		bool RandomiseKeyItems = true;
		bool SpawnRegularItems = true;
		bool SpawnNonDatasetItems = true;
		bool DogTagBeggarMode = false;

		void ParseConfig(CSimpleIniA& ini, const char* category);
		void SetupMainMode(char mainMode = -1);
		unsigned short SettingsHash() const;
		void ParseSettingsHash(unsigned short hash);
		std::string FullSeed();
	};
	Seed DefaultSeed;
	Seed CurrentSeed;

	


	struct Item {
		const char* Name;
		char Id;
		short Ammo = 1;
		char Category = CAT_ITEM; // item
		char Set = SET_MANY; // many
		std::function<void(Item*, int)> InitItem; // [](Item* item, int difficulty) {};
		std::function<void(Location*, int)> InitLocation; // [](Location* location, int difficulty) {};

		unsigned short IC() {
			return (Id + (Category << 8));
		}
	};

	struct KeyItem : Item {
		short IsCardWithLevel = 0;
		Item* AmmoItem = nullptr;
		short MaxProgress = 0; // Usually 0, gets set to match that of a RequiredItemGroup if this item is chosen
		LocationGroup* CurrentLocationGroup = nullptr;
		LocationGroup* OriginalLocationGroup = nullptr;
		short SavedAmmo = SHRT_MIN;
		bool SwappedAmmo = false;
		bool WasCollected = false; // only used when swapped
		//KeyItemGroup* ItemGroup = nullptr;

		void Reset();
		void SetLocationGroup(LocationGroup* locationGroup);
		//void SwapWith(KeyItem* item);
		//void SwapWith(LocationGroup* locationGroup);
		
		bool IsCollected() {
			if (IsCardWithLevel) {
				return (ToolAmmo(Id) >= IsCardWithLevel);
			}
			return (ItemAmmo(this) != ((Category == CAT_ITEM) ? 0 : -1));
		}

		short CurrentAmmo(short newAmmo = SHRT_MIN) {
			return ItemAmmo(this, newAmmo);
		}
	};

	struct RequiredItemGroup {
		std::vector<KeyItem*> Items;
		short MaxProgress = 0;
	};


	struct Position {
		const char* AreaCode = nullptr;
		int PosX = INT_MIN; // INT_MIN = no check
		int PosY = INT_MIN;
		int PosZ = INT_MIN;

		bool SamePosition(int posX, int posY, int posZ) const;
		bool SameLocation(const char* areaCode, int posX, int posY, int posZ) const;
		bool SamePosition(Position const& pos) const;
		bool SameLocation(Position const& pos) const;

		int PosYRounded() const {
			return (((PosY + 50) / 100) * 100);
		}

		// == operator only compares PosX, PosYrounded, PosZ, and all but the last character of the area code
		bool operator==(const Position &rhs) const {
			//bool areaEqual = std::string(AreaCode);
			std::string areaCode(AreaCode);
			return ((PosX == rhs.PosX) && (PosZ == rhs.PosZ) /* && (PosYRounded() == rhs.PosYRounded()) */ && areaCode.compare(0, areaCode.length() - 1, rhs.AreaCode));
		}
	};

	struct Location {
		Item* InitialItem = nullptr;
		char Difficulties = 0;
		Position Position;
		short MinCardLevel = 0;
		short MinProgress = 0;
		short MaxProgress = SHRT_MAX;
		bool IsRespawn = false;
		std::function<void(Location*, int)> InitLocation;
		bool Available = true;

		bool HasDifficulty(char difficulty) {
			char difficultyBit = 1 << ((difficulty / 10) - 1);
			return ((Difficulties & difficultyBit) == difficultyBit);
		}

		// == operator only compares Position, and the initial item's category and id
		bool operator==(const Location& rhs) const {
			if (Position != rhs.Position) return false;
			
			//if (InitialItem == nullptr) return true;
			//return ((InitialItem->Category == rhs.InitialItem->Category) && (InitialItem->Id == rhs.InitialItem->Id));
			return true;
		}
	};

	struct LocationGroup {
		Item* RandomItem = nullptr;
		std::vector<Location*> Locations;
		Item* FallbackItem = nullptr;
		short FallbackAfterProgress = 0;

		short MinProgress() const;
		short MaxProgress() const;
		short MinCard() const;
		bool IsAvailableBy(short progress) const;
		bool IsAvailableAt(short progress) const;
		//void SwapWith(KeyItem* item);
		//void SwapWith(LocationGroup* locationGroup);

		Item* GetInitialItem() const;
		void SetInitialItem(Item* const item);
		Item* CurrentRandomItem() const;

		LocationGroup(Location* location) {
			Locations = std::vector<Location*>({ location });
		}

		LocationGroup(std::vector<Location*> locations) : Locations(locations) {}

		LocationGroup() {}
	};


	// Init functions
	std::function<void(Location*, short)> LocInit_EngineRoom = [](Location* location, short difficulty) {
		location->MinProgress = (difficulty >= 30) ? 15 : 29;
		};
	
	std::function<void(Location*, short)> LocInit_RandomLoadout = [](Location* location, short difficulty) {
		location->Available = CurrentSeed.RandomLoadout;
		};

	std::function<void(Location*, short)> LocInit_RandomLoadoutWithCard = [](Location* location, short difficulty) {
		location->Available = (CurrentSeed.RandomLoadout && CurrentSeed.CardInLoadout);
		};

	std::function<void(Item*, short)> ItemInit_MaxAmmo = [](Item* item, short difficulty) {
		short* maxAmmo = (short*)(*Mem::WeaponData + Mem::WeaponMaxOffset);
		item->Ammo = maxAmmo[item->Id];
		};

	std::function<void(Location*, short)> LocInit_AnyNewGamePlus = [](Location* location, short difficulty) {
		short* continueData = (short*)(*(int*)Mem::ContinueData0);
		location->Available = (continueData[2] >= 1);
		};

	std::function<void(Location*, short)> LocInit_NewGamePlusPlus = [](Location* location, short difficulty) {
		short* continueData = (short*)(*(int*)Mem::ContinueData0);
		location->Available = ((continueData[2] != 0) && ((continueData[2] % 2) == 0));
		};


	// Item definitions
	/*
	Item ITEM_RATION{"Ration", 1, 1, CAT_ITEM, SET_MANY, nullptr, [](Location* location, int difficulty) {
			location->Available = (difficulty < 50);
		} }; */
	Item ITEM_RATION{ "Ration", 1 };
	Item ITEM_MEDICINE{ "Medicine", 3 };
	Item ITEM_BANDAGE{ "Bandage", 4 };
	Item ITEM_PENTAZEMIN{ "Pentazemin", 5, 5 };

	Item ITEM_M9AMMO{ "M9 Ammo", 1, 15, CAT_WEAPON_AMMO };
	Item ITEM_USPAMMO{ "USP Ammo", 2, 15, CAT_WEAPON_AMMO };
	Item ITEM_SOCOMAMMO{ "SOCOM Ammo", 3, 12, CAT_WEAPON_AMMO};
	Item ITEM_PSG1AMMO{ "PSG-1 Ammo", 4, 20, CAT_WEAPON_AMMO };
	Item ITEM_RGB6AMMO{ "RGB6 Ammo", 5, 6, CAT_WEAPON_AMMO };
	Item ITEM_NIKITAAMMO{ "Nikita Ammo", 6, 10, CAT_WEAPON_AMMO };
	Item ITEM_STINGERAMMO{ "Stinger Ammo", 7, 10, CAT_WEAPON_AMMO, SET_MANY };
	Item ITEM_CLAYMORE{ "Claymore", 8, 4, CAT_STANDALONE_AMMO };
	Item ITEM_C4{ "C4",  9, 4, CAT_STANDALONE_AMMO};
	Item ITEM_CHAFFGRENADE{ "Chaff Grenade", 10, 2, CAT_STANDALONE_AMMO };
	Item ITEM_STUNGRENADE{ "Stun Grenade", 11, 2, CAT_STANDALONE_AMMO};
	Item ITEM_AKS74UAMMO{ "AK-74u Ammo", 15, 30, CAT_WEAPON_AMMO };
	Item ITEM_MAGAZINE{ "Magazine", 16, 99, CAT_STANDALONE_AMMO};
	Item ITEM_GRENADE{ "Grenade", 17, 2, CAT_STANDALONE_AMMO };
	Item ITEM_M4AMMO{ "M4 Ammo", 18, 30, CAT_WEAPON_AMMO };
	Item ITEM_PSG1TAMMO{ "PSG-1T Ammo", 19, 5, CAT_WEAPON_AMMO };
	Item ITEM_BOOK{ "Book", 21, 1, CAT_STANDALONE_AMMO };
	Item ITEM_DOGTAG{ "Dog Tag", 33, 1, CAT_STANDALONE_AMMO };

	KeyItem ITEM_SNAKE_M9{ { "M9", 1, ITEM_M9AMMO.Ammo + 1, CAT_STANDALONE_AMMO, SET_SINGLE, ItemInit_MaxAmmo }, 0, &ITEM_M9AMMO };
	KeyItem ITEM_SNAKE_USP{ { "USP", 2, 0, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_USPAMMO };
	
	KeyItem ITEM_RAIDEN_M9{ { "M9", 1, ITEM_M9AMMO.Ammo + 1, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_M9AMMO };
	KeyItem ITEM_RAIDEN_SOCOM{ { "SOCOM", 3, ITEM_SOCOMAMMO.Ammo + 1, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_SOCOMAMMO };
	KeyItem ITEM_RAIDEN_PSG1{ { "PSG-1", 4, ITEM_PSG1AMMO.Ammo + 1, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_PSG1AMMO };
	KeyItem ITEM_RAIDEN_RGB6{ { "RGB6", 5, ITEM_RGB6AMMO.Ammo, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_RGB6AMMO };
	KeyItem ITEM_RAIDEN_NIKITA{ { "Nikita", 6, 20, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_RATION }; // ration??? yeah.
	KeyItem ITEM_RAIDEN_STINGER{ { "Stinger", 7, 20, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_STINGERAMMO };
	KeyItem ITEM_RAIDEN_DMIC{ "Directional Mic", 12, 1, CAT_STANDALONE_AMMO, SET_SINGLE };
	KeyItem ITEM_RAIDEN_HFBLADE{ "HF Blade", 13, 1, CAT_STANDALONE_AMMO, SET_SINGLE};
	KeyItem ITEM_RAIDEN_COOLANT{ "Coolant", 14, 1, CAT_STANDALONE_AMMO, SET_SINGLE };
	KeyItem ITEM_RAIDEN_AKS74U{ { "AKS-74u", 15, ITEM_AKS74UAMMO.Ammo + 1, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_AKS74UAMMO };
	KeyItem ITEM_RAIDEN_M4{ { "M4", 18, ITEM_M4AMMO.Ammo + 1, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_M4AMMO };
	KeyItem ITEM_RAIDEN_PSG1T{ { "PSG-1T", 19, ITEM_PSG1TAMMO.Ammo + 1, CAT_STANDALONE_AMMO, SET_SINGLE }, 0, &ITEM_PSG1TAMMO };
	
	KeyItem ITEM_SNAKE_STEALTH{ "Stealth", 8, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_SNAKE_THERMALGOGGLES{ "Thermal Goggles", 13, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_SNAKE_BOX1{ "Box 1", 16, 25, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_SNAKE_CIGS{ "Cigs", 17, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_SNAKE_CAMERA{ "Camera", 21, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_SNAKE_DIGITALCAMERA{ "Digital Camera", 15, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_SNAKE_WETBOX{ "Wet Box", 24, 3, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_SNAKE_BANDANA{ "Bandana", 32, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_SNAKE_USPSUPPRESSOR{ "USP Suppressor", 35, 1, CAT_ITEM, SET_SINGLE };

	KeyItem ITEM_RAIDEN_BDU{ "BDU", 6, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_BODYARMOR{ "Body Armor", 7, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_STEALTH{ "Stealth", 8, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_MINEDETECTOR{ "Mine Detector", 9, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_SENSORA{ "Sensor A", 10, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_SENSORB{ "Sensor B", 11, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_NVG{ "NVG", 12, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_THERMALGOGGLES{ "Thermal Goggles", 13, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_SCOPE{ "Scope", 14, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_DIGITALCAMERA{ "Digital Camera", 15, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_BOX1{ "Box 1", 16, 25, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_CIGS{ "Cigs", 17, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_CARD{ "Card", 18, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_CARD1{ { "Card 1", 18, 1, CAT_ITEM, SET_SINGLE }, 1 };
	KeyItem ITEM_RAIDEN_CARD2{ { "Card 2", 18, 2, CAT_ITEM, SET_SINGLE }, 2 };
	KeyItem ITEM_RAIDEN_CARD3{ { "Card 3", 18, 3, CAT_ITEM, SET_SINGLE }, 3 };
	KeyItem ITEM_RAIDEN_CARD4{ { "Card 4", 18, 4, CAT_ITEM, SET_SINGLE }, 4, nullptr, 241 };
	KeyItem ITEM_RAIDEN_CARD5{ { "Card 5", 18, 5, CAT_ITEM, SET_SINGLE }, 5 };
	KeyItem ITEM_RAIDEN_SHAVER{ "Shaver", 19, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_PHONE{ "Phone", 20, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_CAMERA{ "Camera", 21, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_BOX2{ "Box 2", 22, 25, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_BOX3{ "Box 3", 23, 25, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_APSENSOR{ "AP Sensor", 25, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_BOX4{ "Box 4", 26, 25, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_BOX5{ "Box 5", 27, 25, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_SOCOMSUPPRESSOR{ { "SOCOM Suppressor", 29, 1, CAT_ITEM, SET_SINGLE }, 0, &ITEM_SOCOMAMMO };
	KeyItem ITEM_RAIDEN_AKSUPPRESSOR{ { "AK Suppressor", 30, 1, CAT_ITEM, SET_SINGLE }, 0, &ITEM_AKS74UAMMO };
	KeyItem ITEM_RAIDEN_MODISC{ "MO Disc", 34, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_INFINITYWIG{ "Infinity Wig", 36, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_BLUEWIG{ "Blue Wig", 37, 1, CAT_ITEM, SET_SINGLE };
	KeyItem ITEM_RAIDEN_ORANGEWIG{ "Orange Wig", 38, 1, CAT_ITEM, SET_SINGLE };



	// Loadouts
	std::unordered_map<MGS2::Stage, std::vector<Item*>> LoadoutData{
		{ Stage::Tanker, { &ITEM_SNAKE_M9, &ITEM_SNAKE_CAMERA, &ITEM_SNAKE_CIGS } }, // also magazine (difficulty-dependent)
		{ Stage::Plant, { &ITEM_RAIDEN_SCOPE } },
		{ Stage::TalesA, {} },
		{ Stage::TalesB, {} },
		{ Stage::TalesC, {} },
		{ Stage::TalesD, {} },
		{ Stage::TalesE, {} },
	};

	// Items that have "ammo"
	std::unordered_set<Item*> ConsumablesData{
		&ITEM_M9AMMO, &ITEM_USPAMMO, &ITEM_SOCOMAMMO, &ITEM_PSG1AMMO, &ITEM_RGB6AMMO,
		&ITEM_NIKITAAMMO, &ITEM_STINGERAMMO, &ITEM_CLAYMORE, &ITEM_C4, &ITEM_CHAFFGRENADE,
		&ITEM_STUNGRENADE, &ITEM_AKS74UAMMO, &ITEM_GRENADE, &ITEM_M4AMMO, &ITEM_PSG1TAMMO,
		&ITEM_BOOK, &ITEM_RATION, &ITEM_MEDICINE, &ITEM_BANDAGE, &ITEM_PENTAZEMIN,
	};

	// Locations
	std::unordered_map<MGS2::Stage, std::vector<Location>> LocationsData{
		{ Stage::Tanker, {
			// Cutscene key items
			{ &ITEM_SNAKE_M9, DIFF_ALL, { "load" }, 0, 9, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_SNAKE_CAMERA, DIFF_ALL, { "load" }, 0, 9, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_SNAKE_CIGS, DIFF_ALL, { "load" }, 0, 9, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_SNAKE_USP, DIFF_ALL, { "cuts" }, 0, 29, SHRT_MAX, false, LocInit_RandomLoadout },
			// Pickups
			{ &ITEM_RATION, 15, { "w00a", -18500, INT_MIN, -17500 }, 0, 14, 30 },
			{ &ITEM_PENTAZEMIN, 63, { "w00a", 18500, INT_MIN, -17500 }, 0, 14, 30 },
			{ &ITEM_CHAFFGRENADE, 63, { "w00a", 10000, INT_MIN, -500 }, 0, 14, 30 },
			{ &ITEM_BANDAGE, 63, { "w00a", 16000, INT_MIN, -17000 }, 0, 14, 30 },
			{ &ITEM_RATION, 1, { "w00a", 5000, INT_MIN, 18750 }, 0, 14, 30 },
			{ &ITEM_BANDAGE, 63, { "w00a", -7750, INT_MIN, 4500 }, 0, 14, 30 },
			{ &ITEM_MEDICINE, 56, { "w00a", 0, INT_MIN, 5500 }, 0, 14, 32 },
			{ &ITEM_M9AMMO, 63, { "w00b", -11500, INT_MIN, -13500 }, 0, 25, 25, true },
			{ &ITEM_M9AMMO, 63, { "w00b", -6600, INT_MIN, -19250 }, 0, 25, 25, true },
			{ &ITEM_RATION, 3, { "w00b", -11250, INT_MIN, -10750 }, 0, 25, 25, true },
			{ &ITEM_RATION, 12, { "w00b", -4550, INT_MIN, -9500 }, 0, 25, 25, true },
			{ &ITEM_SNAKE_WETBOX, 63, { "w00c", 18000, INT_MIN, -15000 }, 0, 29, 30 },
			{ &ITEM_SNAKE_THERMALGOGGLES, 63, { "w00c", 750, INT_MIN, -13000 }, 0, 29, 30 },
			{ &ITEM_RATION, 15, { "w00c", -11500, INT_MIN, -18750 }, 0, 29, 30 },
			{ &ITEM_RATION, 15, { "w01a", -4520, INT_MIN, 3550 }, 0, 15, 30 },
			{ &ITEM_M9AMMO, 63, { "w01a", 4500, INT_MIN, 3550 }, 0, 15, 30 },
			{ &ITEM_M9AMMO, 63, { "w01b", 11000, INT_MIN, -14750 }, 0, 15, 30 },
			{ &ITEM_RATION, 15, { "w01b", 3500, INT_MIN, 2250 }, 0, 15, 30 },
			{ &ITEM_USPAMMO, 63, { "w01b", 1000, INT_MIN, 5500 }, 0, 15, 30 },
			{ &ITEM_RATION, 7, { "w01c", 9500, INT_MIN, -16250 }, 0, 15, 30 },
			{ &ITEM_CHAFFGRENADE, 63, { "w01c", -1500, INT_MIN, -18160 }, 0, 15, 30 },
			{ &ITEM_RATION, 15, { "w01d", -10000, INT_MIN, -18500 }, 0, 15, 30 },
			{ &ITEM_USPAMMO, 63, { "w01d", -9250, INT_MIN, -13000 }, 0, 15, 30 },
			{ &ITEM_M9AMMO, 63, { "w01d", -4750, INT_MIN, -13000 }, 0, 15, 30 },
			{ &ITEM_USPAMMO, 63, { "w01d", -2500, INT_MIN, -11875 }, 0, 15, 30 },
			{ &ITEM_M9AMMO, 63, { "w01d", 7700, INT_MIN, -11850 }, 0, 15, 30 },
			{ &ITEM_SNAKE_BOX1, 63, { "w01d", 10000, INT_MIN, -10000 }, 0, 15, 30 },
			{ &ITEM_RATION, 3, { "w01e", -4500, INT_MIN, -15750 }, 0, 22, 30 },
			{ &ITEM_USPAMMO, 63, { "w01e", 4817, INT_MIN, -18682 }, 0, 22, 30 },
			{ &ITEM_M9AMMO, 63, { "w01f", 9000, INT_MIN, -18500 }, 0, 15, 30 },
			{ &ITEM_RATION, 7, { "w01f", -8600, INT_MIN, -18250 }, 0, 15, 30 },
			{ &ITEM_USPAMMO, 63, { "w01f", -12500, INT_MIN, -12500 }, 0, 15, 30 },
			{ &ITEM_STUNGRENADE, 15, { "w01f", 12500, INT_MIN, -12500 }, 0, 15, 30, false, LocInit_EngineRoom },
			{ &ITEM_M9AMMO, 63, { "w02a", 7000, INT_MIN, -19500 }, 0, 29, 30, false, LocInit_EngineRoom },
			{ &ITEM_RATION, 15, { "w02a", 2250, INT_MIN, -18000 }, 0, 29, 30, false, LocInit_EngineRoom },
			{ &ITEM_USPAMMO, 63, { "w02a", -5500, INT_MIN, -21500 }, 0, 29, 30, false, LocInit_EngineRoom },
			{ &ITEM_GRENADE, 63, { "w02a", -12000, INT_MIN, 1000 }, 0, 29, 30, false, LocInit_EngineRoom },
			{ &ITEM_USPAMMO, 63, { "w02a", -6500, INT_MIN, -10000 }, 0, 29, 30, false, LocInit_EngineRoom },
			{ &ITEM_USPAMMO, 63, { "w02a", -16000, INT_MIN, -22250 }, 0, 29, 30, false, LocInit_EngineRoom },
			{ &ITEM_USPAMMO, 63, { "w02a", 16224, INT_MIN, -22618 }, 0, 29, 30, false, LocInit_EngineRoom },
			{ &ITEM_USPAMMO, 63, { "w02a", -10000, INT_MIN, -22500 }, 0, 29, 30, true, LocInit_EngineRoom },
			{ &ITEM_RATION, 15, { "w03a", -12250, INT_MIN, -42000 }, 0, 30, 30 },
			{ &ITEM_USPAMMO, 63, { "w03a", -14000, INT_MIN, -68500 }, 0, 30, 30 },
			{ &ITEM_USPAMMO, 63, { "w03a", -4000, INT_MIN, -128000 }, 0, 30, 30 },
			{ &ITEM_USPAMMO, 63, { "w03a", -17500, INT_MIN, -125750 }, 0, 30, 30 },
			{ &ITEM_USPAMMO, 63, { "w03a", 10500, INT_MIN, -126500 }, 0, 30, 30 },
			{ &ITEM_USPAMMO, 63, { "w03b", 15750, INT_MIN, -128500 }, 0, 30, 30 },
			{ &ITEM_USPAMMO, 63, { "w03b", 12750, INT_MIN, -100250 }, 0, 30, 30 },
			{ &ITEM_RATION, 15, { "w03b", 13000, INT_MIN, -97250 }, 0, 30, 30 },
			{ &ITEM_M9AMMO, 63, { "w03b", 12300, INT_MIN, -128750 }, 0, 30, 30 },
			{ &ITEM_USPAMMO, 63, { "w03b", 17250, INT_MIN, -87000 }, 0, 32, 32, true },
			{ &ITEM_USPAMMO, 63, { "w03b", 17250, INT_MIN, -86000 }, 0, 32, 32, true },
			{ &ITEM_RATION, 31, { "w03b", 16250, INT_MIN, -87000 }, 0, 32, 32, true },
			{ &ITEM_M9AMMO, 63, { "w04a", -11000, INT_MIN, -500 }, 0, 42, 43 },
			{ &ITEM_M9AMMO, 63, { "w04a", 7000, INT_MIN, 1750 }, 0, 42, 43 },
			{ &ITEM_RATION, 15, { "w04a", 10000, INT_MIN, 3250 }, 0, 42, 43 },
			{ &ITEM_SNAKE_THERMALGOGGLES, 63, { "w04a", -10500, INT_MIN, -3500 }, 0, 42, 43 },
			{ &ITEM_RATION, 7, { "w04a", 11000, INT_MIN, -23500 }, 0, 42, 43, true },
			{ &ITEM_RATION, 7, { "w04a", -11000, INT_MIN, -23500 }, 0, 42, 43 },
		} },
		{ Stage::Plant, {
			// Key items
			{ &ITEM_RAIDEN_SCOPE, DIFF_ALL, { "load" }, 0, 0, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_RAIDEN_SOCOM, DIFF_ALL, { "cuts" }, 0, 58, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_RAIDEN_CIGS, DIFF_ALL, { "cuts" }, 0, 58, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_RAIDEN_COOLANT, DIFF_ALL, { "cuts" }, 0, 92, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_RAIDEN_SENSORA, DIFF_ALL, { "cuts" }, 0, 92, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_RAIDEN_CARD1, DIFF_ALL, { "cuts" }, 0, 92, SHRT_MAX, false, LocInit_RandomLoadoutWithCard },
			{ &ITEM_RAIDEN_BDU, DIFF_ALL, { "cuts" }, 1, 148, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_RAIDEN_PHONE, DIFF_ALL, { "cuts" }, 1, 148, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_RAIDEN_CARD2, DIFF_ALL, { "cuts" }, 1, 148, SHRT_MAX, false, LocInit_RandomLoadoutWithCard },
			{ &ITEM_RAIDEN_CARD3, DIFF_ALL, { "cuts" }, 2, 176, SHRT_MAX, false, LocInit_RandomLoadoutWithCard },
			{ &ITEM_RAIDEN_MODISC, DIFF_ALL, { "cuts" }, 3, 227, SHRT_MAX, false, LocInit_RandomLoadout },
			{ &ITEM_RAIDEN_CARD4, DIFF_ALL, { "cuts" }, 3, 227, SHRT_MAX, false, LocInit_RandomLoadoutWithCard },
			//{ &ITEM_RAIDEN_CARD5, DIFF_ALL, { "cuts" }, 4, 299, SHRT_MAX, false, LocInit_RandomLoadoutWithCard },
			{ &ITEM_RAIDEN_HFBLADE, DIFF_ALL, { "cuts" }, 5, 389, SHRT_MAX, false, LocInit_RandomLoadout },
			// Pickups
			{ &ITEM_RAIDEN_THERMALGOGGLES, 63, { "w11a", -5500, INT_MIN, 26500 }, 0, 9, 21 },
			{ &ITEM_RATION, 15, { "w11a", -15750, INT_MIN, 14250 }, 0, 9, 21 },
			{ &ITEM_RATION, 15, { "w11a", -3750, INT_MIN, 16875 }, 0, 9, 21 },
			{ &ITEM_RAIDEN_M9, 1, { "w11a", -18000, INT_MIN, 20250 }, 0, 9, 21 },
			{ &ITEM_M9AMMO, 3, { "w11a", -7750, INT_MIN, 15125 }, 0, 9, 21 },
			{ &ITEM_M9AMMO, 3, { "w11a", -8625, INT_MIN, 14375 }, 0, 9, 21 },
			{ &ITEM_RATION, 15, { "w11a", -5500, INT_MIN, -3500 }, 0, 9, 21 },
			{ &ITEM_RAIDEN_SHAVER, 56, { "w11a", -10000, INT_MIN, 19000 }, 0, 9, 21 },
			{ &ITEM_RAIDEN_THERMALGOGGLES, 63, { "w11b", -5500, INT_MIN, 26500 }, 1, 104, 108 },
			{ &ITEM_RATION, 15, { "w11b", -15750, INT_MIN, 14250 }, 1, 104, 108 },
			{ &ITEM_RATION, 15, { "w11b", -3750, INT_MIN, 16875 }, 1, 104, 108 },
			{ &ITEM_RAIDEN_M9, 1, { "w11b", -18000, INT_MIN, 20250 }, 1, 104, 108 },
			{ &ITEM_M9AMMO, 3, { "w11b", -7750, INT_MIN, 15125 }, 1, 104, 108 },
			{ &ITEM_M9AMMO, 3, { "w11b", -8625, INT_MIN, 14375 }, 1, 104, 108 },
			{ &ITEM_RATION, 15, { "w11b", -5500, INT_MIN, -1250 }, 1, 104, 108 },
			{ &ITEM_SOCOMAMMO, 63, { "w11b", 500, INT_MIN, -500 }, 1, 104, 108 },
			{ &ITEM_SOCOMAMMO, 63, { "w11b", 5500, INT_MIN, -1000 }, 1, 104, 108 },
			{ &ITEM_RATION, 15, { "w11c", -5500, INT_MIN, -1250 }, 1, 110, 110, true },
			{ &ITEM_SOCOMAMMO, 63, { "w11c", 500, INT_MIN, -500 }, 1, 110, 110 },
			{ &ITEM_SOCOMAMMO, 63, { "w11c", 5500, INT_MIN, -1000 }, 1, 110, 110, true },
			{ &ITEM_BANDAGE, 63, { "w12a", 3000, INT_MIN, -6000 }, 0, 29, 62 },
			{ &ITEM_M9AMMO, 63, { "w12a", -8000, INT_MIN, -2500 }, 0, 29, 62 },
			{ &ITEM_M9AMMO, 1, { "w12a", -4800, INT_MIN, -12175 }, 0, 29, 62 },
			{ &ITEM_CHAFFGRENADE, 63, { "w12a", 6000, INT_MIN, -13000 }, 0, 33, 62 },
			{ &ITEM_SOCOMAMMO, 63, { "w12b", -9500, INT_MIN, -4250 }, 1, 33, 185 },
			{ &ITEM_M9AMMO, 63, { "w12b", 0, INT_MIN, -6250 }, 1, 33, 185 },
			{ &ITEM_M9AMMO, 63, { "w12b", 6250, INT_MIN, -7875 }, 1, 33, 185 },
			{ &ITEM_RATION, 15, { "w12b", 10375, INT_MIN, -4250 }, 1, 33, 185 },
			{ &ITEM_RAIDEN_BOX1, 63, { "w12b", -4750, INT_MIN, 11500 }, 1, 92, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w12b", -8500, INT_MIN, 1000 }, 1, 92, 185 },
			{ &ITEM_BANDAGE, 63, { "w12b", -4500, INT_MIN, 5500 }, 1, 92, 185 },
			{ &ITEM_RATION, 3, { "w12b", -1500, INT_MIN, 7250 }, 1, 92, 185 },
			{ &ITEM_CHAFFGRENADE, 63, { "w12c", 6000, INT_MIN, -13000 }, 1, 102, 185 },
			{ &ITEM_BANDAGE, 63, { "w12c", 3000, INT_MIN, -6000 }, 1, 102, 185 },
			{ &ITEM_M9AMMO, 63, { "w12c", -8000, INT_MIN, -2500 }, 1, 102, 185 },
			{ &ITEM_RAIDEN_M9, 3, { "w12c", -4800, INT_MIN, -12175 }, 1, 102, 185 },
			{ &ITEM_RAIDEN_SOCOMSUPPRESSOR, 1, { "w14a", -52000, INT_MIN, -23750 }, 0, 58, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w14a", -48750, INT_MIN, -24500 }, 0, 58, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w14a", -50500, INT_MIN, -43125 }, 0, 58, 185 },
			{ &ITEM_M9AMMO, 63, { "w14a", -46625, INT_MIN, -34875 }, 0, 58, 185 },
			{ &ITEM_RATION, 15, { "w14a", -49500, INT_MIN, -43125 }, 0, 58, 185 },
			{ &ITEM_CHAFFGRENADE, 63, { "w15a", -47750, INT_MIN, -60000 }, 0, 62, 185 },
			{ &ITEM_CHAFFGRENADE, 63, { "w15b", -47750, INT_MIN, -60000 }, 0, 92, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w16a", -60500, INT_MIN, -85875 }, 0, 62, 185 },
			{ &ITEM_M9AMMO, 63, { "w16a", -43750, INT_MIN, -85875 }, 0, 62, 62 },
			{ &ITEM_PENTAZEMIN, 63, { "w16a", -47125, INT_MIN, -85875 }, 0, 62, 62 },
			{ &ITEM_SOCOMAMMO, 63, { "w16b", -60500, INT_MIN, -85875 }, 0, 92, 185 },
			{ &ITEM_RATION, 15, { "w16b", -60250, INT_MIN, -99500 }, 0, 92, 185 },
			{ &ITEM_M9AMMO, 63, { "w16b", -43750, INT_MIN, -85875 }, 0, 92, 185 },
			{ &ITEM_PENTAZEMIN, 63, { "w16b", -47125, INT_MIN, -85875 }, 0, 92, 185 },
			{ &ITEM_RAIDEN_SENSORB, 63, { "w16b", -48250, INT_MIN, -104000 }, 0, 100, 185 },
			{ &ITEM_M9AMMO, 63, { "w18a", -6500, INT_MIN, -133500 }, 1, 92, 185 },
			{ &ITEM_RATION, 15, { "w18a", 1250, INT_MIN, -119250 }, 1, 92, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w18a", -8500, INT_MIN, -109500 }, 1, 92, 185 },
			{ &ITEM_PSG1AMMO, 63, { "w18a", 8500, INT_MIN, -109500 }, 1, 92, 185 },
			{ &ITEM_STUNGRENADE, 63, { "w19a", 13250, INT_MIN, 2750 }, 1, 92, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w19a", 12750, INT_MIN, 3500 }, 1, 92, 185 },
			{ &ITEM_M9AMMO, 63, { "w20a", 52000, INT_MIN, -87500 }, 1, 92, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w20a", 48000, INT_MIN, -100000 }, 1, 92, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w20a", 56000, INT_MIN, -100000 }, 1, 92, 185 },
			{ &ITEM_M4AMMO, 63, { "w20a", 53500, INT_MIN, -84750 }, 1, 92, 185 },
			{ &ITEM_STUNGRENADE, 63, { "w20a", 51500, INT_MIN, -92000 }, 1, 92, 185 },
			{ &ITEM_RAIDEN_BOX5, 63, { "w20a", 57500, INT_MIN, -94500 }, 1, 92, 185 },
			{ &ITEM_RATION, 15, { "w20a", 42750, INT_MIN, -86000 }, 1, 92, 185 },
			{ &ITEM_RAIDEN_MINEDETECTOR, 63, { "w20a", 46500, INT_MIN, -87000 }, 1, 92, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w20a", 53000, INT_MIN, -93000 }, 5, 327, 327 },
			{ &ITEM_PSG1TAMMO, 63, { "w20a", 55500, INT_MIN, -81500 }, 5, 327, 327 },
			{ &ITEM_STINGERAMMO, 63, { "w20a", 50000, INT_MIN, -96500 }, 5, 327, 327 },
			{ &ITEM_M9AMMO, 63, { "w20a", 52000, INT_MIN, -87500 }, 5, 327, 327 },
			{ &ITEM_SOCOMAMMO, 63, { "w20a", 48000, INT_MIN, -100000 }, 5, 327, 327 },
			{ &ITEM_SOCOMAMMO, 63, { "w20a", 56000, INT_MIN, -100000 }, 5, 327, 327 },
			{ &ITEM_M4AMMO, 63, { "w20a", 53500, INT_MIN, -84750 }, 5, 327, 327 },
			{ &ITEM_STUNGRENADE, 63, { "w20a", 51500, INT_MIN, -92000 }, 5, 327, 327 },
			{ &ITEM_RATION, 15, { "w20a", 42750, INT_MIN, -86000 }, 5, 327, 327 },
			{ &ITEM_RAIDEN_BOX5, 63, { "w20a", 57500, INT_MIN, -94500 }, 5, 327, 327 },
			{ &ITEM_RAIDEN_MINEDETECTOR, 63, { "w20a", 46500, INT_MIN, -87000 }, 5, 327, 327 },
			{ &ITEM_RAIDEN_BOX3, 63, { "w20b", 51500, INT_MIN, -84400 }, 1, 92, 102 },
			{ &ITEM_STUNGRENADE, 63, { "w20b", 47000, INT_MIN, -96250 }, 1, 92, 102 },
			{ &ITEM_CLAYMORE, 63, { "w20b", 42000, INT_MIN, -99000 }, 1, 92, 102 },
			{ &ITEM_RAIDEN_BOX3, 63, { "w20c", 51500, INT_MIN, -84400 }, 1, 116, 118 },
			{ &ITEM_STUNGRENADE, 63, { "w20c", 47000, INT_MIN, -96250 }, 1, 116, 118 },
			{ &ITEM_CLAYMORE, 63, { "w20c", 42000, INT_MIN, -99000 }, 1, 116, 116 },
			{ &ITEM_SOCOMAMMO, 63, { "w20c", 59000, INT_MIN, -80500 }, 1, 116, 118, true },
			{ &ITEM_SOCOMAMMO, 63, { "w20c", 63625, INT_MIN, -90750 }, 1, 118, 118, true },
			{ &ITEM_SOCOMAMMO, 63, { "w20c", 49375, INT_MIN, -101750 }, 1, 118, 118 },
			{ &ITEM_SOCOMAMMO, 63, { "w20c", 46000, INT_MIN, -81875 }, 1, 118, 118, true },
			{ &ITEM_M9AMMO, 63, { "w20c", 66250, INT_MIN, -103500 }, 1, 118, 118, true },
			{ &ITEM_RATION, 15, { "w20c", 65750, INT_MIN, -83500 }, 1, 118, 118, true },
			{ &ITEM_RAIDEN_BOX3, 63, { "w20d", 51500, INT_MIN, -84400 }, 1, 148, 185 },
			{ &ITEM_STUNGRENADE, 63, { "w20d", 47000, INT_MIN, -96250 }, 1, 148, 185 },
			{ &ITEM_CLAYMORE, 63, { "w20d", 42000, INT_MIN, -99000 }, 1, 148, 185 },
			{ &ITEM_AKS74UAMMO, 63, { "w21a", 23000, INT_MIN, -61250 }, 1, 92, 185 },
			{ &ITEM_AKS74UAMMO, 63, { "w21b", 23000, INT_MIN, -61250 }, 5, 327, 327 },
			{ &ITEM_CHAFFGRENADE, 63, { "w21b", 58250, INT_MIN, -45250 }, 5, 327, 327 },
			{ &ITEM_RAIDEN_M9, 63, { "w22a", 55250, INT_MIN, -23250 }, 0, 36, 185 },
			{ &ITEM_M9AMMO, 63, { "w22a", 56500, INT_MIN, -23750 }, 0, 36, 185 },
			{ &ITEM_M9AMMO, 63, { "w22a", 57500, INT_MIN, -22250 }, 0, 36, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w22a", 63000, INT_MIN, -25000 }, 0, 36, 185 },
			{ &ITEM_RATION, 15, { "w22a", 58750, INT_MIN, -18000 }, 0, 36, 185 },
			{ &ITEM_BOOK, 63, { "w22a", 53000, INT_MIN, -29000 }, 0, 36, 185 },
			{ &ITEM_STUNGRENADE, 63, { "w22a", 52500, INT_MIN, -33250 }, 0, 36, 185 },
			{ &ITEM_CHAFFGRENADE, 63, { "w22a", 51000, INT_MIN, -18625 }, 0, 36, 185 },
			{ &ITEM_M9AMMO, 63, { "w22a", 59000, INT_MIN, -26000 }, 0, 36, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w22a", 58625, INT_MIN, -28500 }, 0, 36, 185 },
			{ &ITEM_RATION, 15, { "w22a", 45560, INT_MIN, -20500 }, 1, 92, 185 },
			{ &ITEM_PENTAZEMIN, 63, { "w22a", 48250, INT_MIN, -23940 }, 1, 92, 185 },
			{ &ITEM_BOOK, 63, { "w22a", 45560, INT_MIN, -21500 }, 1, 92, 185 },
			{ &ITEM_RAIDEN_BOX2, 63, { "w22a", 46500, INT_MIN, -18750 }, 1, 92, 185 },
			{ &ITEM_M9AMMO, 63, { "w22a", 57500, INT_MIN, -23250 }, 1, 92, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w22a", 54250, INT_MIN, -19375 }, 1, 92, 185 },
			{ &ITEM_RAIDEN_MINEDETECTOR, 63, { "w22a", 55375, INT_MIN, -23625 }, 1, 92, 185 },
			{ &ITEM_RAIDEN_SOCOMSUPPRESSOR, 63, { "w22a", 57625, INT_MIN, -20375 }, 1, 92, 185 },
			{ &ITEM_C4, 63, { "w22a", 54250, INT_MIN, -39500 }, 2, 148, 185 },
			{ &ITEM_C4, 63, { "w22a", 54060, INT_MIN, -38000 }, 2, 148, 185 },
			{ &ITEM_C4, 63, { "w22a", 58000, INT_MIN, -40000 }, 2, 148, 185 },
			{ &ITEM_CLAYMORE, 63, { "w22a", 54060, INT_MIN, -37000 }, 2, 148, 185 },
			{ &ITEM_RAIDEN_M4, 63, { "w22a", 43750, INT_MIN, -32000 }, 2, 148, 185 },
			{ &ITEM_M4AMMO, 63, { "w22a", 42500, INT_MIN, -31500 }, 2, 148, 185 },
			{ &ITEM_M4AMMO, 63, { "w22a", 41560, INT_MIN, -28500 }, 2, 148, 185 },
			{ &ITEM_RAIDEN_AKS74U, 63, { "w22a", 57125, INT_MIN, -44250 }, 2, 148, 185 },
			{ &ITEM_AKS74UAMMO, 63, { "w22a", 46875, INT_MIN, -44250 }, 2, 148, 185 },
			{ &ITEM_AKS74UAMMO, 63, { "w22a", 50000, INT_MIN, -36250 }, 2, 148, 185 },
			{ &ITEM_AKS74UAMMO, 63, { "w22a", 54000, INT_MIN, -36250 }, 2, 148, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w22a", 51250, INT_MIN, -44375 }, 2, 148, 185 },
			{ &ITEM_M4AMMO, 63, { "w22a", 56750, INT_MIN, -40500 }, 2, 148, 185 },
			{ &ITEM_RAIDEN_AKSUPPRESSOR, 1, { "w22a", 46750, INT_MIN, -40375 }, 2, 148, 185 },
			{ &ITEM_RAIDEN_RGB6, 63, { "w22a", 45875, INT_MIN, -39375 }, 3, 180, 185 },
			{ &ITEM_RGB6AMMO, 63, { "w22a", 47000, INT_MIN, -38250 }, 3, 180, 185 },
			{ &ITEM_RGB6AMMO, 63, { "w22a", 49750, INT_MIN, -40250 }, 3, 180, 185 },
			{ &ITEM_PSG1TAMMO, 63, { "w22a", 49940, INT_MIN, -37250 }, 3, 180, 185 },
			{ &ITEM_RAIDEN_PSG1, 63, { "w22a", 60000, INT_MIN, -32000 }, 3, 180, 185 },
			{ &ITEM_PSG1AMMO, 63, { "w22a", 58250, INT_MIN, -30250 }, 3, 180, 185 },
			{ &ITEM_PSG1AMMO, 63, { "w22a", 58060, INT_MIN, -31250 }, 3, 180, 185 },
			{ &ITEM_RAIDEN_PSG1T, 63, { "w22a", 66750, INT_MIN, -31000 }, 3, 180, 185 },
			{ &ITEM_GRENADE, 63, { "w22a", 48250, INT_MIN, -23250 }, 3, 180, 185 },
			{ &ITEM_GRENADE, 63, { "w22a", 47250, INT_MIN, -22000 }, 3, 180, 185 },
			{ &ITEM_GRENADE, 63, { "w22a", 48250, INT_MIN, -20875 }, 3, 180, 185 },
			{ &ITEM_PSG1TAMMO, 63, { "w22a", 46250, INT_MIN, -23940 }, 3, 180, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w22a", 49500, INT_MIN, -19750 }, 3, 180, 185 },
			{ &ITEM_CLAYMORE, 63, { "w22a", 58000, INT_MIN, -37375 }, 2, 180, 185 },
			{ &ITEM_M4AMMO, 63, { "w22a", 44500, INT_MIN, -31000 }, 2, 180, 185 },
			{ &ITEM_M4AMMO, 63, { "w22a", 45500, INT_MIN, -28250 }, 2, 180, 185 },
			{ &ITEM_CHAFFGRENADE, 1, { "w23a", -13000, INT_MIN, 2000 }, 0, 36, 185 },
			{ &ITEM_CHAFFGRENADE, 63, { "w23b", -13000, INT_MIN, 2000 }, 0, 92, 185 },
			{ &ITEM_BOOK, 63, { "w24a", -750, INT_MIN, -45800 }, 2, 148, 185 },
			{ &ITEM_RAIDEN_SOCOMSUPPRESSOR, 63, { "w24a", 2250, INT_MIN, -45825 }, 2, 148, 185 },
			{ &ITEM_M4AMMO, 63, { "w24a", -3750, INT_MIN, -53250 }, 2, 148, 185 },
			{ &ITEM_M9AMMO, 63, { "w24a", -3750, INT_MIN, -51250 }, 2, 148, 185 },
			{ &ITEM_CHAFFGRENADE, 63, { "w24a", -1000, INT_MIN, -52000 }, 2, 148, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w24a", -18500, INT_MIN, -55750 }, 2, 148, 185 },
			{ &ITEM_C4, 63, { "w24a", -1750, INT_MIN, -45825 }, 2, 148, 185 },
			{ &ITEM_CLAYMORE, 63, { "w24a", 1250, INT_MIN, -45825 }, 2, 148, 185 },
			{ &ITEM_RAIDEN_AKS74U, 1, { "w24a", 2500, INT_MIN, -49750 }, 2, 148, 185 },
			{ &ITEM_M9AMMO, 63, { "w24a", 15750, INT_MIN, -55750 }, 2, 180, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w24a", 17250, INT_MIN, -55750 }, 2, 180, 185 },
			{ &ITEM_BOOK, 63, { "w24a", -750, INT_MIN, -45800 }, 5, 327, 327 },
			{ &ITEM_M4AMMO, 63, { "w24a", -3750, INT_MIN, -53250 }, 5, 327, 327 },
			{ &ITEM_M9AMMO, 63, { "w24a", -3750, INT_MIN, -51250 }, 5, 327, 327 },
			{ &ITEM_CHAFFGRENADE, 63, { "w24a", -1000, INT_MIN, -52000 }, 5, 327, 327 },
			{ &ITEM_SOCOMAMMO, 63, { "w24a", -18500, INT_MIN, -55750 }, 5, 327, 327 },
			{ &ITEM_C4, 63, { "w24a", -1750, INT_MIN, -45825 }, 5, 327, 327 },
			{ &ITEM_CLAYMORE, 63, { "w24a", 1250, INT_MIN, -45825 }, 5, 327, 327 },
			{ &ITEM_RAIDEN_SOCOMSUPPRESSOR, 63, { "w24a", 2250, INT_MIN, -45825 }, 5, 327, 327 },
			{ &ITEM_RATION, 15, { "w24b", 5750, INT_MIN, -73750 }, 2, 152, 185 },
			{ &ITEM_M4AMMO, 63, { "w24b", -11250, INT_MIN, -64000 }, 2, 152, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w24b", -8500, INT_MIN, -69100 }, 2, 152, 185 },
			{ &ITEM_STUNGRENADE, 63, { "w24b", -10500, INT_MIN, -69100 }, 2, 152, 185 },
			{ &ITEM_RAIDEN_THERMALGOGGLES, 63, { "w24c", -10000, INT_MIN, -54500 }, 2, 154, 154 },
			{ &ITEM_RATION, 15, { "w24c", 7750, INT_MIN, -51000 }, 2, 154, 154 },
			{ &ITEM_BANDAGE, 63, { "w24c", -11000, INT_MIN, -48500 }, 2, 154, 154 },
			{ &ITEM_BOOK, 63, { "w24d", -8600, INT_MIN, -48750 }, 2, 150, 185 },
			{ &ITEM_RAIDEN_DMIC, 63, { "w24d", -3500, INT_MIN, -57500 }, 2, 150, 185 },
			{ &ITEM_RATION, 15, { "w24d", 7250, INT_MIN, -65500 }, 2, 150, 185 },
			{ &ITEM_M9AMMO, 63, { "w24d", 8625, INT_MIN, -53000 }, 2, 150, 185 },
			{ &ITEM_AKS74UAMMO, 63, { "w24d", 4500, INT_MIN, -55000 }, 2, 150, 185 },
			{ &ITEM_BANDAGE, 63, { "w24d", 0, INT_MIN, -51250 }, 2, 150, 185 },
			{ &ITEM_SOCOMAMMO, 63, { "w24d", -8625, INT_MIN, -50000 }, 2, 150, 185 },
			{ &ITEM_M4AMMO, 63, { "w24d", -8625, INT_MIN, -52000 }, 2, 150, 185 },
			{ &ITEM_RAIDEN_BOX4, 63, { "w24d", 3750, INT_MIN, -55000 }, 2, 150, 185 },
			{ &ITEM_BOOK, 63, { "w24d", -8250, INT_MIN, -48750 }, 2, 150, 185, true },
			{ &ITEM_SOCOMAMMO, 63, { "w25a", -1500, INT_MIN, -137000 }, 3, 182, 185, true },
			{ &ITEM_PENTAZEMIN, 63, { "w25a", -3000, INT_MIN, -137000 }, 3, 182, 185 },
			{ &ITEM_RAIDEN_STINGER, 63, { "w25a", 2125, INT_MIN, -147500 }, 3, 189, 189 },
			{ &ITEM_STINGERAMMO, 63, { "w25a", -2125, INT_MIN, -145000 }, 3, 189, 189 },
			{ &ITEM_PSG1AMMO, 63, { "w25a", 3000, INT_MIN, -137000 }, 3, 189, 189, true },
			{ &ITEM_PSG1AMMO, 63, { "w25b", -1500, INT_MIN, -156000 }, 3, 193, 193 },
			{ &ITEM_PSG1AMMO, 63, { "w25b", 499, INT_MIN, -163597 }, 3, 193, 193 },
			{ &ITEM_RATION, 15, { "w25b", 8375, INT_MIN, -145000 }, 3, 193, 193 },
			{ &ITEM_RAIDEN_AKSUPPRESSOR, 63, { "w25b", -2000, INT_MIN, -163125 }, 3, 193, 193 },
			{ &ITEM_RATION, 15, { "w25b", -3250, INT_MIN, -147000 }, 3, 193, 193 },
			{ &ITEM_PSG1AMMO, 63, { "w25c", 50500, INT_MIN, -253000 }, 3, 193, 193 },
			{ &ITEM_CHAFFGRENADE, 63, { "w25c", 53500, INT_MIN, -253000 }, 3, 193, 193 },
			{ &ITEM_SOCOMAMMO, 63, { "w25c", 53250, INT_MIN, -227000 }, 3, 193, 193 },
			{ &ITEM_RATION, 15, { "w25c", 59250, INT_MIN, -253000 }, 3, 193, 193 },
			{ &ITEM_AKS74UAMMO, 63, { "w25c", 59000, INT_MIN, -226500 }, 3, 193, 193 },
			{ &ITEM_PSG1AMMO, 63, { "w25d", 23000, INT_MIN, -241500 }, 4, 297, 299 },
			{ &ITEM_CHAFFGRENADE, 63, { "w25d", 23000, INT_MIN, -238500 }, 4, 297, 299 },
			{ &ITEM_RATION, 15, { "w25d", 20000, INT_MIN, -241250 }, 4, 297, 299 },
			{ &ITEM_M4AMMO, 63, { "w25d", 58750, INT_MIN, -236000 }, 4, 297, 299 },
			{ &ITEM_SOCOMAMMO, 63, { "w25d", 59000, INT_MIN, -253000 }, 4, 297, 299 },
			{ &ITEM_SOCOMAMMO, 63, { "w28a", 56750, INT_MIN, -222250 }, 5, 299, 299 },
			{ &ITEM_M9AMMO, 63, { "w31a", -6500, INT_MIN, -228500 }, 3, 203, 299 },
			{ &ITEM_SOCOMAMMO, 63, { "w31a", 3000, INT_MIN, -228500 }, 3, 203, 299 },
			{ &ITEM_M4AMMO, 63, { "w31a", 8500, INT_MIN, -252000 }, 3, 203, 299 },
			{ &ITEM_M4AMMO, 63, { "w31a", -4500, INT_MIN, -243750 }, 3, 203, 299 },
			{ &ITEM_NIKITAAMMO, 63, { "w31a", -5500, INT_MIN, -231750 }, 3, 203, 299, true },
			{ &ITEM_NIKITAAMMO, 63, { "w31a", 6500, INT_MIN, -228500 }, 3, 203, 299, true },
			{ &ITEM_CHAFFGRENADE, 63, { "w31a", -18000, INT_MIN, -236000 }, 3, 203, 299 },
			{ &ITEM_RATION, 15, { "w31a", -12500, INT_MIN, -231500 }, 3, 203, 299 },
			{ &ITEM_RAIDEN_RGB6, 63, { "w31a", 6500, INT_MIN, -252000 }, 3, 203, 299 },
			{ &ITEM_RAIDEN_M4, 63, { "w31a", -18000, INT_MIN, -234000 }, 3, 203, 299 },
			{ &ITEM_SOCOMAMMO, 63, { "w31a", 625, INT_MIN, -246250 }, 3, 227, 299 },
			{ &ITEM_STINGERAMMO, 63, { "w31a", -2000, INT_MIN, -238250 }, 3, 227, 299 },
			{ &ITEM_BOOK, 63, { "w31a", -3750, INT_MIN, -237750 }, 3, 227, 299 },
			{ &ITEM_RATION, 1, { "w31b", 11500, INT_MIN, -249500 }, 3, 203, 299 },
			{ &ITEM_M4AMMO, 63, { "w31b", 9500, INT_MIN, -237500 }, 3, 203, 299 },
			{ &ITEM_RAIDEN_NVG, 63, { "w31b", 4250, INT_MIN, -244750 }, 3, 203, 299 },
			{ &ITEM_GRENADE, 63, { "w31b", -1500, INT_MIN, -2335750 }, 3, 203, 299 },
			{ &ITEM_RAIDEN_PSG1T, 63, { "w31b", -2000, INT_MIN, -223000 }, 3, 203, 299 },
			{ &ITEM_RATION, 15, { "w31b", -9500, INT_MIN, -222000 }, 3, 203, 299 },
			{ &ITEM_RAIDEN_RGB6, 63, { "w31b", -1500, INT_MIN, -249500 }, 3, 203, 299 },
			{ &ITEM_STINGERAMMO, 63, { "w31b", -2000, INT_MIN, -223000 }, 3, 203, 299 },
			{ &ITEM_RAIDEN_NIKITA, 1, { "w31b", 11500, INT_MIN, -249500 }, 3, 203, 299 },
			{ &ITEM_RAIDEN_NIKITA, 2, { "w31b", -750, INT_MIN, -242750 }, 4, 203, 299 },
			{ &ITEM_RATION, 2, { "w31b", -750, INT_MIN, -242750 }, 4, 203, 299 },
			{ &ITEM_RAIDEN_NIKITA, 60, { "w31b", 5000, INT_MIN, -236250 }, 4, 203, 299 },
			{ &ITEM_RATION, 12, { "w31b", 5000, INT_MIN, -236250 }, 4, 203, 299 },
			{ &ITEM_M4AMMO, 63, { "w31b", -2750, INT_MIN, -250750 }, 4, 297, 299 },
			{ &ITEM_M9AMMO, 63, { "w31c", -14000, INT_MIN, -243250 }, 4, 253, 253, true },
			{ &ITEM_RGB6AMMO, 63, { "w31c", -4750, INT_MIN, -244500 }, 4, 253, 253, true },
			{ &ITEM_SOCOMAMMO, 63, { "w31c", -14000, INT_MIN, -235000 }, 4, 253, 253, true },
			{ &ITEM_RATION, 15, { "w31c", -6000, INT_MIN, -235000 }, 4, 253, 253, true },
			{ &ITEM_AKS74UAMMO, 63, { "w31c", -6000, INT_MIN, -242500 }, 4, 253, 253, true },
			{ &ITEM_AKS74UAMMO, 63, { "w31c", -6000, INT_MIN, -241500 }, 4, 253, 253, true },
			{ &ITEM_RATION, 15, { "w31c", -10000, INT_MIN, -244000 }, 4, 253, 253 },
			{ &ITEM_M4AMMO, 63, { "w31c", -6000, INT_MIN, -242500 }, 4, 253, 253, true },
			{ &ITEM_M4AMMO, 63, { "w31c", -6000, INT_MIN, -241500 }, 4, 253, 253 },
			{ &ITEM_M9AMMO, 63, { "w31d", -2000, INT_MIN, -245500 }, 4, 297, 299 },
			{ &ITEM_SOCOMAMMO, 63, { "w31d", -11500, INT_MIN, -252000 }, 4, 297, 299 },
			{ &ITEM_SOCOMAMMO, 63, { "w31d", -18000, INT_MIN, -234000 }, 4, 297, 299 },
			{ &ITEM_AKS74UAMMO, 63, { "w31d", 8500, INT_MIN, -252000 }, 4, 297, 299 },
			{ &ITEM_AKS74UAMMO, 63, { "w31d", 7500, INT_MIN, -243500 }, 4, 297, 299 },
			{ &ITEM_PSG1AMMO, 63, { "w31d", 4750, INT_MIN, -244500 }, 4, 297, 299 },
			{ &ITEM_PSG1TAMMO, 63, { "w31d", -2000, INT_MIN, -238250 }, 4, 297, 299 },
			{ &ITEM_RGB6AMMO, 63, { "w31d", -3750, INT_MIN, -237750 }, 4, 297, 299 },
			{ &ITEM_PENTAZEMIN, 63, { "w31d", -12500, INT_MIN, -226500 }, 4, 297, 299 },
			{ &ITEM_RATION, 15, { "w31d", -500, INT_MIN, -243500 }, 4, 297, 299 },
			{ &ITEM_M9AMMO, 63, { "w31f", -14000, INT_MIN, -243250 }, 4, 257, 299 },
			{ &ITEM_RGB6AMMO, 63, { "w31f", -4750, INT_MIN, -244500 }, 4, 257, 299 },
			{ &ITEM_SOCOMAMMO, 63, { "w31f", -14000, INT_MIN, -235000 }, 4, 257, 299 },
			{ &ITEM_AKS74UAMMO, 63, { "w31f", -6000, INT_MIN, -242500 }, 4, 257, 299 },
			{ &ITEM_RATION, 15, { "w31f", -6000, INT_MIN, -235000 }, 4, 257, 299 },
			{ &ITEM_AKS74UAMMO, 63, { "w31f", -6000, INT_MIN, -241500 }, 4, 257, 299 },
			{ &ITEM_RATION, 15, { "w31f", -10000, INT_MIN, -244000 }, 4, 257, 299 },
			{ &ITEM_RATION, 15, { "w31f", -5000, INT_MIN, -248250 }, 4, 257, 299 },
			{ &ITEM_AKS74UAMMO, 63, { "w31f", -18000, INT_MIN, -252500 }, 4, 257, 299 },
			{ &ITEM_PSG1TAMMO, 63, { "w31f", -6250, INT_MIN, -246750 }, 4, 257, 299 },
			{ &ITEM_C4, 63, { "w31f", -18250, INT_MIN, -258500 }, 4, 257, 299 },
			{ &ITEM_C4, 63, { "w31f", -18250, INT_MIN, -257500 }, 4, 257, 299 },
			{ &ITEM_BOOK, 63, { "w31f", -18250, INT_MIN, -253500 }, 4, 257, 299 },
			{ &ITEM_PENTAZEMIN, 63, { "w31f", -15125, INT_MIN, -252125 }, 4, 257, 299 },
			{ &ITEM_RAIDEN_THERMALGOGGLES, 63, { "w31f", -11250, INT_MIN, -256500 }, 4, 257, 299 },
			{ &ITEM_RAIDEN_BODYARMOR, 63, { "w31f", -4000, INT_MIN, -259500 }, 4, 257, 299 },
			{ &ITEM_M4AMMO, 63, { "w31f", -6000, INT_MIN, -242500 }, 4, 257, 299 },
			{ &ITEM_M4AMMO, 63, { "w31f", -6000, INT_MIN, -241500 }, 4, 257, 299 },
			{ &ITEM_RAIDEN_THERMALGOGGLES, 63, { "w32a", 52000, INT_MIN, -204000 }, 5, 313, 315 },
			{ &ITEM_PSG1AMMO, 3, { "w32a", 53000, INT_MIN, -204000 }, 5, 313, 315, true },
			{ &ITEM_PSG1AMMO, 63, { "w32a", 54500, INT_MIN, -204000 }, 5, 313, 315, true },
			{ &ITEM_PENTAZEMIN, 63, { "w32a", 50000, INT_MIN, -205000 }, 5, 313, 315, true },
			{ &ITEM_PENTAZEMIN, 15, { "w32a", 51000, INT_MIN, -204000 }, 5, 313, 315, true },
			{ &ITEM_PSG1TAMMO, 3, { "w32a", 56000, INT_MIN, -205000 }, 5, 313, 315, true },
			{ &ITEM_PSG1TAMMO, 63, { "w32a", 57500, INT_MIN, -208000 }, 5, 313, 315, true },
			{ &ITEM_RAIDEN_THERMALGOGGLES, 63, { "w32b", 52000, INT_MIN, -204000 }, 5, 317, 317 },
			{ &ITEM_PENTAZEMIN, 63, { "w32b", 50000, INT_MIN, -205000 }, 5, 317, 317, true },
			{ &ITEM_PENTAZEMIN, 15, { "w32b", 51000, INT_MIN, -204000 }, 5, 317, 317, true },
			{ &ITEM_PSG1TAMMO, 3, { "w32b", 56000, INT_MIN, -205000 }, 5, 317, 317, true },
			{ &ITEM_PSG1TAMMO, 63, { "w32b", 57500, INT_MIN, -208000 }, 5, 317, 317, true },
			{ &ITEM_PSG1AMMO, 3, { "w32b", 53000, INT_MIN, -204000 }, 5, 317, 317, true },
			{ &ITEM_PSG1AMMO, 63, { "w32b", 54500, INT_MIN, -204000 }, 5, 317, 317, true },
			{ &ITEM_RATION, 15, { "w41a", -5250, INT_MIN, -3500 }, 5, 375, 377 },
			{ &ITEM_MEDICINE, 63, { "w41a", 2500, INT_MIN, -7200 }, 5, 375, 377 },
			{ &ITEM_RAIDEN_BOX5, 63, { "w42a", 9250, INT_MIN, -13000 }, 5, 375, 377 },
			{ &ITEM_MEDICINE, 63, { "w42a", 9500, INT_MIN, -66000 }, 5, 375, 377 },
			{ &ITEM_RATION, 15, { "w42a", 17750, INT_MIN, -42000 }, 5, 375, 377 },
			{ &ITEM_RATION, 15, { "w43a", 34250, INT_MIN, -42250 }, 5, 379, 389 },
			{ &ITEM_RATION, 15, { "w45a", 2500, INT_MIN, -138750 }, 5, 397, 403 },
			{ &ITEM_STINGERAMMO, 63, { "w46a", -8000, INT_MIN, -12000 }, 5, 408, 411, true },
			{ &ITEM_STINGERAMMO, 63, { "w46a", 8000, INT_MIN, -12000 }, 5, 408, 411, true },
			{ &ITEM_STINGERAMMO, 63, { "w46a", -14000, INT_MIN, 0 }, 5, 408, 411, true },
			{ &ITEM_STINGERAMMO, 63, { "w46a", 14000, INT_MIN, 0 }, 5, 408, 411, true },
			{ &ITEM_STINGERAMMO, 63, { "w46a", -8000, INT_MIN, 12000 }, 5, 408, 411, true },
			{ &ITEM_STINGERAMMO, 63, { "w46a", 8000, INT_MIN, 12000 }, 5, 408, 411, true },
			{ &ITEM_RATION, 31, { "w46a", 0, INT_MIN, 0 }, 5, 408, 411, true },
			{ &ITEM_RATION, 7, { "w61a", 1000, INT_MIN, 21000 }, 5, 469, 469 },
			{ &ITEM_RATION, 3, { "w61a", 12250, INT_MIN, 3500 }, 5, 469, 469 },
		} },
		{ Stage::TalesA, {} },
		{ Stage::TalesB, {} },
		{ Stage::TalesC, {} },
		{ Stage::TalesD, {} },
		{ Stage::TalesE, {} },
	};

	std::unordered_map<MGS2::Stage, std::vector<RequiredItemGroup>> RequiredItemsData{
		{ Stage::Tanker, {
			{ { &ITEM_SNAKE_CAMERA }, 43 },
			{ { &ITEM_SNAKE_USP }, 29 },
			{ { &ITEM_SNAKE_M9, &ITEM_SNAKE_USP }, 16 },
		} },
		{ Stage::Plant, {
			{ { &ITEM_RAIDEN_COOLANT }, 92 }, // bomb disposal (during)
			{ { &ITEM_RAIDEN_M9, &ITEM_RAIDEN_SOCOM }, 116 }, // fatman (before)
			{ { &ITEM_RAIDEN_AKS74U }, 148 }, // s1c1f elevator (before s1c1f)
			{ { &ITEM_RAIDEN_BDU }, 148 }, // s1c1f elevator (before s1c1f)
			{ { &ITEM_RAIDEN_DMIC }, 154 }, // ames (154=inside, 152=b1)
			{ { &ITEM_RAIDEN_PSG1 }, 182 }, // sniping (during)
			{ { &ITEM_RAIDEN_STINGER }, 182 }, // harrier (before)
			{ { &ITEM_RAIDEN_NIKITA }, 203 }, // prez (during)
			//{ { &ITEM_RAIDEN_HFBLADE }, 379 }, // blade demo (jejunum)
		} },
		{ Stage::TalesA, {} }, // M9/USP Fatman, Dmic Jennifer(?)
		{ Stage::TalesB, {} }, // Stinger Harrier(?)
		{ Stage::TalesC, {} }, // USP/M4 sensors
		{ Stage::TalesD, {} }, // Nikita
		{ Stage::TalesE, {} }, // ???
	};



#ifdef _DEBUG
	struct Database {
		sqlite3* DB = nullptr;
		const char* FilePath = nullptr;
		std::vector<sqlite3_stmt*> Statements;

		void FinaliseStatements() {
			for (auto a : Statements) {
				sqlite3_finalize(a);
			}
			Statements.clear();
		}

		void OutputLocationSet(int stage) {
			if (!DB) return;

			sqlite3_stmt* stmtSelect;
			int rc = sqlite3_prepare_v2(DB, 
				"select I.code, L.difficulties, L.area, L.posx, L.posz, L.mincard, L.minprog, L.maxprog, L.callback, L.respawn "
				"from Locations L inner join Items I on L.item_id = I.id "
				"where L.stage = ? and minprog not null and maxprog not null "
				"order by L.area asc, L.minprog asc, L.id asc", -1, &stmtSelect, nullptr);
			Statements.push_back(stmtSelect);
			if (rc != SQLITE_OK) {
				std::cerr << "SQL error: " << sqlite3_errmsg(DB) << std::endl;
				return;
			}

			sqlite3_bind_int(stmtSelect, 1, stage);

			OutputDebugStringA("STAGE LOCATION SET BELOW:\n\n");
			while ((rc = sqlite3_step(stmtSelect)) == SQLITE_ROW) {
				const unsigned char* codeName = sqlite3_column_text(stmtSelect, 0);
				int difficulties = sqlite3_column_int(stmtSelect, 1);
				const unsigned char* area = sqlite3_column_text(stmtSelect, 2);
				int posx = sqlite3_column_int(stmtSelect, 3);
				int posz = sqlite3_column_int(stmtSelect, 4);
				int mincard = sqlite3_column_int(stmtSelect, 5);
				bool minprogNull = (sqlite3_column_type(stmtSelect, 6) == SQLITE_NULL);
				int minprog = minprogNull ? 0 : sqlite3_column_int(stmtSelect, 6);
				bool maxprogNull = (sqlite3_column_type(stmtSelect, 7) == SQLITE_NULL);
				int maxprog = maxprogNull ? SHRT_MAX : sqlite3_column_int(stmtSelect, 7);
				bool callbackNull = (sqlite3_column_type(stmtSelect, 8) == SQLITE_NULL);
				const unsigned char* callback = callbackNull ? nullptr : sqlite3_column_text(stmtSelect, 8);
				bool respawn = (bool)sqlite3_column_int(stmtSelect, 9);
				const char* respawnTxt = respawn ? "true" : "false";

				std::string extra = "";
				if (!callbackNull) {
					extra = fmt::format(", {}, {}, {}, {}", minprog, maxprog, respawnTxt, (const char*)callback);
				}
				else if (respawn) {
					extra = fmt::format(", {}, {}, true", minprog, maxprog);
				}
				else if (!maxprogNull) {
					extra = fmt::format(", {}, {}", minprog, maxprog);
				}
				else if (!minprogNull) {
					extra = fmt::format(", {}", minprog);
				}
				
				std::string entry = fmt::format("{{ &ITEM_{}, {}, {{ \"{}\", {}, INT_MIN, {} }}, {}{} }},\n", (const char*)codeName, difficulties, (const char*)area, posx, posz, mincard, extra);
				OutputDebugStringA(entry.c_str());
			}

			FinaliseStatements();
		}

		void RecordDifficulty(int f, int i, int posX, int posY, int posZ) {
			if (!DB) return;

			char diffIndex = *Mem::Difficulty / 10;
			if ((diffIndex < 1) || (diffIndex > 6)) {
				return;
			}
			char diffValue = 1 << (diffIndex - 1);

			int stage = Mem::Stage();
			short progress = Mem::Progress();
			int respawn = (*Mem::AreaTimeFrames > 60);

			sqlite3_stmt* stmtSelectItem;
			int rc = sqlite3_prepare_v2(DB, "select * from Items where f = ? and i = ? and (stage is null or stage = ?);", -1, &stmtSelectItem, nullptr);
			Statements.push_back(stmtSelectItem);
			if (rc != SQLITE_OK) {
				std::cerr << "SQL error: " << sqlite3_errmsg(DB) << std::endl;
				return FinaliseStatements();
			}

			sqlite3_bind_int(stmtSelectItem, 1, f);
			sqlite3_bind_int(stmtSelectItem, 2, i);
			sqlite3_bind_int(stmtSelectItem, 3, stage);

			rc = sqlite3_step(stmtSelectItem);
			if (rc != SQLITE_ROW) {
				OutputDebugStringA("No Item match!\n");
				return FinaliseStatements();
			}
			int itemId = sqlite3_column_int(stmtSelectItem, 0);

			sqlite3_stmt* stmtSelectLocation;
			rc = sqlite3_prepare_v2(DB, "select id, difficulties, respawn, minprog, maxprog from Locations where stage = ? and area = ? and posx = ? and posy = ? and posz = ? and item_id = ?;", -1, &stmtSelectLocation, nullptr);
			Statements.push_back(stmtSelectLocation);
			if (rc != SQLITE_OK) {
				std::cerr << "SQL error: " << sqlite3_errmsg(DB) << std::endl;
				return FinaliseStatements();
			}

			sqlite3_bind_int(stmtSelectLocation, 1, stage);
			sqlite3_bind_text(stmtSelectLocation, 2, (const char*)Mem::AreaCode, -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmtSelectLocation, 3, posX);
			sqlite3_bind_int(stmtSelectLocation, 4, posY);
			sqlite3_bind_int(stmtSelectLocation, 5, posZ);
			sqlite3_bind_int(stmtSelectLocation, 6, itemId);

			sqlite3_stmt* stmtUpdate;
			bool newRecord = true;
			while (sqlite3_step(stmtSelectLocation) == SQLITE_ROW) {
				// update existing
				int minprog = (sqlite3_column_type(stmtSelectLocation, 3) == SQLITE_NULL) ? -1 : sqlite3_column_int(stmtSelectLocation, 3);
				int maxprog = (sqlite3_column_type(stmtSelectLocation, 4) == SQLITE_NULL) ? -1 : sqlite3_column_int(stmtSelectLocation, 4);

				//if ( ((minprog != -1) && (progress < minprog)) ||
				if (
					((maxprog != -1) && (progress > maxprog))) {
					continue;
				}

				int locationId = sqlite3_column_int(stmtSelectLocation, 0);
				int diffSet = sqlite3_column_int(stmtSelectLocation, 1);
				respawn |= sqlite3_column_int(stmtSelectLocation, 2);

				if ((diffSet & diffValue) == diffValue) {
					//FinaliseStatements();
					newRecord = false;
					continue;
				}
				diffSet |= diffValue;

				rc = sqlite3_prepare_v2(DB, "update Locations set difficulties = ?, respawn = ? where id = ?;", -1, &stmtUpdate, nullptr);
				Statements.push_back(stmtUpdate);
				if (rc != SQLITE_OK) {
					std::cerr << "SQL error: " << sqlite3_errmsg(DB) << std::endl;
					continue;
				}

				sqlite3_bind_int(stmtUpdate, 1, diffSet);
				sqlite3_bind_int(stmtUpdate, 2, respawn);
				sqlite3_bind_int(stmtUpdate, 3, locationId);

				rc = sqlite3_step(stmtUpdate);
				if (rc != SQLITE_DONE) {
					std::cerr << "SQL error: " << sqlite3_errmsg(DB) << std::endl;
					//return FinaliseStatements();
					continue;
				}

				newRecord = false;
				break;
			}

			if (!newRecord) {
				OutputDebugStringA("Already in database at this difficulty.\n");
				return FinaliseStatements();
			}

			// create new
			rc = sqlite3_prepare_v2(DB, "insert into Locations (item_id, stage, area, posx, posy, posz, difficulties, respawn) values (?, ?, ?, ?, ?, ?, ?, ?);", -1, &stmtUpdate, nullptr);
			Statements.push_back(stmtUpdate);
			if (rc != SQLITE_OK) {
				std::cerr << "SQL error: " << sqlite3_errmsg(DB) << std::endl;
				return FinaliseStatements();
			}

			sqlite3_bind_int(stmtUpdate, 1, itemId);
			sqlite3_bind_int(stmtUpdate, 2, stage);
			sqlite3_bind_text(stmtUpdate, 3, (const char*)Mem::AreaCode, -1, SQLITE_STATIC);
			sqlite3_bind_int(stmtUpdate, 4, posX);
			sqlite3_bind_int(stmtUpdate, 5, posY);
			sqlite3_bind_int(stmtUpdate, 6, posZ);
			sqlite3_bind_int(stmtUpdate, 7, diffValue);
			sqlite3_bind_int(stmtUpdate, 8, respawn);

			rc = sqlite3_step(stmtUpdate);
			if (rc == SQLITE_DONE) {
				OutputDebugStringA("Added to database successfully\n");
			}
			else {
				std::cerr << "SQL error: " << sqlite3_errmsg(DB) << std::endl;
			}
			
			FinaliseStatements();
		}

		void ConnectToDb() {
			int rc = sqlite3_open(FilePath, &DB);
			if (rc != SQLITE_OK) {
				std::cerr << "SQL error: " << sqlite3_errmsg(DB) << std::endl;
				sqlite3_close(DB);
				DB = nullptr;
			}
		}

		void DisconnectFromDb() {
			FinaliseStatements();
			if (DB != nullptr) {
				sqlite3_close(DB);
				DB = nullptr;
			}
		}

		bool ToggleDbConnection() {
			if (DB == nullptr) {
				ConnectToDb();
				return true;
			}
			else {
				DisconnectFromDb();
				return false;
			}
		}

		Database(const char* dbFile) {
			FilePath = dbFile;
			ConnectToDb();
		}

		~Database() {
			DisconnectFromDb();
		}
	};


	Database* DB = nullptr;

	void DBToggleAction(Actions::Action action) {
		if (DB == nullptr) return;
		bool on = DB->ToggleDbConnection();
		Log::DisplayToggleMessage("ItemRando DB Connection", on);
	}
#endif


	std::vector<std::shared_ptr<LocationGroup>> AllLocationGroups;
	std::vector<std::shared_ptr<LocationGroup>> RegularLocationGroups;
	std::vector<std::shared_ptr<LocationGroup>> KeyLocationGroups;
	bool RerollNow = true;


}


namespace std {
	template <>
	struct hash<MGS2::ItemRando::Position> {
		size_t operator()(const MGS2::ItemRando::Position& p) const {
			std::string_view str(p.AreaCode, strlen(p.AreaCode) - 1);

			size_t h1 = std::hash<std::string_view>()(str);
			size_t h2 = std::hash<int>()(p.PosX);
			size_t h3 = std::hash<int>()(p.PosZ);
			//size_t h4 = std::hash<int>()(p.PosYRounded());

			//return h1 ^ (h2 << 1) ^ (h3 << 2) *(h4 << 3);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};

	template<>
	struct hash<MGS2::ItemRando::Location> {
		size_t operator()(const MGS2::ItemRando::Location& l) const {
			MGS2::ItemRando::Item* item = l.InitialItem;

			size_t h1 = std::hash<MGS2::ItemRando::Position>()(l.Position);
			size_t h2 = std::hash<int>()(item->Category);
			size_t h3 = std::hash<int>()(item->Id);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};

}
