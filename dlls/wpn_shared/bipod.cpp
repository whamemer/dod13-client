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

extern p_wpninfo_s P_WpnInfo[];

void CBipodWeapon::Spawn( int weapon_id )
{
	Precache();

	m_iId = weapon_id;
	SET_MODEL( ENT( pev ), P_WpnInfo[weapon_id].wmodel );

	m_iDefaultAmmo = P_WpnInfo[m_iId].ammo_default;

	FallInit();
}

void CBipodWeapon::Precache( void )
{
	m_iOverheatEvent = PRECACHE_EVENT( 1, "events/effects/overheat.sc" );
}

int CBipodWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return TRUE;

	return FALSE;
}

BOOL CBipodWeapon::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

	return DefaultDeploy( P_WpnInfo[m_iId].vmodel, P_WpnInfo[m_iId].pmodel, GetDrawAnim(), P_WpnInfo[m_iId].szAnimExt, P_WpnInfo[m_iId].szAnimReloadExt, 0 );
}

BOOL CBipodWeapon::CanHolster( void )
{
	return !m_pPlayer->IsInMGDeploy();
}

void CBipodWeapon::Holster( int skiplocal )
{
	CBasePlayerWeapon::Holster( skiplocal );
}

void CBipodWeapon::PrimaryAttack( void )
{
	ItemInfo info;
	float flSpread;
	vec3_t vecSrc;
	vec3_t vecAiming;
	vec3_t vecNewShot;
	vec3_t vecNewShot2;

	if( PlayerIsWaterSniping() )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
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

	GetItemInfo( &info );

	if( info.iFlags & ITEM_FLAG_HEAT )
	{
		if( m_fInAttack )
			return;

		if( m_flWeaponHeat >= 99.0f )
		{
			m_fInAttack = 1;
			return;
		}

		if( m_flWeaponHeat >= 95.0f )
			PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_iOverheatEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, 0, 0, 0, 0 );
	}

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = 600;

	if( m_pPlayer->IsInMGDeploy() )
		flSpread = P_WpnInfo[m_iId].base_accuracy2;
	else
		flSpread = P_WpnInfo[m_iId].base_accuracy;

	if( !m_pPlayer->IsInMGDeploy() )
	{
		if( m_pPlayer->pev->velocity.Length() > 45.0f )
			flSpread += P_WpnInfo[m_iId].accuracy_penalty;
	}

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, info.iBulletId, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	m_iClip--;

	// the MG42 fires two bullets per shot
	if( m_iId == WEAPON_MG42 )
	{
		m_iClip--;

		vecNewShot2 = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, info.iBulletId, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed + 5 );

		PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_iFireEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot2.x, vecNewShot2.y, m_iClip, 0, 1, 0 );
	}

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_iFireEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, m_iClip, 0, 0, 0 );

	if( info.iFlags & ITEM_FLAG_HEAT )
		m_flWeaponHeat += 3.0f;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_firedelay;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_firedelay;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );

	if( !m_pPlayer->IsInMGDeploy() )
		RemoveStamina( info.flRecoil, m_pPlayer );
}

void CBipodWeapon::CoolThink( void )
{
	m_flWeaponHeat -= 0.6f;

	if( m_flWeaponHeat <= 0.0f )
	{
		m_flWeaponHeat = 0.0f;
		SetThink( NULL );
	}
	else
	{
		pev->nextthink = gpGlobals->time + 0.1f;
	}
}

void CBipodWeapon::SecondaryAttack( void )
{
	if( m_pPlayer->pev->waterlevel > 0 && m_pPlayer->pev->iuser3 != STANCE_PRONE_DEPLOYED )
		return;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_down2uptime;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_down2uptime;

	if( m_pPlayer->pev->iuser3 == STANCE_PRONE || m_pPlayer->pev->vuser1.x == 1.0f )
		SendWeaponAnim( GetUpToDownAnim(), 1 );
	else if( m_pPlayer->pev->iuser3 == STANCE_PRONE_DEPLOYED || m_pPlayer->pev->vuser1.x == SANDBAG_DEPLOYED )
		SendWeaponAnim( GetDownToUpAnim(), 1 );
}

void CBipodWeapon::ForceUndeploy( void )
{
	SendWeaponAnim( GetDownToUpAnim(), 0 );
}

void CBipodWeapon::Reload( void )
{
	float fDelay = P_WpnInfo[m_iId].anim_reloadtime;

	if( m_iId == WEAPON_BREN && m_pPlayer->IsInMGDeploy() )
		fDelay = P_WpnInfo[m_iId].anim_reloadtime2;

	DefaultReload( P_WpnInfo[m_iId].ammo_maxclip, GetReloadAnim(), fDelay );
}

void CBipodWeapon::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if( m_flNextPrimaryAttack > UTIL_WeaponTimeBase() )
		return;

	if( m_flNextSecondaryAttack > UTIL_WeaponTimeBase() )
		return;

	int iAnim = GetIdleAnim();

	if( iAnim >= 0 )
		SendWeaponAnim( iAnim, 1 );
}

bool CBipodWeapon::IsDeployed( void )
{
	return m_pPlayer->IsInMGDeploy();
}

float CBipodWeapon::GetBipodSpread( void )
{
	if( m_pPlayer->IsInMGDeploy() )
		return P_WpnInfo[m_iId].base_accuracy2;

	return P_WpnInfo[m_iId].base_accuracy;
}
