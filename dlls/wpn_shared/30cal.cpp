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

LINK_ENTITY_TO_CLASS( weapon_30cal, C30CAL )

extern p_wpninfo_s P_WpnInfo[];

void C30CAL::Spawn( void )
{
	CBipodWeapon::Spawn( WEAPON_CAL30 );
}

void C30CAL::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_CAL30].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_CAL30].wmodel );

	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( "weapons/chainpull.wav" );
	PRECACHE_SOUND( "weapons/bulletchain.wav" );
	PRECACHE_SOUND( "weapons/clampdown.wav" );
	PRECACHE_SOUND( "weapons/mgbolt.wav" );
	PRECACHE_SOUND( "weapons/30cal_shoot.wav" );
	PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );

	m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/30cal.sc" );
}

int C30CAL::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_66mm_us";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_CAL30].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_CAL30].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_66MM_BIPOD;
	p->iId = m_iId = WEAPON_CAL30;
	p->iBulletId = BULLET_PLAYER_30CAL;
	p->flRecoil = 15.0f;
	p->iWeight = P_WpnInfo[WEAPON_CAL30].misc_weight;

	return 1;
}

int C30CAL::Classify( void )
{
	return CLASS_MACHINEGUN;
}

int C30CAL::GetReloadAnim( void )
{
	return CAL30_RELOAD;
}

int C30CAL::GetDrawAnim( void )
{
	return CAL30_DOWNTOUP;
}

int C30CAL::GetIdleAnim( void )
{
	return -1;
}

int C30CAL::GetUpToDownAnim( void )
{
	switch( m_iClip )
	{
	case 0: return CAL30_UPTODOWNEMPTY;
	case 1: return CAL30_UPTODOWN1;
	case 2: return CAL30_UPTODOWN2;
	case 3: return CAL30_UPTODOWN3;
	case 4: return CAL30_UPTODOWN4;
	case 5: return CAL30_UPTODOWN5;
	case 6: return CAL30_UPTODOWN6;
	case 7: return CAL30_UPTODOWN7;
	case 8: return CAL30_UPTODOWN8;
	default: return CAL30_UPTODOWN;
	}
}

int C30CAL::GetDownToUpAnim( void )
{
	switch( m_iClip )
	{
	case 0: return CAL30_DOWNTOUPEMPTY;
	case 1: return CAL30_DOWNTOUP1;
	case 2: return CAL30_DOWNTOUP2;
	case 3: return CAL30_DOWNTOUP3;
	case 4: return CAL30_DOWNTOUP4;
	case 5: return CAL30_DOWNTOUP5;
	case 6: return CAL30_DOWNTOUP6;
	case 7: return CAL30_DOWNTOUP7;
	case 8: return CAL30_DOWNTOUP8;
	default: return CAL30_DOWNTOUP;
	}
}

class C30CALAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_CAL30].ammo_maxclip, "ammo_66mm_us", P_WpnInfo[WEAPON_CAL30].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_30cal, C30CALAmmoClip )
