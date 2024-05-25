#pragma once

#include "MGS2.framework.h"
#include <vector>
#include <string>

namespace MGS2::Log {
	const char* Category = "Log";

	std::string LogPrefix = "[MGS2ASI] ";
	std::string LogFullPrefix = LogPrefix;

	Level CurrentLevel = Info;

	
	std::string CurrentCategory = "";
	void SetCurrentCategory(std::string category) {
		CurrentCategory = category;
		LogFullPrefix = LogPrefix;
		if (category != "") {
			LogFullPrefix += "[" + category + "] ";
		}
	}


	void Log(std::string message, Level level) {
		if (level > CurrentLevel) {
			return;
		}
		std::string output = LogFullPrefix + message;
		OutputDebugStringA(output.c_str());
	}

	void Log(std::string message, std::string category, Level level) {
		if (level > CurrentLevel) {
			return;
		}
		std::string output = LogPrefix + "[" + category + "] " + message;
		OutputDebugStringA(output.c_str());
	}


	double Now() {
		unsigned long long ft;
		QueryPerformanceCounter((LARGE_INTEGER*)&ft);
		return (ft * Performance::TimerFrequency);
	}

	
	double CurrentTimeout = 0;
	TextConfig CurrentTextConfig;
	std::string CurrentMessage;
	void DisplayText(std::string message, TextConfig config, double timeout) {
		CurrentTimeout = Now() + timeout;
		CurrentTextConfig = config;
		CurrentMessage = message;
	}

	TextConfig DefaultTextConfig{
		true, 10, 440, Left	};
	void DisplayText(std::string message, double timeout) {
		DisplayText(message, DefaultTextConfig, timeout);
	}

	void DisplayToggleMessage(std::string message, bool active, TextConfig config, double timeout) {
		std::string activeText = active ? "[ON] " : "[OFF] ";
		std::string fullMessage = activeText + message;
		DisplayText(fullMessage, config, timeout);
	}

	void DisplayToggleMessage(std::string message, bool active, double timeout) {
		DisplayToggleMessage(message, active, DefaultTextConfig, timeout);
	}

	bool TimedOut(double timeout) {
		return (timeout > Now());
	}

	bool TimedOut() {
		return TimedOut(CurrentTimeout);
	}

	void UncaughtException(const char* category, const char* subCategory, const char* what) {
		static int lastExceptionFrame = 0;
		if (*Mem::RenderedFrames < (lastExceptionFrame + 300)) return;
		int previousExceptionFrame = lastExceptionFrame;
		lastExceptionFrame = *Mem::RenderedFrames;

		std::string msg = what ?
			fmt::format("Exception ({})", what) :
			"Unknown exception";

		if (category) {
			msg = subCategory ?
				fmt::format("[{}/{}] {}", category, subCategory, msg) :
				fmt::format("[{}] {}", category, msg);
		}

		Log::DisplayText(msg, 5);
		OutputDebugStringA(msg.c_str());
		OutputDebugStringA("\n");
	}


	tFUN_Void oFUN_00878f70;
	void __cdecl hkFUN_00878f70() {
		oFUN_00878f70();

		try_mgs2
			if (!strcmp(Mem::AreaCode, "init")) {
				return;
			}

			if (!CurrentTimeout) {
				return;
			}

			if (CurrentTimeout < Now()) {
				CurrentTimeout = 0;
				return;
			}
	
			CurrentTextConfig.Draw(CurrentMessage.c_str());
		catch_mgs2(Category, "878F70");

	}

	TextConfig CustomMessageConfig;
	void ShowCustomMessage(Actions::Action action) {
		if (CustomMessageConfig.Timeout > 0) {
			Log::DisplayText(CustomMessageConfig.Content, CustomMessageConfig, CustomMessageConfig.Timeout);
		}
		else {
			Log::DisplayText(CustomMessageConfig.Content, CustomMessageConfig);
		}
	}

	int LastCheckedAreaStartTime = 0;
	TextConfig AreaStartTimeConfig;
	void ShowAreaStartTime(Actions::Action action) {
		fmt::runtime_format_string<char> formatString = fmt::runtime(AreaStartTimeConfig.Content);
		int newTime = *Mem::GameTimeFrames - *Mem::AreaTimeFrames;
		int vsLastTime = newTime - LastCheckedAreaStartTime;
		if (AreaStartTimeConfig.Timeout > 0) {
			Log::DisplayText(fmt::format(formatString, newTime, vsLastTime), AreaStartTimeConfig.Timeout);
		}
		else {
			Log::DisplayText(fmt::format(formatString, newTime, vsLastTime));
		}
		LastCheckedAreaStartTime = newTime;
	}

	TextConfig FindGCLFunctionConfig;
	void ShowGCLFunctionLocationMessage(Actions::Action action) {
		int originalId = (int)action.Data;
		if (!originalId) return;

		int gclLocation = Mem::GclGetMappedFunction(originalId);

		auto result = fmt::format("GCL ID {:#06x} = {:#06x}", originalId, gclLocation);
		if (FindGCLFunctionConfig.Timeout > 0) {
			Log::DisplayText(result, FindGCLFunctionConfig, FindGCLFunctionConfig.Timeout);
		}
		else {
			Log::DisplayText(result, FindGCLFunctionConfig);
		}
	}

#ifdef _DEBUG
	void DebugAllGCLMaps(Actions::Action action) {
		std::unordered_map<size_t, const char*> gclMap{
			{ 0x16CB, "rain_init" },
			{ 0x41E7, "bonbori_red_init" },
			{ 0x41E8, "bonbori_red_init_0" },
			{ 0x1A4F3, "rad_point_init" },
			{ 0x1A6DD, "line_sph_init" },
			{ 0x223D1, "nullsub_0" },
			{ 0x423C8, "anmtex_init" },
			{ 0x5872D, "fat_ikef_init" },
			{ 0x5C815, "shdwdraw_init" },
			{ 0x7E215, "check_slater_init" },
			{ 0xA4D33, "fort_init" },
			{ 0xA7CFB, "selectscr_init" },
			{ 0xB1261, "ripple_bubble_init" },
			{ 0xB6ED3, "twin_dr_init" },
			{ 0xBA748, "ventilatorlit_init" },
			{ 0xBD712, "scn_break_init" },
			{ 0xC3D82, "vr_map_3d_init" },
			{ 0xD02FD, "mirror_init" },
			{ 0xDED8C, "vrpitfall_init" },
			{ 0x10CF2F, "theater_init" },
			{ 0x12C9D0, "zako_title_init" },
			{ 0x1395B5, "powspplit_init" },
			{ 0x164EE0, "bullet_bar_init" },
			{ 0x170393, "waterlinefall_init" },
			{ 0x175436, "brk_paper_init" },
			{ 0x17A315, "command_init_0" },
			{ 0x17EA77, "tng_monitor_control_42a_init" },
			{ 0x17EAB7, "tng_monitor_control_44a_init" },
			{ 0x18F412, "tngcom_init" },
			{ 0x1AF0D5, "scr_bubble_init" },
			{ 0x1AF92A, "radar_init" },
			{ 0x1B5854, "putspot_init" },
			{ 0x1B970C, "newgame_init_0" },
			{ 0x1CBD93, "fatman_init" },
			{ 0x1D5983, "vr_sys_init" },
			{ 0x1E44B7, "manhat3d_init" },
			{ 0x1F119A, "endingx_init" },
			{ 0x1F2A23, "eddogtag_init" },
			{ 0x206929, "shdwdrw2_init" },
			{ 0x209027, "box_hidden_init" },
			{ 0x22AE7D, "clear_result_init" },
			{ 0x24C03E, "vr_window_init" },
			{ 0x262B46, "st_select_init" },
			{ 0x264C7D, "chim_smo_init" },
			{ 0x2948A0, "padvib4_init" },
			{ 0x2A088B, "gnrl_poly_init" },
			{ 0x2A0B46, "kan_init" },
			{ 0x2A41C7, "dm_fr_init" },
			{ 0x2A4246, "map_3d_init" },
			{ 0x2B8E16, "vamp_init" },
			{ 0x2DAD24, "f_focus_init" },
			{ 0x2DF578, "shavedsnake_init" },
			{ 0x2E4EEB, "boss_pause_init" },
			{ 0x30E9CC, "elevator_init" },
			{ 0x311E8E, "hostage_init" },
			{ 0x3124A2, "scncamvib_init" },
			{ 0x31D58B, "strmfadr_init" },
			{ 0x3259E5, "defender_init" },
			{ 0x3625B1, "brk_magazine_init" },
			{ 0x3640F6, "play_disap_init" },
			{ 0x365DEE, "brk_spotlgt_init" },
			{ 0x37014D, "gllcom_init" },
			{ 0x374069, "har_sdmng_init" },
			{ 0x374BBE, "gunspin_init" },
			{ 0x3780F6, "play_ap_init" },
			{ 0x397FD4, "previous_story_init" },
			{ 0x3A6F25, "bul_c4_init_0" },
			{ 0x3B1909, "node_lamp_init" },
			{ 0x3C2410, "raiden_mask_bubble_init" },
			{ 0x3C56AA, "namedetect_init" },
			{ 0x3EF35F, "lens_flr_gm_init" },
			{ 0x3F55FC, "brooklyn_init_0" },
			{ 0x3F9531, "mcchkscrx_init" },
			{ 0x3FCD73, "command5_init" },
			{ 0x40F2AE, "brk_icebox_init" },
			{ 0x413144, "beltobj_init" },
			{ 0x4152DF, "body_slater_init" },
			{ 0x41F1F7, "beltconv_init" },
			{ 0x438FFA, "scr_waterfilm_init" },
			{ 0x46139A, "glass_scar_init" },
			{ 0x470DEF, "automenu_init" },
			{ 0x4847D2, "set_t_fc_init" },
			{ 0x48F8F8, "spottex_init" },
			{ 0x492A40, "routemdl_init" },
			{ 0x494239, "get_newitem_init" },
			{ 0x49D6F1, "putatach_scn_init" },
			{ 0x4A0018, "easylayout_init" },
			{ 0x4A777D, "wave2_init" },
			{ 0x4B5435, "put_flag_init" },
			{ 0x4BAAFE, "d_splash_fall_init" },
			{ 0x4BAB34, "charaspread_init" },
			{ 0x4CBFC5, "elevator_lamp_init" },
			{ 0x4CEFC1, "shiftmem_init" },
			{ 0x4CFE6E, "vr_select_init" },
			{ 0x4E1BD6, "mirror2_init" },
			{ 0x4F576C, "rai_nude_ik_init" },
			{ 0x4FB2CA, "orga_obj_init" },
			{ 0x502332, "waterfall_init" },
			{ 0x502681, "bridge_exp_init" },
			{ 0x522DB5, "sv_camera_init" },
			{ 0x53209A, "orga_hid_init" },
			{ 0x532262, "brk_object_init" },
			{ 0x532F32, "brk_radle_init" },
			{ 0x53BAF5, "efct_flow_init" },
			{ 0x53CFFA, "har_main_init" },
			{ 0x53E970, "xstagebreak_init" },
			{ 0x547041, "rising_smoke_init" },
			{ 0x54B365, "door_lamp_init" },
			{ 0x54ECA9, "c_light_spot_init" },
			{ 0x55080C, "bgscr_init" },
			{ 0x552C76, "c4_ice_mng_init" },
			{ 0x55B942, "door_init" },
			{ 0x56B5CB, "bladeply_init" },
			{ 0x56E234, "mapcnct_init" },
			{ 0x5734C7, "x_patch_init" },
			{ 0x577FB3, "sv_camera_init_1" },
			{ 0x59EDA8, "wccomm_init" },
			{ 0x59F23C, "drop_body_splush_prog_init" },
			{ 0x5A4809, "brk_glass_init" },
			{ 0x5B6E21, "sp_menu_eng_init" },
			{ 0x5BB4D4, "atcomm_init" },
			{ 0x5C0BAE, "paddemo_init" },
			{ 0x5C2EAE, "ghost_init" },
			{ 0x5C3A78, "pool_water_init" },
			{ 0x5CB281, "manhat3d_init_0" },
			{ 0x5DA36D, "cypher4snipe_init" },
			{ 0x5DAD0E, "sara_umi_init" },
			{ 0x5EC9C1, "gll_init" },
			{ 0x5ED347, "vr_pause_init" },
			{ 0x5FBE23, "windnoisx_init" },
			{ 0x60CF19, "wave4_init" },
			{ 0x61274D, "vr_target1_init" },
			{ 0x61514E, "sea_slater_init" },
			{ 0x6273B5, "sky_prev_init" },
			{ 0x62B1FD, "bgmfader_init" },
			{ 0x641A7B, "plant_sun_init" },
			{ 0x6442CB, "win_rain_init" },
			{ 0x645113, "corp_init_0" },
			{ 0x645C5B, "emma_clay_init" },
			{ 0x64AFEF, "wall_scar_init" },
			{ 0x64FB35, "show_pic_init_0" },
			{ 0x64FE6E, "water_front_prim_init" },
			{ 0x657385, "sky_column_init" },
			{ 0x662F7C, "body_sph_init_2" },
			{ 0x66B097, "fog_control_init" },
			{ 0x66BA4C, "wave_init" },
			{ 0x66BA66, "ripple_init" },
			{ 0x675145, "trample_slater_init" },
			{ 0x6760E9, "breath_init" },
			{ 0x6779AD, "set_tex_init" },
			{ 0x6781AF, "bomb_init_0" },
			{ 0x683BCC, "blood_wl_init" },
			{ 0x68CB9C, "vibrate_init" },
			{ 0x68F6CB, "rain_gas_pers_init" },
			{ 0x690610, "cinema_scr_init" },
			{ 0x6926D0, "hostcomm_init" },
			{ 0x6A030A, "scr_shimmer_init" },
			{ 0x6A0CDA, "brk_hang_light_init" },
			{ 0x6AD20F, "pl_dummy_init" },
			{ 0x6AE654, "wave5_init" },
			{ 0x6B8EE3, "rain_cm_init" },
			{ 0x6B8FE4, "blood_cm_init" },
			{ 0x6B921A, "bubble_cm_init" },
			{ 0x6C577D, "brk_potato_init" },
			{ 0x6DDD6A, "sp_menu_page_init" },
			{ 0x6E0FA8, "brk_vent_init" },
			{ 0x6E6572, "dogtag_mng_init" },
			{ 0x6E7F78, "shipworm_init" },
			{ 0x6EEB2F, "special_init" },
			{ 0x6FA0A2, "slit_light2_init" },
			{ 0x6FDA0F, "fort_obj_init" },
			{ 0x70A68A, "bgmanage_init" },
			{ 0x727B5E, "splash_rot_init" },
			{ 0x72F23C, "putmodel_init" },
			{ 0x741B7A, "orga_holo_init" },
			{ 0x74CD16, "f_bridge_init" },
			{ 0x7561AE, "orga_lgt_init" },
			{ 0x75E815, "brk_tv_init" },
			{ 0x76A03C, "bomb_init_1" },
			{ 0x77568D, "wall_marker_init" },
			{ 0x7798F6, "orga_init" },
			{ 0x77C494, "routeview_init" },
			{ 0x78D685, "bul_clay_init_0" },
			{ 0x7919E8, "gll_ef_init" },
			{ 0x7A3DBF, "set_t_dr_init" },
			{ 0x7A8EC1, "putobj_init" },
			{ 0x7A94A9, "dg_cam_init_0" },
			{ 0x7E3320, "option_w_init" },
			{ 0x8155F1, "clearcode_layout_init" },
			{ 0x81767C, "konami_logo_init" },
			{ 0x818B2C, "vr_clear_init" },
			{ 0x82CB3E, "nullsub_1" },
			{ 0x83C0C3, "vamp_init_0" },
			{ 0x83D857, "lit_man_init" },
			{ 0x83DD4D, "har_kasacka_init" },
			{ 0x84B931, "between_cam_init" },
			{ 0x85E8F4, "lockerd_init" },
			{ 0x86301D, "vr_child_init" },
			{ 0x86D382, "demo_snakearm_init" },
			{ 0x873375, "wdustmng_init" },
			{ 0x8826B9, "doll_init" },
			{ 0x8862F6, "rot_y_object_init" },
			{ 0x8975E6, "flow_paper_init" },
			{ 0x89778A, "float_dust_init" },
			{ 0x89AF5E, "thund_flash_init" },
			{ 0x89C365, "lockercp_init" },
			{ 0x89FEC2, "vr_wall_scar_init" },
			{ 0x8A0C96, "boss_result2_init" },
			{ 0x8A43F9, "vr_spark_init" },
			{ 0x8A8857, "dummytrg_init" },
			{ 0x8AC901, "george_bonbori_init" },
			{ 0x8C58F7, "extinguisher_init" },
			{ 0x8E298D, "cancel_init" },
			{ 0x8F27AC, "demo_c4_init" },
			{ 0x90B7EC, "check_water_level_init" },
			{ 0x912BFE, "fort_ceil_init" },
			{ 0x915101, "campose_init" },
			{ 0x92A625, "nullsub_2" },
			{ 0x92EB54, "nullsub_3" },
			{ 0x9307AB, "spotdraw_init" },
			{ 0x943EAF, "x_patch_init_0" },
			{ 0x9530AE, "frame_init" },
			{ 0x961325, "brk_speech_init" },
			{ 0x96E361, "attachment3a_init" },
			{ 0x971FDA, "wc_flush_init" },
			{ 0x9786F7, "rdr_movie_init" },
			{ 0x987E81, "wind_local_init" },
			{ 0x98A49D, "etc_sight_init" },
			{ 0x98BD5B, "splush_man_init" },
			{ 0x9A75E7, "mobile_init" },
			{ 0x9AFF54, "item_box_init" },
			{ 0x9B3B8B, "emma_init" },
			{ 0x9B3FD5, "cypher_init" },
			{ 0x9B65F0, "timer_init" },
			{ 0x9BC66F, "ropemain2_init" },
			{ 0x9BC670, "ropemain_init" },
			{ 0x9D73D7, "scnvapor_init" },
			{ 0x9E3C4F, "floor_panel_init" },
			{ 0x9E4D4A, "photo_view_init_0" },
			{ 0x9E8AEB, "confirm_init" },
			{ 0x9F11BC, "npc_snake_init" },
			{ 0xA0A179, "bubble_line_init" },
			{ 0xA3D28A, "bossrush_x_init" },
			{ 0xA58F8A, "vr_screen_init" },
			{ 0xA58FA5, "irs_mng_init" },
			{ 0xA63D3B, "z_man_init" },
			{ 0xA710DB, "w32def_init" },
			{ 0xA7CB42, "o2gage_init" },
			{ 0xA8560D, "bonbori_yw_init" },
			{ 0xA895C4, "put_vanime_init" },
			{ 0xA9ED03, "slow_man_init" },
			{ 0xAB381C, "node_init" },
			{ 0xABFD5F, "wt_door_init_0" },
			{ 0xACE3FF, "pdray_init" },
			{ 0xAD8864, "fort_wall_lgt_init" },
			{ 0xAE0F36, "r_server_init" },
			{ 0xAF0A7A, "spin_model_init" },
			{ 0xAF2208, "electric_floor_init" },
			{ 0xAF4CF6, "kmmng_init" },
			{ 0xAFA5E7, "floor_init" },
			{ 0xB030E4, "emb_control_init" },
			{ 0xB23A64, "d_thunder_init_0" },
			{ 0xB2A6B7, "hairevm_init" },
			{ 0xB2FD6C, "short_spark_init" },
			{ 0xB3AA52, "belt_init" },
			{ 0xB3E388, "holdene_init" },
			{ 0xB431C3, "brk_search_init" },
			{ 0xB4E108, "vr_sky_init" },
			{ 0xB4E35C, "vr_wall_init" },
			{ 0xB63A33, "blur_init" },
			{ 0xB66AE4, "flying_warm_init" },
			{ 0xB6E522, "command4_init" },
			{ 0xB75003, "ub_camera_init" },
			{ 0xB80DE5, "rain_parts_init" },
			{ 0xB89202, "fadeobj_init" },
			{ 0xB8B94D, "hako_init" },
			{ 0xB93D5E, "tng_monitor_control_init" },
			{ 0xBB6852, "ee_swim_init" },
			{ 0xBBAD24, "n_focus_init" },
			{ 0xBD400B, "shdwctrl_init" },
			{ 0xBD673F, "rain_slow_init" },
			{ 0xBD9CC1, "w32com_init" },
			{ 0xBDD6A9, "targettrap_init" },
			{ 0xBE0863, "d_splash_floor_init" },
			{ 0xBED0FF, "waterdropmng_init" },
			{ 0xBFB0A1, "vmp_shdwtrg_init" },
			{ 0xC09E6C, "all_slater_init" },
			{ 0xC0E06D, "bonbori_red_init_1" },
			{ 0xC13513, "vr_book_init" },
			{ 0xC1451B, "cylinder_init" },
			{ 0xC1BC23, "pl_subject_demo_init" },
			{ 0xC1FF0F, "sinktank_init" },
			{ 0xC210E3, "ts_subwin_init" },
			{ 0xC2AD32, "pdr_stage_init" },
			{ 0xC34B14, "brk_monitor_init" },
			{ 0xC3515F, "water_mine_init" },
			{ 0xC3C46C, "fk_gover_init" },
			{ 0xC45437, "brk_bottle_init" },
			{ 0xC547A7, "floor_light_man_init" },
			{ 0xC5D24C, "wave6_init" },
			{ 0xC622A4, "demo_slater_init" },
			{ 0xC7280A, "defcomm_init" },
			{ 0xC88AFA, "sdmanage_init" },
			{ 0xC8DBCC, "puddle_init" },
			{ 0xC9DD51, "sky_util_init" },
			{ 0xCB2C3E, "gas_in_water_init" },
			{ 0xCB7DE8, "flashlight_init" },
			{ 0xCBA568, "brk_big_glass_init" },
			{ 0xCBB124, "ceiling_init" },
			{ 0xCBD98B, "bos_kasacka_init" },
			{ 0xCC87A2, "layout_2d_init" },
			{ 0xCC9A07, "attacker_init" },
			{ 0xCDB878, "prez_init" },
			{ 0xCDC7BD, "weapon_sph_init" },
			{ 0xCED375, "vr_goal_init" },
			{ 0xCF777F, "set_t_fw_init" },
			{ 0xD110E2, "fort_hang_init" },
			{ 0xD1279A, "brk_tree_init" },
			{ 0xD16C76, "scr_water_init" },
			{ 0xD20FDE, "huge_sea_init" },
			{ 0xD2BD87, "wind_man_init" },
			{ 0xD2C9AA, "water_init" },
			{ 0xD4090D, "lodctrl_init" },
			{ 0xD51601, "enetrapchk_init" },
			{ 0xD5D6EF, "umi_ex_init" },
			{ 0xD612A2, "emm_slater_init" },
			{ 0xD6ED95, "brooklyn_init" },
			{ 0xD72C78, "brk_build_init" },
			{ 0xD90540, "put_elev_init" },
			{ 0xD99FB0, "w25def_init" },
			{ 0xDABA9E, "set_t_plant_init" },
			{ 0xDBD1FA, "efct_elv_btn_init" },
			{ 0xDC2F1B, "ipupanel_init" },
			{ 0xDC323F, "dust_cm_init" },
			{ 0xDC80BF, "fig_raven_init" },
			{ 0xDDF5CA, "command_init" },
			{ 0xDE0400, "attachment_init" },
			{ 0xDE0401, "attachment2_init" },
			{ 0xDE0402, "attachment3_init" },
			{ 0xDF5435, "brk_plate_init" },
			{ 0xE00FD1, "dv_goggles_init" },
			{ 0xE0568B, "floor_marker_init_0" },
			{ 0xE0DBBA, "_2d_sprt_init" },
			{ 0xE0DCBA, "gnrl_sprt_init_0" },
			{ 0xE22388, "d_fade_io_init" },
			{ 0xE2A088, "pause_init" },
			{ 0xE2ED79, "door_pannel_spark_init" },
			{ 0xE32FC7, "brk_swing_init" },
			{ 0xE3429D, "efct_oil_init" },
			{ 0xE39C0D, "_2d_sprt_pause_init" },
			{ 0xE41776, "orga_rai_init" },
			{ 0xE48F2F, "put_sound_init" },
			{ 0xE4CEA2, "scnbreakpart_init" },
			{ 0xE4D016, "projector_light_init" },
			{ 0xE52073, "dust_area_init" },
			{ 0xE5FF03, "subjectarm_chain_init" },
			{ 0xE76D74, "nullsub_4" },
			{ 0xE9021D, "trap_c4_init" },
			{ 0xE96D82, "codec_init" },
			{ 0xEA5215, "fallflr_init" },
			{ 0xEB7EDA, "gll_photo_init" },
			{ 0xEBEC24, "wt_door_init" },
			{ 0xEC084B, "stage_fire_init" },
			{ 0xECC9DE, "flour_init" },
			{ 0xED1E0B, "vrobj_init" },
			{ 0xED4678, "steam_cm_init_0" },
			{ 0xEE8B90, "gcl_call_se_code" },
			{ 0xEEE657, "emma_locker_init" },
			{ 0xF15E47, "dg_cam_init" },
			{ 0xF26728, "slowdown_init" },
			{ 0xF37560, "blood_sp_init" },
			{ 0xF466B6, "boss_etc2_init" },
			{ 0xF4BBF5, "timer2_init" },
			{ 0xF52ED4, "parrot_init" },
			{ 0xF5BF46, "routemdl2_init" },
			{ 0xF62833, "scr_trans_init" },
			{ 0xF63B18, "sv_camera_init_0" },
			{ 0xF706CD, "_off_man_init" },
			{ 0xF74020, "photo_term_init" },
			{ 0xF77507, "scn_padvib_init" },
			{ 0xF7F777, "watcher_init" },
			{ 0xF91A08, "dsegment_init_0" },
			{ 0xF91B9F, "dsegment_init" },
			{ 0xF92B8E, "invisible_init" },
			{ 0xF9711E, "gw_lines_model_init" },
			{ 0xFA4E80, "scr_hex_init" },
			{ 0xFAEDF1, "solidus_init" },
			{ 0xFC14A5, "titlescr_init" },
			{ 0xFDDA41, "brk_elecpanel_init" },
			{ 0xFE5C5B, "tng_a_init" },
			{ 0xFE5C5C, "tng_b_init" },
			{ 0xFE5F3D, "web_site_init" },
			{ 0xFE6730, "soundtest_init" },
			{ 0xFE6F50, "mdl_p_am_init" },
			{ 0xFFBECE, "mpegstrx_init" },
			{ 0xFFC196, "stgoutline_init" },
			{ 0xFFED03, "etc_init" },
			{ 0x100000F, "lens_flare_init" },
			{ 0x1000011, "blur_init_0" },
			{ 0x1000016, "j_ray_layout_init" },
			{ 0x1000017, "j_vtr_layout_init" },
			{ 0x1000019, "gas_pers_fast_init" },
			{ 0x1000020, "ai_ray_layout_init" },
			{ 0x1000031, "gas2_pers_fast_init" },
			{ 0x1000033, "rain_gas_pers_demo_init" },
			{ 0x100101D, "depend_arms2_init" },
			{ 0x1002003, "slow_man_init_0" },
			{ 0x1006010, "mesg_bomb2_init" },
			{ 0x1006013, "mesg_bomb3_init" },
			{ 0x1006018, "stage_fire_init_0" },
			{ 0x100700E, "demo_dummypoint_blood_init" },
			{ 0x100B000, "water_con_init" },
			{ 0x100B005, "auto_splush_init" },
			{ 0x100D00A, "ray_fall_blood_init_0" },
			{ 0x17078B, "ItemSet" },
			{ 0x73D521, "levels_first_resource_entry" },
			{ 0xF2DFCD, "load_from_saved_linkvarbuf" },
		};
		for (int i = 0; i <= 0x1FFFFFF; i++) {
			int gclLocation = Mem::GclGetMappedFunction(i);
			if (!gclLocation) continue;
			auto it = gclMap.find(i);
			const char* name = (it != gclMap.end()) ? it->second : "";
			std::string output = fmt::format("{:X},{:X},{}\n", i, gclLocation, name);
			OutputDebugStringA(output.c_str());
		}
	}
#endif

	TextConfig VersionConfig{ false, 636, 460, Right, 0x335947 };
	tFUN_Void_Int oFUN_00744a40;
	void _cdecl hkFUN_00744a40(int param_1) {
		oFUN_00744a40(param_1);
		VersionConfig.Draw();
	}


	void Run(CSimpleIniA& ini) {
		DefaultTextConfig.ParseConfig(ini, "MGS2.StatusMessage");

		const char* customMessageCategory = "MGS2.CustomMessage";
		Actions::RegisterAction(Ini, customMessageCategory, &ShowCustomMessage);
		CustomMessageConfig = DefaultTextConfig;
		CustomMessageConfig.ParseConfig(Ini, customMessageCategory);

		const char* findGCLFunctionCategory = "MGS2.FindGCLFunction";
		int gclFunctionId = ConfigParser::ParseInteger(ini, findGCLFunctionCategory, "FunctionID", 0, 0, 0xFFFFFF, true);
		Actions::RegisterAction(Ini, findGCLFunctionCategory, &ShowGCLFunctionLocationMessage, (void*)gclFunctionId);
		FindGCLFunctionConfig = DefaultTextConfig;
		FindGCLFunctionConfig.ParseConfig(Ini, findGCLFunctionCategory);

#ifdef _DEBUG
		Actions::RegisterAction(Ini, "MGS2.OutputGCLMap", &DebugAllGCLMaps);
#endif

		const char* areaStartTimeCategory = "MGS2.AreaStartTime";
		Actions::RegisterAction(Ini, areaStartTimeCategory, &ShowAreaStartTime);
		AreaStartTimeConfig = DefaultTextConfig;
		AreaStartTimeConfig.Content = "Area Start: {} (Last +{})";
		AreaStartTimeConfig.ParseConfig(Ini, areaStartTimeCategory);

		const char* versionCategory = "MGS2.Version";
		VersionConfig.ParseConfig(Ini, versionCategory);
		std::string strVer = fmt::format("bmn/ASI {}", ASI::Version);
		char* cVer = new char[strVer.size() + 1u];
		std::copy(strVer.data(), strVer.data() + strVer.size() + 1, cVer);
		VersionConfig.Content = cVer;
		if (VersionConfig.Enabled) {
			oFUN_00744a40 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x744A40, (BYTE*)hkFUN_00744a40, 6);
		}
		
		oFUN_00878f70 = (tFUN_Void)mem::TrampHook32((BYTE*)0x878F70, (BYTE*)hkFUN_00878f70, 5);
	}

}
