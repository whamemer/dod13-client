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

LINK_ENTITY_TO_CLASS( weapon_webley, CWEBLEY )

extern p_wpninfo_s P_WpnInfo[];

void CWEBLEY::Spawn( void )
{
	Precache();

	m_iId = WEAPON_WEBLEY;
	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_WEBLEY].wmodel );

	m_iDefaultAmmo = P_WpnInfo[WEAPON_WEBLEY].ammo_default;

	FallInit();
}

void CWEBLEY::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_WEBLEY].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_WEBLEY].wmodel );

	PRECACHE_SOUND( "weapons/webley_shoot.wav" );
	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( "weapons/colt_reload_clipin.wav" );
	PRECACHE_SOUND( "weapons/colt_reload_clipout.wav" );
	PRECACHE_SOUND( "weapons/colt_draw_pullslide.wav" );

	m_usFireWebley = PRECACHE_EVENT( 1, "events/weapons/webley.sc" );
}

int CWEBLEY::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_12mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_WEBLEY].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_WEBLEY].ammo_maxclip;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_PISTOL;
	p->iId = m_iId = WEAPON_WEBLEY;
	p->iWeight = P_WpnInfo[WEAPON_WEBLEY].misc_weight;

	return 1;
}

int CWEBLEY::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return TRUE;

	return FALSE;
}

BOOL CWEBLEY::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_pPlayer->CheckPlayerSpeed();

	return TimedDeploy( P_WpnInfo[WEAPON_WEBLEY].vmodel, P_WpnInfo[WEAPON_WEBLEY].pmodel, WEBLEY_DRAW, P_WpnInfo[WEAPON_WEBLEY].szAnimExt, P_WpnInfo[WEAPON_WEBLEY].szAnimReloadExt, P_WpnInfo[WEAPON_WEBLEY].anim_drawtime, 1.0f, UseDecrement() != 0 );
}

BOOL CWEBLEY::CanHolster( void )
{
	return TRUE;
}

void CWEBLEY::PrimaryAttack( void )
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

	m_iClip--;

	flSpread = P_WpnInfo[m_iId].base_accuracy;

	if( m_pPlayer->pev->velocity.Length() > 45.0f )
		flSpread += P_WpnInfo[m_iId].accuracy_penalty;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_WEBLEY, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireWebley, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, 0, 0, m_iClip == 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_WEBLEY].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;

	m_fInAttack = 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0f, 15.0f );

	RemoveStamina( 1.0f, m_pPlayer );
}

void CWEBLEY::SecondaryAttack( void )
{
	/* Nothing. */
}

void CWEBLEY::Reload( void )
{
	DefaultReload( P_WpnInfo[WEAPON_WEBLEY].ammo_maxclip, WEBLEY_RELOAD, P_WpnInfo[WEAPON_WEBLEY].anim_reloadtime );
}

void CWEBLEY::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( WEBLEY_IDLE, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
	}
}

class CWebleyAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		int bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_WEBLEY].ammo_maxclip, "ammo_12mm", P_WpnInfo[WEAPON_WEBLEY].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_webley, CWebleyAmmoClip )
