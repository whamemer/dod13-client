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

LINK_ENTITY_TO_CLASS( weapon_handgrenade_ex, CHandGrenadeEx )

extern p_wpninfo_s P_WpnInfo[];

void CHandGrenadeEx::Spawn( void )
{
	Precache();

	m_iId = WEAPON_HANDGRENADEX;
	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_HANDGRENADEX].wmodel );

	m_iDefaultAmmo = 1;

	FallInit();
}

void CHandGrenadeEx::Precache( void )
{
	PRECACHE_SOUND( "weapons/grenthrow.wav" );
}

int CHandGrenadeEx::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_agrensex";
	p->iMaxAmmo1 = 1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 4;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_HANDGRENADEX;
	p->iWeight = 100;
	p->iFlags = ITEM_FLAG_GRENADE;

	return 1;
}

BOOL CHandGrenadeEx::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();

	m_flReleaseThrow = -1.0f;
	m_flStartThrow = 0.0f;

	return FALSE;
}

BOOL CHandGrenadeEx::CanHolster( void )
{
	return FALSE;
}

void CHandGrenadeEx::Holster( int skiplocal )
{
	/* Nothing. */
}

void CHandGrenadeEx::SetDmgTime( float time )
{
	m_flTimeToExplode = time;
}

BOOL CHandGrenadeEx::CanDeploy( void )
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

void CHandGrenadeEx::PrimaryAttack( void )
{
	StartThrow( FALSE );
}

void CHandGrenadeEx::SecondaryAttack( void )
{
	StartThrow( TRUE );
}

void CHandGrenadeEx::StartThrow( BOOL bUnderhand )
{
	if( gpGlobals->time > m_flTimeToExplode )
		DropGren();

	if( m_flStartThrow == 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
	{
		m_flReleaseThrow = 0.0f;
		m_flStartThrow = 1.0f;
		m_bUnderhand = bUnderhand;

		SendWeaponAnim( HANDGRENADE_EX_PINPULL, UseDecrement() != 0 );

		if( m_flTimeToExplode >= gpGlobals->time )
			m_flReleaseThrow = gpGlobals->time + 0.01f;
	}
}

void CHandGrenadeEx::DropGren( void )
{
	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
	{
		SendWeaponAnim( HANDGRENADE_EX_THROW, UseDecrement() != 0 );

		vec3_t vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if( vecAngles.x < 0 )
			vecAngles.x = -10 + vecAngles.x * ( ( 90 - 10 ) / 90.0 );
		else
			vecAngles.x = -10 + vecAngles.x * ( ( 90 + 10 ) / 90.0 );

		float flVel = ( 90 - vecAngles.x ) * 4;

		if( flVel > 600 )
			flVel = 600;

		UTIL_MakeVectors( vecAngles );

		vec3_t vecStart = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
		vec3_t vecEnd = vecStart + gpGlobals->v_forward * 16;
		vec3_t vecVelocity;
		float flFriction;

		float time = m_flTimeToExplode - gpGlobals->time;

		if( time < 0 )
			time = 0;

		if( m_bUnderhand )
		{
			vecEnd = m_pPlayer->pev->origin + gpGlobals->v_forward * 16;
			vecVelocity = gpGlobals->v_forward * 300;
			flFriction = 0.6f;
		}
		else
		{
			float flDot = DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );

			vecVelocity = gpGlobals->v_forward * flDot + gpGlobals->v_forward * flVel;
			flFriction = 0.8f;
		}

		TraceResult ptr;
		UTIL_TraceLine( vecStart, vecEnd, ignore_monsters, ENT( m_pPlayer->pev ), &ptr );

		if( ptr.flFraction < 1.0f )
			vecEnd = ptr.vecEndPos;

		if( pev->iuser1 )
			CGrenade::ShootPickup( m_pPlayer->pev, vecEnd, vecVelocity, time, G_HANDGRENADE, flFriction );
		else
			CGrenade::ShootPickup( m_pPlayer->pev, vecEnd, vecVelocity, time, G_MILLSGRENADE, flFriction );

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flStartThrow = 0.0f;
	}
}

void CHandGrenadeEx::WeaponIdle( void )
{
	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
	{
		RetireWeapon();

		m_pPlayer->ClearHasWeapon( WEAPON_HANDGRENADEX );

		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1f;

		m_flStartThrow = 0.0f;
		return;
	}

	if( gpGlobals->time > m_flTimeToExplode )
		DropGren();

	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && gpGlobals->time > m_flTimeWeaponIdle && m_flReleaseThrow == -1.0f )
	{
		m_flTimeWeaponIdle = gpGlobals->time + 1.0f;

		SendWeaponAnim( HANDGRENADE_EX_IDLE, UseDecrement() != 0 );
	}

	if( m_flReleaseThrow <= gpGlobals->time )
	{
		if( m_flStartThrow == 1.0f )
		{
			if( m_flReleaseThrow != -1.0f && m_flTimeToExplode >= gpGlobals->time )
			{
				if( m_bUnderhand )
					m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
				else
					m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

				m_flStartThrow = 2.0f;
				m_flReleaseThrow = gpGlobals->time + 0.3f;
			}
		}
		else if( m_flStartThrow == 2.0f && m_flReleaseThrow != -1.0f && m_flTimeToExplode >= gpGlobals->time )
		{
			DropGren();
		}
	}
}
