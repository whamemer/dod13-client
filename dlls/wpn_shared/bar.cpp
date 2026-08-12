//
// bar.cpp
//
// implementation of CBAR class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"

extern struct p_wpninfo_s *WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_bar, CBAR )

enum BAR_e 
{
    BAR_UP_IDLE = 0,
    BAR_UP_RELOAD,
    BAR_UP_DRAW,
    BAR_UP_SHOOT,
    BAR_UP_TO_DOWN,
    BAR_DOWN_IDLE,
    BAR_DOWN_RELOAD,
    BAR_DOWN_SHOOT,
    BAR_DOWN_TO_UP
};

void CBAR::Spawn( void )
{
    CBipodWeapon::Spawn( WEAPON_BAR );
}

void CBAR::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_BAR].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_BAR].wmodel );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/bar_shoot.wav" );

    m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/bar.sc" );
}

int CBAR::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_55mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_BAR].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_BAR].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_BAR;
    p->iBulletId = BULLET_PLAYER_BAR;
    p->flRecoil = 2.0f;
    p->iWeight = WpnInfo[WEAPON_BAR].misc_weight;
    return 1;
}

int CBAR::GetReloadAnim( void )
{
    return !CBipodWeapon::IsDeployed() ? BAR_UP_RELOAD : BAR_DOWN_RELOAD;
}

int CBAR::GetDrawAnim( void )
{
    return BAR_UP_DRAW;
}

int CBAR::GetIdleAnim( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
        return CBipodWeapon::IsDeployed() ? BAR_DOWN_IDLE : BAR_UP_IDLE;
    
    return -1;
}

int CBAR::GetUpToDownAnim( void )
{
    return BAR_UP_TO_DOWN;
}

int CBAR::GetDownToUpAnim( void )
{
    return BAR_DOWN_TO_UP;
}

class CBARAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_BAR].ammo_maxclip, "ammo_55mm", WpnInfo[WEAPON_BAR].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_bar, CBARAmmoClip )