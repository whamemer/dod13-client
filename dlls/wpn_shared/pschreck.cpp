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

LINK_ENTITY_TO_CLASS( weapon_pschreck, CPschreck )

extern p_wpninfo_s P_WpnInfo[];

extern void ClientSetSensitivity( int sens );

void CPschreck::Reload( void )
{
	if( m_iClip > 0 )
		return;

	if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		if( !DefaultReload( P_WpnInfo[WEAPON_PSCHRECK].ammo_maxclip, PSCHRECK_RELOAD_AIMED, P_WpnInfo[WEAPON_PSCHRECK].anim_reloadtime ) )
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_reloadtime;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_reloadtime;
	}
	else
	{
		if( !DefaultReload( P_WpnInfo[WEAPON_PSCHRECK].ammo_maxclip, PSCHRECK_RELOAD_IDLE, P_WpnInfo[WEAPON_PSCHRECK].anim_reloadtime2 ) )
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_reloadtime2;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_reloadtime2;
	}

	if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		m_iWeaponState = 0;
		ClientSetSensitivity( 0 );
		m_iWeaponState = WPNSTATE_ROCKET_SLOW;
	}
}

void CPschreck::Spawn( void )
{
	Precache();

	m_iId = WEAPON_PSCHRECK;
	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_PSCHRECK].wmodel );

	m_iDefaultAmmo = P_WpnInfo[WEAPON_PSCHRECK].ammo_default;

	FallInit();

	m_iClip = P_WpnInfo[WEAPON_PSCHRECK].ammo_maxclip;
}

void CPschreck::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_PSCHRECK].wmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_PSCHRECK].vmodel );

	PRECACHE_SOUND( "items/ammopickup.wav" );
	PRECACHE_SOUND( "weapons/rocketfire1.wav" );
	PRECACHE_SOUND( "weapons/glauncher.wav" );

	m_usFirePschreck = PRECACHE_EVENT( 1, "events/weapons/pschreck.sc" );
}

int CPschreck::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_PSCHRECK].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 1;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_PSCHRECK;
	p->iFlags = ITEM_FLAG_ROCKET;
	p->iWeight = 20;

	return 1;
}

BOOL CPschreck::Deploy( void )
{
	m_iWeaponState &= WPNSTATE_SCOPED;

	return TimedDeploy( P_WpnInfo[WEAPON_PSCHRECK].vmodel, P_WpnInfo[WEAPON_PSCHRECK].pmodel, PSCHRECK_DRAW, P_WpnInfo[WEAPON_PSCHRECK].szAnimExt, P_WpnInfo[WEAPON_PSCHRECK].szAnimReloadExt, P_WpnInfo[WEAPON_PSCHRECK].anim_drawtime, P_WpnInfo[WEAPON_PSCHRECK].anim_drawtime, UseDecrement() != 0 );
}

BOOL CPschreck::CanHolster( void )
{
	return !( m_iWeaponState & WPNSTATE_SCOPED );
}

void CPschreck::Holster( int skiplocal )
{
	ClientSetSensitivity( 0 );

	m_iWeaponState &= ~( WPNSTATE_SCOPED | WPNSTATE_ROCKET_SLOW );

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

	CBasePlayerWeapon::Holster( skiplocal );
}

void CPschreck::PrimaryAttack( void )
{
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

	if( m_pPlayer->pev->waterlevel > 1 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
	}
	else if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		m_pPlayer->m_iWeaponVolume = 1000;

		PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFirePschreck, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, 0, 0, m_iClip == 0, 0 );

		m_iClip--;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_firedelay;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.17f;
	}
	else
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_firedelay;
	}
}

void CPschreck::SecondaryAttack( void )
{
	if( m_pPlayer->pev->waterlevel <= 1 )
	{
		if( !( m_iWeaponState & WPNSTATE_SCOPED ) )
			Raise();
		else
			Lower();
	}
	else if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		Lower();
	}
}

void CPschreck::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		if( m_iWeaponState & WPNSTATE_SCOPED )
			SendWeaponAnim( PSCHRECK_AIMED, 1 );
		else
			SendWeaponAnim( PSCHRECK_IDLE, 1 );

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0f;
	}
}

void CPschreck::Raise( void )
{
	SendWeaponAnim( PSCHRECK_RAISE, 1 );

	m_iWeaponState |= WPNSTATE_SCOPED;
	ClientSetSensitivity( 1 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_down2uptime;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_down2uptime;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_down2uptime;
}

void CPschreck::Lower( void )
{
	SendWeaponAnim( PSCHRECK_LOWER, 1 );

	m_iWeaponState &= ~WPNSTATE_SCOPED;
	ClientSetSensitivity( 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_up2downtime;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_up2downtime;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PSCHRECK].anim_up2downtime;
}

void CPschreck::UnSlow( void )
{
	m_iWeaponState = 0;
	ClientSetSensitivity( 0 );
}

void CPschreck::ReSlow( void )
{
	m_iWeaponState = WPNSTATE_SCOPED;
	ClientSetSensitivity( 1 );
}

class CPschreckAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/w_pschreck_rocket.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual void Precache( void )
	{
		PRECACHE_MODEL( "models/w_pschreck_rocket.mdl" );
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		if( pOther->GiveAmmo( 2, "rockets", 5 ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( ammo_pschreck, CPschreckAmmo )
