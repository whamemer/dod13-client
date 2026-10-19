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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_bren, CBREN )

extern p_wpninfo_s P_WpnInfo[];

void CBREN::Spawn( void )
{
	CBipodWeapon::Spawn( WEAPON_BREN );
}

void CBREN::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_BREN].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_BREN].wmodel );

	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( "weapons/bren_shoot.wav" );

	m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/bren.sc" );
}

int CBREN::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_55mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_BREN].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_BREN].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_BREN;
	p->iBulletId = BULLET_PLAYER_BREN;
	p->flRecoil = 2.0f;
	p->iWeight = P_WpnInfo[WEAPON_BREN].misc_weight;

	return 1;
}

int CBREN::Classify( void )
{
	return CLASS_MACHINEGUN;
}

int CBREN::GetReloadAnim( void )
{
	return !IsDeployed() ? BREN_UP_RELOAD : BREN_DOWN_RELOAD;
}

int CBREN::GetDrawAnim( void )
{
	return BREN_UP_DRAW;
}

int CBREN::GetIdleAnim( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
		return IsDeployed() ? BREN_DOWN_IDLE : BREN_UP_IDLE;

	return -1;
}

int CBREN::GetUpToDownAnim( void )
{
	return BREN_UP_TO_DOWN;
}

int CBREN::GetDownToUpAnim( void )
{
	return BREN_DOWN_TO_UP;
}

class CBRENAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_BREN].ammo_maxclip, "ammo_55mm", P_WpnInfo[WEAPON_BREN].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_bren, CBRENAmmoClip )
