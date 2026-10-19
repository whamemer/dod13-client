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

LINK_ENTITY_TO_CLASS( weapon_mg34, CMG34 )

extern p_wpninfo_s P_WpnInfo[];

void CMG34::Spawn( void )
{
	CBipodWeapon::Spawn( WEAPON_MG34 );
}

void CMG34::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_MG34].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_MG34].wmodel );

	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );
	PRECACHE_SOUND( "weapons/tommy_reload_clipout.wav" );
	PRECACHE_SOUND( "weapons/clampdown.wav" );
	PRECACHE_SOUND( "weapons/mgbolt.wav" );
	PRECACHE_SOUND( "weapons/mg34_shoot.wav" );

	m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/mg34.sc" );
}

int CMG34::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_66mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_MG34].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_MG34].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_66MM_BIPOD;
	p->iId = m_iId = WEAPON_MG34;
	p->iBulletId = BULLET_PLAYER_MG34;
	p->flRecoil = 15.0f;
	p->iWeight = P_WpnInfo[WEAPON_MG34].misc_weight;

	return 1;
}

int CMG34::Classify( void )
{
	return CLASS_MACHINEGUN;
}

int CMG34::GetReloadAnim( void )
{
	return MG34_RELOAD;
}

int CMG34::GetDrawAnim( void )
{
	return GetDownToUpAnim();
}

int CMG34::GetUpToDownAnim( void )
{
	return m_iClip <= 0 ? MG34_UPTODOWNEMPTY : MG34_UPTODOWN;
}

int CMG34::GetDownToUpAnim( void )
{
	return m_iClip <= 0 ? MG34_DOWNTOUPEMPTY : MG34_DOWNTOUP;
}

int CMG34::GetIdleAnim( void )
{
	if( IsDeployed() )
		return m_iClip <= 0 ? MG34_DOWNIDLEEMPTY : MG34_DOWNIDLE;

	return m_iClip <= 0 ? MG34_UPIDLEEMPTY : MG34_UPIDLE;
}

class CMG34AmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_MG34].ammo_maxclip, "ammo_66mm", P_WpnInfo[WEAPON_MG34].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_mg34, CMG34AmmoClip )
