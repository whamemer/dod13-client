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

void CPistol::Spawn( int weapon_id )
{
	Precache();

	m_iId = weapon_id;
	SET_MODEL( ENT( pev ), P_WpnInfo[weapon_id].wmodel );

	m_iDefaultAmmo = P_WpnInfo[m_iId].ammo_default;

	FallInit();
}

BOOL CPistol::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_pPlayer->CheckPlayerSpeed();

	return TimedDeploy( P_WpnInfo[m_iId].vmodel, P_WpnInfo[m_iId].pmodel, GetDrawAnim(), P_WpnInfo[m_iId].szAnimExt, P_WpnInfo[m_iId].szAnimReloadExt, P_WpnInfo[m_iId].anim_drawtime, P_WpnInfo[m_iId].anim_drawtime, UseDecrement() != 0 );
}

BOOL CPistol::CanHolster( void )
{
	return TRUE;
}

void CPistol::PrimaryAttack( void )
{
	ItemInfo pistol;
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

	m_iClip--;

	GetItemInfo( &pistol );

	flSpread = P_WpnInfo[m_iId].base_accuracy;

	if( m_pPlayer->pev->velocity.Length() > 45.0f )
		flSpread += P_WpnInfo[m_iId].accuracy_penalty;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, pistol.iBulletId, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_iFireEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, 0, 0, m_iClip == 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_firedelay;

	m_fInAttack = 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0f, 15.0f );

	RemoveStamina( 1.0f, m_pPlayer );
}

void CPistol::Reload( void )
{
	DefaultReload( P_WpnInfo[m_iId].ammo_maxclip, GetReloadAnim(), P_WpnInfo[m_iId].anim_reloadtime );
}

void CPistol::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( GetIdleAnim(), 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.0f;
	}
}
