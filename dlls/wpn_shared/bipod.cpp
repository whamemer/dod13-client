#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

void CBipodWeapon::Spawn( int weapon_id )
{
    Precache();
    m_iId = weapon_id;

    FallInit();
}

void CBipodWeapon::Precache( void )
{
    m_iOverheatEvent = PRECACHE_EVENT( 1, "events/effects/overheat.sc" );
}

int CBipodWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
    if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CBipodWeapon::CanHolster( void )
{
	// TODO: WHAMER
}

void CBipodWeapon::Holster( int skiplocal )
{
	CBasePlayerWeapon::Holster( skiplocal );
}

void CBipodWeapon::PrimaryAttack( void )
{
	if( m_pPlayer->pev->waterlevel == 3 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
		return;
	}

	if( m_iClip <= 0 )
	{
		if( !m_fFireOnEmpty )
			Reload();
		else
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

	// TODO: WHAMER
}

void CBipodWeapon::CoolThink( void )
{
	if( m_flWeaponHeat - 0.6f <= 0.0f )
	{
		m_flWeaponHeat = 0.0f;
		// TODO: WHAMER
	}
	else
	{
		pev->nextthink = gpGlobals->time + 0.1f;
	}
}

void CBipodWeapon::SecondaryAttack( void )
{
	// TODO: WHAMER
}

void CBipodWeapon::ForceUndeploy( void )
{

}

void CBipodWeapon::Reload( void )
{

}

void CBipodWeapon::WeaponIdle( void )
{

}

bool CBipodWeapon::IsDeployed( void )
{
	
}

float CBipodWeapon::GetBipodSpread( void )
{

}