//
// bren.cpp
//
// implementation of CBREN class
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

LINK_ENTITY_TO_CLASS( weapon_bren, CBREN )

enum BREN_e 
{
    BREN_UP_IDLE = 0,
    BREN_UP_RELOAD,
    BREN_UP_DRAW,
    BREN_UP_SHOOT,
    BREN_UP_TO_DOWN,
    BREN_DOWN_IDLE,
    BREN_DOWN_RELOAD,
    BREN_DOWN_SHOOT,
    BREN_DOWN_TO_UP
};

void CBREN::Spawn( void )
{
    CBipodWeapon::Spawn( WEAPON_BREN );
}

void CBREN::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_BREN].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_BREN].wmodel );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/bar_shoot.wav" );

    m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/bren.sc" );
}

int CBREN::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_55mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_BREN].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_BREN].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_BREN;
    p->iBulletId = BULLET_PLAYER_BREN;
    p->flRecoil = 2.0f;
    p->iWeight = WpnInfo[WEAPON_BREN].misc_weight;
    return 1;
}

int CBREN::GetReloadAnim( void )
{
    return CBipodWeapon::IsDeployed() ? BREN_UP_RELOAD : BREN_DOWN_RELOAD;
}

int CBREN::GetDrawAnim( void )
{
    return BREN_UP_DRAW;
}

int CBREN::GetIdleAnim( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
        return CBipodWeapon::IsDeployed() ? BREN_DOWN_IDLE : BREN_UP_IDLE;
    
    return -1;
}

int CBREN::GetUpToDownAnim( void )
{
    return BREN_UP_TO_DOWN;
}

int CBREN::GetDownToUpAnim( void )
{
    return BREN_DOWN_TO_UP;
}

class CBRENAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
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

LINK_ENTITY_TO_CLASS( ammo_bren, CBRENAmmoClip )