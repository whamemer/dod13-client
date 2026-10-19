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

LINK_ENTITY_TO_CLASS( weapon_m1carbine, CM1Carbine )

extern p_wpninfo_s P_WpnInfo[];

void CM1Carbine::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_M1CARBINE].wmodel );
	m_iId = WEAPON_M1CARBINE;

	m_iDefaultAmmo = P_WpnInfo[WEAPON_M1CARBINE].ammo_default;

	FallInit();
}

int CM1Carbine::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_M1CARBINE].wmodel );
		return TRUE;
	}

	return FALSE;
}

void CM1Carbine::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_M1CARBINE].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_M1CARBINE].wmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_FOLDINGCARBINE].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_FOLDINGCARBINE].wmodel );

	PRECACHE_SOUND( "weapons/carbine_shoot.wav" );
	PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );
	PRECACHE_SOUND( "weapons/tommy_reload_clipout.wav" );
	PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

	m_usFireM1Carbine = PRECACHE_EVENT( 1, "events/weapons/m1carbine.sc" );
}

int CM1Carbine::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_66mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_M1CARBINE].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_M1CARBINE].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_M1CARBINE;
	p->iWeight = P_WpnInfo[WEAPON_M1CARBINE].misc_weight;

	return 1;
}

BOOL CM1Carbine::SpawnDeploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

	return DefaultDeploy( P_WpnInfo[WEAPON_M1CARBINE].vmodel, P_WpnInfo[WEAPON_M1CARBINE].pmodel, M1CARBINE_DRAW, P_WpnInfo[WEAPON_M1CARBINE].szAnimExt, P_WpnInfo[WEAPON_M1CARBINE].szAnimReloadExt, 0 );
}

BOOL CM1Carbine::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

	return DefaultDeploy( P_WpnInfo[WEAPON_M1CARBINE].vmodel, P_WpnInfo[WEAPON_M1CARBINE].pmodel, M1CARBINE_DRAW, P_WpnInfo[WEAPON_M1CARBINE].szAnimExt, P_WpnInfo[WEAPON_M1CARBINE].szAnimReloadExt, 0 );
}

BOOL CM1Carbine::CanHolster( void )
{
	return TRUE;
}

void CM1Carbine::PrimaryAttack( void )
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
		flSpread += P_WpnInfo[m_iId].accuracy_penalty;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_M1CARBINE, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireM1Carbine, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, (int)m_pPlayer->pev->punchangle.x, 0, m_iClip, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_M1CARBINE].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.05f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	m_fInAttack = 1;

	RemoveStamina( 2.0f, m_pPlayer );
}

void CM1Carbine::SecondaryAttack( void )
{
	/* Nothing. */
}

void CM1Carbine::Reload( void )
{
	DefaultReload( P_WpnInfo[WEAPON_M1CARBINE].ammo_maxclip, M1CARBINE_RELOAD, P_WpnInfo[WEAPON_M1CARBINE].anim_reloadtime );
}

void CM1Carbine::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( M1CARBINE_IDLE, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	}
}

int CM1Carbine::Classify( void )
{
	return CLASS_RIFLE;
}

class CM1CarbineAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_M1CARBINE].ammo_maxclip, "ammo_66mm", P_WpnInfo[WEAPON_M1CARBINE].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_m1carbine, CM1CarbineAmmoClip )
