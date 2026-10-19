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

void CMeleeWeapon::Spawn( int weapon_id )
{
	m_iId = weapon_id;
	SET_MODEL( ENT( pev ), P_WpnInfo[weapon_id].wmodel );

	m_iDefaultAmmo = P_WpnInfo[m_iId].ammo_default;

	FallInit();

	Precache();
}

void CMeleeWeapon::Precache( void )
{
	PRECACHE_SOUND( "weapons/knife_slash1.wav" );
	PRECACHE_SOUND( "weapons/knife_slash2.wav" );
	PRECACHE_SOUND( "weapons/knife_hitwall1.wav" );
	PRECACHE_SOUND( "weapons/knife_hit1.wav" );
	PRECACHE_SOUND( "weapons/knife_hit2.wav" );
	PRECACHE_SOUND( "weapons/knife_hit3.wav" );
	PRECACHE_SOUND( "weapons/knife_hit4.wav" );
	PRECACHE_SOUND( "weapons/knifeswing.wav" );
	PRECACHE_SOUND( "weapons/knifeswing2.wav" );

	m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/melee.sc" );
}

int CMeleeWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
	return FALSE;
}

BOOL CMeleeWeapon::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_pPlayer->m_iFOV = ZoomOut();

	SendWeaponAnim( 3, 1 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75f;

	return DefaultDeploy( P_WpnInfo[m_iId].vmodel, P_WpnInfo[m_iId].pmodel, GetDrawAnim(), P_WpnInfo[m_iId].szAnimExt, P_WpnInfo[m_iId].szAnimReloadExt, 0 );
}

void CMeleeWeapon::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int i, j, k;
	float distance;
	float *minmaxs[2] = { mins, maxs };
	TraceResult tmpTrace;
	vec3_t vecHullEnd = tr.vecEndPos;
	vec3_t vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ( ( vecHullEnd - vecSrc ) * 2 );
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );

	if( tmpTrace.flFraction < 1.0f )
	{
		tr = tmpTrace;
		return;
	}

	for( i = 0; i < 2; i++ )
	{
		for( j = 0; j < 2; j++ )
		{
			for( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );

				if( tmpTrace.flFraction < 1.0f )
				{
					float thisDistance = ( tmpTrace.vecEndPos - vecSrc ).Length();

					if( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

void CMeleeWeapon::PrimaryAttack( void )
{
	Swing( 1 );
}

void CMeleeWeapon::Smack( void )
{
	DecalGunshot( &m_trHit, 5 );
}

void CMeleeWeapon::SwingAgain( void )
{
	Swing( 0 );
}

int CMeleeWeapon::Swing( int fFirst )
{
	int fDidHit = FALSE;
	TraceResult tr;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );

	vec3_t vecSrc = m_pPlayer->GetGunPosition();
	vec3_t vecEnd = vecSrc + gpGlobals->v_forward * 48.0f;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	if( tr.flFraction >= 1.0f )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );

		if( tr.flFraction < 1.0f )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

			if( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_HULL_MIN, VEC_HULL_MAX, ENT( m_pPlayer->pev ) );

			// This is the point on the actual surface (the hull could have hit space)
			vecEnd = tr.vecEndPos;
		}
	}

	if( tr.flFraction >= 1.0f )
	{
		if( fFirst )
		{
			// miss
			PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_iFireEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, GetSlashAnim( m_iSwing ), 0, 1, 0 );

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35f;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		// hit
		fDidHit = TRUE;

		PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_iFireEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, GetSlashAnim( m_iSwing ), 0, 0, 0 );

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4f;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;

		m_trHit = tr;

		SetThink( &CMeleeWeapon::Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + 0.2f;

		m_pPlayer->m_iWeaponVolume = 512;
	}

	return fDidHit;
}

int CMeleeWeapon::Stab( int fFirst )
{
	int fDidHit = FALSE;
	TraceResult tr;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );

	vec3_t vecSrc = m_pPlayer->GetGunPosition();
	vec3_t vecEnd = vecSrc + gpGlobals->v_forward * 32.0f;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	if( tr.flFraction >= 1.0f )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );

		if( tr.flFraction < 1.0f )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

			if( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_HULL_MIN, VEC_HULL_MAX, ENT( m_pPlayer->pev ) );

			// This is the point on the actual surface (the hull could have hit space)
			vecEnd = tr.vecEndPos;
		}
	}

	if( tr.flFraction >= 1.0f )
	{
		if( fFirst )
		{
			// miss
			PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_iFireEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, GetSlashAnim( m_iSwing ), 0, 1, 0 );

			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		// hit
		fDidHit = TRUE;

		PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_iFireEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, GetSlashAnim( m_iSwing ), 0, 0, 0 );

		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1f;

		m_trHit = tr;

		SetThink( &CMeleeWeapon::Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + 0.2f;

		m_pPlayer->m_iWeaponVolume = 512;
	}

	return fDidHit;
}

void CMeleeWeapon::WeaponIdle( void )
{
	if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if( m_flNextPrimaryAttack > UTIL_WeaponTimeBase() )
		return;

	if( m_flNextSecondaryAttack > UTIL_WeaponTimeBase() )
		return;

	int iAnim = GetIdleAnim();

	if( iAnim >= 0 )
		SendWeaponAnim( iAnim, 1 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
}
