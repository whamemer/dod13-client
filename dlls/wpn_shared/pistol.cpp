#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

void CPistol::Spawn( int weapon_id )
{
    Precache();
	m_iId = weapon_id;

	FallInit();
}

void CPistol::PrimaryAttack( void )
{

}

void CPistol::Reload( void )
{
    DefaultReload( GetReloadAnim(), _MAX_CLIP, 1.5f );
}

void CPistol::WeaponIdle( void )
{

}