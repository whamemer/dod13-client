//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined ( EV_HLDMH )
#define EV_HLDMH

// bullet types
typedef	enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM,
	BULLET_PLAYER_MP5,
	BULLET_PLAYER_357,
	BULLET_PLAYER_BUCKSHOT,
	BULLET_PLAYER_CROWBAR,

	BULLET_PLAYER_COLT,
	BULLET_PLAYER_LUGER,
	BULLET_PLAYER_GARAND,
	BULLET_PLAYER_SCOPEDKAR,
	BULLET_PLAYER_THOMPSON,
	BULLET_PLAYER_MP44,
    BULLET_PLAYER_SPRING,
	BULLET_PLAYER_KAR,
	BULLET_PLAYER_BAR,
	BULLET_PLAYER_MP40,
	BULLET_PLAYER_MG42,
	BULLET_PLAYER_MG34,
	BULLET_PLAYER_30CAL,
	BULLET_PLAYER_M1CARBINE,
	BULLET_PLAYER_GREASEGUN,
	BULLET_PLAYER_FG42,
	BULLET_PLAYER_K43,
	BULLET_PLAYER_ENFIELD,
    BULLET_PLAYER_STEN,
	BULLET_PLAYER_BREN,
	BULLET_PLAYER_WEBLEY,

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM
}Bullet;


#endif // EV_HLDMH
