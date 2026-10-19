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

LINK_ENTITY_TO_CLASS( weapon_k43, CK43 )

extern p_wpninfo_s P_WpnInfo[];

void CK43::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_K43].wmodel );
	m_iId = WEAPON_K43;

	m_iDefaultAmmo = P_WpnInfo[WEAPON_K43].ammo_default;

	FallInit();
}

int CK43::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_K43].wmodel );
		return TRUE;
	}

	return FALSE;
}

void CK43::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_K43].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_K43].wmodel );

	PRECACHE_SOUND( "weapons/k43_shoot.wav" );

	m_usFireK43 = PRECACHE_EVENT( 1, "events/weapons/k43.sc" );
}

int CK43::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_66mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_K43].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_K43].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_K43;
	p->iWeight = P_WpnInfo[WEAPON_K43].misc_weight;

	return 1;
}

BOOL CK43::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	return TimedDeploy( P_WpnInfo[WEAPON_K43].vmodel, P_WpnInfo[WEAPON_K43].pmodel, K43_DRAW, P_WpnInfo[WEAPON_K43].szAnimExt, P_WpnInfo[WEAPON_K43].szAnimReloadExt, 0.79f, 0.79f, UseDecrement() != 0 );
}

BOOL CK43::CanHolster( void )
{
	return TRUE;
}

void CK43::PrimaryAttack( void )
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

	if( m_fInAttack )
		return;

	if( m_iClip <= 0 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		m_fInAttack = 1;
		return;
	}

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = 600;

	m_iClip--;

	flSpread = P_WpnInfo[m_iId].base_accuracy;

	if( m_pPlayer->pev->velocity.Length() > 45.0f )
		flSpread += 0.1;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_K43, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireK43, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, (int)m_pPlayer->pev->punchangle.x, 0, m_iClip, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_K43].anim_firedelay;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_K43].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	m_fInAttack = 1;

	RemoveStamina( 2.0f, m_pPlayer );
}

void CK43::SecondaryAttack( void )
{
	RifleMeleeAttack( m_pPlayer, 24 );

	m_pPlayer->SetAnimation( PLAYER_BUT_SWING );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireK43, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, 1, 0, m_iClip == 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;
}

void CK43::Reload( void )
{
	DefaultReload( P_WpnInfo[WEAPON_K43].ammo_maxclip, K43_RELOAD, P_WpnInfo[WEAPON_K43].anim_reloadtime );
}

void CK43::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( K43_IDLE, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	}
}

int CK43::Classify( void )
{
	return CLASS_RIFLE;
}

class CK43AmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_K43].ammo_maxclip, "ammo_66mm", P_WpnInfo[WEAPON_K43].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_k43, CK43AmmoClip )
