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

LINK_ENTITY_TO_CLASS( weapon_piat, CPIAT )

extern p_wpninfo_s P_WpnInfo[];

extern void ClientSetSensitivity( int sens );

void CPIAT::Reload( void )
{
	if( m_iClip > 0 )
		return;

	float fDelay = P_WpnInfo[WEAPON_PIAT].anim_reloadtime;
	BOOL bReload;

	if( m_iWeaponState & WPNSTATE_SCOPED )
		bReload = DefaultReload( P_WpnInfo[WEAPON_PIAT].ammo_maxclip, PIAT_RELOAD_AIMED, fDelay );
	else
		bReload = DefaultReload( P_WpnInfo[WEAPON_PIAT].ammo_maxclip, PIAT_RELOAD_IDLE, fDelay );

	if( bReload )
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_reloadtime;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_reloadtime;

		if( m_iWeaponState & WPNSTATE_SCOPED )
		{
			m_iWeaponState = 0;
			ClientSetSensitivity( 0 );
			m_iWeaponState = WPNSTATE_ROCKET_SLOW;
		}
	}
}

void CPIAT::Spawn( void )
{
	Precache();

	m_iId = WEAPON_PIAT;
	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_PIAT].wmodel );

	m_iDefaultAmmo = P_WpnInfo[WEAPON_PIAT].ammo_default;

	FallInit();

	m_iClip = P_WpnInfo[WEAPON_PIAT].ammo_maxclip;
}

void CPIAT::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_PIAT].wmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_PIAT].vmodel );

	PRECACHE_SOUND( "items/ammopickup.wav" );
	PRECACHE_SOUND( "weapons/rocketfire1.wav" );
	PRECACHE_SOUND( "weapons/glauncher.wav" );

	m_usFirePIAT = PRECACHE_EVENT( 1, "events/weapons/piat.sc" );
}

int CPIAT::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_PIAT].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 1;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_PIAT;
	p->iFlags = ITEM_FLAG_ROCKET;
	p->iWeight = 20;

	return 1;
}

BOOL CPIAT::Deploy( void )
{
	m_iWeaponState &= WPNSTATE_SCOPED;

	int iAnim = ( m_iClip != 1 ) + PIAT_DRAW;

	return TimedDeploy( P_WpnInfo[WEAPON_PIAT].vmodel, P_WpnInfo[WEAPON_PIAT].pmodel, iAnim, P_WpnInfo[WEAPON_PIAT].szAnimExt, P_WpnInfo[WEAPON_PIAT].szAnimReloadExt, P_WpnInfo[WEAPON_PIAT].anim_drawtime, P_WpnInfo[WEAPON_PIAT].anim_drawtime, UseDecrement() != 0 );
}

BOOL CPIAT::CanHolster( void )
{
	return !( m_iWeaponState & WPNSTATE_SCOPED );
}

void CPIAT::Holster( int skiplocal )
{
	ClientSetSensitivity( 0 );

	m_iWeaponState &= ~( WPNSTATE_SCOPED | WPNSTATE_ROCKET_SLOW );

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

	CBasePlayerWeapon::Holster( skiplocal );
}

void CPIAT::PrimaryAttack( void )
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

		PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFirePIAT, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, 0, 0, m_iClip == 0, 0 );

		m_iClip--;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_firedelay;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_firedelay;
	}
	else
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_firedelay;
	}
}

void CPIAT::SecondaryAttack( void )
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

void CPIAT::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		if( m_iWeaponState & WPNSTATE_SCOPED )
			SendWeaponAnim( ( m_iClip != 1 ) + PIAT_AIMED, 1 );
		else
			SendWeaponAnim( m_iClip != 1, 1 );

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0f;
	}
}

void CPIAT::Raise( void )
{
	SendWeaponAnim( ( m_iClip != 1 ) + PIAT_RAISE, 1 );

	ReSlow();

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_down2uptime;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_down2uptime;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_down2uptime;
}

void CPIAT::Lower( void )
{
	SendWeaponAnim( ( m_iClip != 1 ) + PIAT_LOWER, 1 );

	m_iWeaponState = 0;
	ClientSetSensitivity( 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_up2downtime;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_up2downtime;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_PIAT].anim_up2downtime;
}

void CPIAT::UnSlow( void )
{
	m_iWeaponState = 0;
	ClientSetSensitivity( 0 );
}

void CPIAT::ReSlow( void )
{
	m_iWeaponState = WPNSTATE_SCOPED;
	ClientSetSensitivity( 1 );
}

class CPIATAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/w_piat_rocket.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual void Precache( void )
	{
		PRECACHE_MODEL( "models/w_piat_rocket.mdl" );
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

LINK_ENTITY_TO_CLASS( ammo_piat, CPIATAmmo )
