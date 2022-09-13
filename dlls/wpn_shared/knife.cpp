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
//
// knife.cpp
//
// implementation of CMeleeWeapon class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"
#include "skill.h"

#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

extern struct p_wpninfo_s P_WpnInfo[];

enum KNIFE_e 
{
    KNIFE_IDLE = 0,
    KNIFE_SLASH1,
    KNIFE_SLASH2,
    KNIFE_DRAW
};

void CMeleeWeapon::Spawn( int weapon_id )
{
    m_iId = weapon_id;
    SET_MODEL( ENT( pev ), P_WpnInfo[m_iId].wmodel );

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
    int result = FALSE;

	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
        result = TRUE;
        if( pPlayer->pev->team == 2 )
        {
            if( pPlayer->IsParatrooper() && m_iId == WEAPON_GERKNIFE )
                    SET_MODEL( ENT( pev ), P_WpnInfo[35].wmodel );
            else
                    SET_MODEL( ENT( pev ), P_WpnInfo[m_iId].wmodel );
        }
	}
	return result;
}

BOOL CMeleeWeapon::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

    SendWeaponAnim( KNIFE_DRAW, KNIFE_SLASH1 );

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75f;
    int iAnim = GetDrawAnim();

    return DefaultDeploy( P_WpnInfo[m_iId].vmodel, P_WpnInfo[m_iId].pmodel, iAnim, P_WpnInfo[m_iId].szAnimExt, P_WpnInfo[m_iId].szAnimReloadExt, 0 );
}

void CMeleeWeapon::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	TraceResult tmpTrace;
	float distance = 1000000.0f;
	float *minmaxs[2] = { mins, maxs };
	Vector vecHullEnd = tr.vecEndPos;

	vecHullEnd = vecSrc + (( vecHullEnd - vecSrc ) * 2 );
	TRACE_LINE( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );

	if( tmpTrace.flFraction < 1 )
	{
		tr = tmpTrace;
		return;
	}

	for( int i = 0; i < 2; i++ )
	{
		for( int j = 0; j < 2; j++)
		{
			for( int k = 0; k < 2; k++ )
			{
				Vector vecEnd;
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				TRACE_LINE( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );

				if( tmpTrace.flFraction < 1 )
				{
					float flThisDistance = ( tmpTrace.vecEndPos - vecSrc ).Length();

					if( flThisDistance < distance )
					{
						tr = tmpTrace;
						distance = flThisDistance;
					}
				}
			}
		}
	}
}

void CMeleeWeapon::PrimaryAttack( void )
{
    Swing( TRUE );
}

void CMeleeWeapon::Smack( void )
{
    DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
}

void CMeleeWeapon::SwingAgain( void )
{
    Swing( FALSE );
}

int CMeleeWeapon::Swing(int fFirst)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

	TraceResult tr;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	if( tr.flFraction >= 1 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );

		if( tr.flFraction < 1 )
		{
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

			if( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT( m_pPlayer->pev ) );

			vecEnd = tr.vecEndPos;
		}
	}

	if( tr.flFraction >= 1 )
	{
		if( fFirst )
		{
			switch ( ( m_iSwing++ ) % 2 )
			{
				case 0:
					SendWeaponAnim( KNIFE_SLASH1 );
					break;
				case 1:
					SendWeaponAnim( KNIFE_SLASH2 );
					break;
			}

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4f;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;

			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
			ClearMultiDamage();
		}

		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		if( pEntity )
		{
			pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgCrowbar, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET );
		}

		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

		float flVol = 1;
		int fHitWorld = TRUE;

		if( pEntity )
		{
			if( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				switch( RANDOM_LONG( 0, 3 ) )
				{
					case 0:
						EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hit1.wav", VOL_NORM, ATTN_NORM );
						break;
					case 1:
						EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM );
						break;
					case 2:
						EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hit3.wav", VOL_NORM, ATTN_NORM );
						break;
					case 3:
						EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM );
						break;
				}

				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if( !pEntity->IsAlive() )
					return TRUE;

				flVol = 0.1;
				fHitWorld = FALSE;
			}
		}

		if( fHitWorld )
		{
			TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + ( vecEnd - vecSrc ) * 2, BULLET_PLAYER_CROWBAR );
			EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
		}

		m_trHit = tr;

		SetThink( &CMeleeWeapon::Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + 0.2f;
		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	return fDidHit;
}

int CMeleeWeapon::Stab( int fFirst )
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	TraceResult tr;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	if( tr.flFraction >= 1 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );

		if( tr.flFraction < 1 )
		{
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

			if( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT( m_pPlayer->pev ) );

			vecEnd = tr.vecEndPos;
		}
	}

	if( tr.flFraction >= 1 )
	{
		if( fFirst )
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;

			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		fDidHit = TRUE;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1f;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;

		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		ClearMultiDamage();
		if( pEntity )
			pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgCrowbar, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET );
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		float flVol = 1;
		int fHitWorld = TRUE;

		if( fHitWorld )
		{
			TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + ( vecEnd - vecSrc ) * 2, BULLET_PLAYER_CROWBAR );
			EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
		}

		m_trHit = tr;
		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

		SetThink( &CMeleeWeapon::Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + 0.2f;
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	return fDidHit;
}

void CMeleeWeapon::WeaponIdle( void )
{
	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
		if( m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() )
			if( m_flNextSecondaryAttack <= UTIL_WeaponTimeBase() )
			{
				int iAnim = GetIdleAnim();

				if( iAnim >= 0 )
					SendWeaponAnim( iAnim );
				
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
			}
}