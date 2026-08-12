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

#define KNIFE_WALLHIT_VOLUME 512

extern struct p_wpninfo_s *WpnInfo;

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
    SET_MODEL( ENT( pev ), WpnInfo[m_iId].wmodel );
	FallInit();
    m_iDefaultAmmo = WpnInfo[m_iId].ammo_default;
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

    SendWeaponAnim( KNIFE_DRAW, KNIFE_SLASH1 );

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75f;
    int iAnim = GetDrawAnim();

    return DefaultDeploy( WpnInfo[m_iId].vmodel, WpnInfo[m_iId].pmodel, iAnim, WpnInfo[m_iId].szAnimExt, WpnInfo[m_iId].szAnimReloadExt, 0 );
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

int CMeleeWeapon::Swing( int fFirst )
{
	TraceResult tr;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 48.0f;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );
	UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );
	
	if( tr.flFraction < 1.0f )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		if( !pHit || pHit->IsBSPModel() )
			FindHullIntersection( vecSrc, tr, VEC_HULL_MIN, VEC_HULL_MAX, m_pPlayer->edict() );
		vecEnd = tr.vecEndPos;
	}

	if( tr.flFraction >= 1.0f )
	{
		PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_iFireEvent, 
		0.0f, g_vecZero, g_vecZero, 0, 0, GetSlashAnim(),
		0, 0, 0 );

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4f;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;

		SetThink( &CMeleeWeapon::Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + 0.2;
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME;
		return TRUE;
	}

	if( !fFirst )
		return FALSE;
	
	PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_iFireEvent, 
	0.0f, g_vecZero, g_vecZero, 0, 0, GetSlashAnim(),
	0, 1, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	return FALSE;
}

// TODO: WHAMER
int CMeleeWeapon::Stab( int fFirst )
{
	//Full Copy CMeleeWeapon::Swing() on CLIENT part, idk why;
}


void CMeleeWeapon::WeaponIdle( void )
{
	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
		if( m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() )
			if( m_flNextSecondaryAttack <= UTIL_WeaponTimeBase() )
			{
				int iAnim = GetIdleAnim();

				if( iAnim > 0 )
					SendWeaponAnim( iAnim );
				
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
			}
}