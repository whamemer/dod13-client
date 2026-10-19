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

LINK_ENTITY_TO_CLASS( weapon_stickgrenade, CStickGrenade )

extern p_wpninfo_s P_WpnInfo[];

void CStickGrenade::Spawn( void )
{
	Precache();

	m_iId = WEAPON_STICKGRENADE;
	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_STICKGRENADE].wmodel );

	m_iDefaultAmmo = 1;

	FallInit();
}

void CStickGrenade::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_STICKGRENADE].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_STICKGRENADE].wmodel );

	PRECACHE_SOUND( "weapons/grenthrow.wav" );
}

int CStickGrenade::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_ggrens";
	p->iMaxAmmo1 = 5;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 4;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_STICKGRENADE;
	p->iWeight = 1;
	p->iFlags = ITEM_FLAG_GRENADE;

	return 1;
}

BOOL CStickGrenade::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();

	m_flReleaseThrow = -1.0f;

	SendWeaponAnim( STICKGRENADE_DRAW, UseDecrement() != 0 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_STICKGRENADE].anim_drawtime;

	return DefaultDeploy( P_WpnInfo[WEAPON_STICKGRENADE].vmodel, P_WpnInfo[WEAPON_STICKGRENADE].pmodel, STICKGRENADE_DRAW, P_WpnInfo[WEAPON_STICKGRENADE].szAnimExt, P_WpnInfo[WEAPON_STICKGRENADE].szAnimReloadExt, 0 );
}

BOOL CStickGrenade::CanHolster( void )
{
	return m_flStartThrow == 0.0f;
}

void CStickGrenade::DropGren( void )
{
	if( m_flStartThrow != 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		SendWeaponAnim( STICKGRENADE_THROW, UseDecrement() != 0 );

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

		CGrenade::ShootPickup( m_pPlayer->pev, vecEnd, gpGlobals->v_forward, 5.0f, G_STICKGRENADE, 0.8f );

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flStartThrow = 0.0f;

		m_flNextPrimaryAttack = gpGlobals->time + 0.5f;
		m_flNextSecondaryAttack = gpGlobals->time + 0.5f;
		m_flTimeWeaponIdle = gpGlobals->time + 0.5f;
	}
}

void CStickGrenade::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		SendWeaponAnim( STICKGRENADE_HOLSTER, UseDecrement() != 0 );
	}
	else
	{
		m_pPlayer->ClearHasWeapon( WEAPON_STICKGRENADE );

		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1f;
	}

	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM );
}

void CStickGrenade::PrimaryAttack( void )
{
	StartThrow( FALSE );
}

void CStickGrenade::SecondaryAttack( void )
{
	StartThrow( TRUE );
}

void CStickGrenade::StartThrow( BOOL underhand )
{
	if( m_flStartThrow == 0.0f )
	{
		m_flReleaseThrow = 0.0f;
		m_flStartThrow = 1.0f;
		m_bUnderhand = underhand;

		SendWeaponAnim( STICKGRENADE_PINPULL, UseDecrement() != 0 );

		m_flTimeWeaponIdle = gpGlobals->time + 0.1f;
	}
}

void CStickGrenade::WeaponIdle( void )
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
			SendWeaponAnim( STICKGRENADE_IDLE, UseDecrement() != 0 );
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
			float flFriction = 0.8f;

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
				flFriction = 0.6f;

				vecSrc = m_pPlayer->pev->origin + gpGlobals->v_forward * 16;
				vecThrow = gpGlobals->v_forward * 300;
			}

			TraceResult tr;
			UTIL_TraceLine( eyes, vecSrc, ignore_monsters, ENT( m_pPlayer->pev ), &tr );

			if( tr.flFraction < 1.0f )
				vecSrc = tr.vecEndPos;

			CGrenade::ShootPickup( m_pPlayer->pev, vecSrc, vecThrow, 5.0f, G_STICKGRENADE, flFriction );

			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/grenthrow.wav", 0.8, 0.8 );

			SendWeaponAnim( STICKGRENADE_THROW, UseDecrement() != 0 );

			m_flStartThrow = 0.0f;

			m_flNextPrimaryAttack = gpGlobals->time + 1.0f;
			m_flTimeWeaponIdle = gpGlobals->time + 1.0f;

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
			SendWeaponAnim( STICKGRENADE_DRAW, UseDecrement() != 0 );

			m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT( 10.0f, 15.0f );
			m_flReleaseThrow = -1.0f;
		}
	}
}

BOOL CStickGrenade::CanDeploy( void )
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}
