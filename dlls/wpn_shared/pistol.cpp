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

void CPistol::Spawn( void )
{
    FallInit();
}

void CPistol::PrimaryAttack( void )
{
    Vector p_vecDirShooting, p_vecSrc;

    if( hz ) // TODO
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
        }
    }
}