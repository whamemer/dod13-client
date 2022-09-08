//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined( PLAYERINFO_H )
#define PLAYERINFO_H

#include "hud.h"

extern struct hud_player_info_s g_PlayerInfoList[MAX_PLAYERS + 1];
extern struct extra_player_info_t g_PlayerExtraInfo[MAX_PLAYERS + 1];

#endif // PLAYERINFO_H