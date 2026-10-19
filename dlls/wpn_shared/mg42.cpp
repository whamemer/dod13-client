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

LINK_ENTITY_TO_CLASS( weapon_mg42, CMG42 )

extern p_wpninfo_s P_WpnInfo[];

void CMG42::Spawn( void )
{
	CBipodWeapon::Spawn( WEAPON_MG42 );
}

void CMG42::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_MG42].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_MG42].wmodel );

	PRECACHE_SOUND( "weapons/weaponempty.wav" );
	PRECACHE_SOUND( "weapons/chainpull.wav" );
	PRECACHE_SOUND( "weapons/bulletchain.wav" );
	PRECACHE_SOUND( "weapons/clampdown.wav" );
	PRECACHE_SOUND( "weapons/mgbolt.wav" );
	PRECACHE_SOUND( "weapons/mg42_shoot.wav" );
	PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );
	PRECACHE_SOUND( "weapons/mgoverheat.wav" );

	m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/mg42.sc" );

	CBipodWeapon::Precache();
}

int CMG42::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_66mm_ger";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_MG42].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_MG42].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_66MM_GER_BIPOD;
	p->iId = m_iId = WEAPON_MG42;
	p->iBulletId = BULLET_PLAYER_MG42;
	p->flRecoil = 15.0f;
	p->iWeight = P_WpnInfo[WEAPON_MG42].misc_weight;

	return 1;
}

int CMG42::Classify( void )
{
	return CLASS_MACHINEGUN;
}

int CMG42::GetReloadAnim( void )
{
	return MG42_RELOAD;
}

int CMG42::GetDrawAnim( void )
{
	return GetDownToUpAnim();
}

int CMG42::GetIdleAnim( void )
{
	return -1;
}

int CMG42::GetUpToDownAnim( void )
{
	switch( m_iClip )
	{
	case 0: return MG42_UPTODOWNEMPTY;
	case 1: return MG42_UPTODOWN1;
	case 2: return MG42_UPTODOWN2;
	case 3: return MG42_UPTODOWN3;
	case 4: return MG42_UPTODOWN4;
	case 5: return MG42_UPTODOWN5;
	case 6: return MG42_UPTODOWN6;
	case 7: return MG42_UPTODOWN7;
	case 8: return MG42_UPTODOWN8;
	default: return MG42_UPTODOWN;
	}
}

int CMG42::GetDownToUpAnim( void )
{
	switch( m_iClip )
	{
	case 0: return MG42_DOWNTOUPEMPTY;
	case 1: return MG42_DOWNTOUP1;
	case 2: return MG42_DOWNTOUP2;
	case 3: return MG42_DOWNTOUP3;
	case 4: return MG42_DOWNTOUP4;
	case 5: return MG42_DOWNTOUP5;
	case 6: return MG42_DOWNTOUP6;
	case 7: return MG42_DOWNTOUP7;
	case 8: return MG42_DOWNTOUP8;
	default: return MG42_DOWNTOUP;
	}
}

class CMG42AmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_MG42].ammo_maxclip, "ammo_66mm_ger", P_WpnInfo[WEAPON_MG42].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_mg42, CMG42AmmoClip )
