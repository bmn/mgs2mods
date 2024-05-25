// Item Randomiser v3
// This module is incomplete

#include "MGS2.ItemRando3.h"
#include <numeric>

#ifdef _DEBUG
#include "lib/sqlite/sqlite3.h"
#endif

namespace MGS2::ItemRando3 {

	// == operator only compares PosX, PosYrounded, PosZ, and all but the last character of the area code
	bool Location::operator==(const Location& rhs) const {
		if (Type != rhs.Type) return false;
		if ((Type != Types::Area) && (MinProgress != rhs.MinProgress)) return false;

		if (X != rhs.X) return false;
		// if (Y != rhs.Y) return false;
		if (Z != rhs.Z) return false;

		std::string_view strAreaCode = std::string_view(AreaCode).substr(0, AreaCode.length() - 1);
		std::string_view strAreaCodeRHS = std::string_view(rhs.AreaCode).substr(0, rhs.AreaCode.length() - 1);
		if (strAreaCode.compare(strAreaCodeRHS) != 0) return false;

		return true;
	}

	void LocationGroup::Init() {
		size_t count = Locations.size();
		switch (count) {
			case 0:
				break;
			case 1:
				OriginalPickup = Locations.front()->OriginalPickup;
				break;
			default:
				auto locations = Locations;
				//std::shuffle(locations.begin(), locations.end(), ); // TODO put in non-seed-specific rng
				OriginalPickup = locations.front()->OriginalPickup;
				break;
		}
	}

	void Seed::ParseConfig(CSimpleIniA& ini, const char* category) {
		if (!category) category = Category;

		Enabled = ini.GetBoolValue(category, "Active", Enabled);
		Logic = ini.GetBoolValue(category, "Logic", Logic);
		
		LocationTypes = (ini.GetBoolValue(category, "RandomCutscenes", false)) ?
			Location::Types::Area :
			Location::Types::Area + Location::Types::Cutscene + Location::Types::Loadout;

		std::map<std::string, RandomisationModes> fallbackModeMap{
			{ "none", RandomisationModes::NoRandomisation },
			{ "set", RandomisationModes::SameSet },
			{ "sameset", RandomisationModes::SameSet },
			{ "any", RandomisationModes::AnySet },
			{ "anyset", RandomisationModes::AnySet },
		};
		RandomisationModes defaultFallbackMode = FallbackMode;
		FallbackMode = ConfigParser::ParseValueMap(ini, category, "FallbackType", fallbackModeMap, RandomisationModes::UndefinedRandomisationMode);
		if (FallbackMode == RandomisationModes::UndefinedRandomisationMode) {
			FallbackMode = ConfigParser::ParseInteger(ini, category, "FallbackType", defaultFallbackMode,
				RandomisationModes::NoRandomisation, RandomisationModes::AnySet, true);
		}

		std::map<std::string, CardModes> cardModeMap{
			{ "none", CardModes::NoCards },
			{ "add", CardModes::AddCardLevel },
			{ "overwrite", CardModes::ReplaceCardLevel },
		};
		CardModes defaultCardMode = CardMode;
		CardMode = ConfigParser::ParseValueMap(ini, category, "CardType", cardModeMap, CardModes::UndefinedCardMode);
		if (CardMode == CardModes::UndefinedCardMode) {
			CardMode = ConfigParser::ParseInteger(ini, category, "CardType", defaultCardMode,
				CardModes::NoCards, CardModes::ReplaceCardLevel, true);
		}

		std::map<std::string, MainModes> mainModeMap{
			{ "none", MainModes::NoMainMode },
			{ "no", MainModes::NoMainMode },
			{ "off", MainModes::NoMainMode },
			{ "default", MainModes::DefaultMainMode },
			{ "normal", MainModes::DefaultMainMode },
			{ "standard", MainModes::DefaultMainMode },
			{ "tags", MainModes::CacheTags },
			{ "cachetags", MainModes::CacheTags },
			{ "randomtags", MainModes::RandomCacheTags },
			{ "randomcachetags", MainModes::RandomCacheTags },		
		};
		MainMode = ConfigParser::ParseValueMap(ini, category, "Mode", mainModeMap, MainModes::DefaultMainMode);
		SetupMainMode();
	}

	void Seed::SetupMainMode(MainModes mainMode) {
		if (mainMode == MainModes::UndefinedMainMode) {
			mainMode = MainMode;
		}
		switch (mainMode) {
			case MainModes::NoMainMode:
				Enabled = false;
				break;
			case MainModes::RandomCacheTags:
				if (FallbackMode == RandomisationModes::AnySet) {
					FallbackMode = RandomisationModes::SameSet;
				}
				DogTagItems = true;
				SpawnRegularWorldPickups = false;
				SpawnUnmatchedWorldPickups = false;
				SpawnDroppedPickups = false;
				break;
			case MainModes::CacheTags:
				RandomisationMode = RandomisationModes::NoRandomisation;
				DogTagItems = true;
				SpawnRegularWorldPickups = false;
				SpawnUnmatchedWorldPickups = false;
				SpawnDroppedPickups = false;
				break;
		}
	}

	bool Seed::IncludesLocationType(int type) const {
		return ((LocationTypes & type) == type);
	}

	unsigned short Seed::GetSettingsHash() const {
		// less-common options at the back = potential for 2-character seed part
		size_t i = 0;
		unsigned int result = 0;

		char mainMode = (MainMode < 0) ? 0 : MainMode;
		result |= MainMode << i; i += 2; // increase if MainMode goes over 4 possible values (currently 4)
		result |= RandomisationMode << i; i += 2; // inc. if RandomisationMode > 4 poss values (curr. 3)
		result |= FallbackMode << i; i += 2; // inc. if FallbackMode > 4 poss values (curr. 3)
		result |= CardMode << i; i += 2; // inc. if CardMode > 4 poss values (curr. 3)
		result |= AmmoMode << i; i += 2; // inc. if AmmoMode > 4 poss values (curr. 3)
		result |= LogicDifficulty << i; i += 2; // inc. if LogicDifficulty > 4 poss values (curr. 3)
		result |= LocationTypes << i; i += 3; // TODO can we avoid this?
		result |= Logic << i++;

		return result;
	}

	void Seed::SetSettingsHash(unsigned short hash) {
		MainMode = (MainModes)(hash & 3); hash >>= 2;
		RandomisationMode = (RandomisationModes)(hash & 3); hash >>= 2;
		FallbackMode = (RandomisationModes)(hash & 3); hash >>= 2;
		CardMode = (CardModes)(hash & 3); hash >>= 2;
		AmmoMode = (AmmoModes)(hash & 3); hash >>= 2;
		LogicDifficulty = (LogicDifficulties)(hash & 3); hash >>= 2;
		LocationTypes = (char)(hash & 7); hash >>= 3;
		Logic = (bool)(hash & 1); hash >>= 1;
		
		SetupMainMode();
	}

	std::string Seed::FullSeed() const {
		return fmt::format("{:X}/{:X}", SeedNumber, GetSettingsHash());
	}



	const Pickup* LocationGroup::CurrentRandomPickup() const {
		if (FallbackPickup && (!FallbackPickup->IsCollected()) && (Mem::Progress() > FallbackAfterProgress)) {
			return FallbackPickup;
		}
		return RandomPickup;
	}

	bool Location::IsAvailableBy(short progress, bool allowBefore) const {
		if (progress == SHRT_MIN) {
			progress = Mem::Progress();
		}
		if (!CurrentDataSet.IsCardAvailable(RequiredCardLevel, progress)) return false;
		if ((!allowBefore) && (progress < MinProgress)) return false;
		if (progress > MaxProgress) return false;
		return true;
	}

	bool Location::IsAvailableBefore(short progress) const {
		return IsAvailableBy(progress, true);
	}

	bool Location::InDifficulty(DifficultyBits difficulty) const {
		return ((Difficulties & difficulty) == difficulty);
	}

	bool LocationGroup::IsAvailableAt(short progress, bool allowBefore) const {
		for (auto location : Locations) {
			if (location->IsAvailableBy(progress, allowBefore)) {
				return true;
			}
		}
		return false;
	}

	bool LocationGroup::IsAvailableBefore(short progress) const {
		return IsAvailableAt(progress, true);
	}

	short LocationGroup::MinProgress() const {
		short progress = SHRT_MAX;
		for (auto location : Locations) {
			if (location->MinProgress < progress) {
				progress = location->MinProgress;
			}
		}
		return progress;
	}

	short LocationGroup::MaxProgress() const {
		short progress = SHRT_MIN;
		for (auto location : Locations) {
			if (location->MaxProgress > progress) {
				progress = location->MaxProgress;
			}
		}
		return progress;
	}


	short Pickup::SetAmmo(short ammo, bool addContinue) const {
		short* currentData = (Set == Sets::Weapons) ? (short*)*Mem::WeaponData : (short*)*Mem::ItemData;

		if (ammo >= -1) {
			currentData[Id] = ammo;
			if (addContinue) {
				short* continueData = currentData + Mem::WeaponItemContinueOffset;
				continueData[Id] = ammo;
			}
		}

		return currentData[Id];
	}

	short Pickup::GetAmmo() const {
		return SetAmmo();
	}

	short Pickup::GetMaxAmmo() const {
		short* maxData = (Set == Sets::Weapons) ? 
			(short*)(*Mem::WeaponData + Mem::WeaponMaxOffset) :
			(short*)(*Mem::ItemData + Mem::ItemMaxOffset);

		return maxData[Id];
	}

	bool Pickup::IsCollected(short ammo) const {
		short currentAmmo = (ammo == SHRT_MIN) ? GetAmmo() : ammo;
		return (currentAmmo == AmmoIfCollected);
	}

	bool Pickup::operator==(const Pickup& rhs) const {
		if (Id != rhs.Id) return false;
		if (Category != rhs.Category) return false;
		return true;
	}
	/*
	void KeyPickupPair_Deprecated::SaveCurrentAmmo() {
		OriginalStoredAmmo = OriginalPickup->GetAmmo();
	}

	void KeyPickupPair_Deprecated::SwapInventoryAmmo() {
		if (OriginalStoredAmmo == SHRT_MIN) return;
		bool isCollected = OriginalPickup->IsCollected(OriginalStoredAmmo);
		short targetAmmo = isCollected ? RandomPickup->AmmoIfCollected : RandomPickup->AmmoIfMissing;
		RandomPickup->SetAmmo(targetAmmo, true);
		IsSwapped = true;
	}

	void KeyPickupPair_Deprecated::ReturnInventoryAmmo() {
		OriginalPickup->SetAmmo(OriginalStoredAmmo);
		OriginalStoredAmmo = SHRT_MIN;
		IsSwapped = false;
	}
	*/
	void DataSet::Init() {
		CardAvailability.clear();
		LGsByLocation.clear();
		LGsByOriginalPickup.clear();
		LGsByRandomPickup.clear();
		CurrentStage = Mem::Stage();
		PopulateLocations();
		RandomiseLocations();
		CurrentSeed.NeedsReroll = false;

		// apply randomised initial loadout
		if (CurrentSeed.LocationTypes & Location::Types::Loadout) {
			for (auto a : LGsByOriginalPickup) {
				auto& pickup = a.first;
				AmmoState[pickup.Set][pickup.Id] = pickup.AmmoIfMissing;
			}
			AmmoStateDirty = true;
			RestoreAmmoState(true);
		}
	}

	void DataSet::PopulateLocations() {
		auto itLocs = PickupLocations.find(CurrentStage);
		if (itLocs == PickupLocations.end()) return;

		auto& locations = itLocs->second;
		for (const auto& location : locations) {

			std::shared_ptr<LocationGroup> locationGroup;

			const Pickup* pickup = location.OriginalPickup;

			if ((pickup->IsCardWithLevel) && (CurrentSeed.CardMode == Seed::CardModes::NoCards)) {
				continue;
			}
			if (!CurrentSeed.IncludesLocationType(location.Type)) {
				continue;
			}

			if (pickup->IsKeyPickup) {
				auto itPickupLocGrp = LGsByOriginalPickup.find(*pickup);

				if (itPickupLocGrp == LGsByOriginalPickup.end()) {
					locationGroup = std::make_shared<LocationGroup>();
					locationGroup->OriginalPickup = pickup;
					LGsByOriginalPickup[*pickup] = locationGroup;
				}
				else {
					locationGroup = itPickupLocGrp->second;
				}

				LGsByLocation[location] = locationGroup;
			}
			else {
				auto itLocGrp = LGsByLocation.find(location);
				if (itLocGrp == LGsByLocation.end()) {
					locationGroup = std::make_shared<LocationGroup>();
					locationGroup->OriginalPickup = pickup;
					LGsByLocation[location] = locationGroup;
				}
				else {
					locationGroup = itLocGrp->second;
				}
			}

			locationGroup->Locations.push_back(&location);

		}
	}

	std::vector<FallbackPickupDefinition> DataSet::RandomiseKeyPickupLocations(std::vector<RequiredKeyPickup>& requiredKeyPickups, std::vector<std::shared_ptr<LocationGroup>>& shuffledLocationGroups, bool nextAttempt) {
		static int attempts = 0;

		if (nextAttempt) {
			std::shuffle(shuffledLocationGroups.begin(), shuffledLocationGroups.end(), CurrentSeed.RNG); // reshuffle
			attempts++;
		}
		else {
			attempts = 0;
		}

		CardAvailability.clear();
		std::vector<FallbackPickupDefinition> requiredFallbackKeyPickups;

		for (auto& rkp : requiredKeyPickups) {
			auto pickup = rkp.RandomPickup(CurrentSeed.LogicDifficulty);
			if (LGsByOriginalPickup.find(*pickup) == LGsByOriginalPickup.end()) continue; // this item doesn't need to be shuffled
			if (LGsByRandomPickup.find(*pickup) != LGsByRandomPickup.end()) continue; // this item has already been shuffled

			bool successfullyMoved = false;
			size_t failures = 0;
			short minProgressAvailable;

			for (auto shuffledLG : shuffledLocationGroups) {
				if (shuffledLG->RandomPickup) continue; // this location already has a random item
				if (!shuffledLG->IsAvailableBefore(rkp.MaxProgress)) continue;
				if (!shuffledLG->IsAvailableAt(rkp.MaxProgress)) {
					requiredFallbackKeyPickups.push_back({ &rkp, pickup, shuffledLG->MaxProgress() });
				}

				shuffledLG->RandomPickup = pickup;
				auto itKLG = LGsByOriginalPickup.find(*shuffledLG->OriginalPickup);
				if (itKLG == LGsByOriginalPickup.end()) continue; // shouldn't ever happen, but...
				auto locationSrc = itKLG->second;
				locationSrc->RandomPickup = pickup;
				LGsByRandomPickup[*pickup] = locationSrc;
				minProgressAvailable = shuffledLG->MinProgress();

				successfullyMoved = true;
				break;
			}

			if (!successfullyMoved) {
				if (attempts > 50) {
					// just let it go :(
					failures++;
					std::string msg = fmt::format("[ItemRando] {} required pickup(s) not randomised correctly", failures);
					Log::DisplayText(msg, 5);
				}
				else {
					// try again with a completely new shuffle
					return RandomiseKeyPickupLocations(requiredKeyPickups, shuffledLocationGroups, true);
				}
			}

			// card coming through in order so let's keep it clean
			if (pickup->IsCardWithLevel) {
				auto& avail = CurrentDataSet.CardAvailability;
				if (avail.size() && (avail.back() > minProgressAvailable)) {
					avail.back() = minProgressAvailable;
				}
				avail.push_back(minProgressAvailable);
			}
		}

		return requiredFallbackKeyPickups;
	}

	void DataSet::RandomiseLocations() {
		if (CurrentSeed.RandomisationMode == Seed::RandomisationModes::NoRandomisation) return;

		// make a vector of all locationgroups to work with
		std::vector<std::shared_ptr<LocationGroup>> shuffledLocGrps;
		std::transform(LGsByLocation.begin(), LGsByLocation.end(), std::back_inserter(shuffledLocGrps),
			[](const auto& pair) { return pair.second; });

		std::vector<std::shared_ptr<LocationGroup>> shuffledKeyLocGrps;
		std::transform(LGsByOriginalPickup.begin(), LGsByOriginalPickup.end(), std::back_inserter(shuffledKeyLocGrps),
			[](const auto& pair) { return pair.second; });

		// shuffle them
		std::shuffle(shuffledLocGrps.begin(), shuffledLocGrps.end(), CurrentSeed.RNG);
		std::shuffle(shuffledKeyLocGrps.begin(), shuffledKeyLocGrps.end(), CurrentSeed.RNG);
		
		// go through the non-shuffled to find randoms, removing from shuffled each time
		std::vector<FallbackPickupDefinition> requiredFallbackKeyPickups;
		bool sameSet = (CurrentSeed.RandomisationMode == Seed::RandomisationModes::SameSet);

		// shuffle required pickups
		if (CurrentSeed.Logic) {
			auto itRKP = RequiredKeyPickups.find(CurrentStage);
			if (itRKP != RequiredKeyPickups.end()) {
				auto& requiredKeyPickups = itRKP->second;
				requiredFallbackKeyPickups = RandomiseKeyPickupLocations(requiredKeyPickups, sameSet ? shuffledKeyLocGrps : shuffledLocGrps);
			}
		}

		// shuffle other key items
		for (auto itKLG : LGsByOriginalPickup) {
			auto pickup = itKLG.first;
			auto locGroup = itKLG.second;

			if (LGsByRandomPickup.find(pickup) == LGsByRandomPickup.end()) continue; // pickup has already been randomised

			for (std::shared_ptr<LocationGroup> shuffledLG : sameSet ? shuffledKeyLocGrps : shuffledLocGrps) {
				if (shuffledLG->RandomPickup) continue; // this location already has a random pickup
				shuffledLG->RandomPickup = locGroup->OriginalPickup;
				if (pickup.IsKeyPickup) {
					LGsByRandomPickup[pickup] = locGroup;
				}
				break;
			}
			
		}

		// shuffle regular items
		for (auto& itLG : LGsByLocation) {
			auto locGroup = itLG.second;

			if (locGroup->OriginalPickup->IsKeyPickup) continue;

			for (auto location : shuffledLocGrps) {
				if (location->RandomPickup) continue; // already has a random pickup
				location->RandomPickup = locGroup->OriginalPickup;
				break;
			}
		}

		// fallbacks
		if (requiredFallbackKeyPickups.size()) {
			bool sameFallbackSet = (CurrentSeed.FallbackMode == Seed::RandomisationModes::SameSet);
			auto& fallbackSet = sameFallbackSet ? shuffledKeyLocGrps : shuffledLocGrps;
			std::shuffle(fallbackSet.begin(), fallbackSet.end(), CurrentSeed.RNG);

			for (auto &itFPD : requiredFallbackKeyPickups) {
				for (auto shuffledLG : fallbackSet) {
					if (shuffledLG->FallbackPickup) continue; // location already has a fallback pickup
					if (!shuffledLG->IsAvailableAt(itFPD.RequiredPickupDefinition->MaxProgress)) continue;

					shuffledLG->FallbackPickup = itFPD.SelectedPickup;
					shuffledLG->FallbackAfterProgress = itFPD.InitialRandomMaxProgress;
				}
			}
		}
	}

	bool DataSet::IsCardAvailable(short level, short progress) const {
		if ((int)CardAvailability.size() <= level) return true;

		short minProgress = CardAvailability.at(level - 1);
		return (progress >= minProgress);
	}

	void DataSet::StoreAmmoState() {
		memset(AmmoState, 0, sizeof(AmmoState));

		// no point in storing state for items that aren't randomised
		for (auto& a : LGsByOriginalPickup) {
			auto &pickup = a.first;
			if (pickup.GetMaxAmmo() == 9999) continue;

			AmmoState[pickup.Set][pickup.Id] = pickup.GetAmmo();
			AmmoStateStored = true;
		}
	}

	void DataSet::DisplayNewPickups(const std::vector<std::string>& pickupNames) const {
		std::string strPickups = (pickupNames.size() == 1) ? pickupNames.front() :
			std::accumulate(pickupNames.begin(), pickupNames.end(), std::string(),
				[](std::string s1, std::string s2) {
					return s1.empty() ? s2 : s1 + ", " + s2;
				});
		std::string msg = fmt::format("New: {}", strPickups);
		Log::DisplayText(msg, 5);
	}

	void DataSet::NeutraliseAmmoState() {
		for (auto& a : LGsByOriginalPickup) {
			auto& pickup = a.first;
			if (pickup.GetMaxAmmo() == 9999) continue;

			pickup.SetAmmo(pickup.AmmoIfMissing);
			AmmoStateDirty = true;
		}
	}

	void DataSet::RestoreAmmoState(bool awardNewPickups) {
		std::unordered_map<const Pickup, short, PickupHasher> ammoToAward;
		std::vector<std::string> awardedPickupNames;

		// again, this is only for randomised items
		if (awardNewPickups) {
			for (auto a : LGsByOriginalPickup) {
				auto& pickup = a.first;
				auto locGroup = a.second;

				if (pickup.GetMaxAmmo() == 9999) continue;

				short storedAmmo = AmmoState[pickup.Set][pickup.Id];
				short liveAmmo = pickup.GetAmmo();

				if (liveAmmo > pickup.AmmoIfMissing) {
					// award the random pickup (if awardable, e.g. uncollected weapon's ammo = no)
					auto* randomPickup = locGroup->CurrentRandomPickup();
					if (randomPickup == nullptr) continue;

					short randomStoredAmmo = AmmoState[randomPickup->Set][randomPickup->Id];

					bool isStandalonePickup = (randomPickup->Category != Pickup::Categories::WeaponAmmoPickup);
					bool isCollectedWeaponAmmo = ((!isStandalonePickup) && (randomStoredAmmo > randomPickup->AmmoIfMissing));

					if (isStandalonePickup || isCollectedWeaponAmmo) {
						if (randomStoredAmmo < 0) randomStoredAmmo = 0;
						short randomMaxAmmo = randomPickup->GetMaxAmmo();

						short ammoToAdd = 0;
						switch (locGroup->Locations.front()->AmmoOverride) {
							case Location::AmmoOverrides::NoAmmoOverride:
								ammoToAdd = randomPickup->Ammo;
								break;
							case Location::AmmoOverrides::MaxAmmo:
								ammoToAdd = randomMaxAmmo;
								break;
							case Location::AmmoOverrides::ByDifficulty:
								if (*Mem::Difficulty > 20) {
									ammoToAdd = randomPickup->Ammo;
								}
								else if (*Mem::Difficulty == 10) {
									ammoToAdd = randomMaxAmmo;
								}
								else { // 20 (EZ)
									// not sure how reliable this is
									ammoToAdd = ((randomPickup->Category == Pickup::Categories::ItemPickup) || (!randomPickup->SecondaryItem)) ?
										(randomPickup->Ammo * 3) : ((randomPickup->SecondaryItem->Ammo * 3) + 1);
								}
								break;
							case Location::AmmoOverrides::NoAmmo:
								continue;
						}
						short ammoToSet = randomStoredAmmo + ammoToAdd;
						if (ammoToSet > randomMaxAmmo) ammoToSet = randomMaxAmmo;

						AmmoState[randomPickup->Set][randomPickup->Id] = ammoToSet;

						std::string name(randomPickup->Name);
						awardedPickupNames.push_back(name);
					}
				}
			}

			if (awardedPickupNames.size()) {
				DataSet::DisplayNewPickups(awardedPickupNames);
			}

		}

		// reset the actual values
		for (auto a : LGsByOriginalPickup) {
			auto& pickup = a.first;
			auto locGroup = a.second;

			if (pickup.GetMaxAmmo() == 9999) continue;

			// if cutscenes aren't randomised and ammo has changed, let it be
			if ((!awardNewPickups) && (pickup.GetAmmo() != pickup.AmmoIfMissing)) continue;
			// otherwise, restore the previous or apply random
			pickup.SetAmmo(AmmoState[pickup.Set][pickup.Id]);
		}

		AmmoStateStored = false;
		AmmoStateDirty = false;
	}

	PickupSpawn::PickupSpawn() {}
	PickupSpawn::PickupSpawn(int param_1, int param_2, int param_3) {
		CallParams[0] = param_1;
		CallParams[1] = param_2;
		CallParams[2] = param_3;

		Ptr.B = (int*)(param_1 + 0xA8);
		Ptr.F = (int*)(param_1 + 0x40);
		Ptr.I = (int*)(param_1 + 0x44);
		Ptr.N = (int*)(param_1 + 0x48);
		Ptr.T = (int*)(param_1 + 0x58);
		Ptr.U = (int*)(param_1 + 0x5C);
		Ptr.X = (int*)(param_1 + 0x8C);
		Ptr.PX = (float*)(param_1 + 0x230);
		Ptr.PY = (float*)(param_1 + 0x234);
		Ptr.PZ = (float*)(param_1 + 0x238);

		Param.F = Mem::GclParamInit('f') ? Mem::GclParamReadNext() : 0;
		Param.I = Mem::GclParamInit('i') ? Mem::GclParamReadNext() : 0;
		Param.N = Mem::GclParamInit('n') ? Mem::GclParamReadNext() : 0;
		Mem::GclParamInit('p');
		Param.PX = Mem::GclParamReadNext();
		Param.PY = Mem::GclParamReadNext();
		Param.PZ = Mem::GclParamReadNext();
		Param.T = Mem::GclParamInit('t') ? Mem::GclParamReadNext() : -1;
		Param.X = Mem::GclParamInit('x') ? Mem::GclParamReadNext() : 0;
	}

	int PickupSpawn::RunHook() {
		return oFUN_00799210(CallParams[0], CallParams[1], CallParams[2]);
	}

	int PickupSpawn::ApplyPickup(const Pickup* pickup, bool runHook) {
		int result = runHook ? RunHook() : 0;
		
		*Ptr.F = pickup->Category;
		*Ptr.I = pickup->Id;
		*Ptr.N = pickup->Ammo;

		return result;
	}

	
	const Pickup* PickupSpawn::CacheTagBestPickup() const {
		float minStock = 1.01f;
		std::vector<const Pickup*> candidateItems;
		std::vector<const Pickup*> unfoundCandidateItems;
		for (auto pickup : PickupsWithConsumables) {
			if ((pickup == &PICKUP_RATION) && (*Mem::Difficulty == Difficulty::EuroExtreme)) continue;
			short current = pickup->GetAmmo();
			short max = pickup->GetMaxAmmo();
			if ((!max) || (max > 999)) continue;
			if (current < 0) {
				if (pickup->Category != Pickup::Categories::WeaponAmmoPickup) { // i.e. it's a standalone weapon pickup such as grenades
					unfoundCandidateItems.push_back(pickup);
				}
				continue;
			}
			if ((pickup->Category == Pickup::Categories::ItemPickup) && (current == 0)) { // i.e. item pickup such as rations
				unfoundCandidateItems.push_back(pickup);
				continue;
			}
			float stock = (float)current / (float)max;
			if (stock <= minStock) {
				if (stock < minStock) {
					candidateItems.clear();
				}
				candidateItems.push_back(pickup);
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




	// before and after running main() every frame
	void __cdecl hkFUN_008781e0(int param_1) {
		try_mgs2
			if (Mem::Stage() == MGS2::Stage::None) {
				if (!CurrentSeed.NeedsReroll) {
					CurrentSeed.NeedsReroll = true;
					//CurrentSeed.WriteReport();
					CurrentSeed = DefaultSeed;
					//RegisterNewGameWarning();
				}
				return oFUN_008781e0(param_1);
			}

			if ((*(int*)(param_1 + 0x30) == 0) && (*(int*)0xF6E4E0 != 0)) {
				if ((Mem::Stage() == Stage::Plant) && (Mem::Progress() == 1)) {
					CurrentDataSet.Init();
				}
				else if ((Mem::Stage() == Stage::Tanker) && (Mem::Progress() == 12)) {
					CurrentDataSet.Init();
				}
				else if (CurrentSeed.NeedsReroll) return oFUN_008781e0(param_1); // bail

				if (!CurrentDataSet.AmmoStateStored) {
					CurrentDataSet.StoreAmmoState();
				}
				oFUN_008781e0(param_1);
				return;
			}
		catch_mgs2(Category, "8781E0");

		// run the original
		oFUN_008781e0(param_1);
	}

	// before/after running main GCL for stage
	int hkFUN_004e4090(int param_1) {
		try_mgs2
			if (!CurrentDataSet.AmmoStateStored) {
				CurrentDataSet.StoreAmmoState();
			}
			if (CurrentDataSet.AmmoStateStored) {
				CurrentDataSet.NeutraliseAmmoState();
			}
			int result = oFUN_004e4090(param_1);
			if (CurrentDataSet.AmmoStateDirty) {
				CurrentDataSet.RestoreAmmoState(CurrentSeed.LocationTypes & Location::Types::Cutscene);
			}

			return result;
		catch_mgs2(Category, "4E4090");

		return oFUN_004e4090(param_1);
	}

	// on pickup spawn (item_box_init)
	int hkFUN_00799210(int param_1, int param_2, int param_3) {
		bool hookCalled = false;
		int result = 0;

		try_mgs2
			CurrentDataSet.CurrentPickupSpawn = PickupSpawn(param_1, param_2, param_3);
			auto& spawn = CurrentDataSet.CurrentPickupSpawn;

			bool isPostStart = (*Mem::AreaTimeFrames > 0);
			bool isDogTag = ((spawn.Param.I == PICKUP_DOGTAG.Id) && (spawn.Param.F == PICKUP_DOGTAG.Category));

			if (isDogTag) {
				if (CurrentSeed.DogTagItems) {
					auto pickup = spawn.CacheTagBestPickup();
					int result = spawn.ApplyPickup(pickup, true); // cachetags dog tag spawn
					*spawn.Ptr.T = -1; // remove the custom pickup caption (DT name)
					return result;
				}
				return spawn.RunHook(); // usual dog tag spawn
			}

			std::shared_ptr<LocationGroup> locationGroup;

			// try to find the item in the key items set
			Pickup testPickup((char)spawn.Param.I, (Pickup::Categories)spawn.Param.F);
			auto it = CurrentDataSet.LGsByOriginalPickup.find(testPickup);
			if (it != CurrentDataSet.LGsByOriginalPickup.end()) {
				locationGroup = it->second;
			}
			else if (strlen(Mem::AreaCode) <= 4) {
				// key item not found or not a key item
				// -> try to find the location
				std::string strAreaCode(Mem::AreaCode);
				Location testLocation{ .AreaCode = strAreaCode, .X = spawn.Param.PX, .Y = (spawn.Param.PY + 1), .Z = spawn.Param.PZ };
				auto it = CurrentDataSet.LGsByLocation.find(testLocation);
				if (it == CurrentDataSet.LGsByLocation.end()) {
					// no match, figure out what type of spawn we have
					if (isPostStart) {
						if (CurrentSeed.SpawnDroppedPickups) return spawn.RunHook(); // usual dropped pickup spawn
						else return -1; // sabotage dropped pickup spawn
					}
					if (CurrentSeed.SpawnUnmatchedWorldPickups) return spawn.RunHook(); // usual unmatched world pickup spawn
					else return -1; // sabotage unmatched world pickup spawn
				}
				locationGroup = it->second;
				if (locationGroup->Locations.front()->CanRespawn) return spawn.RunHook(); // usual respawnable spawn
				else if (!CurrentSeed.SpawnRegularWorldPickups) return -1; // sabotage regular world pickup
			}

			if ((!locationGroup) || (!locationGroup->RandomPickup)) {
				return spawn.RunHook(); // fallback to usual spawn if other cases didn't match
			}

			const Pickup* randomPickup = locationGroup->CurrentRandomPickup();
			return spawn.ApplyPickup(randomPickup, true);
		catch_mgs2(Category, "799210");

		return -1;
	}

	// item_box_init main param reading func
	int hkFUN_007996d0(int param_1) {
		try_mgs2

		catch_mgs2(Category, "7996D0");

		return oFUN_007996d0(param_1);
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false)) || ItemRando::Enabled)	return;
		if (ConfigParser::ParseInteger<int>(ini, Category, "Version", 2, 0, INT_MAX, true) != 3) return;

		DefaultSeed.ParseConfig(ini);
		CurrentSeed = DefaultSeed;

		// tmp
		//DefaultSeed.RandomisationMode = Seed::RandomisationModes::AnySet;
		DefaultSeed.LocationTypes = 7;


#ifdef _DEBUG
		const char* databasePath = ini.GetValue(Category, "DatabasePath");
		if (databasePath) {
			DB = new Database(databasePath);
			Actions::RegisterAction(ini, "ItemRando.DB.Toggle", &DBToggleAction);

			DB->OutputLocationSet(Stage::Tanker);
			DB->OutputLocationSet(Stage::Plant);
		}
#endif

		oFUN_004e4090 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x4E4090, (BYTE*)hkFUN_004e4090, 8);
		oFUN_00799210 = (tFUN_Int_IntIntInt)mem::TrampHook32((BYTE*)0x799210, (BYTE*)hkFUN_00799210, 6);
		oFUN_007996d0 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x7996D0, (BYTE*)hkFUN_007996d0, 7); // params
		oFUN_008781e0 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x8781E0, (BYTE*)hkFUN_008781e0, 9);

	}

}
