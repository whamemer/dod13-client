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

LINK_ENTITY_TO_CLASS( weapon_handgrenade, CHandGrenade )

extern p_wpninfo_s P_WpnInfo[];

void CHandGrenade::Spawn( void )
{
	Precache();

	m_iId = WEAPON_HANDGRENADE;
	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_HANDGRENADE].wmodel );

	m_iDefaultAmmo = 1;

	FallInit();
}

void CHandGrenade::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_HANDGRENADE].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_HANDGRENADE].wmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_MILLSGRENADE].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_MILLSGRENADE].wmodel );

	PRECACHE_SOUND( "weapons/grenthrow.wav" );
}

int CHandGrenade::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_agrens";
	p->iMaxAmmo1 = 3;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_HANDGRENADE;
	p->iWeight = 1;
	p->iFlags = ITEM_FLAG_GRENADE;

	return 1;
}

BOOL CHandGrenade::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();

	m_flReleaseThrow = -1.0f;

	SendWeaponAnim( HANDGRENADE_DRAW, UseDecrement() != 0 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.85f;

	return FALSE;
}

BOOL CHandGrenade::CanHolster( void )
{
	return m_flStartThrow == 0.0f;
}

void CHandGrenade::DropGren( void )
{
	if( m_flStartThrow != 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		SendWeaponAnim( HANDGRENADE_THROW, UseDecrement() != 0 );

		if( m_bUnderhand )
			m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
		else
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		vec3_t vecStart = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
		vec3_t vecEnd = vecStart + gpGlobals->v_forward * 16.0f;

		TraceResult ptr;
		UTIL_TraceLine( vecStart, vecEnd, ignore_monsters, ENT( m_pPlayer->pev ), &ptr );

		if( ptr.flFraction < 1.0f )
			vecEnd = ptr.vecEndPos;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flStartThrow = 0.0f;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
	}
}

void CHandGrenade::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		SendWeaponAnim( HANDGRENADE_HOLSTER, UseDecrement() != 0 );
	}
	else
	{
		m_pPlayer->ClearHasWeapon( WEAPON_HANDGRENADE );

		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1f;
	}

	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM );
}

void CHandGrenade::PrimaryAttack( void )
{
	StartThrow( FALSE );
}

void CHandGrenade::SecondaryAttack( void )
{
	StartThrow( TRUE );
}

void CHandGrenade::StartThrow( BOOL bUnderhand )
{
	if( m_flStartThrow == 0.0f )
	{
		m_flReleaseThrow = 0.0f;
		m_flStartThrow = 1.0f;
		m_bUnderhand = bUnderhand;

		SendWeaponAnim( HANDGRENADE_PINPULL, UseDecrement() != 0 );

		m_flTimeWeaponIdle = gpGlobals->time + 0.1f;
	}
}

void CHandGrenade::WeaponIdle( void )
{
	if( m_flReleaseThrow == 0.0f )
		m_flReleaseThrow = gpGlobals->time;

	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
	{
		if( m_flTimeWeaponIdle > gpGlobals->time )
			return;

		if( m_flStartThrow == 0.0f && m_flReleaseThrow < 0.0f )
		{
			m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT( 10.0f, 15.0f );
			SendWeaponAnim( HANDGRENADE_IDLE, UseDecrement() != 0 );
			return;
		}

		if( m_flStartThrow == 1.0f && m_flTimeWeaponIdle < gpGlobals->time && m_flReleaseThrow != 0.0f )
		{
			// the pin was released, time to throw
			m_flStartThrow = gpGlobals->time + 0.5f;

			if( m_bUnderhand )
				m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
			else
				m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

			angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

			if( angThrow.x < 0 )
				angThrow.x = -10 + angThrow.x * ( ( 90 - 10 ) / 90.0 );
			else
				angThrow.x = -10 + angThrow.x * ( ( 90 + 10 ) / 90.0 );

			flVel = ( 90 - angThrow.x ) * 4;

			if( flVel > 600 )
				flVel = 600;

			UTIL_MakeVectors( angThrow );

			float flDot = DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );

			vecThrow = gpGlobals->v_forward * flDot + gpGlobals->v_forward * flVel;

			return;
		}

		if( m_flStartThrow != 0.0f && m_flStartThrow != 1.0f && gpGlobals->time > m_flStartThrow )
		{
			angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

			if( angThrow.x < 0 )
				angThrow.x = -10 + angThrow.x * ( ( 90 - 10 ) / 90.0 );
			else
				angThrow.x = -10 + angThrow.x * ( ( 90 + 10 ) / 90.0 );

			UTIL_MakeVectors( angThrow );

			vec3_t eyes = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;

			vecSrc = eyes + gpGlobals->v_forward * 16;

			if( m_bUnderhand )
			{
				vecSrc = m_pPlayer->pev->origin + gpGlobals->v_forward * 16;
				vecThrow = gpGlobals->v_forward * 300;
			}

			TraceResult tr;
			UTIL_TraceLine( eyes, vecSrc, ignore_monsters, ENT( m_pPlayer->pev ), &tr );

			if( tr.flFraction < 1.0f )
				vecSrc = tr.vecEndPos;

			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/grenthrow.wav", 0.8, 0.8 );

			SendWeaponAnim( HANDGRENADE_THROW, UseDecrement() != 0 );

			m_flStartThrow = 0.0f;

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;

			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

			return;
		}
	}

	if( m_flReleaseThrow > 0.0f && m_flStartThrow == 0.0f && gpGlobals->time > m_flTimeWeaponIdle )
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0.0f;

		if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		{
			RetireWeapon();
		}
		else
		{
			SendWeaponAnim( HANDGRENADE_DRAW, UseDecrement() != 0 );

			m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT( 10.0f, 15.0f );
			m_flReleaseThrow = -1.0f;
		}
	}
}

BOOL CHandGrenade::CanDeploy( void )
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}
