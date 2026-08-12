/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "cvardef.h"
#include "dod_shared.h"
#include "weapons.h"

char weaponnames[64][64] =
{
    "weapon_amerknife",
    "weapon_gerknife",
    "weapon_colt",
    "weapon_luger",
    "weapon_garand",
    "weapon_scopedkar",
    "weapon_thompson",
    "weapon_mp44",
    "weapon_spring",
    "weapon_kar",
    "weapon_bar",
    "weapon_mp40",
    "weapon_handgrenade",
    "weapon_stickgrenade",
    "weapon_stickgrenade_ex",
    "weapon_handgrenade_ex",
    "weapon_mg42",
    "weapon_30cal",
    "weapon_spade",
    "weapon_m1carbine",
    "weapon_mg34",
    "weapon_greasegun",
    "weapon_fg42",
    "weapon_k43",
    "weapon_enfield",
    "weapon_sten",
    "weapon_bren",
    "weapon_webley",
    "weapon_bazooka",
    "weapon_pschreck",
    "weapon_piat",
    "weapon_mortar",
    "weapon_binoculars",
    "weapon_satchel"
};

cvar_t *pcvar_classlimitcvars[28];

char classlimitcvarNames[27][32] =
{
    "mp_limitalliesgarand",
    "mp_limitalliescarbine",
    "mp_limitalliesthompson",
    "mp_limitalliesgreasegun",
    "mp_limitalliesspring",
    "mp_limitalliesbar",
    "mp_limitallies30cal",
    "mp_limitalliesbazooka",
    "mp_limitalliesmortar",
    "mp_limitaxiskar",
    "mp_limitaxisk43",
    "mp_limitaxismp40",
    "mp_limitaxismp44",
    "mp_limitaxisscopedkar",
    "mp_limitaxisfg42",
    "mp_limitaxisfg42s",
    "mp_limitaxismg34",
    "mp_limitaxismg42",
    "mp_limitaxispschreck",
    "mp_limitaxismortar",
    "mp_limitbritlight",
    "mp_limitbritassault",
    "mp_limitbritsniper",
    "mp_limitbritmg",
    "mp_limitbritpiat",
    "mp_limitbritmortar"
};

char *sPlayerModelFiles[6] =
{
    "models/player.mdl",
    "models/player/us-inf/us-inf.mdl",
    "models/player/axis-inf/axis-inf.mdl",
    "models/player/us-para/us-para.mdl",
    "models/player/axis-para/axis-para.mdl",
    "models/player/brit-inf/brit-inf.mdl"
};

char *s_USVoiceFiles[28] =
{
    "player/usattack.wav",
    "player/ushold.wav",
    "player/usfallback.wav",
    "player/usflankleft.wav",
    "player/usflankright.wav",
    "player/ussticktogether.wav",
    "player/uscover.wav",
    "player/ususegrenades.wav",
    "player/usceasefire.wav",
    "player/usyessir.wav",
    "player/usnegative.wav",
    "player/usbackup.wav",
    "player/usfireinhole.wav",
    "player/usgrenade.wav",
    "player/ussniper.wav",
    "player/ustakingfireleft.wav",
    "player/ustakingfireright.wav",
    "player/usareaclear.wav",
    "player/usgogogo.wav",
    "player/usdisplace.wav",
    "player/usenemyahead.wav",
    "player/usbehindus.wav",
    "player/usmgahead.wav",
    "player/usmoveupmg.wav",
    "player/usneedammo.wav",
    "player/ususebazooka.wav",
    "player/uspanzerschreck.wav"
};

char *s_BRITVoiceFiles[28] =
{
    "player/britattack.wav",
    "player/brithold.wav",
    "player/britfallback.wav",
    "player/britflankleft.wav",
    "player/britflankright.wav",
    "player/britsticktogether.wav",
    "player/britcover.wav",
    "player/britusegrenades.wav",
    "player/britceasefire.wav",
    "player/brityessir.wav",
    "player/britnegative.wav",
    "player/britbackup.wav",
    "player/britfireinhole.wav",
    "player/britgrenade.wav",
    "player/britsniper.wav",
    "player/brittakingfireleft.wav",
    "player/brittakingfireright.wav",
    "player/britareaclear.wav",
    "player/britgogogo.wav",
    "player/britdisplace.wav",
    "player/britenemyahead.wav",
    "player/britbehindus.wav",
    "player/britmgahead.wav",
    "player/britmoveupmg.wav",
    "player/britneedammo.wav",
    "player/britusepiat.wav",
    "player/britpanzerschreck.wav"
};

char *s_GERVoiceFiles[29] =
{
    "player/gerattack.wav",
    "player/gerhold.wav",
    "player/gerfallback.wav",
    "player/gerflankleft.wav",
    "player/gerflankright.wav",
    "player/gersticktogether.wav",
    "player/gercover.wav",
    "player/gerusegrenades.wav",
    "player/gerceasefire.wav",
    "player/geryessir.wav",
    "player/gernegative.wav",
    "player/gerbackup.wav",
    "player/gerfireinhole.wav",
    "player/gergrenade.wav",
    "player/gersniper.wav",
    "player/gertakingfireleft.wav",
    "player/gertakingfireright.wav",
    "player/gerareaclear.wav",
    "player/gergogogo.wav",
    "player/gerdisplace.wav",
    "player/gerenemyahead.wav",
    "player/gerbehindus.wav",
    "player/germgahead.wav",
    "player/germoveupmg.wav",
    "player/gerneedammo.wav",
    "player/gerusepanzerschreck.wav",
    "player/gerbazooka.wav",
    "player/gerpiat.wav"
};

// WHAMER: TODO
int iVoiceToHandSignal[29] =
{
    0,
    10,
    15,
    1,
    9,
    11,
    0,
    13,
    20,
    25,
    3,
    2,
    5,
    8,
    1,
    4,
    7,
    6,
    12,
    20,
    18,
    21,
    22,
    23,
    24,
    5,
    20,
    4,
    4
};

char *s_VoiceCommands[29][4] =
{
    { "", "", "", "" },
    { "voice_attack", "#Voice_subtitle_attack", "#Voice_subtitle_moveout", "" },
    { "voice_hold", "#Voice_subtitle_hold", "", "" },
    { "voice_fallback", "#Voice_subtitle_fallback", "", "" },
    { "voice_left", "#Voice_subtitle_left", "", "" },
    { "voice_right", "#Voice_subtitle_right", "", "" },
    { "voice_sticktogether", "#Voice_subtitle_sticktogether", "" , "" },
    { "voice_cover", "#Voice_subtitle_cover", "", "" },
    { "voice_usegrens", "#Voice_subtitle_usegrens", "", "" },
    { "voice_ceasefire", "#Voice_subtitle_ceasefire", "", "" },
    { "voice_yessir", "#Voice_subtitle_yessir", "", "" },
    { "voice_negative", "#Voice_subtitle_negative", "", "" },
    { "voice_backup", "#Voice_subtitle_backup", "", "" },
    { "voice_fireinhole", "#Voice_subtitle_fireinhole", "", "" },
    { "voice_greande", "#Voice_subtitle_grenade", "", "" },
    { "voice_sniper", "#Voice_subtitle_sniper", "", "" },
    { "voice_fireleft", "#Voice_subtitle_fireleft", "", "" },
    { "voice_fireright", "#Voice_subtitle_fireright", "", "" },
    { "voice_areaclear", "#Voice_subtitle_areaclear", "", "" },
    { "voice_gogogo", "#Voice_subtitle_gogogo", "", "" },
    { "voice_displace", "#Voice_subtitle_displace", "", "" },
    { "voice_enemyahead", "#Voice_subtitle_enemyahead", "", "" },
    { "voice_enemybehind", "#Voice_subtitle_enemybehind", "", "" },
    { "voice_mgahead", "#Voice_subtitle_mgahead", "", "" },
    { "voice_moveupmg", "#Voice_subtitle_moveupmg_30cal", "#Voice_subtitle_moveupmg_mg", "#Voice_subtitle_moveupmg_bren" },
    { "voice_needammo", "#Voice_subtitle_needammo", "", "" },
    { "voice_usebazooka", "#Voice_subtitle_usebazooka", "#Voice_subtitle_usepschreck", "#Voice_subtitle_usepiat" },
    { "voice_bazookaspotted", "#Voice_subtitle_pschreckspotted", "#Voice_subtitle_bazookaspotted", "#Voice_subtitle_pschreckspotted" },
    { "voice_bazookaspotted", "#Voice_subtitle_pschreckspotted", "#Voice_subtitle_piatspotted", "#Voice_subtitle_pschreckspotted" }
};

char *s_HandSignalSubtitles[25][3] =
{
    { "#Voice_subtitle_sticktogether", "", "" },
    { "#Voice_subtitle_fallback", "", "" },
    { "#Voice_subtitle_negative", "", "" },
    { "#Voice_subtitle_yessir", "", "" },
    { "#Voice_subtitle_sniper", "", "" },
    { "#Voice_subtitle_backup", "", "" },
    { "#Voice_subtitle_fireright", "", "" },
    { "#Voice_subtitle_fireleft", "", "" },
    { "#Voice_subtitle_grenade", "", "" },
    { "#Voice_subtitle_left", "", "" },
    { "#Voice_subtitle_attack", "#Voice_subtitle_moveout", "" },
    { "#Voice_subtitle_right", "", "" },
    { "#Voice_subtitle_areaclear", "", "" },
    { "#Voice_subtitle_cover", "", "" },
    { "#Voice_subtitle_cover", "", "" },
    { "#Voice_subtitle_hold", "", "" },
    { "#Voice_subtitle_backup", "", "" },
    { "#Voice_subtitle_enemyahead", "", "" },
    { "#Voice_subtitle_displace", "", "" },
    { "#Voice_subtitle_gogogo", "", "" },
    { "#Voice_subtitle_enemyahead", "", "" },
    { "#Voice_subtitle_enemybehind", "", "" },
    { "#Voice_subtitle_mgahead", "", "" },
    { "#Voice_subtitle_moveupmg_30cal", "#Voice_subtitle_moveupmg_mg", "#Voice_subtitle_moveupmg_bren" },
    { "#Voice_subtitle_ceasefire", "", "" }
};

char *sHandSignals[30];

DodClassInfo_t g_ParaClassInfo[21] =
{
    { "", "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" },
    { "cls_garand", "#class_alliedpara_garand", "us-para", 1, 600, 42, 0, 2, 0, 0, 0, 2, 0, 2, "" },
    { "cls_carbine", "#class_alliedpara_carbine", "us-para", 1, 600, 1048586, 0, 2, 0, 1, 1, 5, 0, 2, "" },
    { "cls_tommy", "#class_alliedpara_thompson", "us-para", 1, 600, 138, 0, 1, 0, 2, 1, 0, 0, 2, "" },
    { "cls_grease", "#class_alliedpara_grease", "us-para", 1, 600, 4194314, 0, 1, 0, 4, 1, 6, 0, 2, "" },
    { "cls_spring", "#class_alliedpara_spring", "us-para", 1, 600, 522, 0, 0, 0, 3, 0, 1, 0, 2, "" },
    { "cls_bar", "#class_alliedpara_bar", "us-para", 1, 600, 2058, 0, 1, 0, 5, 0, 4, 0, 2, "" },
    { "cls_30cal", "#class_alliedpara_30cal", "us-para", 1, 600, 262154, 0, 0, 0, 6, 0, 3, 0,    2, "" },
    { "cls_bazooka", "#class_alliedpara_bazooka", "us-para", 1, 600, 536870922, 0, 0, 0, 6, 0, 3, 0, 2, "" },
    { "cls_mortar", "#class_alliedpara_mortar", "us-para", 1, 600, 10, 1, 0, 0, 6, 0, 3, 0, 2, "" },
    { "cls_k98", "#class_axispara_kar98", "axis-para", 2, 600, 1044, 0, 2, 0, 0, 0, 2, 0, 1, "" },
    { "cls_k43", "#class_axis_k43", "axis-para", 2, 600, 16777236, 0, 2, 0, 1, 0, 5, 0, 2, "" },
    { "cls_mp40", "#class_axispara_mp40", "axis-para", 2, 600, 4116, 0, 1, 0, 3, 0, 0, 0, 3, "" },
    { "cls_mp44", "#class_axispara_mp44", "axis-para", 2, 600, 276, 0, 1, 0, 4, 0, 4, 0, 4, "" },
    { "cls_k98s", "#class_axispara_scopedkar", "axis-para", 2, 600, 84, 0, 0, 0, 2, 0, 1, 0, 3, "" },
    { "cls_fg42", "#class_axispara_fg42bipod", "axis-para", 2, 600, 8388628, 0, 1, 0, 5, 0, 6, 0, 6, "" },
    { "cls_fg42_s", "#class_axispara_fg42scope", "axis-para", 2, 600, 8388628, 0, 1, 0, 5, 0, 6, 0, 6, "" },
    { "cls_mg34", "#class_axispara_mg34", "axis-para", 2, 600, 2097172, 0, 0, 0, 6, 0, 3, 0, 7, "" },
    { "cls_mg42", "#class_axispara_mg42", "axis-para", 2, 600, 131092, 0, 0, 0, 6, 0, 3, 0, 7, "" },
    { "cls_pschreck", "#class_axispara_pschreck", "axis-para", 2, 600, 1073741844, 0, 0, 0, 6, 0, 3, 0, 7   , "" },
    { "cls_germortar", "#class_axispara_mortar", "axis-para", 2, 600, 20, 1, 0, 0, 6, 0, 3, 0, 7, "" }
};

DodClassInfo_t g_ClassInfo[28] =
{
    { "", "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" },
    { "cls_garand", "#class_allied_garand", "us-inf", 1, 600, 42, 0, 2, 0, 0, 2, 0, 0, 3, "" },
    { "cls_carbine", "#class_allied_carbine", "us-inf", 1, 600, 1048586, 0, 2, 0, 0, 0, 0, 0, 3, "" },
    { "cls_tommy", "#class_allied_thompson", "us-inf", 1, 600, 138, 0, 1, 0, 1, 0, 1, 1, 4, "" },
    { "cls_grease", "#class_allied_grease", "us-inf", 1, 600, 4194314, 0, 1, 0, 1, 2, 1, 1, 4, "" },
    { "cls_spring", "#class_allied_sniper", "us-inf", 1, 600, 522, 0, 0, 0, 4, 1, 4, 4, 7, "" },
    { "cls_bar", "#class_allied_heavy", "us-inf", 1, 600, 2058, 0, 1, 0, 2, 2, 2, 2, 5, "" },
    { "cls_30cal", "#class_allied_mg", "us-inf", 1, 600, 262154, 0, 0, 0, 3, 1, 3, 3, 6, "" },
    { "cls_bazooka", "#class_allied_bazooka", "us-inf", 1, 600, 536870922, 0, 0, 0, 3, 1, 3, 3, 6, "" },
    { "cls_mortar", "#class_allied_mortar", "us-inf", 1, 600, 10, 1, 0, 0, 3, 1, 3, 3, 6, "" },
    { "cls_k98", "#class_axis_kar98", "axis-inf", 2, 600, 525328, 0, 2, 0, 0, 0, 0, 0, 2, "" },
    { "cls_k43", "#class_axis_k43", "axis-inf", 2, 600, 17301520, 0, 2, 0, 1, 0, 1, 1, 3, "" },
    { "cls_mp40", "#class_axis_mp40", "axis-inf", 2, 600, 528400, 0, 1, 0, 2, 1, 2, 2, 4, "" },
    { "cls_mp44", "#class_axis_mp44", "axis-inf", 2, 600, 524560, 0, 1, 0, 3, 0, 3, 3, 5, "" },
    { "cls_k98s", "#class_axis_sniper", "axis-inf", 2, 600, 524368, 0, 0, 0, 5, 1, 5, 5, 7, "" },
    { "cls_fg42", "#class_axispara_fg42bipod", "axis-para", 2, 600, 8388628, 0, 1, 0, 5, 0, 6, 0, 7, "" },
    { "cls_fg42_s", "#class_axispara_fg42scope", "axis-para", 2, 600, 8388628, 0, 1, 0, 5, 0, 6, 0, 7, "" },
    { "cls_mg34", "#class_axis_mg34", "axis-inf", 2, 600, 2621456, 0, 0, 0, 4, 0, 4, 4, 6, "" },
    { "cls_mg42", "#class_axis_mg42", "axis-inf", 2, 600, 655376, 0, 0, 0, 4, 0, 4, 4, 6, "" },
    { "cls_pschreck", "#class_axis_pschreck", "axis-inf", 2, 600, 1074266128, 0, 0, 0, 4, 0, 4, 4, 6, "" },
    { "cls_germortar", "#class_axis_mortar", "axis-inf", 2, 600, 524304, 1, 0, 0, 4, 0, 4, 4, 6, "" },
    { "cls_enfield", "#class_brit_light", "brit-inf", 1, 600, 301989890, 0, 2, 0, 0, 0, 0, 0, 2, "" },
    { "cls_sten", "#class_brit_medium", "brit-inf", 1, 600, 335544322, 0, 1, 0, 1, 0, 1, 0, 3, "" },
    { "cls_enfields", "#class_brit_sniper", "brit-inf", 1, 600, 301989890, 0, 0, 0, 3, 1, 3, 0, 5, "" },
    { "cls_bren", "#class_brit_heavy", "brit-inf", 1, 600, 402653186, 0, 1, 0, 2, 0, 2, 0, 4, "" },
    { "cls_piat ", "#class_brit_piat", "brit-inf", 1, 600, -1879048190, 0, 0, 0, 2, 0, 2, 0, 4, "" },
    { "cls_britmortar ", "#class_brit_mortar", "brit-inf", 1, 600, 268435458, 1, 1, 0, 2, 0, 2, 0, 4, "" },
    { "cls_random", "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" }
};

p_wpninfo_s P_WpnInfo[41] =
{
    // WEAPON_NONE 0
    { "", "", "", "", "", "", "", "", "", "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0 },
    // WEAPON_AMERKNIFE 1
    { "models/p_amerk.mdl", "models/v_amerk.mdl", "models/w_amerk.mdl", "", "", "", "", "", "knife", "knife", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.1f, 0.0 },
    // WEAPON_GERKNIFE 2
    { "models/p_paraknife.mdl", "models/v_paraknife.mdl", "models/w_paraknife.mdl", "", "", "", "", "", "knife", "knife", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.1f, 0.0 },
    // WEAPON_COLT 3
    { "models/p_colt.mdl", "models/v_colt.mdl", "models/allied_ammo.mdl", "", "", "", "", "", "pistol", "pistol", 2.75f, 0.1f, 0.0, 0.0, 0.68f, 0.0, 0.0, 21, 21, 7, 21, 10, 0.035f, 0.1f, 0.0 },
    // WEAPON_LUGER 4
    { "models/p_luger.mdl", "models/v_luger.mdl", "models/axis_ammo.mdl", "", "", "", "", "", "pistol", "pistol", 2.1f, 0.1f, 2.1f, 0.0, 0.68f, 0.0, 0.0, 24, 24, 8, 24, 10, 0.035f, 0.1f, 0.0 },
    // WEAPON_GARAND 5
    { "models/p_garand.mdl", "models/v_garand.mdl", "models/w_garand.mdl", "models/p_garand_l.mdl", "", "", "", "", "rifle", "garand", 1.8f, 0.4f, 0.0, 0.0, 1.25f, 0.0, 0.0, 88, 88, 8, 88, 20, 0.016f, 0.1f, 0.0 },
    // WEAPON_SCOPEDKAR 6
    { "models/p_k98s.mdl", "models/v_scoped98k.mdl", "models/w_scoped98k.mdl", "models/p_k98s_l.mdl", "", "", "", "", "bolt", "bolt", 3.12f, 1.61f, 0.0, 0.0, 1.0f, 0.0, 0.0, 65, 65, 5, 65, 20, 0.06f, 0.1f, 0.004f },
    // WEAPON_THOMPSON 7
    { "models/p_tommy.mdl", "models/v_tommy.mdl", "models/w_tommy.mdl", "models/p_tommy_l.mdl", "", "", "", "", "tommy", "tommy", 2.8f, 0.085f, 0.0, 0.0, 1.1f, 0.0, 0.0, 210, 210, 30, 210, 20, 0.025f, 0.1f, 0.0 },
    // WEAPON_MP44 8
    { "models/p_stg44.mdl", "models/v_mp44.mdl", "models/w_mp44.mdl", "", "", "", "", "", "stg44", "stg44", 3.13f, 0.09f, 0.0, 0.0, 1.0f, 0.0, 0.0, 210, 210, 30, 210, 20, 0.025f, 0.1f, 0.0 },
    // WEAPON_SPRING 9
    { "models/p_spring.mdl", "models/v_spring.mdl", "models/w_spring.mdl", "models/p_spring_l.mdl", "", "", "", "", "bolt", "bolt", 2.7f, 1.85f, 0.0, 0.0, 1.33f, 0.0, 0.0, 55, 55, 5, 55, 20, 0.06f, 0.1f, 0.004f },
    // WEAPON_KAR 10
    { "models/p_k98.mdl", "models/v_98k.mdl", "models/w_98k.mdl", "models/p_k98_l.mdl", "", "", "", "", "bolt", "bolt", 3.12f, 1.6f, 0.0, 0.0, 1.0f, 0.0, 0.0, 65, 65, 5, 65, 20, 0.014f, 0.15f, 0.0 },
    // WEAPON_BAR 11
    { "models/p_barbu.mdl", "models/v_bar.mdl", "models/w_bar.mdl", "", "models/p_barbu.mdl", "models/p_barbd.mdl", "models/p_barbd.mdl", "models/p_barbd.mdl", "rifle", "bar", 3.72f, 0.12f, 3.86f, 0.0, 1.5f, 1.5f, 1.5f, 260, 260, 20, 20, 20, 0.025f, 0.1f, 0.02f },
    // WEAPON_MP40 12
    { "models/p_mp40.mdl", "models/v_mp40.mdl", "models/w_mp40.mdl", "", "", "", "", "", "mp40", "mp40", 2.85f, 0.09f, 0.0, 0.0, 1.0f, 0.0, 0.0, 210, 210, 30, 210, 20, 0.025f, 0.1f, 0.0 },
    // WEAPON_HANDGRENADE 13
    { "models/p_grenade.mdl", "models/v_grenade.mdl", "models/w_grenade.mdl", "", "", "", "", "", "gren", "gren", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.1f, 0.0 },
    // WEAPON_STICKGRENADE 14
    { "models/p_stick.mdl", "models/v_stick.mdl", "models/w_stick.mdl", "", "", "", "", "", "stick", "stick", 0.0, 0.0, 0.0, 0.0, 0.75f, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.1f, 0.0 },
    // WEAPON_STICKGRENADEX 15
    { "models/p_stick.mdl", "models/v_stick.mdl", "models/w_stick.mdl", "", "", "", "", "", "primstick", "gren", 0.0, 0.0, 0.0, 0.0, 0.75f, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.1f, 0.0 },
    // WEAPON_HANDGRENADEX 16
    { "models/p_grenade.mdl", "models/v_grenade.mdl", "models/w_grenade.mdl", "", "", "", "", "", "primgren", "gren", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.1f, 0.0 },
    // WEAPON_MG42 17
    { "models/p_mg42bu.mdl", "models/v_mg42.mdl", "models/w_mg42.mdl", "", "models/p_mg42bu.mdl", "models/p_mg42bd.mdl", "models/p_mg42sr.mdl", "models/p_mg42pr.mdl", "mg", "mg42", 6.5f, 0.1f, 0.0, 0.0, 0.0, 0.0, 0.6f, 500, 500, 250, 250, 40, 0.2f, 0.1f, 0.025f },
    // WEAPON_CAL30 18
    { "models/p_30cal.mdl", "models/v_30cal.mdl", "models/w_30cal.mdl", "", "models/p_30cal.mdl", "models/p_30cal.mdl", "models/p_30calsr.mdl", "models/p_30calpr.mdl", "30cal", "30cal", 5.0f, 0.1f, 0.0, 0.0, 0.0, 0.0, 0.6f, 300, 300, 150, 150, 40, 0.2f, 0.1f, 0.02f },
    // WEAPON_SPADE 19
    { "models/p_spade.mdl", "models/v_spade.mdl", "models/axis_ammo.mdl", "", "", "", "", "", "spade", "knife", 5.0f, 0.1f, 0.0, 0.0, 0.0, 0.0, 0.0, 300, 300, 150, 150, 40, 0.0, 0.1f, 0.0 },
    // WEAPON_M1CARBINE 20
    { "models/p_m1carb.mdl", "models/v_m1carbine.mdl", "models/w_m1carb.mdl", "models/p_m1carb_l.mdl", "", "", "", "", "rifle", "m1carb", 3.14f, 0.1f, 0.0, 0.0, 0.0, 0.0, 0.0, 165, 165, 15, 165, 20, 0.019f, 0.1f, 0.0 },
    // WEAPON_MG34 21
    { "models/p_mg34bu.mdl", "models/v_mg34.mdl", "models/w_mg34.mdl", "", "models/p_mg34bu.mdl", "models/p_mg34bd.mdl", "models/p_mg34sr.mdl", "models/p_mg34pr.mdl", "mg", "mg34", 5.7f, 0.1f, 0.0, 0.0, 0.0, 0.0, 0.6f, 450, 450, 75, 75, 40, 0.2f, 0.1f, 0.02f },
    // WEAPON_GREASEGUN 22
    { "models/p_grease.mdl", "models/v_greasegun.mdl", "models/w_greasegun.mdl", "models/p_grease_l.mdl", "", "", "", "", "grease", "grease", 4.1f, 0.15f, 0.0, 0.0, 0.0, 0.0, 0.0, 210, 210, 30, 210, 20, 0.025f, 0.1f, 0.0 },
    // WEAPON_FG42 23
    { "models/p_fg42bu.mdl", "models/v_fg42.mdl", "models/w_fg42.mdl", "", "models/p_fg42bu.mdl", "models/p_fg42bd.mdl", "models/p_fg42sr.mdl", "models/p_fg42pr.mdl", "tommy", "fg42", 3.85f, 0.085f, 3.71f, 0.0, 0.0, 0.0, 0.0, 180, 180, 20, 20, 20, 0.025f, 0.1f, 0.02f },
    // WEAPON_K43 24
    { "models/p_k43.mdl", "models/v_k43.mdl", "models/w_k43.mdl", "", "", "", "", "", "rifle", "k43", 4.4f, 0.4f, 0.0, 0.0, 0.0, 0.0, 0.0, 80, 80, 10, 80, 20, 0.019f, 0.1f, 0.0 },
    // WEAPON_ENFIELD 25
    { "models/p_enfield.mdl", "models/v_enfield.mdl", "models/w_enfield.mdl", "models/p_enfield_l.mdl", "", "", "", "", "bolt", "bolt", 3.66f, 1.65f, 0.0, 0.0, 0.8f, 0.0, 0.0, 60, 60, 5, 60, 20, 0.014f, 0.15f, 0.0 },
    // WEAPON_STEN 26
    { "models/p_sten.mdl", "models/v_sten.mdl", "models/w_sten.mdl", "", "", "", "", "", "sten", "sten", 2.9f, 0.085f, 0.0, 0.0, 0.0, 0.0, 0.0, 210, 210, 30, 210, 20, 0.025f, 0.1f, 0.0 },
    // WEAPON_BREN 27
    { "models/p_brenbu.mdl", "models/v_bren.mdl", "models/w_bren.mdl", "models/p_bren_l.mdl", "models/p_brenbu.mdl", "models/p_brenbd.mdl", "models/p_brensr.mdl", "models/p_brenbr.mdl", "bren", "bren", 3.6f, 0.12f, 4.2f, 0.3f, 1.0f, 1.29f, 1.575f, 180, 180, 30, 30, 20, 0.025f, 0.1f, 0.02f },
    // WEAPON_WEBLEY 28
    { "models/p_webley.mdl", "models/v_webley.mdl", "models/allied_ammo.mdl", "", "", "", "", "", "pistol", "webley", 4.0f, 0.1f, 0.0, 0.0, 0.9f, 0.0, 0.0, 18, 18, 8, 18, 10, 0.035f, 0.1f, 0.0 },
    // WEAPON_BAZOOKA 29
    { "models/p_bazooka.mdl", "models/v_bazooka.mdl", "models/w_bazooka.mdl", "models/p_bazooka_l.mdl", "", "", "", "", "bazooka", "bazooka", 3.0f, 1.0f, 0.0, 0.0, 1.0f, 1.0f, 0.8f, 5, 5, 1, 5, 20, 0.0, 0.1f, 0.0 },
    // WEAPON_PSCHRECK 30
    { "models/p_pschreck.mdl", "models/v_panzerschreck.mdl", "models/w_pschreck.mdl", "models/p_pschreck_l.mdl", "", "", "", "", "pschreck", "pschreck", 3.0f, 1.0f, 0.0, 0.0, 1.0f, 1.0f, 0.8f, 5, 5, 1, 5, 20, 0.0, 0.1f, 0.0 },
    // WEAPON_PIAT 31
    { "models/p_piat.mdl", "models/v_piat.mdl", "models/w_piat.mdl", "", "", "", "", "", "piat", "piat", 3.4f, 1.0f, 0.0, 0.0, 1.0f, 1.0f, 0.8f, 5, 5, 1, 5, 20, 0.0, 0.1f, 0.0 },
    // WEAPON_MORTAR 32
    { "models/p_mortar.mdl", "models/v_mortar.mdl", "models/w_mortar.mdl", "", "", "", "", "", "piat", "mortar", 3.4f, 1.0f, 0.0, 0.0, 1.0f, 1.0f, 0.8f, 5, 5, 1, 5, 20, 0.0, 0.1f, 0.0 },
    // WEAPON_BINOC 33
    { "models/p_spring.mdl", "models/v_binoculars.mdl", "models/w_spring.mdl", "models/p_spring_l.mdl", "", "", "", "", "bolt", "bolt", 1.0f, 0.5f, 0.0, 0.0, 1.33f, 0.0, 0.0, 55, 55, 5, 55, 5, 0.06f, 0.1f, 0.004f },
    // WEAPON_BINOCULARS 34
    { "models/p_spring.mdl", "models/v_binoculars.mdl", "models/w_spring.mdl", "", "", "", "", "", "bolt", "bolt", 1.0f, 0.5f, 0.0, 0.0, 1.33f, 0.0, 0.0, 55, 55, 5, 55, 5, 0.0, 0.0, 0.0 },
    // WEAPON_GERPARAKNIFE 35
    { "models/p_paraknife.mdl", "models/v_paraknife.mdl", "models/axis_ammo.mdl", "", "", "", "", "", "", "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.1f, 0.0 },
    // WEAPON_MILLS 36
    { "models/p_mills.mdl", "models/v_mills.mdl", "models/w_mills.mdl", "", "", "", "", "", "mills", "mills", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.1f, 0.0 },
    // WEAPON_SCOPED_FG42 37
    { "models/p_fg42s.mdl", "models/v_scopedfg42.mdl", "models/w_fg42s.mdl", "", "", "", "", "", "tommy", "fg42", 3.85f, 0.3f, 3.71f, 0.0, 0.3f, 0.0, 0.0, 180, 180, 20, 20, 20, 0.025f, 0.1f, 0.017f },
    // WEAPON_SCOPEDENFIELD 38
    { "models/p_enfields.mdl", "models/v_enfield_scoped.mdl", "models/w_enfield_scoped.mdl", "models/p_enfields_l.mdl", "", "", "", "", "bolt", "bolt", 3.54f, 1.65f, 0.0, 0.0, 0.8f, 0.0, 0.0, 60, 60, 5, 60, 20, 0.06f, 0.1f, 0.004f },
    // WEAPON_FOLDINGCARBINE 39
    { "models/p_fcarb.mdl", "models/v_fcarb.mdl", "models/w_fcarb.mdl", "models/p_fcarb_l.mdl", "", "", "", "", "fcarb", "fcarb", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 20, 0.019f, 0.1f, 0.0 },
    // WEAPON_FAIRBAIRN 40
    { "models/p_fairbairn.mdl", "models/v_fairbairn.mdl", "models/allied_ammo.mdl", "", "", "", "", "", "knife", "knife", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 20, 0.0, 0.1f, 0.0 },
};

char *sHelmetModels[7] =
{
    "models/helmet_us.mdl",
    "models/helmet_axis.mdl",
    "models/hat_axis.mdl",
    "models/helmet_brit.mdl",
    "models/helmet_allie_para.mdl",
    "models/helmet_allie_para_med.mdl",
    "models/helmet_axis_para.mdl"
};

char *szMapMarkerIcons[15] =
{
    "sprites/mapmarkers/redx.spr",
    "sprites/mapmarkers/greencheck.spr",
    "sprites/mapmarkers/mg.spr",
    "sprites/mapmarkers/sniper.spr",
    "sprites/mapmarkers/camper.spr",
    "sprites/mapmarkers/usflag.spr",
    "sprites/mapmarkers/britflag.spr",
    "sprites/mapmarkers/gerflag.spr",
    "sprites/mapmarkers/number_one.spr",
    "sprites/mapmarkers/number_two.spr",
    "sprites/mapmarkers/number_three.spr",
    "sprites/mapmarkers/arrow_n.spr",
    "sprites/mapmarkers/arrow_e.spr",
    "sprites/mapmarkers/arrow_s.spr",
    "sprites/mapmarkers/arrow_w.spr"
};