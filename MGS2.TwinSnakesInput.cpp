#include "MGS2.framework.h"

namespace MGS2::TwinSnakesInput {
	bool Active = true;
	//bool DisablePS2OnlyButtons = true;
	bool TtsStart = true;
	bool FastEquipMenu = true;
	bool AutoAimX = false;

	unsigned int UnmodifiedButtons;


	void ToggleAction(Actions::Action action) {
		Active = !Active;
		Log::DisplayToggleMessage("Twin Snakes Input", Active);
	}


	// Register BEFORE turbo so it's earlier in the chain
	tFUN_Void_PshortPint oFUN_008cfe60;
	void __cdecl hkFUN_008cfe60(short* param_1, int* param_2) {

		// only for the 2nd map
		if ((!Active) || ((int)param_1 != 0xEDADD8)) {
			return oFUN_008cfe60(param_1, param_2);
		}

		unsigned int* currentInput = (unsigned int*)(param_2 + 1);
		unsigned int ci = *currentInput;
		unsigned int newInput = 0;


		// L3 only if A (cross)
		if (
			((ci & ConfigParser::PS2GamepadButtons["triangle"]) != 0) &&
			((ci & ConfigParser::PS2GamepadButtons["cross"]) != 0)
			) {
			newInput += ConfigParser::PS2GamepadButtons["l3"];
		}

		// start only if a (cross)/b (square)
		if ((TtsStart) && ((ci & ConfigParser::PS2GamepadButtons["start"]) != 0)) {
			if ((ci & ConfigParser::PS2GamepadButtons["square"]) != 0) {
				newInput += ConfigParser::PS2GamepadButtons["start"];
			}
			else if ((ci & ConfigParser::PS2GamepadButtons["cross"]) != 0) {
				newInput += ConfigParser::PS2GamepadButtons["select"];
			}
		}
		else {
			if ((ci & ConfigParser::PS2GamepadButtons["square"]) != 0) {
				newInput += ConfigParser::PS2GamepadButtons["circle"];
			}
			if ((ci & ConfigParser::PS2GamepadButtons["cross"]) != 0) {
				newInput += ConfigParser::PS2GamepadButtons["square"];
			}
		}

		// L1 if B (square) (or X (circle) if AutoAimX)
		if (
			((ci & ConfigParser::PS2GamepadButtons["square"]) != 0) ||
			(((ci & ConfigParser::PS2GamepadButtons["circle"]) != 0) && (AutoAimX))
			) {
			newInput += ConfigParser::PS2GamepadButtons["l1"];
		}

		if ((ci & ConfigParser::PS2GamepadButtons["circle"]) != 0) {
			newInput += ConfigParser::PS2GamepadButtons["cross"];
		}
		

		newInput += ci & UnmodifiedButtons;
		*currentInput = newInput;

		// do we also need to modify the tap array? by AND/NOT?

		oFUN_008cfe60(param_1, param_2);
	}


	void Run(CSimpleIniA& ini) {
		const char* category = "TwinSnakesInput";

		if (ini.IsEmpty() || (!ini.GetBoolValue(category, "Enabled", false))) {
			return;
		}

		Actions::RegisterAction(ini, "Turbo.Toggle", &ToggleAction);

		TtsStart = ini.GetBoolValue(category, "Start", TtsStart);
		FastEquipMenu = ini.GetBoolValue(category, "FastEquipMenu", FastEquipMenu);
		AutoAimX = ini.GetBoolValue(category, "AutoAimX", AutoAimX);
		//DisablePS2OnlyButtons = ini.GetBoolValue(category, "DisablePS2OnlyButtons", DisablePS2OnlyButtons);

		UnmodifiedButtons = ConfigParser::PS2GamepadButtons["up"] +
			ConfigParser::PS2GamepadButtons["right"] +
			ConfigParser::PS2GamepadButtons["down"] +
			ConfigParser::PS2GamepadButtons["left"] +
			ConfigParser::PS2GamepadButtons["r1"] +
			ConfigParser::PS2GamepadButtons["l2"] +
			ConfigParser::PS2GamepadButtons["r2"] +
			ConfigParser::PS2GamepadButtons["r3"] +
			ConfigParser::PS2GamepadButtons["triangle"];

		if (!TtsStart) {
			UnmodifiedButtons += ConfigParser::PS2GamepadButtons["start"] + ConfigParser::PS2GamepadButtons["select"];
		}
		/*
		if (!FastEquipMenu) { // don't need to require this for L2/R2??? depends how we do fast menu
			UnmodifiedButtons += ConfigParser::PS2GamepadButtons["l2"] + ConfigParser::PS2GamepadButtons["r2"];
		}
		*/
		
		oFUN_008cfe60 = (tFUN_Void_PshortPint)mem::TrampHook32((BYTE*)0x8CFE60, (BYTE*)hkFUN_008cfe60, 8);
	}

}
