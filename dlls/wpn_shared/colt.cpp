//
// colt.cpp
//
// implementation of CCOLT class
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

LINK_ENTITY_TO_CLASS( weapon_colt, CCOLT )

enum COLT_e
{
    COLT_IDLE = 0,
    COLT_SHOOT,
    COLT_SHOOT2,
    COLT_RELOAD_EMPTY,
    COLT_RELOAD,
    COLT_DRAW,
    COLT_SHOOT_EMPTY,
    COLT_IDLE_EMPTY
};

void CCOLT::Spawn( void )
{
    CPistol::Spawn( WEAPON_COLT );
}

void CCOLT::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_COLT].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_COLT].wmodel );

    PRECACHE_SOUND( "weapons/colt_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/colt_reload_clipin.wav" );
    PRECACHE_SOUND( "weapons/colt_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/colt_draw_pullslide.wav" );

    m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/colt.sc" );
}

int CCOLT::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_12mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_COLT].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_COLT].ammo_maxclip;
    p->iSlot = 1;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_PISTOL;
    p->iId = m_iId = WEAPON_COLT;
    p->iBulletId = BULLET_PLAYER_COLT;
    p->flSpread = 0.035f;
    p->iWeight = WpnInfo[WEAPON_COLT].misc_weight;
    return 1;
}

int CCOLT::GetReloadAnim( void )
{
    return m_iClip == COLT_IDLE ? COLT_RELOAD_EMPTY : COLT_RELOAD;
}

int CCOLT::GetDrawAnim( void )
{
    return COLT_DRAW;
}

int CCOLT::GetIdleAnim( void )
{
    return m_iClip == COLT_IDLE ? COLT_IDLE_EMPTY : COLT_IDLE;
}

class CColtAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_COLT].ammo_maxclip, "ammo_12mm", WpnInfo[WEAPON_COLT].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_colt, CColtAmmoClip )