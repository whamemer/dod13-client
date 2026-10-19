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

LINK_ENTITY_TO_CLASS( weapon_fg42, CFG42 )
LINK_ENTITY_TO_CLASS( weapon_scopedfg42, CFG42 )

extern p_wpninfo_s P_WpnInfo[];

void CFG42::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_FG42].wmodel );
	m_iId = WEAPON_FG42;

	m_iDefaultAmmo = P_WpnInfo[WEAPON_FG42].ammo_default;

	FallInit();
}

int CFG42::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return TRUE;

	return FALSE;
}

void CFG42::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_FG42].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_FG42].wmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_SCOPED_FG42].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_SCOPED_FG42].wmodel );

	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( "weapons/fg42_shoot.wav" );

	m_usFireFG42 = PRECACHE_EVENT( 1, "events/weapons/fg42.sc" );
}

int CFG42::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_55mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_FG42].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_FG42].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_FG42;
	p->iWeight = P_WpnInfo[WEAPON_FG42].misc_weight;

	return 1;
}

BOOL CFG42::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.93f;

	if( m_iWeaponState & WPNSTATE_SCOPED )
		return DefaultDeploy( P_WpnInfo[WEAPON_SCOPED_FG42].vmodel, P_WpnInfo[WEAPON_SCOPED_FG42].pmodel, FG42_UP_DRAW, P_WpnInfo[WEAPON_SCOPED_FG42].szAnimExt, P_WpnInfo[WEAPON_SCOPED_FG42].szAnimReloadExt, 0 );

	return DefaultDeploy( P_WpnInfo[WEAPON_FG42].vmodel, P_WpnInfo[WEAPON_FG42].pmodel, FG42_UP_DRAW, P_WpnInfo[WEAPON_FG42].szAnimExt, P_WpnInfo[WEAPON_FG42].szAnimReloadExt, 0 );
}

void CFG42::PrimaryAttack( void )
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

	if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		if( GetFOV() > 0 )
			flSpread = 0.008f;
		else
			flSpread = P_WpnInfo[m_iId].base_accuracy;
	}
	else
	{
		if( m_pPlayer->IsInMGDeploy() )
			flSpread = P_WpnInfo[m_iId].base_accuracy2;
		else
			flSpread = P_WpnInfo[m_iId].base_accuracy;
	}

	if( m_pPlayer->pev->velocity.Length() > 45.0f )
		flSpread += P_WpnInfo[m_iId].accuracy_penalty;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_FG42, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireFG42, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.y, vecNewShot.z, 0, 0, m_iClip == 0, GetFOV() > 0 );

	if( GetFOV() <= 0 )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_FG42].anim_firedelay;
	else
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_SCOPED_FG42].anim_firedelay;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );

	if( GetFOV() > 0 && m_iClip <= 0 )
	{
		SetThink( &CBasePlayerWeapon::ThinkZoomOut );
		pev->nextthink = gpGlobals->time + 0.5f;
	}

	RemoveStamina( 2.0f, m_pPlayer );
}

void CFG42::SecondaryAttack( void )
{
	if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		m_pPlayer->m_iFOV = ChangeFOV( 20 );
		UpdateZoomSpeed();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2f;
	}
	else
	{
		if( m_pPlayer->pev->iuser3 == STANCE_PRONE || m_pPlayer->pev->vuser1.x == 1.0f )
		{
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
			SendWeaponAnim( FG42_UP_TO_DOWN, 1 );
		}
		else if( m_pPlayer->pev->iuser3 == STANCE_PRONE_DEPLOYED || m_pPlayer->pev->vuser1.x == SANDBAG_DEPLOYED )
		{
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.98f;
			SendWeaponAnim( FG42_DOWN_TO_UP, 1 );
		}

		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3f;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3f;
	}
}

void CFG42::Reload( void )
{
	float fDelay;

	if( GetFOV() > 0 )
	{
		m_pPlayer->m_iFOV = ZoomOut();
		UpdateZoomSpeed();
	}

	fDelay = P_WpnInfo[WEAPON_FG42].anim_reloadtime;

	if( m_pPlayer->IsInMGDeploy() )
		DefaultReload( P_WpnInfo[WEAPON_FG42].ammo_maxclip, FG42_DOWN_RELOAD, fDelay );
	else
		DefaultReload( P_WpnInfo[WEAPON_FG42].ammo_maxclip, FG42_UP_RELOAD, fDelay );
}

void CFG42::Holster( int skiplocal )
{
	if( !m_pPlayer->IsInMGDeploy() )
	{
		SetThink( NULL );

		CBasePlayerWeapon::Holster( skiplocal );
	}
}

BOOL CFG42::CanHolster( void )
{
	return !m_pPlayer->IsInMGDeploy();
}

BOOL CFG42::IsDeployed( void )
{
	return m_pPlayer->IsInMGDeploy();
}

void CFG42::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		if( !m_pPlayer->IsInMGDeploy() )
			SendWeaponAnim( FG42_UP_IDLE, 1 );
		else
			SendWeaponAnim( FG42_DOWN_IDLE, 1 );

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	}
}

int CFG42::Classify( void )
{
	return ( m_iWeaponState & WPNSTATE_SCOPED ) == 0 ? CLASS_MACHINEGUN : CLASS_SCOPED_RIFLE;
}

class CFG42AmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_FG42].ammo_maxclip, "ammo_55mm", P_WpnInfo[WEAPON_FG42].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_fg42, CFG42AmmoClip )
