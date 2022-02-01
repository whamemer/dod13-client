#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "gamerules.h"
#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_pistol, CPistol );

void CPistol::Spawn( int weapon_id )
{
	m_iId = weapon_id;
	SET_MODEL( ENT( pev ), "models/w_.mdl" );
	m_iDefaultAmmo = _DEFAULT_GIVE;

	FallInit();
    Precache();
}

void CPistol::PrimaryAttack( void )
{
    Vector p_vecDirShooting, p_vecSrc;

    if( PlayerIsWaterSniping() ) // TODO
    {
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
    }
    else if( !m_fInAttack )
    {
        if( m_iClip <= 0 )
        {
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
            m_fInAttack = 1;
        }
        else
        {
            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

            ShouldFadeOnDeath(  );

        }
    }
}

BOOL CPistol::Deploy( void )
{

}

void CPistol::Reload( void )
{

}

void CPistol::WeaponIdle( void )
{

}

int CPistol::GetIdleAnim( void )
{

}

int CPistol::GetDrawAnim( void )
{

}

int CPistol::GetReloadAnim( void )
{

}

LINK_ENTITY_TO_CLASS( weapon_colt, CCOLT );

void CCOLT::Spawn( void )
{

}

void CCOLT::Precache( void )
{

}

int CCOLT::GetItemInfo( ItemInfo *p )
{

}

int CCOLT::GetSlashAnim( int m_iSwing )
{

}

int CCOLT::GetDrawAnim( void )
{

}

int CCOLT::GetIdleAnim( void )
{

}

void CColtAmmoClip::Spawn( void )
{

}

BOOL CColtAmmoClip::AddAmmo( CBaseEntity *pOther )
{

}

LINK_ENTITY_TO_CLASS( weapon_colt, CLuger );

void CLuger::Spawn( void )
{

}

void CLuger::Precache( void )
{

}

int CLuger::GetItemInfo( ItemInfo *p )
{

}

int CLuger::GetSlashAnim( int m_iSwing )
{

}

int CLuger::GetDrawAnim( void )
{

}

int CLuger::GetIdleAnim( void )
{

}

void CLugerAmmoClip::Spawn( void )
{

}

BOOL CLugerAmmoClip::AddAmmo( CBaseEntity *pOther )
{

}

LINK_ENTITY_TO_CLASS( weapon_colt, CWEBLEY );

void CWEBLEY::Spawn( void )
{

}

void CWEBLEY::Precache( void )
{

}

int CWEBLEY::GetItemInfo( ItemInfo *p )
{

}

int CWEBLEY::GetSlashAnim( int m_iSwing )
{

}

int CWEBLEY::GetDrawAnim( void )
{

}

int CWEBLEY::GetIdleAnim( void )
{

}

void CWebleyAmmoClip::Spawn( void )
{

}

BOOL CWebleyAmmoClip::AddAmmo( CBaseEntity *pOther )
{

}