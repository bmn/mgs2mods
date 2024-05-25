#pragma once
#include "MGS2.framework.h"
#include <iostream>
#include <functional>
#include <unordered_set>

#ifdef _DEBUG
#include "lib/sqlite/sqlite3.h"
#endif
#include <random>


namespace MGS2::ItemRando3 {
	const char* Category = "ItemRando";

	struct Seed;
	struct Pickup;
	struct Location;
	struct LocationGroup;
	struct RequiredKeyPickup;
	struct FallbackPickupDefinition;

	enum DifficultyBits {
		NoDifficulty = 0,
		VeryEasy = 1,
		Easy = 2,
		Normal = 4,
		Hard = 8,
		Extreme = 16,
		EuroExtreme = 32,
		AllDifficulties = 63
	};


	struct _GlobalSettings {
		bool IncludeSeedInSaves = true;

		std::filesystem::path ReportDirectory;
		std::string ReportFilename; // todo fmt runtime?
	} GlobalSettings;


	struct Seed {
		// const/static
		enum MainModes {
			NoMainMode, DefaultMainMode, CacheTags, RandomCacheTags,
			UndefinedMainMode = -1
		};

		enum RandomisationModes {
			NoRandomisation, SameSet, AnySet,
			UndefinedRandomisationMode = -1
		};

		enum CardModes {
			NoCards, AddCardLevel, ReplaceCardLevel,
			UndefinedCardMode = -1
		};

		enum AmmoModes {
			DefaultAmmoMode, RampUp, Shuffled,
			UndefinedAmmoMode = -1
		};

		enum LogicDifficulties {
			Easy, Medium, Hard
		};

		int MIN_SEED_NUMBER = 0;
		int MAX_SEED_NUMBER = 0xFFFFFF;


		// non user-visible settings
		bool Enabled = true;
		int SeedNumber = 0;
		bool DogTagItems = false;
		bool SpawnRegularWorldPickups = true;
		bool SpawnUnmatchedWorldPickups = true;
		bool SpawnDroppedPickups = true;
		bool SpawnRespawnablePickups = true;

		// not sure
		int LocationTypes = 1; // TODO get this to work with the actual type in Location

		// user-visible settings
		MainModes MainMode = MainModes::DefaultMainMode;
		RandomisationModes RandomisationMode = RandomisationModes::SameSet;
		RandomisationModes FallbackMode = RandomisationModes::AnySet;
		CardModes CardMode = CardModes::NoCards;
		AmmoModes AmmoMode = AmmoModes::DefaultAmmoMode; // not used yet
		bool Logic = true;
		LogicDifficulties LogicDifficulty = LogicDifficulties::Easy;
		//bool DifficultyAgnostic = false;

		// other stuff
		std::mt19937 RNG;
		bool NeedsReroll = true;


		void ParseConfig(CSimpleIniA& ini, const char* category = nullptr);
		void SetupMainMode(MainModes mainMode = MainModes::UndefinedMainMode);
		unsigned short GetSettingsHash() const;
		void SetSettingsHash(unsigned short hash);
		std::string FullSeed() const;
		void WriteReport();

		bool IncludesLocationType(int type) const;

		Seed() {
			std::random_device rd;
			RNG = std::mt19937(rd());
		}
	};
	Seed CurrentSeed;
	Seed DefaultSeed;


	// the new Item
	struct Pickup {
		enum Type {
			KeyWeapon,
			WeaponAmmo,
			StandaloneWeapon,
			KeyItem,
			StandaloneItem
		};

		enum Sets {
			Weapons,
			Items
		};

		enum Categories {
			ItemPickup = 0,
			WeaponAmmoPickup = 1,
			StandaloneAmmoPickup = 2
		};

		std::string_view Name;
		char Id = 0;
		short Ammo = 1;
		bool IsKeyPickup = false;
		const Pickup* SecondaryItem = nullptr;
		short IsCardWithLevel = 0;

		Categories Category = Categories::ItemPickup;
		Sets Set = Sets::Items;
		short AmmoIfMissing = 0;
		short AmmoIfCollected = 1;

		Pickup(char id, Categories category) : Id(id), Category(category) {}

		constexpr Pickup(std::string_view name, Type type, char id, short ammo = 1, const Pickup* secondaryItem = nullptr, short isCardWithLevel = 0)
			: Name(name), Id(id), Ammo(ammo), SecondaryItem(secondaryItem), IsCardWithLevel(isCardWithLevel) {
			switch (type) {
				case KeyWeapon:
					IsKeyPickup = true;
					Category = Categories::StandaloneAmmoPickup;
					Set = Sets::Weapons;
					AmmoIfMissing = -1;
					AmmoIfCollected = 0;
					break;
				case WeaponAmmo:
					Category = Categories::WeaponAmmoPickup;
					Set = Sets::Weapons;
					AmmoIfMissing = -1;
					AmmoIfCollected = 0;
					break;
				case StandaloneWeapon:
					Category = Categories::StandaloneAmmoPickup;
					Set = Sets::Weapons;
					AmmoIfMissing = -1;
					AmmoIfCollected = 0; // 1?
					break;
				case KeyItem:
					IsKeyPickup = true;
					Category = Categories::ItemPickup;
					Set = Sets::Items;
					break;
				case StandaloneItem:
					Category = Categories::ItemPickup;
					Set = Sets::Items;
					break;
			}
		};

		short SetAmmo(short ammo = SHRT_MIN, bool addContinue = false) const;
		short GetAmmo() const;
		short GetMaxAmmo() const;
		bool IsCollected(short ammo = SHRT_MIN) const;

		bool operator==(const Pickup& rhs) const;
	};


	struct Location {
		enum Limitations {
			NoLimitation,
			NeedsBDUAndAK
		};

		enum Types {
			NoType = 0,
			Area = 1,
			Cutscene = 2,
			Loadout = 4
		};

		enum AmmoOverrides {
			NoAmmoOverride, NoAmmo, MaxAmmo, ByDifficulty
		};

		const Pickup* OriginalPickup;
		std::string AreaCode;
		int X = INT_MIN;
		int Y = INT_MIN;
		int Z = INT_MIN;
		char Difficulties = 63;
		short RequiredCardLevel = 0;
		short MinProgress = 0;
		short MaxProgress = 0;
		bool CanRespawn = false;
		unsigned int Limitation = Limitations::NoLimitation;
		Types Type = Types::Area;
		AmmoOverrides AmmoOverride = AmmoOverrides::NoAmmoOverride;


		bool IsAvailableBy(short progress = SHRT_MIN, bool allowBefore = false) const;
		bool IsAvailableBefore(short progress = SHRT_MIN) const;
		bool InDifficulty(DifficultyBits difficulty = DifficultyBits::NoDifficulty) const;

		bool operator==(const Location& rhs) const;
	};

	struct LocationHasher {
		std::size_t operator()(const Location& l) const {
			std::string_view strAreaCode = std::string_view(l.AreaCode).substr(0, l.AreaCode.length() - 1);

			size_t h1 = std::hash<std::string_view>()(strAreaCode);
			size_t h2 = std::hash<int>()(l.X);
			size_t h3 = std::hash<int>()(l.Y);
			size_t h4 = std::hash<int>()(l.Z);

			// if Area, hash=Type
			// if not Area, hash=Type/MinProgress
			size_t h5a = std::hash<int>()(l.Type);
			size_t h5b = std::hash<int>()(l.MinProgress);
			size_t h5 = ((l.Type & Location::Types::Area) == Location::Types::Area) ? h5a : h5a ^ (h5b << 1);

			return h1 ^ (h2 << 1) ^ (h4 << 2) ^ (h5 << 3); // not including Y
			//return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
		}
	};

	struct PickupHasher {
		std::size_t operator()(const Pickup& p) const {
			size_t h1 = std::hash<int>()(p.Id);
			size_t h2 = std::hash<int>()(p.Category);

			return h1 ^ (h2 << 1);
		}
	};


	struct RequiredKeyPickup {
		short MaxProgress = 0;
		unsigned int RejectedLimitations = Location::Limitations::NoLimitation;
		std::vector<const Pickup*> EasyPickups;
		std::vector<const Pickup*> MediumPickups;
		std::vector<const Pickup*> HardPickups;

		std::vector<const Pickup*>& PickupSet(Seed::LogicDifficulties difficulty = Seed::LogicDifficulties::Easy) {
			switch (difficulty) {
				case Seed::LogicDifficulties::Medium:
					return MediumPickups;
				case Seed::LogicDifficulties::Hard:
					return HardPickups;
				case Seed::LogicDifficulties::Easy:
				default:
					return EasyPickups;
			}
		}

		const Pickup* RandomPickup(Seed::LogicDifficulties difficulty = Seed::LogicDifficulties::Easy) {
			auto pickupSet = PickupSet(difficulty);
			size_t count = pickupSet.size();
			if (count == 0) return nullptr;
			int index = CurrentSeed.RNG() % count;
			return pickupSet[index];
		}
	};


	struct LocationGroup {
		std::vector<const Location*> Locations;
		const Pickup* OriginalPickup;
		const Pickup* RandomPickup;
		const Pickup* FallbackPickup;
		short FallbackAfterProgress = SHRT_MAX;

		void Init();
		const Pickup* CurrentRandomPickup() const;
		bool IsAvailableAt(short progress = SHRT_MIN, bool allowBefore = false) const;
		bool IsAvailableBefore(short progress = SHRT_MIN) const;
		short MinProgress() const;
		short MaxProgress() const;
	};


	struct FallbackPickupDefinition {
		RequiredKeyPickup* RequiredPickupDefinition;
		const Pickup* SelectedPickup;
		short InitialRandomMaxProgress;
	};


	struct PickupSpawn {
		struct Pointers {
			int* B;
			int* F;
			int* I;
			int* N;
			int* T;
			int* U;
			int* X;
			float* PX;
			float* PY;
			float* PZ;
		} Ptr;

		struct Params {
			int B;
			int F;
			int I;
			int N;
			int T;
			int U;
			int X;
			int PX;
			int PY;
			int PZ;
		} Param;

		int CallParams[3];

		const Pickup* CacheTagBestPickup() const;

		int RunHook();
		int ApplyPickup(const Pickup* pickup, bool runHook = false);
		PickupSpawn(int param_1, int param_2, int param_3);
		PickupSpawn();
	};


	struct DataSet {
		MGS2::Stage CurrentStage = Stage::None;
		std::unordered_map<Location, std::shared_ptr<LocationGroup>, LocationHasher> LGsByLocation;
		std::vector<short> CardAvailability;

		std::unordered_map<const Pickup, std::shared_ptr<LocationGroup>, PickupHasher> LGsByOriginalPickup;
		std::unordered_map<const Pickup, std::shared_ptr<LocationGroup>, PickupHasher> LGsByRandomPickup;

		PickupSpawn CurrentPickupSpawn;

		short AmmoState[2][30]; // TODO get the right array size
		bool AmmoStateStored = false;
		bool AmmoStateDirty = false;

		void Init();
		void PopulateLocations();
		void RandomiseLocations();
		std::vector<FallbackPickupDefinition> RandomiseKeyPickupLocations(std::vector<RequiredKeyPickup>& requiredKeyPickups, std::vector<std::shared_ptr<LocationGroup>>& shuffledLocationGroups, bool nextAttempt = false);
		void StoreAmmoState();
		void NeutraliseAmmoState();
		void RestoreAmmoState(bool awardNewPickups = false);
		void DisplayNewPickups(const std::vector<std::string>& pickupNames) const;

		bool IsCardAvailable(short level, short progress) const;
	};
	DataSet CurrentDataSet;

	

	constexpr Pickup PICKUP_RATION("Ration", Pickup::Type::StandaloneItem, 1);
	constexpr Pickup PICKUP_MEDICINE("Medicine", Pickup::Type::StandaloneItem, 3);
	constexpr Pickup PICKUP_BANDAGE("Bandage", Pickup::Type::StandaloneItem, 4);
	constexpr Pickup PICKUP_PENTAZEMIN("Pentazemin", Pickup::Type::StandaloneItem, 5, 5);
	constexpr Pickup PICKUP_DOGTAG("Dog Tag", Pickup::Type::StandaloneItem, 33);

	constexpr Pickup PICKUP_M9AMMO("M9 Ammo", Pickup::Type::WeaponAmmo, 1, 15);
	constexpr Pickup PICKUP_USPAMMO("USP Ammo", Pickup::Type::WeaponAmmo, 2, 15);
	constexpr Pickup PICKUP_SOCOMAMMO("SOCOM Ammo", Pickup::Type::WeaponAmmo, 3, 12);
	constexpr Pickup PICKUP_PSG1AMMO("PSG-1 Ammo", Pickup::Type::WeaponAmmo, 4, 20);
	constexpr Pickup PICKUP_RGB6AMMO("RGB6 Ammo", Pickup::Type::WeaponAmmo, 5, 6);
	constexpr Pickup PICKUP_NIKITAAMMO("Nikita Ammo", Pickup::Type::WeaponAmmo, 6, 10);
	constexpr Pickup PICKUP_STINGERAMMO("Stinger Ammo", Pickup::Type::WeaponAmmo, 7, 10);
	constexpr Pickup PICKUP_CLAYMORE("Claymore", Pickup::Type::StandaloneWeapon, 8, 4);
	constexpr Pickup PICKUP_C4("C4", Pickup::Type::StandaloneWeapon, 9, 4);
	constexpr Pickup PICKUP_CHAFFGRENADE("Chaff Grenade", Pickup::Type::StandaloneWeapon, 10, 2);
	constexpr Pickup PICKUP_STUNGRENADE("Stun Grenade", Pickup::Type::StandaloneWeapon, 11, 2);
	constexpr Pickup PICKUP_AKS74UAMMO("AK-74u Ammo", Pickup::Type::WeaponAmmo, 15, 30);
	constexpr Pickup PICKUP_MAGAZINE("Magazine", Pickup::Type::StandaloneWeapon, 16, 99);
	constexpr Pickup PICKUP_GRENADE("Grenade", Pickup::Type::StandaloneWeapon, 17, 2);
	constexpr Pickup PICKUP_M4AMMO("M4 Ammo", Pickup::Type::WeaponAmmo, 18, 30);
	constexpr Pickup PICKUP_PSG1TAMMO("PSG-1T Ammo", Pickup::Type::WeaponAmmo, 19, 5);
	constexpr Pickup PICKUP_BOOK("Book", Pickup::Type::StandaloneWeapon, 21);
	
	constexpr Pickup PICKUP_M9("M9", Pickup::Type::KeyWeapon, 1, PICKUP_M9AMMO.Ammo + 1, &PICKUP_M9AMMO);
	constexpr Pickup PICKUP_USP("USP", Pickup::Type::KeyWeapon, 2, PICKUP_USPAMMO.Ammo + 1, &PICKUP_USPAMMO);
	constexpr Pickup PICKUP_SOCOM("SOCOM", Pickup::Type::KeyWeapon, 3, PICKUP_SOCOMAMMO.Ammo + 1, &PICKUP_SOCOMAMMO);
	constexpr Pickup PICKUP_PSG1("PSG-1", Pickup::Type::KeyWeapon, 4, PICKUP_PSG1AMMO.Ammo + 1, &PICKUP_PSG1AMMO);
	constexpr Pickup PICKUP_RGB6("RGB6", Pickup::Type::KeyWeapon, 5, PICKUP_RGB6AMMO.Ammo, &PICKUP_RGB6AMMO);
	constexpr Pickup PICKUP_NIKITA("Nikita", Pickup::Type::KeyWeapon, 6, 20, &PICKUP_RATION);
	constexpr Pickup PICKUP_STINGER("Stinger", Pickup::Type::KeyWeapon, 7, 20, &PICKUP_STINGERAMMO);
	constexpr Pickup PICKUP_DMIC("D.Mic", Pickup::Type::KeyWeapon, 12);
	constexpr Pickup PICKUP_HFBLADE("HF Blade", Pickup::Type::KeyWeapon, 13);
	constexpr Pickup PICKUP_COOLANT("Coolant", Pickup::Type::KeyWeapon, 14);
	constexpr Pickup PICKUP_AKS74U("AKS-74u", Pickup::Type::KeyWeapon, 15, PICKUP_AKS74UAMMO.Ammo + 1, &PICKUP_AKS74UAMMO);
	constexpr Pickup PICKUP_M4("M4", Pickup::Type::KeyWeapon, 18, PICKUP_M4AMMO.Ammo + 1, &PICKUP_M4AMMO);
	constexpr Pickup PICKUP_PSG1T("PSG-1T", Pickup::Type::KeyWeapon, 19, PICKUP_PSG1TAMMO.Ammo + 1, &PICKUP_PSG1TAMMO);

	constexpr Pickup PICKUP_STEALTH("Stealth", Pickup::Type::KeyItem, 8);
	constexpr Pickup PICKUP_THERMALGOGGLES("Thermal Goggles", Pickup::Type::KeyItem, 13);
	constexpr Pickup PICKUP_BOX1("Box 1", Pickup::Type::KeyItem, 16, 25);
	constexpr Pickup PICKUP_CIGS("Cigs", Pickup::Type::KeyItem, 17);
	constexpr Pickup PICKUP_CAMERA("Camera", Pickup::Type::KeyItem, 21);
	constexpr Pickup PICKUP_DIGITALCAMERA("Digital Camera", Pickup::Type::KeyItem, 15);
	constexpr Pickup PICKUP_WETBOX("Wet Box", Pickup::Type::KeyItem, 24, 3);
	constexpr Pickup PICKUP_BANDANA("Bandana", Pickup::Type::KeyItem, 32);
	constexpr Pickup PICKUP_USPSUPPRESSOR("USP Supp.", Pickup::Type::KeyItem, 35);
	constexpr Pickup PICKUP_BDU("BDU", Pickup::Type::KeyItem, 6);
	constexpr Pickup PICKUP_BODYARMOR("Body Armor", Pickup::Type::KeyItem, 7);
	constexpr Pickup PICKUP_MINEDETECTOR("Mine Detector", Pickup::Type::KeyItem, 9);
	constexpr Pickup PICKUP_SENSORA("Sensor A", Pickup::Type::KeyItem, 10);
	constexpr Pickup PICKUP_SENSORB("Sensor B", Pickup::Type::KeyItem, 11);
	constexpr Pickup PICKUP_NVG("NVG", Pickup::Type::KeyItem, 12);
	constexpr Pickup PICKUP_SCOPE("Scope", Pickup::Type::KeyItem, 14);
	constexpr Pickup PICKUP_CARD("Card", Pickup::Type::KeyItem, 18); // generic card item
	constexpr Pickup PICKUP_CARD1("Card 1", Pickup::Type::KeyItem, 18, 1);
	constexpr Pickup PICKUP_CARD2("Card 2", Pickup::Type::KeyItem, 18, 2);
	constexpr Pickup PICKUP_CARD3("Card 3", Pickup::Type::KeyItem, 18, 3);
	constexpr Pickup PICKUP_CARD4("Card 4", Pickup::Type::KeyItem, 18, 4);
	constexpr Pickup PICKUP_CARD5("Card 5", Pickup::Type::KeyItem, 18, 5);
	constexpr Pickup PICKUP_SHAVER("Shaver", Pickup::Type::KeyItem, 19);
	constexpr Pickup PICKUP_PHONE("Phone", Pickup::Type::KeyItem, 20);
	constexpr Pickup PICKUP_BOX2("Box 2", Pickup::Type::KeyItem, 22, 25);
	constexpr Pickup PICKUP_BOX3("Box 3", Pickup::Type::KeyItem, 23, 25);
	constexpr Pickup PICKUP_APSENSOR("AP Sensor", Pickup::Type::KeyItem, 25);
	constexpr Pickup PICKUP_BOX4("Box 4", Pickup::Type::KeyItem, 26, 25);
	constexpr Pickup PICKUP_BOX5("Box 5", Pickup::Type::KeyItem, 27, 25);
	constexpr Pickup PICKUP_SOCOMSUPPRESSOR("SOCOM Supp.", Pickup::Type::KeyItem, 29, 1, &PICKUP_SOCOMAMMO);
	constexpr Pickup PICKUP_AKSUPPRESSOR("AK Supp.", Pickup::Type::KeyItem, 30, 1, &PICKUP_AKS74UAMMO);
	constexpr Pickup PICKUP_MODISC("MO Disc", Pickup::Type::KeyItem, 34);
	constexpr Pickup PICKUP_INFINITYWIG("Infinity Wig", Pickup::Type::KeyItem, 36);
	constexpr Pickup PICKUP_BLUEWIG("Blue Wig", Pickup::Type::KeyItem, 37);
	constexpr Pickup PICKUP_ORANGEWIG("Orange Wig", Pickup::Type::KeyItem, 38);


	std::unordered_map<MGS2::Stage, std::vector<const Pickup*>> Loadouts{
		{ Stage::Tanker, { &PICKUP_M9, &PICKUP_CAMERA, &PICKUP_CIGS } }, // also magazine (difficulty-dependent)
		{ Stage::Plant, { &PICKUP_SCOPE } },
		{ Stage::TalesA, {} },
		{ Stage::TalesB, {} },
		{ Stage::TalesC, {} },
		{ Stage::TalesD, {} },
		{ Stage::TalesE, {} },
	};

	std::unordered_map<MGS2::Stage, std::vector<RequiredKeyPickup>> RequiredKeyPickups{
		{ Stage::Tanker, {
			{ 16, 0, { &PICKUP_M9 }, { &PICKUP_M9, &PICKUP_USP } },
			{ 29, 0, { &PICKUP_USP } },
			{ 43, 0, { &PICKUP_CAMERA } },
		} },
		{ Stage::Plant, {
			// cards
			{ 92, 0, { &PICKUP_CARD1 } },
			{ 148, 0, { &PICKUP_CARD2 } },
			{ 182, 0, { &PICKUP_CARD3 } },
			{ 241, 0, { &PICKUP_CARD4 } },
			// bomb disposal (during)
			{ 92, 0, { &PICKUP_COOLANT } },
			// fatman (before) - any chance to allow no weapon?
			{ 116, 0, { &PICKUP_M9, &PICKUP_SOCOM } },
			// s1c1f elevator (before/including s1c1f) - TODO limitation not necessary i think
			{ 148, 0, { &PICKUP_AKS74U } },
			{ 148, 0, { &PICKUP_BDU } },
			// ames (154=inside, 152=b1)
			{ 152, 0, { &PICKUP_DMIC } },
			// sniping (during)
			{ 182, 0, { &PICKUP_PSG1 } },
			// harrier (before)
			{ 182, 0, { &PICKUP_STINGER }, { &PICKUP_STINGER, &PICKUP_RGB6 }, { &PICKUP_STINGER, &PICKUP_RGB6, &PICKUP_SOCOM } },
			// prez (during)
			{ 203, 0, { &PICKUP_NIKITA } },
			// Vamp 2: already guaranteed to have the PSG1
			// blade demo (jejunum) ???
			{ 379, 0, { &PICKUP_HFBLADE } },
			// rays (during) ???
			{ 411, 0, { &PICKUP_STINGER }, { &PICKUP_STINGER, &PICKUP_RGB6 } },
		} },
		{ Stage::TalesA, {} }, // M9/USP Fatman, Dmic Jennifer(?)
		{ Stage::TalesB, {} }, // Stinger Harrier(?)
		{ Stage::TalesC, {} }, // USP/M4 sensors
		{ Stage::TalesD, {} }, // Nikita
		{ Stage::TalesE, {} }, // ???
	};



	std::unordered_map<MGS2::Stage, std::vector<Location>> PickupLocations{
		{ Stage::Tanker, {
			// Cutscene items
			{ .OriginalPickup = &PICKUP_M9, .MinProgress = 9, .Type = Location::Types::Loadout, .AmmoOverride = Location::AmmoOverrides::ByDifficulty },
			{ .OriginalPickup = &PICKUP_CAMERA, .MinProgress = 9, .Type = Location::Types::Loadout },
			{ .OriginalPickup = &PICKUP_CIGS, .MinProgress = 9, .Type = Location::Types::Loadout },
			{ .OriginalPickup = &PICKUP_USP, .MinProgress = 29, .Type = Location::Types::Cutscene, .AmmoOverride = Location::AmmoOverrides::NoAmmo },
			// World items
			{ &PICKUP_RATION, "w00a", -18500, 1, -17500, 15, 0, 14, 30 },
			{ &PICKUP_PENTAZEMIN, "w00a", 18500, 1, -17500, 63, 0, 14, 30 },
			{ &PICKUP_CHAFFGRENADE, "w00a", 10000, 2001, -500, 63, 0, 14, 30 },
			{ &PICKUP_BANDAGE, "w00a", 16000, 3001, -17000, 63, 0, 14, 30 },
			{ &PICKUP_RATION, "w00a", 5000, 1, 18750, 1, 0, 14, 30 },
			{ &PICKUP_BANDAGE, "w00a", -7750, 1, 4500, 63, 0, 14, 30 },
			{ &PICKUP_MEDICINE, "w00a", 0, 6001, 5500, 56, 0, 14, 32 },
			{ &PICKUP_M9AMMO, "w00b", -11500, 12001, -13500, 63, 0, 25, 25, true },
			{ &PICKUP_M9AMMO, "w00b", -6600, 12001, -19250, 63, 0, 25, 25, true },
			{ &PICKUP_RATION, "w00b", -11250, 12001, -10750, 3, 0, 25, 25, true },
			{ &PICKUP_RATION, "w00b", -4550, 12001, -9500, 12, 0, 25, 25, true },
			{ &PICKUP_WETBOX, "w00c", 18000, 12751, -15000, 63, 0, 29, 30 },
			{ &PICKUP_THERMALGOGGLES, "w00c", 750, 27501, -13000, 63, 0, 29, 30 },
			{ &PICKUP_RATION, "w00c", -11500, 12001, -18750, 15, 0, 29, 30 },
			{ &PICKUP_RATION, "w01a", -4520, 201, 3550, 15, 0, 15, 30 },
			{ &PICKUP_M9AMMO, "w01a", 4500, 201, 3550, 63, 0, 15, 30 },
			{ &PICKUP_M9AMMO, "w01b", 11000, 3001, -14750, 63, 0, 15, 30 },
			{ &PICKUP_RATION, "w01b", 3500, 3001, 2250, 15, 0, 15, 30 },
			{ &PICKUP_USPAMMO, "w01b", 1000, 3201, 5500, 63, 0, 15, 30 },
			{ &PICKUP_RATION, "w01c", 9500, 6001, -16250, 7, 0, 15, 30 },
			{ &PICKUP_CHAFFGRENADE, "w01c", -1500, 6201, -18160, 63, 0, 15, 30 },
			{ &PICKUP_RATION, "w01d", -10000, 9001, -18500, 15, 0, 15, 30 },
			{ &PICKUP_USPAMMO, "w01d", -9250, 9001, -13000, 63, 0, 15, 30 },
			{ &PICKUP_M9AMMO, "w01d", -4750, 9001, -13000, 63, 0, 15, 30 },
			{ &PICKUP_USPAMMO, "w01d", -2500, 9001, -11875, 63, 0, 15, 30 },
			{ &PICKUP_M9AMMO, "w01d", 7700, 9001, -11850, 63, 0, 15, 30 },
			{ &PICKUP_BOX1, "w01d", 10000, 9001, -10000, 63, 0, 15, 30 },
			{ &PICKUP_RATION, "w01e", -4500, 12001, -15750, 3, 0, 22, 30 },
			{ &PICKUP_USPAMMO, "w01e", 4817, 12001, -18682, 63, 0, 22, 30 },
			{ &PICKUP_M9AMMO, "w01f", 9000, 1, -18500, 63, 0, 15, 30 },
			{ &PICKUP_RATION, "w01f", -8600, 1, -18250, 7, 0, 15, 30 },
			{ &PICKUP_USPAMMO, "w01f", -12500, -4999, -12500, 63, 0, 15, 30 },
			{ &PICKUP_STUNGRENADE, "w01f", 12500, -4999, -12500, 15, 0, 15, 30 },
			{ &PICKUP_M9AMMO, "w02a", 7000, -12999, -19500, 63, 0, 29, 30 },
			{ &PICKUP_RATION, "w02a", 2250, -13999, -18000, 15, 0, 29, 30 },
			{ &PICKUP_USPAMMO, "w02a", -5500, -12999, -21500, 63, 0, 29, 30 },
			{ &PICKUP_GRENADE, "w02a", -12000, -12998, 1000, 63, 0, 29, 30 },
			{ &PICKUP_USPAMMO, "w02a", -6500, -8999, -10000, 63, 0, 29, 30 },
			{ &PICKUP_USPAMMO, "w02a", -16000, -3999, -22250, 63, 0, 29, 30 },
			{ &PICKUP_USPAMMO, "w02a", 16224, -3998, -22618, 63, 0, 29, 30 },
			{ &PICKUP_USPAMMO, "w02a", -10000, -3999, -22500, 63, 0, 29, 30, true },
			{ &PICKUP_RATION, "w03a", -12250, -4999, -42000, 15, 0, 30, 30 },
			{ &PICKUP_USPAMMO, "w03a", -14000, -4999, -68500, 63, 0, 30, 30 },
			{ &PICKUP_USPAMMO, "w03a", -4000, -4999, -128000, 63, 0, 30, 30 },
			{ &PICKUP_USPAMMO, "w03a", -17500, -4999, -125750, 63, 0, 30, 30 },
			{ &PICKUP_USPAMMO, "w03a", 10500, -4999, -126500, 63, 0, 30, 30 },
			{ &PICKUP_USPAMMO, "w03b", 15750, -4999, -128500, 63, 0, 30, 30 },
			{ &PICKUP_USPAMMO, "w03b", 12750, -4999, -100250, 63, 0, 30, 30 },
			{ &PICKUP_RATION, "w03b", 13000, -4999, -97250, 15, 0, 30, 30 },
			{ &PICKUP_M9AMMO, "w03b", 12300, -4999, -128750, 63, 0, 30, 30 },
			{ &PICKUP_USPAMMO, "w03b", 17250, -4999, -87000, 63, 0, 32, 32, true },
			{ &PICKUP_USPAMMO, "w03b", 17250, -4999, -86000, 63, 0, 32, 32, true },
			{ &PICKUP_RATION, "w03b", 16250, -4999, -87000, 31, 0, 32, 32, true },
			{ &PICKUP_M9AMMO, "w04a", -11000, -8999, -500, 63, 0, 42, 43 },
			{ &PICKUP_M9AMMO, "w04a", 7000, -4999, 1750, 63, 0, 42, 43 },
			{ &PICKUP_RATION, "w04a", 10000, -4998, 3250, 15, 0, 42, 43 },
			{ &PICKUP_THERMALGOGGLES, "w04a", -10500, -4998, -3500, 63, 0, 42, 43 },
			{ &PICKUP_RATION, "w04a", 11000, -8999, -23500, 7, 0, 42, 43, true },
			{ &PICKUP_RATION, "w04a", -11000, -8999, -23500, 7, 0, 42, 43 },
		} },
		{ Stage::Plant, {
			// Cards
			{.OriginalPickup{&PICKUP_CARD1}, .MinProgress{92}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_CARD2}, .MinProgress{148}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_CARD3}, .MinProgress{176}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_CARD4}, .MinProgress{227}, .Type{Location::Types::Cutscene}},
			// Cutscene items
			{.OriginalPickup{&PICKUP_SCOPE}, .MinProgress{9}, .Type{Location::Types::Loadout}},
			{.OriginalPickup{&PICKUP_SOCOM}, .MinProgress{58}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_CIGS}, .MinProgress{58}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_COOLANT}, .MinProgress{92}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_SENSORA}, .MinProgress{92}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_BDU}, .MinProgress{148}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_PHONE}, .MinProgress{148}, .Type{Location::Types::Cutscene}},
			{.OriginalPickup{&PICKUP_MODISC}, .MinProgress{227}, .Type{Location::Types::Cutscene}},
			//{.OriginalPickup{&PICKUP_CARD5}, .MinProgress{299}, .Type{Location::Types::Cutscene}}, // no point, you need emma for the one room it gets you into
			{.OriginalPickup{&PICKUP_HFBLADE}, .MinProgress{389}, .Type{Location::Types::Cutscene}},
			// World items
			{ &PICKUP_THERMALGOGGLES, "w11a", -5500, -54999, 26500, 63, 0, 9, 21 },
			{ &PICKUP_RATION, "w11a", -15750, -44799, 14250, 15, 0, 9, 21 },
			{ &PICKUP_RATION, "w11a", -3750, -45999, 16875, 15, 0, 9, 21 },
			{ &PICKUP_M9, "w11a", -18000, -45999, 20250, 1, 0, 9, 21 },
			{ &PICKUP_M9AMMO, "w11a", -7750, -43999, 15125, 3, 0, 9, 21 },
			{ &PICKUP_M9AMMO, "w11a", -8625, -43999, 14375, 3, 0, 9, 21 },
			{ &PICKUP_RATION, "w11a", -5500, -43999, -3500, 15, 0, 9, 21 },
			{ &PICKUP_SHAVER, "w11a", -10000, -45999, 19000, 56, 0, 9, 21 },
			{ &PICKUP_THERMALGOGGLES, "w11b", -5500, -54999, 26500, 63, 1, 104, 108 },
			{ &PICKUP_RATION, "w11b", -15750, -44799, 14250, 15, 1, 104, 108 },
			{ &PICKUP_RATION, "w11b", -3750, -45999, 16875, 15, 1, 104, 108 },
			{ &PICKUP_M9, "w11b", -18000, -45999, 20250, 1, 1, 104, 108 },
			{ &PICKUP_M9AMMO, "w11b", -7750, -43999, 15125, 3, 1, 104, 108 },
			{ &PICKUP_M9AMMO, "w11b", -8625, -43999, 14375, 3, 1, 104, 108 },
			{ &PICKUP_RATION, "w11b", -5500, -44999, -1250, 15, 1, 104, 108 },
			{ &PICKUP_SOCOMAMMO, "w11b", 500, -44999, -500, 63, 1, 104, 108 },
			{ &PICKUP_SOCOMAMMO, "w11b", 5500, -44996, -1000, 63, 1, 104, 108 },
			{ &PICKUP_RATION, "w11c", -5500, -44999, -1250, 15, 1, 110, 110, true },
			{ &PICKUP_SOCOMAMMO, "w11c", 500, -44999, -500, 63, 1, 110, 110 },
			{ &PICKUP_SOCOMAMMO, "w11c", 5500, -44995, -1000, 63, 1, 110, 110, true },
			{ &PICKUP_CHAFFGRENADE, "w12a", 6000, 5001, -13000, 63, 0, 29, 62 },
			{ &PICKUP_BANDAGE, "w12a", 3000, 6001, -6000, 63, 0, 29, 62 },
			{ &PICKUP_M9AMMO, "w12a", -8000, 5001, -2500, 63, 0, 29, 62 },
			{ &PICKUP_M9AMMO, "w12a", -4800, 5001, -12175, 1, 0, 29, 62 },
			{ &PICKUP_BOX1, "w12b", -4750, 1, 11500, 63, 1, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w12b", -9500, 201, -4250, 63, 0, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w12b", -8500, 1, 1000, 63, 1, 29, 185 },
			{ &PICKUP_M9AMMO, "w12b", 0, 1, -6250, 63, 0, 29, 185 },
			{ &PICKUP_M9AMMO, "w12b", 6250, 5001, -7875, 63, 0, 29, 185 },
			{ &PICKUP_RATION, "w12b", 10375, 201, -4250, 15, 0, 29, 185 },
			{ &PICKUP_BANDAGE, "w12b", -4500, 1, 5500, 63, 1, 29, 185 },
			{ &PICKUP_RATION, "w12b", -1500, 1, 7250, 3, 1, 29, 185 },
			{ &PICKUP_CHAFFGRENADE, "w12c", 6000, 5001, -13000, 63, 1, 102, 185 },
			{ &PICKUP_BANDAGE, "w12c", 3000, 6001, -6000, 63, 1, 102, 185 },
			{ &PICKUP_M9AMMO, "w12c", -8000, 5001, -2500, 63, 1, 102, 185 },
			{ &PICKUP_M9, "w12c", -4800, 5001, -12175, 3, 1, 102, 185 },
			{ &PICKUP_SOCOMSUPPRESSOR, "w14a", -52000, 1, -23750, 1, 0, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w14a", -48750, -1999, -24500, 63, 0, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w14a", -50500, 201, -43125, 63, 0, 29, 185 },
			{ &PICKUP_M9AMMO, "w14a", -46625, 2, -34875, 63, 0, 29, 185 },
			{ &PICKUP_RATION, "w14a", -49500, 201, -43125, 15, 0, 29, 185 },
			{ &PICKUP_CHAFFGRENADE, "w15a", -47750, 1, -60000, 63, 0, 29, 185 },
			{ &PICKUP_CHAFFGRENADE, "w15b", -47750, 1, -60000, 63, 0, 92, 185 },
			{ &PICKUP_M9AMMO, "w16a", -43750, 1, -85875, 63, 0, 29, 62 },
			{ &PICKUP_PENTAZEMIN, "w16a", -47125, 1, -85875, 63, 0, 29, 62 },
			{ &PICKUP_SOCOMAMMO, "w16a", -60500, 1, -85875, 63, 0, 92, 185 },
			{ &PICKUP_SOCOMAMMO, "w16b", -60500, 1, -85875, 63, 0, 92, 185 },
			{ &PICKUP_RATION, "w16b", -60250, 1, -99500, 15, 0, 92, 185 },
			{ &PICKUP_M9AMMO, "w16b", -43750, 1, -85875, 63, 0, 92, 185 },
			{ &PICKUP_PENTAZEMIN, "w16b", -47125, 1, -85875, 63, 0, 92, 185 },
			{ &PICKUP_SENSORB, "w16b", -48250, 1, -104000, 63, 0, 100, 185 },
			{ &PICKUP_M9AMMO, "w18a", -6500, -3999, -133500, 63, 1, 29, 185 },
			{ &PICKUP_RATION, "w18a", 1250, -998, -119250, 15, 1, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w18a", -8500, -999, -109500, 63, 1, 29, 185 },
			{ &PICKUP_PSG1AMMO, "w18a", 8500, -999, -109500, 63, 1, 29, 185 },
			{ &PICKUP_STUNGRENADE, "w19a", 13250, -4499, 2750, 63, 1, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w19a", 12750, -4499, 3500, 63, 1, 29, 185 },
			{ &PICKUP_M9AMMO, "w20a", 52000, 1, -87500, 63, 1, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w20a", 48000, 1, -100000, 63, 1, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w20a", 56000, 1, -100000, 63, 1, 29, 185 },
			{ &PICKUP_M4AMMO, "w20a", 53500, 1, -84750, 63, 1, 29, 185 },
			{ &PICKUP_STUNGRENADE, "w20a", 51500, 1, -92000, 63, 1, 29, 185 },
			{ &PICKUP_BOX5, "w20a", 57500, 1001, -94500, 63, 1, 29, 185 },
			{ &PICKUP_RATION, "w20a", 42750, -4499, -86000, 15, 1, 29, 185 },
			{ &PICKUP_MINEDETECTOR, "w20a", 46500, -4499, -87000, 63, 1, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w20a", 53000, -4499, -93000, 63, 5, 327, 327 },
			{ &PICKUP_PSG1TAMMO, "w20a", 55500, 1, -81500, 63, 5, 327, 327 },
			{ &PICKUP_STINGERAMMO, "w20a", 50000, 1, -96500, 63, 5, 327, 327 },
			{ &PICKUP_M9AMMO, "w20a", 52000, 1, -87500, 63, 5, 327, 327 },
			{ &PICKUP_SOCOMAMMO, "w20a", 48000, 1, -100000, 63, 5, 327, 327 },
			{ &PICKUP_SOCOMAMMO, "w20a", 56000, 1, -100000, 63, 5, 327, 327 },
			{ &PICKUP_M4AMMO, "w20a", 53500, 1, -84750, 63, 5, 327, 327 },
			{ &PICKUP_STUNGRENADE, "w20a", 51500, 1, -92000, 63, 5, 327, 327 },
			{ &PICKUP_RATION, "w20a", 42750, -4499, -86000, 15, 5, 327, 327 },
			{ &PICKUP_BOX5, "w20a", 57500, 1001, -94500, 63, 5, 327, 327 },
			{ &PICKUP_MINEDETECTOR, "w20a", 46500, -4499, -87000, 63, 5, 327, 327 },
			{ &PICKUP_BOX3, "w20b", 51500, 11501, -84400, 63, 1, 29, 102 },
			{ &PICKUP_STUNGRENADE, "w20b", 47000, 11501, -96250, 63, 1, 29, 102 },
			{ &PICKUP_CLAYMORE, "w20b", 42000, 5501, -99000, 63, 1, 29, 102 },
			{ &PICKUP_BOX3, "w20c", 51500, 11501, -84400, 63, 1, 116, 118 },
			{ &PICKUP_STUNGRENADE, "w20c", 47000, 11501, -96250, 63, 1, 116, 118 },
			{ &PICKUP_CLAYMORE, "w20c", 42000, 5501, -99000, 63, 1, 116, 116 },
			{ &PICKUP_SOCOMAMMO, "w20c", 59000, 11501, -80500, 63, 1, 116, 118, true },
			{ &PICKUP_SOCOMAMMO, "w20c", 63625, 11501, -90750, 63, 1, 118, 118, true },
			{ &PICKUP_SOCOMAMMO, "w20c", 49375, 11501, -101750, 63, 1, 118, 118 },
			{ &PICKUP_SOCOMAMMO, "w20c", 46000, 11501, -81875, 63, 1, 118, 118, true },
			{ &PICKUP_M9AMMO, "w20c", 66250, 11501, -103500, 63, 1, 118, 118, true },
			{ &PICKUP_RATION, "w20c", 65750, 11501, -83500, 15, 1, 118, 118, true },
			{ &PICKUP_BOX3, "w20d", 51500, 11501, -84400, 63, 1, 148, 185 },
			{ &PICKUP_STUNGRENADE, "w20d", 47000, 11501, -96250, 63, 1, 148, 185 },
			{ &PICKUP_CLAYMORE, "w20d", 42000, 5501, -99000, 63, 1, 148, 185 },
			{ &PICKUP_AKS74UAMMO, "w21a", 23000, -1499, -61250, 63, 1, 29, 185 },
			{ &PICKUP_AKS74UAMMO, "w21b", 23000, -1499, -61250, 63, 5, 327, 327 },
			{ &PICKUP_CHAFFGRENADE, "w21b", 58250, 1, -45250, 63, 5, 327, 327 },
			{ &PICKUP_RGB6, "w22a", 45875, 1, -39375, 63, 3, 29, 185 },
			{ &PICKUP_RGB6AMMO, "w22a", 47000, 1001, -38250, 63, 3, 29, 185 },
			{ &PICKUP_RGB6AMMO, "w22a", 49750, 1, -40250, 63, 3, 29, 185 },
			{ &PICKUP_PSG1TAMMO, "w22a", 49940, 201, -37250, 63, 3, 29, 185 },
			{ &PICKUP_C4, "w22a", 54250, 1, -39500, 63, 2, 29, 185 },
			{ &PICKUP_C4, "w22a", 54060, 251, -38000, 63, 2, 29, 185 },
			{ &PICKUP_C4, "w22a", 58000, 1, -40000, 63, 2, 29, 185 },
			{ &PICKUP_CLAYMORE, "w22a", 54060, 251, -37000, 63, 2, 29, 185 },
			{ &PICKUP_M4, "w22a", 43750, 1, -32000, 63, 2, 29, 185 },
			{ &PICKUP_M4AMMO, "w22a", 42500, 1, -31500, 63, 2, 29, 185 },
			{ &PICKUP_M4AMMO, "w22a", 41560, 201, -28500, 63, 2, 29, 185 },
			{ &PICKUP_PSG1, "w22a", 60000, 1, -32000, 63, 3, 29, 185 },
			{ &PICKUP_PSG1AMMO, "w22a", 58250, 1, -30250, 63, 3, 29, 185 },
			{ &PICKUP_PSG1AMMO, "w22a", 58060, 201, -31250, 63, 3, 29, 185 },
			{ &PICKUP_PSG1T, "w22a", 66750, 1, -31000, 63, 3, 29, 185 },
			{ &PICKUP_GRENADE, "w22a", 48250, 1, -23250, 63, 3, 29, 185 },
			{ &PICKUP_GRENADE, "w22a", 47250, 1, -22000, 63, 3, 29, 185 },
			{ &PICKUP_GRENADE, "w22a", 48250, 1, -20875, 63, 3, 29, 185 },
			{ &PICKUP_PSG1TAMMO, "w22a", 46250, 201, -23940, 63, 3, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w22a", 49500, 1, -19750, 63, 3, 29, 185 },
			{ &PICKUP_M9, "w22a", 55250, 1, -23250, 63, 0, 29, 185 },
			{ &PICKUP_M9AMMO, "w22a", 56500, 1, -23750, 63, 0, 29, 185 },
			{ &PICKUP_M9AMMO, "w22a", 57500, 1, -22250, 63, 0, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w22a", 63000, 1, -25000, 63, 0, 29, 185 },
			{ &PICKUP_RATION, "w22a", 58750, 1, -18000, 15, 0, 29, 185 },
			{ &PICKUP_AKS74U, "w22a", 57125, -4999, -44250, 63, 2, 29, 185 },
			{ &PICKUP_AKS74UAMMO, "w22a", 46875, -4999, -44250, 63, 2, 29, 185 },
			{ &PICKUP_AKS74UAMMO, "w22a", 50000, -4999, -36250, 63, 2, 29, 185 },
			{ &PICKUP_AKS74UAMMO, "w22a", 54000, -4999, -36250, 63, 2, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w22a", 51250, -4999, -44375, 63, 2, 29, 185 },
			{ &PICKUP_M4AMMO, "w22a", 56750, -4999, -40500, 63, 2, 29, 185 },
			{ &PICKUP_AKSUPPRESSOR, "w22a", 46750, -4999, -40375, 1, 2, 29, 185 },
			{ &PICKUP_RATION, "w22a", 45560, -4799, -20500, 15, 1, 29, 185 },
			{ &PICKUP_PENTAZEMIN, "w22a", 48250, -4799, -23940, 63, 1, 29, 185 },
			{ &PICKUP_BOOK, "w22a", 45560, -4799, -21500, 63, 1, 29, 185 },
			{ &PICKUP_BOX2, "w22a", 46500, -4999, -18750, 63, 1, 29, 185 },
			{ &PICKUP_M9AMMO, "w22a", 57500, -4999, -23250, 63, 1, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w22a", 54250, -4999, -19375, 63, 1, 29, 185 },
			{ &PICKUP_MINEDETECTOR, "w22a", 55375, -4999, -23625, 63, 1, 29, 185 },
			{ &PICKUP_SOCOMSUPPRESSOR, "w22a", 57625, -4999, -20375, 63, 1, 29, 185 },
			{ &PICKUP_BOOK, "w22a", 53000, -2999, -29000, 63, 0, 29, 185 },
			{ &PICKUP_STUNGRENADE, "w22a", 52500, -4999, -33250, 63, 0, 29, 185 },
			{ &PICKUP_CHAFFGRENADE, "w22a", 51000, -4999, -18625, 63, 0, 29, 185 },
			{ &PICKUP_M9AMMO, "w22a", 59000, -2999, -26000, 63, 0, 29, 185 },
			{ &PICKUP_SOCOMAMMO, "w22a", 58625, -3999, -28500, 63, 0, 29, 185 },
			{ &PICKUP_CLAYMORE, "w22a", 58000, 1, -37375, 63, 2, 29, 185 },
			{ &PICKUP_M4AMMO, "w22a", 44500, 1, -31000, 63, 2, 29, 185 },
			{ &PICKUP_M4AMMO, "w22a", 45500, 1, -28250, 63, 2, 29, 185 },
			{ &PICKUP_CHAFFGRENADE, "w23a", -13000, -4499, 2000, 1, 0, 29, 185 },
			{ &PICKUP_CHAFFGRENADE, "w23b", -13000, -4499, 2000, 63, 0, 92, 185 },
			{ &PICKUP_BOOK, "w24a", -750, 201, -45800, 63, 2, 148, 185 },
			{ &PICKUP_SOCOMSUPPRESSOR, "w24a", 2250, 201, -45825, 63, 2, 148, 185 },
			{ &PICKUP_M4AMMO, "w24a", -3750, 201, -53250, 63, 2, 148, 185 },
			{ &PICKUP_M9AMMO, "w24a", -3750, 201, -51250, 63, 2, 148, 185 },
			{ &PICKUP_CHAFFGRENADE, "w24a", -1000, 1001, -52000, 63, 2, 148, 185 },
			{ &PICKUP_SOCOMAMMO, "w24a", -18500, -1498, -55750, 63, 2, 148, 185 },
			{ &PICKUP_C4, "w24a", -1750, 201, -45825, 63, 2, 148, 185 },
			{ &PICKUP_CLAYMORE, "w24a", 1250, 201, -45825, 63, 2, 148, 185 },
			{ &PICKUP_AKS74U, "w24a", 2500, 1, -49750, 1, 2, 148, 185 },
			{ &PICKUP_M9AMMO, "w24a", 15750, -1498, -55750, 63, 2, 180, 185 },
			{ &PICKUP_SOCOMAMMO, "w24a", 17250, -1498, -55750, 63, 2, 180, 185 },
			{ &PICKUP_BOOK, "w24a", -750, 201, -45800, 63, 5, 327, 327 },
			{ &PICKUP_M4AMMO, "w24a", -3750, 201, -53250, 63, 5, 327, 327 },
			{ &PICKUP_M9AMMO, "w24a", -3750, 201, -51250, 63, 5, 327, 327 },
			{ &PICKUP_CHAFFGRENADE, "w24a", -1000, 1001, -52000, 63, 5, 327, 327 },
			{ &PICKUP_SOCOMAMMO, "w24a", -18500, -1498, -55750, 63, 5, 327, 327 },
			{ &PICKUP_C4, "w24a", -1750, 201, -45825, 63, 5, 327, 327 },
			{ &PICKUP_CLAYMORE, "w24a", 1250, 201, -45825, 63, 5, 327, 327 },
			{ &PICKUP_SOCOMSUPPRESSOR, "w24a", 2250, 201, -45825, 63, 5, 327, 327 },
			{ &PICKUP_RATION, "w24b", 5750, -4299, -73750, 15, 2, 152, 185 },
			{ &PICKUP_M4AMMO, "w24b", -11250, -4497, -64000, 63, 2, 152, 185 },
			{ &PICKUP_SOCOMAMMO, "w24b", -8500, -4299, -69100, 63, 2, 152, 185 },
			{ &PICKUP_STUNGRENADE, "w24b", -10500, -4299, -69100, 63, 2, 152, 185 },
			{ &PICKUP_THERMALGOGGLES, "w24c", -10000, -7499, -54500, 63, 2, 154, 154 },
			{ &PICKUP_RATION, "w24c", 7750, -8497, -51000, 15, 2, 154, 154 },
			{ &PICKUP_BANDAGE, "w24c", -11000, -8497, -48500, 63, 2, 154, 154 },
			{ &PICKUP_BOOK, "w24d", -8600, -13749, -48750, 63, 2, 150, 185 },
			{ &PICKUP_DMIC, "w24d", -3500, -15999, -57500, 63, 2, 150, 185 },
			{ &PICKUP_RATION, "w24d", 7250, -15999, -65500, 15, 2, 150, 185 },
			{ &PICKUP_M9AMMO, "w24d", 8625, -15799, -53000, 63, 2, 150, 185 },
			{ &PICKUP_AKS74UAMMO, "w24d", 4500, -15999, -55000, 63, 2, 150, 185 },
			{ &PICKUP_BANDAGE, "w24d", 0, -15999, -51250, 63, 2, 150, 185 },
			{ &PICKUP_SOCOMAMMO, "w24d", -8625, -15799, -50000, 63, 2, 150, 185 },
			{ &PICKUP_M4AMMO, "w24d", -8625, -15799, -52000, 63, 2, 150, 185 },
			{ &PICKUP_BOX4, "w24d", 3750, -15999, -55000, 63, 2, 150, 185 },
			{ &PICKUP_BOOK, "w24d", -8250, -13749, -48750, 63, 2, 150, 185, true },
			{ &PICKUP_SOCOMAMMO, "w25a", -1500, 501, -137000, 63, 3, 182, 185, true },
			{ &PICKUP_PENTAZEMIN, "w25a", -3000, 501, -137000, 63, 3, 182, 185 },
			{ &PICKUP_STINGER, "w25a", 2125, -999, -147500, 63, 3, 189, 189 },
			{ &PICKUP_STINGERAMMO, "w25a", -2125, -999, -145000, 63, 3, 189, 189 },
			{ &PICKUP_PSG1AMMO, "w25a", 3000, 501, -137000, 63, 3, 189, 189, true },
			{ &PICKUP_RATION, "w25a", 2358, 1, -150400, 15, 3, 189, 189 },
			{ &PICKUP_PSG1AMMO, "w25b", -1500, -5780, -156000, 63, 3, 193, 193 },
			{ &PICKUP_PSG1AMMO, "w25b", 499, -3688, -163597, 63, 3, 193, 193 },
			{ &PICKUP_RATION, "w25b", 8375, -3889, -145000, 15, 3, 193, 193 },
			{ &PICKUP_AKSUPPRESSOR, "w25b", -2000, -4111, -163125, 63, 3, 193, 193 },
			{ &PICKUP_RATION, "w25b", -3250, -44999, -147000, 15, 3, 193, 193 },
			{ &PICKUP_PSG1AMMO, "w25c", 50500, 1, -253000, 63, 3, 193, 193 },
			{ &PICKUP_CHAFFGRENADE, "w25c", 53500, 1, -253000, 63, 3, 193, 193 },
			{ &PICKUP_SOCOMAMMO, "w25c", 53250, 1, -227000, 63, 3, 193, 193 },
			{ &PICKUP_RATION, "w25c", 59250, -2998, -253000, 15, 3, 193, 193 },
			{ &PICKUP_AKS74UAMMO, "w25c", 59000, -2998, -226500, 63, 3, 193, 193 },
			{ &PICKUP_PSG1AMMO, "w25d", 23000, -1499, -241500, 63, 4, 297, 299 },
			{ &PICKUP_CHAFFGRENADE, "w25d", 23000, -1499, -238500, 63, 4, 297, 299 },
			{ &PICKUP_RATION, "w25d", 20000, -1499, -241250, 15, 4, 297, 299 },
			{ &PICKUP_M4AMMO, "w25d", 58750, -2996, -236000, 63, 4, 297, 299 },
			{ &PICKUP_SOCOMAMMO, "w25d", 59000, -2996, -253000, 63, 4, 297, 299 },
			{ &PICKUP_SOCOMAMMO, "w28a", 56750, 2, -222250, 63, 5, 299, 299 },
			{ &PICKUP_M9AMMO, "w31a", -6500, -1999, -228500, 63, 3, 203, 299 },
			{ &PICKUP_SOCOMAMMO, "w31a", 3000, -999, -228500, 63, 3, 203, 299 },
			{ &PICKUP_M4AMMO, "w31a", 8500, 2, -252000, 63, 3, 203, 299 },
			{ &PICKUP_M4AMMO, "w31a", -4500, 1, -243750, 63, 3, 203, 299 },
			{ &PICKUP_NIKITAAMMO, "w31a", -5500, 1001, -231750, 63, 3, 203, 299, true },
			{ &PICKUP_NIKITAAMMO, "w31a", 6500, -1999, -228500, 63, 3, 203, 299, true },
			{ &PICKUP_CHAFFGRENADE, "w31a", -18000, 2, -236000, 63, 3, 203, 299 },
			{ &PICKUP_RATION, "w31a", -12500, 2, -231500, 15, 3, 203, 299 },
			{ &PICKUP_RGB6, "w31a", 6500, 2, -252000, 63, 3, 203, 299 },
			{ &PICKUP_M4, "w31a", -18000, 2, -234000, 63, 3, 203, 299 },
			{ &PICKUP_SOCOMAMMO, "w31a", 625, 1, -246250, 63, 3, 227, 299 },
			{ &PICKUP_STINGERAMMO, "w31a", -2000, 251, -238250, 63, 3, 227, 299 },
			{ &PICKUP_BOOK, "w31a", -3750, 251, -237750, 63, 3, 227, 299 },
			{ &PICKUP_RATION, "w31b", 11500, -10499, -249500, 1, 3, 203, 299 },
			{ &PICKUP_M4AMMO, "w31b", 9500, -10499, -237500, 63, 3, 203, 299 },
			{ &PICKUP_NVG, "w31b", 4250, -10499, -244750, 63, 3, 203, 299 },
			{ &PICKUP_GRENADE, "w31b", -1500, -6249, -2335750, 63, 3, 203, 299 },
			{ &PICKUP_PSG1T, "w31b", -2000, -10499, -223000, 63, 3, 203, 299 },
			{ &PICKUP_RATION, "w31b", -9500, -10499, -222000, 15, 3, 203, 299 },
			{ &PICKUP_RGB6, "w31b", -1500, -10499, -249500, 63, 3, 203, 299 },
			{ &PICKUP_STINGERAMMO, "w31b", -2000, -10499, -223000, 63, 3, 203, 299 },
			{ &PICKUP_NIKITA, "w31b", 11500, -10499, -249500, 1, 3, 203, 299 },
			{ &PICKUP_NIKITA, "w31b", -750, -10499, -242750, 2, 4, 203, 299 },
			{ &PICKUP_RATION, "w31b", -750, -10499, -242750, 2, 4, 203, 299 },
			{ &PICKUP_NIKITA, "w31b", 5000, -10499, -236250, 60, 4, 203, 299 },
			{ &PICKUP_RATION, "w31b", 5000, -10499, -236250, 12, 4, 203, 299 },
			{ &PICKUP_M4AMMO, "w31b", -2750, -3999, -250750, 63, 4, 297, 299 },
			{ &PICKUP_M9AMMO, "w31c", -14000, -6249, -243250, 63, 4, 253, 253, true },
			{ &PICKUP_RGB6AMMO, "w31c", -4750, -6249, -244500, 63, 4, 253, 253, true },
			{ &PICKUP_SOCOMAMMO, "w31c", -14000, -6249, -235000, 63, 4, 253, 253, true },
			{ &PICKUP_RATION, "w31c", -6000, -6249, -235000, 15, 4, 253, 253, true },
			{ &PICKUP_AKS74UAMMO, "w31c", -6000, -6249, -242500, 63, 4, 253, 253, true },
			{ &PICKUP_AKS74UAMMO, "w31c", -6000, -6249, -241500, 63, 4, 253, 253, true },
			{ &PICKUP_RATION, "w31c", -10000, -39999, -244000, 15, 4, 253, 253 },
			{ &PICKUP_M4AMMO, "w31c", -6000, -6249, -242500, 63, 4, 253, 253, true },
			{ &PICKUP_M4AMMO, "w31c", -6000, -6249, -241500, 63, 4, 253, 253 },
			{ &PICKUP_M9AMMO, "w31d", -2000, 1, -245500, 63, 4, 297, 299 },
			{ &PICKUP_SOCOMAMMO, "w31d", -11500, 2, -252000, 63, 4, 297, 299 },
			{ &PICKUP_SOCOMAMMO, "w31d", -18000, 2, -234000, 63, 4, 297, 299 },
			{ &PICKUP_AKS74UAMMO, "w31d", 8500, 2, -252000, 63, 4, 297, 299 },
			{ &PICKUP_AKS74UAMMO, "w31d", 7500, 1, -243500, 63, 4, 297, 299 },
			{ &PICKUP_PSG1AMMO, "w31d", 4750, 1, -244500, 63, 4, 297, 299 },
			{ &PICKUP_PSG1TAMMO, "w31d", -2000, 251, -238250, 63, 4, 297, 299 },
			{ &PICKUP_RGB6AMMO, "w31d", -3750, 251, -237750, 63, 4, 297, 299 },
			{ &PICKUP_PENTAZEMIN, "w31d", -12500, 2, -226500, 63, 4, 297, 299 },
			{ &PICKUP_RATION, "w31d", -500, 1, -243500, 15, 4, 297, 299 },
			{ &PICKUP_M9AMMO, "w31f", -14000, -6249, -243250, 63, 4, 257, 299 },
			{ &PICKUP_RGB6AMMO, "w31f", -4750, -6249, -244500, 63, 4, 257, 299 },
			{ &PICKUP_SOCOMAMMO, "w31f", -14000, -6249, -235000, 63, 4, 257, 299 },
			{ &PICKUP_AKS74UAMMO, "w31f", -6000, -6249, -242500, 63, 4, 257, 299 },
			{ &PICKUP_RATION, "w31f", -6000, -6249, -235000, 15, 4, 257, 299 },
			{ &PICKUP_AKS74UAMMO, "w31f", -6000, -6249, -241500, 63, 4, 257, 299 },
			{ &PICKUP_RATION, "w31f", -10000, -39999, -244000, 15, 4, 257, 299 },
			{ &PICKUP_RATION, "w31f", -5000, -6249, -248250, 15, 4, 257, 299 },
			{ &PICKUP_AKS74UAMMO, "w31f", -18000, -10749, -252500, 63, 4, 257, 299 },
			{ &PICKUP_PSG1TAMMO, "w31f", -6250, -10749, -246750, 63, 4, 257, 299 },
			{ &PICKUP_C4, "w31f", -18250, -6949, -258500, 63, 4, 257, 299 },
			{ &PICKUP_C4, "w31f", -18250, -6949, -257500, 63, 4, 257, 299 },
			{ &PICKUP_BOOK, "w31f", -18250, -6949, -253500, 63, 4, 257, 299 },
			{ &PICKUP_PENTAZEMIN, "w31f", -15125, -6999, -252125, 63, 4, 257, 299 },
			{ &PICKUP_THERMALGOGGLES, "w31f", -11250, -6949, -256500, 63, 4, 257, 299 },
			{ &PICKUP_BODYARMOR, "w31f", -4000, -10749, -259500, 63, 4, 257, 299 },
			{ &PICKUP_M4AMMO, "w31f", -6000, -6249, -242500, 63, 4, 257, 299 },
			{ &PICKUP_M4AMMO, "w31f", -6000, -6249, -241500, 63, 4, 257, 299 },
			{ &PICKUP_THERMALGOGGLES, "w32a", 52000, -38499, -204000, 63, 5, 313, 315 },
			{ &PICKUP_PSG1AMMO, "w32a", 53000, -38499, -204000, 3, 5, 313, 315, true },
			{ &PICKUP_PSG1AMMO, "w32a", 54500, -38499, -204000, 63, 5, 313, 315, true },
			{ &PICKUP_PENTAZEMIN, "w32a", 50000, -38499, -205000, 63, 5, 313, 315, true },
			{ &PICKUP_PENTAZEMIN, "w32a", 51000, -38499, -204000, 15, 5, 313, 315, true },
			{ &PICKUP_PSG1TAMMO, "w32a", 56000, -38499, -205000, 3, 5, 313, 315, true },
			{ &PICKUP_PSG1TAMMO, "w32a", 57500, -38499, -208000, 63, 5, 313, 315, true },
			{ &PICKUP_THERMALGOGGLES, "w32b", 52000, -38499, -204000, 63, 5, 317, 317 },
			{ &PICKUP_PENTAZEMIN, "w32b", 50000, -38499, -205000, 63, 5, 317, 317, true },
			{ &PICKUP_PENTAZEMIN, "w32b", 51000, -38499, -204000, 15, 5, 317, 317, true },
			{ &PICKUP_PSG1TAMMO, "w32b", 56000, -38499, -205000, 3, 5, 317, 317, true },
			{ &PICKUP_PSG1TAMMO, "w32b", 57500, -38499, -208000, 63, 5, 317, 317, true },
			{ &PICKUP_PSG1AMMO, "w32b", 53000, -38499, -204000, 3, 5, 317, 317, true },
			{ &PICKUP_PSG1AMMO, "w32b", 54500, -38499, -204000, 63, 5, 317, 317, true },
			{ &PICKUP_RATION, "w41a", -5250, 26, -3500, 15, 5, 375, 377 },
			{ &PICKUP_MEDICINE, "w41a", 2500, 201, -7200, 63, 5, 375, 377 },
			{ &PICKUP_BOX5, "w42a", 9250, 4001, -13000, 63, 5, 375, 377 },
			{ &PICKUP_MEDICINE, "w42a", 9500, 1, -66000, 63, 5, 375, 377 },
			{ &PICKUP_RATION, "w42a", 17750, 1, -42000, 15, 5, 375, 377 },
			{ &PICKUP_RATION, "w43a", 34250, 4001, -42250, 15, 5, 379, 389 },
			{ &PICKUP_RATION, "w45a", 2500, 1, -138750, 15, 5, 397, 403 },
			{ &PICKUP_STINGERAMMO, "w46a", -8000, 4001, -12000, 63, 5, 408, 411, true },
			{ &PICKUP_STINGERAMMO, "w46a", 8000, 4001, -12000, 63, 5, 408, 411, true },
			{ &PICKUP_STINGERAMMO, "w46a", -14000, 4001, 0, 63, 5, 408, 411, true },
			{ &PICKUP_STINGERAMMO, "w46a", 14000, 4001, 0, 63, 5, 408, 411, true },
			{ &PICKUP_STINGERAMMO, "w46a", -8000, 4001, 12000, 63, 5, 408, 411, true },
			{ &PICKUP_STINGERAMMO, "w46a", 8000, 4001, 12000, 63, 5, 408, 411, true },
			{ &PICKUP_RATION, "w46a", 0, 4001, 0, 31, 5, 408, 411, true },
			{ &PICKUP_RATION, "w61a", 1000, 17781, 21000, 7, 5, 469, 469 },
			{ &PICKUP_RATION, "w61a", 12250, 15306, 3500, 3, 5, 469, 469 },
		} },
		{ Stage::TalesA, {} },
		{ Stage::TalesB, {} },
		{ Stage::TalesC, {} },
		{ Stage::TalesD, {} },
		{ Stage::TalesE, {} },
	};

	std::unordered_set<const Pickup*> PickupsWithConsumables{
		&PICKUP_M9AMMO, &PICKUP_USPAMMO, &PICKUP_SOCOMAMMO, &PICKUP_PSG1AMMO, &PICKUP_RGB6AMMO,
		&PICKUP_NIKITAAMMO, &PICKUP_STINGERAMMO, &PICKUP_CLAYMORE, &PICKUP_C4, &PICKUP_CHAFFGRENADE,
		&PICKUP_STUNGRENADE, &PICKUP_AKS74UAMMO, &PICKUP_GRENADE, &PICKUP_M4AMMO, &PICKUP_PSG1TAMMO,
		&PICKUP_BOOK, &PICKUP_RATION, &PICKUP_MEDICINE, &PICKUP_BANDAGE, &PICKUP_PENTAZEMIN,
	};


	// i think may be populated on first load by looking through actual inventories
	std::vector<const Pickup*> ActiveKeyPickups;


	tFUN_Int_Int oFUN_004e4090;
	tFUN_Int_IntIntInt oFUN_00799210;
	tFUN_Int_Int oFUN_007996d0;
	tFUN_Void_Int oFUN_008781e0;


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
				"select I.code, L.difficulties, L.area, L.posx, L.posz, L.mincard, L.minprog, L.maxprog, L.callback, L.respawn, L.posy "
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
				int posy = sqlite3_column_int(stmtSelect, 10);

				std::string extra = "";
				if (respawn) {
					extra = fmt::format(", {}, {}, true", minprog, maxprog);
				}
				else if (!maxprogNull) {
					extra = fmt::format(", {}, {}", minprog, maxprog);
				}
				else if (!minprogNull) {
					extra = fmt::format(", {}", minprog);
				}

				std::string entry = fmt::format("{{ &PICKUP_{}, \"{}\", {}, {}, {}, {}, {}{} }},\n", (const char*)codeName, (const char*)area, posx, posy, posz, difficulties, mincard, extra);
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

				if (
					((maxprog != -1) && (progress > maxprog))) {
					continue;
				}

				int locationId = sqlite3_column_int(stmtSelectLocation, 0);
				int diffSet = sqlite3_column_int(stmtSelectLocation, 1);
				respawn |= sqlite3_column_int(stmtSelectLocation, 2);

				if ((diffSet & diffValue) == diffValue) {
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

}
