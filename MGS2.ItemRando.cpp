// Item Randomiser v2
// This module is incomplete and may be replaced by ItemRando3 in the future

#include "MGS2.framework.h"
#include "MGS2.ItemRando.h"
#include "lib/tinyfiledialogs/tinyfiledialogs.h"
#include <algorithm>
#include <random>
#include <numeric>
#include <fstream>

namespace MGS2::ItemRando {
	const char* Category = "ItemRando";

	// Vars
	bool AmmoStateStored = false;
	std::unordered_map<KeyItem*, short> AmmoStateMap;
	std::mt19937 RNG;
	std::random_device RD;

	tFUN_Int_Byte const ParamInitFunc = (tFUN_Int_Byte)0x8DEE10;
	tFUN_Int_Void const ParamFunc = (tFUN_Int_Void)0x8DF040;


	const char* NewGameWarning;

	bool NewGameWarningCallback() {
		return CurrentSeed.Enabled;
	}

	void RegisterNewGameWarning() {
		NewGameInfo::RemoveWarning(NewGameWarning);

		if (CurrentSeed.UserSeed) {
			NewGameWarning = _strdup(fmt::format("Item Rando {} {}",
				ASI::Version,
				CurrentSeed.FullSeed()
			).c_str());
		}
		else {
			NewGameWarning = _strdup(fmt::format("Item Randomiser {} M{}{}{}{}",
				ASI::Version,
				(int)DefaultSeed.MainMode,
				DefaultSeed.Logic ? "L" : "",
				DefaultSeed.FallbackType ? ((DefaultSeed.FallbackType == FALLBACK_ANYSET) ? "F+" : "F=") : "",
				DefaultSeed.CardInLoadout ? ((DefaultSeed.CardType == CARD_ADD) ? "C+" : "C=") : ""
			).c_str());
		}

		NewGameInfo::AddWarning(NewGameWarning, &NewGameWarningCallback);
	}

	void WriteReport() {
		if (!SaveReport) return;

		std::string filename;
		try {
			auto format = fmt::runtime(ReportFilename);
			filename = fmt::format(format, "hi.csv"); // todo do it properly
		}
		catch (fmt::v10::format_error) {
			std::string errorMsg = std::string("[ItemRando] Invalid format string ReportFilename: ") + ReportFilename;
			Log::DisplayText(errorMsg, 10);
			return;
		}

		std::filesystem::path path = ReportDirectory;
		path.append(filename);

		std::ofstream output(path.string());
		if (!output.is_open()) {
			std::string msg = fmt::format("[ItemRando] Failed to open {}", path.string());
			return;
		}

		output << "Full Seed\n";
		output << fmt::format("{}\n", CurrentSeed.FullSeed());
		
		output << "\n";

		output << "Key Items\n";
		output << "Location,Original Item,Random Item,Fallback Item\n";
		for (auto group : KeyLocationGroups) {
			Location* location = group->Locations.front();
			Item* originalItem = group->GetInitialItem();
			Item* randomItem = group->CurrentRandomItem();
			Item* fallbackItem = group->FallbackItem;

			const char* sLocation = location ? location->Position.AreaCode : "";
			const char* sOriginalItem = originalItem ? originalItem->Name : "";
			const char* sRandomItem = randomItem ? randomItem->Name : "";
			const char* sFallbackItem = fallbackItem ? fallbackItem->Name : "";

			output << fmt::format("{},{},{},{}\n", sLocation, sOriginalItem, sRandomItem, sFallbackItem);
		}

		output << "\n";

		output << "Regular Items\n";
		output << "Location,Original Item,Random Item,Fallback Item\n";
		for (auto group : RegularLocationGroups) {
			Location* location = group->Locations.front();
			Item* originalItem = group->GetInitialItem();
			Item* randomItem = group->CurrentRandomItem();
			Item* fallbackItem = group->FallbackItem;

			const char* sLocation = location ? location->Position.AreaCode : "";
			const char* sOriginalItem = originalItem ? originalItem->Name : "";
			const char* sRandomItem = randomItem ? randomItem->Name : "";
			const char* sFallbackItem = fallbackItem ? fallbackItem->Name : "";

			output << fmt::format("{},{},{},{}\n", sLocation, sOriginalItem, sRandomItem, sFallbackItem);
		}

		output.close();
		
		std::string msg = fmt::format("[ItemRando] Wrote report to {}", path.string());
		Log::DisplayText(msg);
	}

	unsigned int GetUserSeed() {
		char* input = tinyfd_inputBox("Input Seed", "Seeds can be one of these formats: AAAAAA, /BBBB, AAAAAA/BBBB - where AAAAAA is the seed, and BBBB contains other settings. You can omit the seed to copy only the settings, or omit the settings to keep your existing settings.", "0");
		if (!input) return 0;
		auto parts = ConfigParser::SplitString(input, '/');
		if (!parts.size()) return 0;

		try {
			long long llSeed = std::stoll(parts[0], nullptr, 16);
			if (llSeed < 0) llSeed = 0;
			if (llSeed > MAX_SEED) llSeed = MAX_SEED;
			int seed = static_cast<int>(llSeed);

			unsigned short settings = (parts.size() >= 2) ? std::stoi(parts[1], nullptr, 16) : 0;
			CurrentSeed = DefaultSeed;
			if (seed) {
				CurrentSeed.Enabled = true;
				CurrentSeed.UserSeed = seed;
			}
			if (settings) {
				CurrentSeed.Enabled = true;
				CurrentSeed.ParseSettingsHash(settings);
			}
			RegisterNewGameWarning();
			std::string msg = fmt::format("[ItemRando] Set seed to {}", CurrentSeed.FullSeed());
			Log::DisplayText(msg);
			return seed;
		}
		catch (std::out_of_range) {
			Log::DisplayText("[ItemRando] Seed entry encountered an error");
			return 0;
		}

		return 0;
	}

	void GetUserSeedAction(Actions::Action action) {
		GetUserSeed();
	}

	std::string CopySeedToClipboard() {
		if (!OpenClipboard(nullptr)) {
			std::cerr << "Failed to open clipboard" << std::endl;
			return nullptr;
		}
		EmptyClipboard();
		std::string text = CurrentSeed.FullSeed();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
		if (hMem != nullptr) {
			char* clipboardData = static_cast<char*>(GlobalLock(hMem));
			if (clipboardData) {
				strcpy(clipboardData, text.c_str());
				GlobalUnlock(hMem);
				SetClipboardData(CF_TEXT, hMem);
			}
		}
		CloseClipboard();
		return text;
	}

	void CopySeedToClipboardAction(Actions::Action action) {
		std::string text = CopySeedToClipboard();
		if (text.empty()) return;
		std::string msg = fmt::format("[ItemRando] Copied seed {} to clipboard", text);
		Log::DisplayText(msg);
	}


	// Seed
	std::string Seed::FullSeed() {
		return fmt::format("{:X}/{:X}", UserSeed, SettingsHash());
	}

	void Seed::ParseConfig(CSimpleIniA& ini, const char* category) {
		Enabled = ini.GetBoolValue(category, "Active", Enabled);
		RandomiseKeyItems = true;
		Logic = ini.GetBoolValue(category, "Logic", Logic);
		RandomLoadout = ini.GetBoolValue(category, "RandomLoadout", RandomLoadout);
		CardInLoadout = ini.GetBoolValue(category, "RandomCards", CardInLoadout);
		DigicamInLoadout = ini.GetBoolValue(category, "RandomDigitalCamera", DigicamInLoadout);
		SpecialsInLoadout = ini.GetBoolValue(category, "RandomSpecialItems", SpecialsInLoadout);
		ChangeModels = ini.GetBoolValue(category, "ChangeModels", ChangeModels);
		SaveSeed = ini.GetBoolValue(category, "SaveSeed", SaveSeed);

		std::map<std::string, char> fallbackModeMap{
			{ "none", FALLBACK_NONE },
			{ "set", FALLBACK_SAMESET },
			{ "sameset", FALLBACK_SAMESET },
			{ "any", FALLBACK_ANYSET },
			{ "anyset", FALLBACK_ANYSET },
		};
		char defaultFallbackMode = FallbackType;
		FallbackType = ConfigParser::ParseValueMap(ini, category, "FallbackType", fallbackModeMap, (char)99);
		if (FallbackType == 99) {
			FallbackType = ConfigParser::ParseInteger(ini, category, "FallbackType", defaultFallbackMode, FALLBACK_NONE, FALLBACK_ANYSET, true);
		}

		std::map<std::string, char> cardModeMap{
			{ "add", CARD_ADD },
			{ "overwrite", CARD_OVERWRITE },
		};
		char defaultCardMode = CardType;
		CardType = ConfigParser::ParseValueMap(ini, category, "CardType", cardModeMap, (char)99);
		if (CardType == 99) {
			CardType = ConfigParser::ParseInteger(ini, category, "CardType", defaultCardMode, CARD_ADD, CARD_OVERWRITE, true);
		}

		std::map<std::string, char> mainModeMap{
			{ "none", MODE_NONE },
			{ "no", MODE_NONE },
			{ "off", MODE_NONE },
			{ "default", MODE_DEFAULT },
			{ "normal", MODE_DEFAULT },
			{ "standard", MODE_DEFAULT },
			{ "tags", MODE_TAGSUPPLY },
			{ "tagsupply", MODE_TAGSUPPLY },
			{ "cachetags", MODE_TAGSUPPLY },
			{ "tagsnorando", MODE_TAGSUPPLY_NORANDO },
			{ "tagsupplynorando", MODE_TAGSUPPLY_NORANDO },
			{ "cachetagsnorando", MODE_TAGSUPPLY_NORANDO },
		};
		MainMode = ConfigParser::ParseValueMap(ini, category, "Mode", mainModeMap, MODE_DEFAULT);
		SetupMainMode();
	}

	void Seed::SetupMainMode(char mainMode) {
		if (mainMode == -1) {
			mainMode = MainMode;
		}
		switch (mainMode) {
			case MODE_NONE:
				Enabled = false;
				break;
			case MODE_TAGSUPPLY_NORANDO:
				RandomiseKeyItems = false;
				[[fallthrough]];
			case MODE_TAGSUPPLY:
				SpawnRegularItems = false;
				//SpawnNonDatasetItems = true; // already the case
				DogTagBeggarMode = true;
				FallbackType = FALLBACK_SAMESET;
				break;
		}
	}

	unsigned short Seed::SettingsHash() const {
		// less-common options at the back = potential for 2-character seed part
		size_t i = 0;
		unsigned int result = 0;
		result |= MainMode << i; i += 2;
		result |= Logic << i++;
		result |= FallbackType << i; i += 2;
		result |= RandomLoadout << i++;
		result |= CardInLoadout << i++;
		result |= CardType << i++;
		result |= DigicamInLoadout << i++;
		result |= SpecialsInLoadout << i++;
		return result;
	}

	void Seed::ParseSettingsHash(unsigned short hash) {
		MainMode = (char)(hash & 3); hash >>= 2;
		Logic = (bool)(hash & 1); hash >>= 1;
		FallbackType = (char)(hash & 3); hash >>= 2;
		RandomLoadout = (bool)(hash & 1); hash >>= 1;
		CardInLoadout = (bool)(hash & 1); hash >>= 1;
		CardType = (bool)(hash & 1); hash >>= 1;
		DigicamInLoadout = (bool)(hash & 1); hash >>= 1;
		SpecialsInLoadout = (bool)(hash & 1); hash >>= 1;
		SetupMainMode();
	}



	// KeyItem 
	void KeyItem::Reset() {
		MaxProgress = 0;
		CurrentLocationGroup = nullptr;
		OriginalLocationGroup = nullptr;
		SavedAmmo = SHRT_MIN;
		SwappedAmmo = false;
	}

	void KeyItem::SetLocationGroup(LocationGroup* locationGroup) {
		if (locationGroup) {
			CurrentLocationGroup = locationGroup;
			locationGroup->RandomItem = this;
		}
	}
	/*
	void KeyItem::SwapWith(KeyItem* item) {
		static int test = 0;
		LocationGroup* newLocGroup = item->CurrentLocationGroup;
		if (newLocGroup) {
			KeyItem* oldSwappedItem = (KeyItem*)SwappedItem;
			item->SetLocationGroup(CurrentLocationGroup);
			SetLocationGroup(newLocGroup);
			SwappedItem = item->SwappedItem;
			item->SwappedItem = oldSwappedItem;
			
		}
	}

	void KeyItem::SwapWith(LocationGroup* locationGroup) {
		KeyItem* newItem = (KeyItem*)locationGroup->GetInitialItem();
		// must be a key item
		if (newItem->Set != SET_SINGLE) {
			throw new std::bad_cast();
		}
		SwapWith(newItem);
	}
	*/

	// Position
	bool Position::SamePosition(int posX, int posY, int posZ) const {
		return ((posX == PosX) && (posY == PosY) && (posZ == PosZ));
	}

	bool Position::SameLocation(const char* areaCode, int posX, int posY, int posZ) const {
		return ((!strcmp(areaCode, AreaCode)) && SamePosition(posX, posY, posZ));
	}

	bool Position::SamePosition(Position const& pos) const {
		return SamePosition(pos.PosX, pos.PosY, pos.PosZ);
	}

	bool Position::SameLocation(Position const& pos) const {
		return SameLocation(pos.AreaCode, pos.PosX, pos.PosY, pos.PosZ);
	}


	// LocationGroup
	Item* LocationGroup::GetInitialItem() const {
		if (Locations.empty()) {
			return nullptr;
		}
		return Locations.front()->InitialItem;
	}

	void LocationGroup::SetInitialItem(Item* const item) {
		for (Location* loc : Locations) {
			loc->InitialItem = item;
		}
	}

	bool LocationGroup::IsAvailableAt(short progress) const {
		for (Location* loc : Locations) {
			if ((loc->MinProgress <= progress) && (loc->MaxProgress >= progress)) {
				return true;
			}
		}
		return false;
	}

	short LocationGroup::MinProgress() const {
		short result = SHRT_MAX;
		for (Location* loc : Locations) {
			if (loc->MinProgress < result) {
				result = loc->MinProgress;
			}
		}
		return result;
	}

	short LocationGroup::MaxProgress() const {
		short result = 0;
		for (Location* loc : Locations) {
			if (loc->MaxProgress > result) {
				result = loc->MaxProgress;
			}
		}
		return result;
	}

	short LocationGroup::MinCard() const {
		short result = SHRT_MAX;
		for (Location* loc : Locations) {
			if (loc->MinCardLevel < result) {
				result = loc->MinCardLevel;
			}
		}
		return result;
	}

	bool LocationGroup::IsAvailableBy(short progress) const {
		return (MinProgress() <= progress);
	}

	Item* LocationGroup::CurrentRandomItem() const {
		if (
			(FallbackItem) &&
			(FallbackAfterProgress < Mem::Progress()) &&
			(!((KeyItem*)FallbackItem)->WasCollected)
			) {
			return FallbackItem;
		}
		return RandomItem;
	}

	/*
	void LocationGroup::SwapWith(KeyItem* item) {
		item->SwapWith(this);
	}

	void LocationGroup::SwapWith(LocationGroup* locationGroup) {
		Item* newItem = locationGroup->RandomItem;
		locationGroup->RandomItem = RandomItem;
		RandomItem = newItem;
	}
	*/


	// Ammo change
	short WeaponAmmo(char index, short current, short max, bool addContinue) {
		short* weaponCurrent = (short*)*Mem::WeaponData;
		short* weaponMax = weaponCurrent + Mem::WeaponMaxOffset;
		short* weaponContinueCurrent = weaponCurrent + Mem::WeaponItemContinueOffset;
		short* weaponContinueMax = weaponMax + Mem::WeaponItemContinueOffset;

		if (current >= -1) {
			weaponCurrent[index] = current;
			if (addContinue) {
				weaponContinueCurrent[index] = current;
			}
		}

		if (max >= -1) {
			weaponMax[index] = max;
			if (addContinue) {
				weaponContinueMax[index] = max;
			}
		}

		return weaponCurrent[index];
	}
	short ToolAmmo(char index, short current, short max, bool addContinue) {
		short* itemCurrent = (short*)*Mem::ItemData;
		short* itemMax = itemCurrent + Mem::ItemMaxOffset;
		short* itemContinueCurrent = itemCurrent + Mem::WeaponItemContinueOffset;
		short* itemContinueMax = itemMax + Mem::WeaponItemContinueOffset;

		if (current >= -1) {
			itemCurrent[index] = current;
			if (addContinue) {
				itemContinueCurrent[index] = current;
			}
		}

		if (max >= -1) {
			itemMax[index] = max;
			if (addContinue) {
				itemContinueMax[index] = max;
			}
		}

		return itemCurrent[index];
	}
	short ItemAmmo(Item* item, short current, short max, bool addContinue) {
		if (item->Category == CAT_ITEM) {
			return ToolAmmo(item->Id, current, max, addContinue);
		}
		else {
			return WeaponAmmo(item->Id, current, max, addContinue);
		}
	}

	short WeaponMaxAmmo(char index) {
		short* weaponMax = (short*)(*Mem::WeaponData + Mem::WeaponMaxOffset);
		return weaponMax[index];
	}
	short ToolMaxAmmo(char index) {
		short* itemMax = (short*)(*Mem::ItemData + Mem::ItemMaxOffset);
		return itemMax[index];
	}
	short ItemMaxAmmo(Item* item) {
		if (item->Category == CAT_ITEM) {
			return ToolMaxAmmo(item->Id);
		}
		else {
			return WeaponMaxAmmo(item->Id);
		}
	}

	Item* CurrentLeastStockedConsumableItem() {
		float minStock = 1.01f;
		std::vector<Item*> candidateItems;
		std::vector<Item*> unfoundCandidateItems;
		for (auto item : ConsumablesData) {
			if ((item == &ITEM_RATION) && (*Mem::Difficulty == Difficulty::EuroExtreme)) continue;
			short current = ItemAmmo(item);
			short max = ItemMaxAmmo(item);
			if ((!max) || (max > 999)) continue;
			if (current < 0) {
				if (item->Category != CAT_WEAPON_AMMO) {
					unfoundCandidateItems.push_back(item);
				}
				continue;
			}
			if ((item->Category == CAT_ITEM) && (current == 0)) {
				unfoundCandidateItems.push_back(item);
				continue;
			}
			float stock = (float)current / (float)max;
			if (stock <= minStock) {
				if (stock < minStock) {
					candidateItems.clear();
				}
				candidateItems.push_back(item);
				minStock = stock;
			}
		}
		if (minStock > 0) {
			// if all found items have at least some ammo, try to add a new unfound item
			size_t count = unfoundCandidateItems.size();
			if (count) {
				return unfoundCandidateItems[MGS2::RNG(count)];
			}
		}
		// if no unfound items, or at least one found item is at 0, give a found item
		size_t count = candidateItems.size();
		return count ? candidateItems[MGS2::RNG(count)] : nullptr;
	}


	// TODO this currently isn't populated in dog tag mode
	std::unordered_map<int, Item*> AvailableItems;
	void AddToAvailableItems(Item* item) {
		short ic = item->IC();
		if (AvailableItems.find(ic) != AvailableItems.end()) return;
		AvailableItems[ic] = item;
	}


	void GenerateLocationGroups(std::vector<Location>& locationSet) {
		// clear existing groups
		AllLocationGroups.clear();
		RegularLocationGroups.clear();
		KeyLocationGroups.clear();
		AvailableItems.clear();

		std::unordered_map<Item*, std::vector<Location*>> keyItemLocationMap;
		std::unordered_map<Location, std::vector<Location*>> regularItemLocationMap;

		for (auto& loc : locationSet) {
			Item* initialItem = loc.InitialItem;
			AddToAvailableItems(initialItem); // no work
			// run item's item initialiser
			// TODO avoid running the same init many times
			if (initialItem->InitItem) {
				initialItem->InitItem(initialItem, *Mem::Difficulty);
			}
			// run the item's location initialiser
			if (initialItem->InitLocation) {
				initialItem->InitLocation(&loc, *Mem::Difficulty);
			}
			// take it out of the pool if not in a good difficulty (maybe put this further up?)
			if (!loc.HasDifficulty(*Mem::Difficulty)) {
				continue;
			}
			// ignore respawns
			// TODO have respawns do cool things e.g random ammo
			if (loc.IsRespawn) {
				continue;
			}
			// run the location's initialiser
			if (loc.InitLocation) {
				loc.InitLocation(&loc, *Mem::Difficulty);
			}

			if (!loc.Available) {
				continue;
			}

			// todo change the non-set modes to put same-position items in the same group (e.g. M9 and M9 ammo in Strut F)
			if (loc.InitialItem->Set == SET_SINGLE) {
				keyItemLocationMap[loc.InitialItem].push_back(&loc);
			}
			else {
				/*
				std::shared_ptr<LocationGroup> locGroup = std::make_shared<LocationGroup>(&loc);
				AllLocationGroups.push_back(locGroup);
				RegularLocationGroups.push_back(locGroup);
				*/
				regularItemLocationMap[loc].push_back(&loc);
			}
		}

		for (auto& group : keyItemLocationMap) {
			std::shared_ptr<LocationGroup> locGroup = std::make_shared<LocationGroup>(group.second);
			KeyItem* item = (KeyItem*)group.first;
			AddToAvailableItems(item); // no work
			item->Reset();
			item->OriginalLocationGroup = locGroup.get();
			AllLocationGroups.push_back(locGroup);
			KeyLocationGroups.push_back(locGroup);
		}

		for (auto& group : regularItemLocationMap) {
			// an item is considered the same location group if the area (except for the last character) is the same,
			//   pos X and Z are the same, and the item category and id are the same
			std::shared_ptr<LocationGroup> locGroup = std::make_shared<LocationGroup>(group.second);
			AllLocationGroups.push_back(locGroup);
			RegularLocationGroups.push_back(locGroup);
		}
	}

	uint32_t GetCurrentSeed() {
		int* continueData = (int*)(*(uintptr_t*)Mem::ContinueData1);
		return continueData[0x6FC];
	}

	void SetCurrentSeed(int seed) {
		CurrentSeed.UserSeed = seed;
		int* currentData = (int*)(*(uintptr_t*)Mem::CurrentData1);
		int* continueData = (int*)(*(uintptr_t*)Mem::ContinueData1);
		currentData[0x6FC] = seed;
		continueData[0x6FC] = seed;
	}

	std::vector<Item*> InitialLoadout;
	std::vector<Item*> RandomisedLoadout;

	void Reroll() {
		if (!CurrentSeed.Enabled) return;
		if (!CurrentSeed.RandomiseKeyItems) return;

		AmmoStateMap.clear();
		Stage stage = Mem::Stage();

		if (stage == Stage::None) return;
		RerollNow = false;

		int seed = 0;
		if (CurrentSeed.UserSeed) {
			seed = CurrentSeed.UserSeed;
		}
		else if (CurrentSeed.SaveSeed) {
			seed = GetCurrentSeed();
			if (seed) {
				CurrentSeed.UserSeed = seed;
#ifdef _DEBUG
				std::string msg = fmt::format("[ItemRando] Seed from save file: {}", seed);
				Log::DisplayText(msg);
#endif
			}
		}
		if (!seed) {
			seed = RD() % MAX_SEED;
			SetCurrentSeed(seed);
#ifdef _DEBUG
			std::string msg = fmt::format("[ItemRando] New seed: {}", seed);
			Log::DisplayText(msg);
#endif
		}
		seed += stage; // tweak the random behaviour going between tanker and plant
		RNG.seed(seed);


		// select location set TODO anything but tanker
		std::vector<Location> &currentLocationSet = LocationsData[stage];
		std::vector<Item*> currentLoadout = LoadoutData[stage];
		std::vector<RequiredItemGroup> &currentRequiredItemSet = RequiredItemsData[stage];

		// make a copy of the groups to work on
		GenerateLocationGroups(currentLocationSet);
		std::vector<std::shared_ptr<LocationGroup>> shuffledKeyLocationGroups;
		std::vector<std::shared_ptr<LocationGroup>> shuffledRegularLocationGroups;
		std::vector<std::shared_ptr<LocationGroup>> shuffledFallbackLocationGroups;
		
		shuffledKeyLocationGroups = KeyLocationGroups;
		shuffledRegularLocationGroups = RegularLocationGroups;
		
		if (CurrentSeed.FallbackType == FALLBACK_ANYSET) {
			shuffledFallbackLocationGroups = RegularLocationGroups;
		}
		else {
			shuffledFallbackLocationGroups = KeyLocationGroups;
		}

		std::shuffle(shuffledKeyLocationGroups.begin(), shuffledKeyLocationGroups.end(), RNG);
		std::shuffle(shuffledRegularLocationGroups.begin(), shuffledRegularLocationGroups.end(), RNG);
		std::shuffle(shuffledFallbackLocationGroups.begin(), shuffledFallbackLocationGroups.end(), RNG);

		std::vector<KeyItem*> itemsNeedingFallback;
		RandomisedLoadout.clear();

		// Cards shuffle
		if ((CurrentSeed.CardInLoadout) && (stage == Stage::Plant)) {
			KeyItem* cardItems[]{ &ITEM_RAIDEN_CARD1, &ITEM_RAIDEN_CARD2, &ITEM_RAIDEN_CARD3, &ITEM_RAIDEN_CARD4 };
			//KeyItem* movingItem = &ITEM_RAIDEN_CARD;
			for (int i = 0; i < 5; i++) {
				KeyItem* movingItem = cardItems[i];
				for (auto sklg : shuffledKeyLocationGroups) {
					if (sklg->RandomItem != nullptr) continue;
					
					if (CurrentSeed.Logic) {
						if (sklg->MinCard() > i) continue;
					}

					sklg->RandomItem = movingItem;
					movingItem->CurrentLocationGroup = sklg.get();
					
					if ((CurrentSeed.FallbackType != FALLBACK_NONE) && (movingItem->MaxProgress) && (!sklg->IsAvailableAt(movingItem->MaxProgress))) {
						itemsNeedingFallback.push_back(movingItem);
					}
					
					if (std::find(currentLoadout.begin(), currentLoadout.end(), sklg->GetInitialItem()) != currentLoadout.end()) {
						RandomisedLoadout.push_back(movingItem);
					}

					break;
				}
			}
		}

		// RequiredItemGroup shuffle
		for (auto rig : currentRequiredItemSet) {
			KeyItem* movingItem = nullptr;

			// get a random item from the group that hasn't been shuffled yet
			std::vector<KeyItem*>& items = rig.Items;
			if (items.size() > 1) {
				std::shuffle(items.begin(), items.end(), RNG);

				for (auto item : items) {
					if (item->CurrentLocationGroup == nullptr) {
						movingItem = item;
						break;
					}
				}
			}
			else {
				movingItem = items.front();
			}

			if (movingItem == nullptr) continue;

			// shuffle it
			for (auto sklg : shuffledKeyLocationGroups) {
				if (sklg->RandomItem != nullptr) continue;
				if (CurrentSeed.Logic) {
					if (!sklg->IsAvailableBy(rig.MaxProgress)) continue;
				}

				sklg->RandomItem = movingItem;
				movingItem->CurrentLocationGroup = sklg.get();
				movingItem->MaxProgress = rig.MaxProgress;

				if ((CurrentSeed.FallbackType != FALLBACK_NONE) && (!sklg->IsAvailableAt(rig.MaxProgress))) {
					if (movingItem == &ITEM_RAIDEN_HFBLADE) {
						/*
						auto it = std::find_if(KeyLocationGroups.begin(), KeyLocationGroups.end(), [](const auto& group) {
							return (group->GetInitialItem() == &ITEM_RAIDEN_HFBLADE);
							});
						if (it != KeyLocationGroups.end()) {
							it->get()->FallbackItem = &ITEM_RAIDEN_HFBLADE;
							it->get()->FallbackAfterProgress = rig.MaxProgress;
						}
						*/
						// todo remove
					}
					else {
						itemsNeedingFallback.push_back(movingItem);
					}
				}

				if (std::find(currentLoadout.begin(), currentLoadout.end(), sklg->GetInitialItem()) != currentLoadout.end()) {
					RandomisedLoadout.push_back(movingItem);
				}

				break;
			}
		}
		// shuffle the other key items
		for (auto klg : KeyLocationGroups) {
			KeyItem* movingItem = (KeyItem*)klg->GetInitialItem();
			if (movingItem->CurrentLocationGroup != nullptr) continue; // already done this one
			for (auto sklg : shuffledKeyLocationGroups) {
				if (sklg->RandomItem != nullptr) continue;

				sklg->RandomItem = movingItem;
				movingItem->CurrentLocationGroup = sklg.get();

				if (std::find(currentLoadout.begin(), currentLoadout.end(), sklg->GetInitialItem()) != currentLoadout.end()) {
					RandomisedLoadout.push_back(movingItem);
				}
				break;
			}
		}

		// fallbacks
		for (auto ki : itemsNeedingFallback) {
			for (auto sklg : shuffledFallbackLocationGroups) {
				if (sklg->FallbackItem != nullptr) continue; // already got a fallback here

				Item* item = sklg->RandomItem;
				if ((item != nullptr) && (item->Set == SET_SINGLE)) {
					KeyItem* keyItem = (KeyItem*)sklg->RandomItem;
					if (keyItem->MaxProgress) continue; // don't fallback the target's random item if that's also time sensitive
				}
				
				if (sklg->MaxProgress() == SHRT_MAX) continue; // avoid cutscene items that would already have been collected
				if (!sklg->IsAvailableAt(ki->MaxProgress)) continue;

				sklg->FallbackItem = ki;
				sklg->FallbackAfterProgress = ki->CurrentLocationGroup->MaxProgress();

				break;
			}
		}

		// shuffle everything else
		/*
		if ((CurrentSeed.Enabled == MODE_SAMESET) || (CurrentSeed.Enabled == MODE_SAMESET_NOLOGIC)) {
			// this shuffle only works when key items and regular items are segregated (MODE_SAMESET), doesn't work with MODE_ANYSET
			size_t ct = RegularLocationGroups.size();
			for (size_t i = 0; i < ct; i++) {
				shuffledRegularLocationGroups[i]->RandomItem = RegularLocationGroups[i]->GetInitialItem();
			}
		}
		else {
		*/
		// i think this would work even with the old sameset/anyset
		size_t i = 0;
		size_t shuffleSize = shuffledRegularLocationGroups.size();
		for (auto group : RegularLocationGroups) {
			for (; i < shuffleSize; i++) {
				auto shuffleGroup = shuffledRegularLocationGroups[i];
				if (shuffleGroup->RandomItem != nullptr) continue; // already rolled
				shuffleGroup->RandomItem = group->GetInitialItem();
				break;
			}
		}
	}

	bool AmmoStateSwapped = false;
	// TODO make this so it can handle key items in non-key item locations (e.g. only store KI values if item is SET_SINGLE)
	void StoreAmmoState(bool swap = false, bool clobber = false) {
		for (auto group : KeyLocationGroups) {
			KeyItem* randomItem = (KeyItem*)group->RandomItem;
			if (clobber || (randomItem->SavedAmmo == SHRT_MIN)) {
				randomItem->SavedAmmo = ItemAmmo(randomItem); // store for later restore
				Item* originalItem = (Item*)group->GetInitialItem();
				if (originalItem->Set == SET_SINGLE) {
					KeyItem* originalKeyItem = (KeyItem*)originalItem;
					originalKeyItem->WasCollected = originalKeyItem->IsCollected();
				}
			}
		}
		if (swap) {
			for (auto group : KeyLocationGroups) {
				KeyItem* originalItem = (KeyItem*)group->GetInitialItem();
				if (originalItem->SwappedAmmo) continue;

				Item* randomItem = group->RandomItem;
				short randomModifier = (randomItem->Category == CAT_ITEM); // add 1 if item (0 = off, 1 = on) vs (-1 = off, 0 = on)
				short originalModifier = (originalItem->Category == CAT_ITEM);
				bool randomCollected = false; // if the random item isn't a key item, assume it's not collected

				if (group->RandomItem->Set == SET_SINGLE) {
					KeyItem* randomKeyItem = (KeyItem*)group->RandomItem;
					// always spawn if card
					randomCollected = (randomKeyItem->IsCardWithLevel) ? false : (randomKeyItem->SavedAmmo >= randomModifier);
				}

				ItemAmmo(originalItem, -1 + originalModifier + randomCollected);
				originalItem->SwappedAmmo = true;
			}
			AmmoStateSwapped = true;
		}

		AmmoStateStored = true;
	}

	void RestoreAmmoState(bool forceSwap = true) {
		if (!AmmoStateStored) return;
		// restore ammo state for key items

		for (auto group : KeyLocationGroups) {
			KeyItem* item = (KeyItem*)group->RandomItem;
			if ((forceSwap || item->SwappedAmmo) && (item->SavedAmmo != SHRT_MIN)) {
				ItemAmmo(item, item->SavedAmmo);
			}
			item->SavedAmmo = SHRT_MIN;
			item->SwappedAmmo = false;
		}
		AmmoStateSwapped = false;
		AmmoStateStored = false;
	}

	void ClearAmmoState() {
		if (!AmmoStateStored) return;
		for (auto group : KeyLocationGroups) {
			KeyItem* item = (KeyItem*)group->RandomItem;
			item->SavedAmmo = SHRT_MIN;
			item->SwappedAmmo = false;
		}
		AmmoStateSwapped = false;
		AmmoStateStored = false;
	}

	void DisplayNewItems(const std::vector<const char*> &itemNames) {
		std::string strItems = (itemNames.size() == 1) ? itemNames.front() :
			std::accumulate(itemNames.begin(), itemNames.end(), std::string(),
				[](std::string s1, std::string s2) {
					return s1.empty() ? s2 : s1 + ", " + s2;
				});
		std::string msg = fmt::format("New items: {}", strItems);
		Log::DisplayText(msg, 5);
	}

	void AddCutsceneItems() {
		if (!CurrentSeed.RandomLoadout) return;

		RestoreAmmoState();

		short progress = Mem::Progress();
		
		std::vector<KeyItem*> itemsToAdd;
		std::vector<const char*> itemNamesToAdd;
		for (auto group : KeyLocationGroups) {
			if (!group->Locations.size()) continue;
			Location* loc = group->Locations.front();
			if (progress != loc->MinProgress) continue;

			if (strcmp(loc->Position.AreaCode, "cuts")) continue;

			// restore manually-saved ammo state, hopefully don't need any more
			KeyItem* originalItem = (KeyItem*)group->GetInitialItem();
			auto it = AmmoStateMap.find(originalItem);
			if (it != AmmoStateMap.end()) {
				ItemAmmo(originalItem, it->second, SHRT_MIN, true);
				AmmoStateMap.erase(it);
			}

			KeyItem* randomItem = (KeyItem*)group->CurrentRandomItem();
			itemsToAdd.push_back(randomItem);
		}

		size_t addedCt = 0;
		for (KeyItem* item : itemsToAdd) {
			//if (item == &ITEM_RAIDEN_CARD) {
			if (item->IsCardWithLevel) {
				short oldCardLevel = ItemAmmo(item);
				if (CurrentSeed.CardType == CARD_ADD) {
					ItemAmmo(item, oldCardLevel + 1);
					itemNamesToAdd.push_back(ITEM_RAIDEN_CARD.Name);
				}
				else if (item->IsCardWithLevel > oldCardLevel) {
					ItemAmmo(item, item->IsCardWithLevel);
					itemNamesToAdd.push_back(item->Name);
				}
			}
			else {
				ItemAmmo(item, item->Ammo, SHRT_MIN, true);
				itemNamesToAdd.push_back(item->Name);
			}
			addedCt++;
		}

		if (addedCt) {
			DisplayNewItems(itemNamesToAdd);
		}
	}


	void ApplyRandomisedLoadout(std::vector<Item*> &loadout) {
		if (!RandomisedLoadout.size()) return;
		//for (auto* item : loadout) {
		ItemAmmo(&ITEM_RAIDEN_HFBLADE, -1); // for hf blade mod
		for (auto group : KeyLocationGroups) {
			KeyItem* item = (KeyItem*)group->GetInitialItem();
			ItemAmmo(item, (item->Category == CAT_ITEM) ? 0 : -1);
		}
		std::vector<const char*> itemNames;
		for (auto a : RandomisedLoadout) {
			KeyItem* item = (KeyItem*)a;
			if (item->IsCardWithLevel) {
				short oldCardLevel = ItemAmmo(item);
				if (CurrentSeed.CardType == CARD_ADD) {
					ItemAmmo(item, oldCardLevel + 1);
				}
				else if (item->IsCardWithLevel > oldCardLevel) {
					ItemAmmo(item, item->IsCardWithLevel);
				}
			}
			else {
				ItemAmmo(item, item->Ammo);
			}
			itemNames.push_back(item->Name);
		}

		if (MGS2::Stage() == Stage::Tanker) {
			bool noM9 = true;
			bool usp = false;
			for (auto item : RandomisedLoadout) {
				if (item == &ITEM_SNAKE_M9) {
					noM9 = false;
				}
				else if (item == &ITEM_SNAKE_USP) {
					usp = true;
				}
			}
			if (noM9) {
				EquipShortcuts::ChangeWeapon(usp ? 2 : 0); // doesn't work?
				ItemAmmo(&ITEM_MAGAZINE, 0);
			}
		}

		DisplayNewItems(itemNames);
		OutputDebugStringA("Applied rando loadout\n");

		// todo magazines
	}


	void AfterPlayerSetup() {
		if (!CurrentSeed.Enabled) return;
		if (!CurrentSeed.RandomiseKeyItems) return;

		static MGS2::Stage savedStage = Stage::None;
		MGS2::Stage previousStage = savedStage;
		MGS2::Stage stage = Mem::Stage();
		savedStage = stage;

		static short savedProgress = 0;
		short previousProgress = savedProgress;
		short progress = Mem::Progress();
		savedProgress = progress;

		static uintptr_t savedInventory = *Mem::WeaponData;
		uintptr_t previousInventory = savedInventory;
		uintptr_t inventory = *Mem::WeaponData;
		savedInventory = inventory;

		bool rerolledThisFrame = false;

		if (stage == Stage::None) return;
		else if (RerollNow || (stage != previousStage)) {
			ClearAmmoState();
			Reroll();
			rerolledThisFrame = true;
		}

		if ((progress != previousProgress) || rerolledThisFrame) {
			// start of tanker
			if (stage == Stage::Tanker) {
				if (progress == 14) {
#ifdef _DEBUG
					if (DB) {
						DB->OutputLocationSet(Stage::Tanker);
					}
#endif
					if (!rerolledThisFrame) {
						Reroll();
					}
					if (CurrentSeed.RandomLoadout) {
						ApplyRandomisedLoadout(LoadoutData[Stage::Tanker]);
					}
				}
			}
			// start of plant
			else if (stage == Stage::Plant) {
				if (progress == 9) {
#ifdef _DEBUG
					if (DB) {
						DB->OutputLocationSet(Stage::Plant);
					}
#endif
					if (!rerolledThisFrame) {
						Reroll();
					}
					if (CurrentSeed.RandomLoadout) {
						ApplyRandomisedLoadout(LoadoutData[Stage::Plant]);
					}
				}
				else if ((progress == 79) || (progress == 120)) {
					if (progress == 79) {
						AmmoStateMap[&ITEM_RAIDEN_COOLANT] = ItemAmmo(&ITEM_RAIDEN_COOLANT);
					}
					else if (progress == 120) {
						AmmoStateMap[&ITEM_RAIDEN_BDU] = ItemAmmo(&ITEM_RAIDEN_BDU);
						AmmoStateMap[&ITEM_RAIDEN_PHONE] = ItemAmmo(&ITEM_RAIDEN_PHONE);
					}
					if (CurrentSeed.CardInLoadout) {
						AmmoStateMap[&ITEM_RAIDEN_CARD] = ItemAmmo(&ITEM_RAIDEN_CARD);
					}
					OutputDebugStringA("Saved card state for later\n");
				}
				else if ((progress == 375) || (progress == 387) || (progress == 469)) {
					// don't overwrite the inventory swaps in arsenal
					ClearAmmoState();
				}
			}
			// start of tales
			else if ((stage >= Stage::TalesA) && (stage <= Stage::TalesE) && (progress == 1)) {
#ifdef _DEBUG
				if (DB) {
					DB->OutputLocationSet(stage);
				}
#endif
				if (!rerolledThisFrame) {
					Reroll();
				}
				if (CurrentSeed.RandomLoadout) {
					ApplyRandomisedLoadout(LoadoutData[stage]);
				}
			}
			
			AddCutsceneItems();
		}

		StoreAmmoState(true, false); // swap/no clobber
	}


	// item_box_init
	// TODO implement a RandomiseKeyItems check without breaking tags
	tFUN_Int_IntIntInt oFUN_00799210;
	int hkFUN_00799210(int param_1, int param_2, int param_3) {
		try_mgs2
			bool applyModification = false;
			bool matchedItem = false;
			int  valF, valI, valN, valT, valX, valPX, valPY, valPZ, result;
			std::unordered_map<int*, int> mods;

			int* optB = (int*)(param_1 + 0xA8);
			int* optF = (int*)(param_1 + 0x40);
			int* optI = (int*)(param_1 + 0x44);
			int* optN = (int*)(param_1 + 0x48);
			int* optT = (int*)(param_1 + 0x58);
			int* optU = (int*)(param_1 + 0x5C);
			int* optX = (int*)(param_1 + 0x8C);
			float* optPX = (float*)(param_1 + 0x230);
			float* optPY = (float*)(param_1 + 0x234);
			float* optPZ = (float*)(param_1 + 0x238);

			// create location for this item to test against
			Item testItem{ nullptr };
			Location testLocation{ &testItem, 0, { Mem::AreaCode } };

			if (!CurrentSeed.Enabled) goto bail;

			if (!ParamInitFunc('f')) goto bail;
			valF = ParamFunc();

			if (!ParamInitFunc('i')) goto bail;
			valI = ParamFunc();

			if (!ParamInitFunc('n')) goto bail;
			valN = ParamFunc();

			if (!ParamInitFunc('p')) goto bail;
			valPX = ParamFunc();
			valPY = ParamFunc();
			valPZ = ParamFunc();

			valT = (ParamInitFunc('t')) ? ParamFunc() : -1;
			valX = (ParamInitFunc('x')) ? ParamFunc() : 0;

			// special behaviours
			if (CurrentSeed.DogTagBeggarMode) {
				if (valI == ITEM_DOGTAG.Id) {
					Item* item = CurrentLeastStockedConsumableItem();
					if (!item) goto bail;
					mods[optF] = item->Category;
					mods[optI] = item->Id;
					mods[optN] = item->Ammo;
					mods[optT] = -1;
					mods[optX] = 0;
					goto apply;
				}
			}

			testItem.Id = valI;
			testItem.Ammo = valN;
			testItem.Category = valF;
			testLocation.Position.PosX = valPX;
			testLocation.Position.PosY = valPY; // might be 1 out?
			testLocation.Position.PosZ = valPZ;

			if (CurrentSeed.RandomiseKeyItems) {
				for (auto& locationGroup : AllLocationGroups) {
					for (auto location : locationGroup->Locations) {
						if (testLocation != *location) continue;

						Item* item = locationGroup->CurrentRandomItem();
						mods[optF] = item->Category;
						mods[optI] = item->Id;
						mods[optN] = item->Ammo;
						if (item->Set == SET_SINGLE) {
							KeyItem* keyItem = (KeyItem*)item;
							if (keyItem->IsCardWithLevel) {
								short curLevel = keyItem->SavedAmmo;
								short levelsToAdd = item->Ammo - curLevel;
								if (CurrentSeed.CardType == CARD_OVERWRITE) {
									mods[optN] = (levelsToAdd > 0) ? levelsToAdd : 0;
									if (levelsToAdd <= 0) return -1;
								}
								else {
									mods[optN] = (levelsToAdd > 0) ? 1 : 0;
									if (levelsToAdd <= 0) return -1;
								}
							}
							else if ((keyItem->Category == CAT_STANDALONE_AMMO) && keyItem->WasCollected) {
								Item* ammoItem = keyItem->AmmoItem;
								if (!ammoItem) {
									return -1;
								}
								mods[optF] = ammoItem->Category;
								mods[optI] = ammoItem->Id;
								mods[optN] = ammoItem->Ammo;
								goto apply;
							}
						}
						else { // SET_MANY
							if (!CurrentSeed.SpawnRegularItems) return -1;
						}

	#ifdef _DEBUG
						std::string output = fmt::format("Rerolling to {} (F={} I={})\n\n", item->Name, (int)item->Category, (int)item->Id);
						OutputDebugStringA(output.c_str());
	#endif
					}
				}
			}
			// no randomise key items
			else {
				for (auto& locationGroup : AllLocationGroups) {
					for (auto location : locationGroup->Locations) {
						if (testLocation != *location) continue;
						matchedItem = true;
						Item* item = locationGroup->CurrentRandomItem();
						if (!item) continue;
						if (item->Set == SET_SINGLE) continue;
						else if (!CurrentSeed.SpawnRegularItems) return -1;
						mods[optF] = item->Category;
						mods[optI] = item->Id;
						mods[optN] = item->Ammo;
						goto apply;
					}
				}
			}

			// not randomised, do extra stuff
			if (!CurrentSeed.SpawnRegularItems) {
				unsigned short ic = testItem.IC();
				auto it = AvailableItems.find(ic);
				if (it != AvailableItems.end()) {
					Item* item = it->second;
					if (item->Set != SET_SINGLE) return -1;
				}
			}

		apply:
			if (mods.size()) {
				result = oFUN_00799210(param_1, param_2, param_3);
				for (auto mod : mods) {
					*(mod.first) = mod.second;
				}
				return result;
			}
			else if ((!matchedItem) && (!CurrentSeed.SpawnNonDatasetItems)) {
				return -1;
			}
			else {
				OutputDebugStringA("No reroll\n\n");
			}

		bail:
			return oFUN_00799210(param_1, param_2, param_3);
		catch_mgs2(Category, "799210");

		// fallback
		return oFUN_00799210(param_1, param_2, param_3);
	}

	// item_box_init overall param reader (not used)
	tFUN_Int_Int oFUN_007996d0;
	int hkFUN_007996d0(int param_1) {
		int result = oFUN_007996d0(param_1);

#ifdef _DEBUG
		try_mgs2
			if (DB) {
				int* optF = (int*)(param_1 + 0x40);
				int* optI = (int*)(param_1 + 0x44);
				int* optN = (int*)(param_1 + 0x48);
				float* optPX = (float*)(param_1 + 0x230);
				float* optPY = (float*)(param_1 + 0x234);
				float* optPZ = (float*)(param_1 + 0x238);
				DB->RecordDifficulty(*optF, *optI, (int)*optPX, (int)*optPY, (int)*optPZ);
			}
		catch_mgs2(Category, "7996D0");
#endif

		return result;
	}


	// before and after running main() every frame
	tFUN_Void_Int oFUN_008781e0;
	void __cdecl hkFUN_008781e0(int param_1) {
		try_mgs2
			if (Mem::Stage() == MGS2::Stage::None) {
				if (!RerollNow) {
					RerollNow = true;
					WriteReport();
					CurrentSeed = DefaultSeed;
					RegisterNewGameWarning();
				}
				return oFUN_008781e0(param_1);
			}

			if ((*(int*)(param_1 + 0x30) == 0) && (*(int*)0xF6E4E0 != 0)) {
				StoreAmmoState(); // no clobber no swap, this is just to keep hold of state for cutscene items
				oFUN_008781e0(param_1);
				if (AmmoStateSwapped) {
					RestoreAmmoState(); // restores state when entering gameplay only
				}
				return;
			}
		catch_mgs2(Category, "8781E0");

		// run the original
		oFUN_008781e0(param_1);
	}
	

	tFUN_Int_Int oFUN_004e4090;
	int hkFUN_004e4090(int param_1) {
		int result = oFUN_004e4090(param_1);

		try_mgs2
			AfterPlayerSetup();
		catch_mgs2(Category, "4E4090");

		return result;
	}



	void Run(CSimpleIniA& ini) {
		return;

		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) return;
		if (ConfigParser::ParseInteger<int>(ini, Category, "Version", 2, 0, INT_MAX, true) == 2) Enabled = true;
		else return;

		RNG = std::mt19937{ RD() };

		DefaultSeed.ParseConfig(ini, Category);
		DefaultSeed.UserSeed = ConfigParser::ParseInteger<int>(ini, "ItemRando.UserSeed", "Seed", 0, 0, MAX_SEED, true);

		RegisterNewGameWarning();

		Actions::RegisterAction(ini, "ItemRando.UserSeed", &GetUserSeedAction);
		Actions::RegisterAction(ini, "ItemRando.CopySeed", &CopySeedToClipboardAction);

		if ((ReportDirectory = ini.GetValue(Category, "ReportDirectory", "")) != "") {
			if (std::filesystem::is_directory(ReportDirectory)) {
				SaveReport = true;
				ReportFilename = ini.GetValue(Category, "ReportFilename", ReportFilename.c_str());
			}
		}
		
#ifdef _DEBUG
		const char* databasePath = ini.GetValue(Category, "DatabasePath");
		if (databasePath) {
			DB = new Database(databasePath);
			Actions::RegisterAction(ini, "ItemRando.DB.Toggle", &DBToggleAction);
			//DefaultSeed.Enabled = false;
		}
#endif

		CurrentSeed = DefaultSeed;
		RegisterNewGameWarning();

		oFUN_007996d0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7996D0, (BYTE*)hkFUN_007996d0, 7); // params
		oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);
		
		//oFUN_008e0280 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x8E0280, (BYTE*)hkFUN_008e0280, 8);
		oFUN_004e4090 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x4E4090, (BYTE*)hkFUN_004e4090, 8);

		//oFUN_0042d780 = (tFUN_Int_IntIntInt)mem::TrampHook32((BYTE*)0x42D780, (BYTE*)hkFUN_0042d780, 6);
		//oFUN_004c3a00 = (tFUN_Void_IntIntInt)mem::TrampHook32((BYTE*)0x4C3A00, (BYTE*)hkFUN_004c3a00, 5);
		
		oFUN_00799210 = (tFUN_Int_IntIntInt)mem::TrampHook32((BYTE*)0x799210, (BYTE*)hkFUN_00799210, 6);
	}

}
