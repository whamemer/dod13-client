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
#include "player.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_greasegun, CGreaseGun )

extern p_wpninfo_s P_WpnInfo[];

void CGreaseGun::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_GREASEGUN].wmodel );
	m_iId = WEAPON_GREASEGUN;

	m_iDefaultAmmo = P_WpnInfo[WEAPON_GREASEGUN].ammo_default;

	FallInit();
}

void CGreaseGun::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_GREASEGUN].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_GREASEGUN].wmodel );

	PRECACHE_SOUND( "weapons/greasegun_shoot.wav" );
	PRECACHE_SOUND( "weapons/357_cock1.wav" );

	m_usFireGreaseGun = PRECACHE_EVENT( 1, "events/weapons/greasegun.sc" );
}

int CGreaseGun::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_44mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_GREASEGUN].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_GREASEGUN].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_GREASEGUN;
	p->iWeight = P_WpnInfo[WEAPON_GREASEGUN].misc_weight;

	return 1;
}

int CGreaseGun::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return TRUE;

	return FALSE;
}

BOOL CGreaseGun::SpawnDeploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	return TimedDeploy( P_WpnInfo[WEAPON_GREASEGUN].vmodel, P_WpnInfo[WEAPON_GREASEGUN].pmodel, GREASEGUN_DRAW, P_WpnInfo[WEAPON_GREASEGUN].szAnimExt, P_WpnInfo[WEAPON_GREASEGUN].szAnimReloadExt, 1.6f, 1.6f, UseDecrement() != 0 );
}

BOOL CGreaseGun::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

	return DefaultDeploy( P_WpnInfo[WEAPON_GREASEGUN].vmodel, P_WpnInfo[WEAPON_GREASEGUN].pmodel, GREASEGUN_FAST_DRAW, P_WpnInfo[WEAPON_GREASEGUN].szAnimExt, P_WpnInfo[WEAPON_GREASEGUN].szAnimReloadExt, 0 );
}

BOOL CGreaseGun::CanHolster( void )
{
	return TRUE;
}

void CGreaseGun::PrimaryAttack( void )
{
	float flSpread;
	vec3_t vecSrc;
	vec3_t vecAiming;
	vec3_t vecNewShot;

	if( PlayerIsWaterSniping() )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		return;
	}

	if( m_iClip <= 0 )
	{
		if( !m_fInAttack )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
			m_fInAttack = 1;
		}

		return;
	}

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = 600;

	m_iClip--;

	flSpread = P_WpnInfo[m_iId].base_accuracy;

	if( m_pPlayer->pev->velocity.Length() > 45.0f )
		flSpread += P_WpnInfo[m_iId].accuracy_penalty;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_GREASEGUN, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireGreaseGun, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, 0, 0, m_iClip == 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_GREASEGUN].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );

	RemoveStamina( 2.0f, m_pPlayer );
}

void CGreaseGun::Reload( void )
{
	DefaultReload( P_WpnInfo[WEAPON_GREASEGUN].ammo_maxclip, GREASEGUN_RELOAD, P_WpnInfo[WEAPON_GREASEGUN].anim_reloadtime );
}

void CGreaseGun::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( m_iClip <= 0 ? GREASEGUN_IDLE_EMPTY : GREASEGUN_IDLE, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	}
}

int CGreaseGun::Classify( void )
{
	return CLASS_SMG;
}

class CGreaseGunAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_GREASEGUN].ammo_maxclip, "ammo_44mm", P_WpnInfo[WEAPON_GREASEGUN].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_greasegun, CGreaseGunAmmoClip )
