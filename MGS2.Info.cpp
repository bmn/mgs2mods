#include "MGS2.framework.h"
#include <bitset>
#include <numeric>

namespace MGS2::Info {
	const char* Category = "Info";

	bool Active = true;
	
	TextConfig RandConfig{ false, 10, 372, (TextAlignment)Left, 0xB4B4B4, "RNG {}" };
	TextConfig InputDisplayConfig{ false, 10, 372, (TextAlignment)Left };
	TextConfig TurboSpeedConfig{ false, 10, 389, (TextAlignment)Left, 0xB4B4B4, "{:02d} /sec" };
	TextConfig PositionConfig{ false, 10, 406, (TextAlignment)Left, 0xB4B4B4, "A: {Angle:06.2f}\nX:{X:+011.4f}\nY: {Y:+011.4f}\nZ: {Z:+011.4f}" };
	TextConfig Hold3CameraConfig{ .Enabled = false, .PosX = 270, .PosY = 231, .Align = (TextAlignment)Right, .Color = 0x648736, .OutlineColor = -1 };

	int InputDisplayFrames = 0;
	int InputDisplayFramesPrevSecond = 0;

	char InputDisplayBitOrder[]{ 12,14,15,13,4,7,5,6,11,8,0,1,2,3,17,10 };
	const char* InputDisplayBitNames[]{
		"U", "D", "L", "R", "Tr", "Sq", "O", "X", "St", "Sl", "L1", "R1", "L2", "R2", "L3", "R3"
	};

	std::map<unsigned short, const char*> Hold3SubjectNames{
		{ 10, "Front" },
		{ 20, "Front-Right" },
		{ 30, "Front-Left" },
		{ 40, "Marines Logo" },
		{ 130, "Scott Dolph" },
		{ 140, "Hostile Olga" },
		{ 150, "Sleeping Olga" },
		{ 160, "Unfortunate Marine" },
		{ 170, "Mystery" },
		{ 180, "Idol Poster" },
		{ 190, "EyeWire Poster" },
		{ 200, "Couple Posters" },
	};
	mem::PatchSet Hold3CameraPatches = mem::PatchSet{
		mem::Patch{ (void*)0x7DFB53, "\xEB" }, // always run shutter check when in camera
		mem::Patch{ (void*)0x7DFB70, "\x90\x90\x90\x90\x90\x90" }, // same
	};


	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		if (!Active) {
			oFUN_00878f70();
			return;
		}

		try_mgs2
			if (strcmp((char*)0x118ADEC, "init") == 0)
				throw "Game not initialised yet";
			
			if (InputDisplayConfig.Enabled) {
				std::bitset<32> bits((unsigned long long)*Mem::PlayerInput);

				std::vector<const char*> bitArray;
				char j = 0;
				bool go = false;
				for (char i = 0; i < 16; i++) {
					char k = InputDisplayBitOrder[i];
					if (bits.test(k)) {
						bitArray.push_back(InputDisplayBitNames[i]);
						go = true;
					}
				}
				if (go) {
					std::string bitString = std::accumulate(bitArray.begin(), bitArray.end(), std::string(" "));
					InputDisplayConfig.Draw(bitString.c_str());
				}
			}
			
			if (TurboSpeedConfig.Enabled) {
				if (*Mem::PlayerNewInput != 0)
					InputDisplayFrames++;

				if ((*Mem::RenderedFrames % 60) == 0) {
					InputDisplayFramesPrevSecond = InputDisplayFrames;
					InputDisplayFrames = 0;
				}
				
				MGS2::TextSetup(TurboSpeedConfig);
				MGS2::TextDraw(TurboSpeedConfig.Content, InputDisplayFramesPrevSecond);
			}

			if (PositionConfig.Enabled) {
				uintptr_t pPos = *Mem::PositionData;
				if (pPos != 0) {
					float posX = *(float*)pPos;
					float posY = *(float*)(pPos + 4);
					float posZ = *(float*)(pPos + 8);
					short angle = *(short*)(pPos + 0x2A);

					unsigned short cwAngle = (0x1800 - angle) % 0x1000;
					float degAngle = (float)cwAngle * 360 / 0x1000;

					std::string posString = fmt::format(fmt::runtime(PositionConfig.Content),
						"X"_a = posX,
						"Y"_a = posY,
						"Z"_a = posZ,
						"Angle"_a = degAngle,
						"RawAngle"_a = cwAngle
						);
					PositionConfig.Draw(posString.c_str());
				}
			}

			if (Hold3CameraConfig.Enabled) {
				unsigned short h3CamSubject = *(unsigned short*)0x118D93E;
				if ((h3CamSubject != 0) && (h3CamSubject != 1000) && (Hold3SubjectNames.contains(h3CamSubject))) {
					const char* h3CamString = Hold3SubjectNames.at(h3CamSubject);
					Hold3CameraConfig.Draw(h3CamString);
				}
			}

		catch_mgs2(Category, "878F70");

		oFUN_00878f70();
	}

	// rand display
	tFUN_Int_Void oFUN_00580810;
	int __cdecl hkFUN_00580810() {
		int result = oFUN_00580810();

		try_mgs2
			if (RandConfig.Enabled) {
				std::string randString = fmt::format(fmt::runtime(RandConfig.Content), result);
				Log::DisplayText(randString, RandConfig);
			}
		catch_mgs2(Category, "580810");

		return result;
	}



	void ToggleAction(Actions::Action action) {
		if (Hold3CameraConfig.Enabled) {
			Hold3CameraPatches.Patch(Active);
		}
		Active = !Active;
	}

	void ToggleTurboSpeed(Actions::Action action) {
		if (!Active) {
			Active = true;
		}
		TurboSpeedConfig.Enabled = !TurboSpeedConfig.Enabled;
	}

	void ToggleInputDisplay(Actions::Action action) {
		if (!Active) {
			Active = true;
		}
		InputDisplayConfig.Enabled = !InputDisplayConfig.Enabled;
	}
	
	void TogglePosition(Actions::Action action) {
		if (!Active) {
			Active = true;
		}
		PositionConfig.Enabled = !PositionConfig.Enabled;
	}

	void ToggleHold3Camera(Actions::Action action) {
		if (!Active) {
			Active = true;
		}
		Hold3CameraPatches.Patch(Hold3CameraConfig.Enabled);
		Hold3CameraConfig.Enabled = !Hold3CameraConfig.Enabled;
	}

	void ToggleRand(Actions::Action action) {
		if (!Active) {
			Active = true;
		}
		RandConfig.Enabled = !RandConfig.Enabled;
	}

	
	bool NewGameInfoHold3CameraCallback() {
		return ((Active) && (Hold3CameraConfig.Enabled) && (NewGameInfo::IsTankerSelected()));
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		TurboSpeedConfig.ParseConfig(ini, "Info.TurboSpeed");
		InputDisplayConfig.ParseConfig(ini, "Info.InputDisplay");
		PositionConfig.ParseConfig(ini, "Info.Position");
		Hold3CameraConfig.ParseConfig(ini, "Info.CameraId");
		RandConfig.ParseConfig(ini, "Info.Rand");
		
		Actions::RegisterAction(ini, "Info.Toggle", &ToggleAction);
		Actions::RegisterAction(ini, "Info.TurboSpeed", &ToggleTurboSpeed);
		Actions::RegisterAction(ini, "Info.InputDisplay", &ToggleInputDisplay);
		Actions::RegisterAction(ini, "Info.Position", &TogglePosition);
		Actions::RegisterAction(ini, "Info.CameraId", &ToggleHold3Camera);
		Actions::RegisterAction(ini, "Info.Rand", &ToggleRand);

		Active = ini.GetBoolValue(Category, "Active", Active);

		if ((Active) && (Hold3CameraConfig.Enabled)) {
			Hold3CameraPatches.Patch();
		}
		NewGameInfo::AddWarning("Camera Subject ID", &NewGameInfoHold3CameraCallback);

		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
		oFUN_00580810 = (tFUN_Int_Void)mem::TrampHook32((BYTE*)0x580810, (BYTE*)hkFUN_00580810, 6);
	}

}
