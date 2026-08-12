//
// mg34.cpp
//
// implementation of CMG34 class
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

LINK_ENTITY_TO_CLASS( weapon_mg34, CMG34 )

enum MG34_e
{
    MG34_DOWNIDLE = 0,
    MG34_DOWNIDLEEMPTY,
    MG34_DOWNTOUP,
    MG34_DOWNTOUPEMPTY,
    MG34_DOWNSHOOT,
    MG34_DOWNSHOOTEMPTY,
    MG34_UPIDLE,
    MG34_UPIDLEEMPTY,
    MG34_UPTODOWN,
    MG34_UPTODOWNEMPTY,
    MG34_UPSHOOT,
    MG34_UPSHOOTEMPTY,
    MG34_RELOAD
};

void CMG34::Spawn( void )
{
    CBipodWeapon::Spawn( WEAPON_MG34 );
}

void CMG34::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_MG34].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_MG34].wmodel );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/clampdown.wav" );
    PRECACHE_SOUND( "weapons/mgbolt.wav" );
    PRECACHE_SOUND( "weapons/mg34_shoot.wav" );

    m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/mg34.sc" );
}

int CMG34::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_66mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_MG34].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_MG34].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_66MM_BIPOD;
    p->iId = m_iId = WEAPON_MG42;
    p->iBulletId = BULLET_PLAYER_MG34;
    p->flRecoil = 15.0f;
    p->iWeight = WpnInfo[WEAPON_MG34].misc_weight;
    return 1;
}

int CMG34::GetReloadAnim( void )
{
    return MG34_RELOAD;
}

int CMG34::GetDrawAnim( void )
{
    return GetDownToUpAnim();
}

int CMG34::GetIdleAnim( void )
{
    if( IsDeployed() )
        return MG34_DOWNIDLE;
    else
        return MG34_UPIDLE;
}

int CMG34::GetUpToDownAnim( void )
{
    return MG34_UPTODOWN;
}

int CMG34::GetDownToUpAnim( void )
{
    return MG34_DOWNTOUP;
}

class CMG34AmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_MG34].ammo_maxclip, "ammo_66_ger", WpnInfo[WEAPON_MG34].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_mg34, CMG34AmmoClip )