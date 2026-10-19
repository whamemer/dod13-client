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

LINK_ENTITY_TO_CLASS( weapon_stickgrenade_ex, CStickGrenadeEx )

extern p_wpninfo_s P_WpnInfo[];

void CStickGrenadeEx::Spawn( void )
{
	Precache();

	m_iId = WEAPON_STICKGRENADEX;
	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_STICKGRENADEX].wmodel );

	m_iDefaultAmmo = 1;

	FallInit();
}

void CStickGrenadeEx::Precache( void )
{
	PRECACHE_SOUND( "weapons/grenthrow.wav" );
}

int CStickGrenadeEx::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_ggrensex";
	p->iMaxAmmo1 = 1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 4;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_STICKGRENADEX;
	p->iWeight = 100;
	p->iFlags = ITEM_FLAG_GRENADE;

	return 1;
}

BOOL CStickGrenadeEx::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();

	m_flReleaseThrow = -1.0f;

	SendWeaponAnim( STICKGRENADE_DRAW, UseDecrement() != 0 );

	return DefaultDeploy( P_WpnInfo[WEAPON_STICKGRENADEX].vmodel, P_WpnInfo[WEAPON_STICKGRENADEX].pmodel, STICKGRENADE_EX_DRAW, P_WpnInfo[WEAPON_STICKGRENADEX].szAnimExt, P_WpnInfo[WEAPON_STICKGRENADEX].szAnimReloadExt, 0 );
}

BOOL CStickGrenadeEx::CanHolster( void )
{
	return FALSE;
}

void CStickGrenadeEx::Holster( int skiplocal )
{
	/* Nothing. */
}

void CStickGrenadeEx::SetDmgTime( float time )
{
	m_flTimeToExplode = time;
}

BOOL CStickGrenadeEx::CanDeploy( void )
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

void CStickGrenadeEx::PrimaryAttack( void )
{
	StartThrow( FALSE );
}

void CStickGrenadeEx::SecondaryAttack( void )
{
	StartThrow( TRUE );
}

void CStickGrenadeEx::StartThrow( BOOL bUnderhand )
{
	if( gpGlobals->time > m_flTimeToExplode )
		DropGren();

	if( m_flStartThrow == 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
	{
		m_flReleaseThrow = 0.0f;
		m_flStartThrow = 1.0f;
		m_bUnderhand = bUnderhand;

		SendWeaponAnim( STICKGRENADE_EX_PINPULL, UseDecrement() != 0 );

		if( m_flTimeToExplode >= gpGlobals->time )
			m_flReleaseThrow = gpGlobals->time + 0.01f;
	}
}

void CStickGrenadeEx::DropGren( void )
{
	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
	{
		SendWeaponAnim( STICKGRENADE_EX_THROW, UseDecrement() != 0 );

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

		CGrenade::ShootPickup( m_pPlayer->pev, vecEnd, vecVelocity, time, G_STICKGRENADE, flFriction );

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flStartThrow = 0.0f;
	}
}

void CStickGrenadeEx::WeaponIdle( void )
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

		SendWeaponAnim( STICKGRENADE_EX_IDLE, UseDecrement() != 0 );
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
