//
// 30cal.cpp
//
// implementation of C30CAL class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "dod_shared.h"

extern p_wpninfo_s *WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_30cal, C30CAL )

void C30CAL::Spawn( void )
{
    CBipodWeapon::Spawn( WEAPON_CAL30 );
}

void C30CAL::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_CAL30].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_CAL30].wmodel );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/chainpull.wav" );
    PRECACHE_SOUND( "weapons/bulletchain.wav" );
    PRECACHE_SOUND( "weapons/clampdown.wav" );
    PRECACHE_SOUND( "weapons/mgbolt.wav" );
    PRECACHE_SOUND( "weapons/30cal_shoot.wav" );
    PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );

    m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/30cal.sc" );
}

int C30CAL::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_66mm_us";
    p->iMaxAmmo1 = WpnInfo[WEAPON_CAL30].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_CAL30].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_66MM_BIPOD;
    p->iId = m_iId = WEAPON_CAL30;
    p->iBulletId = BULLET_PLAYER_30CAL;
    p->flRecoil = 15.0f;
    p->iWeight = WpnInfo[WEAPON_CAL30].misc_weight;
    return 1;
}

int C30CAL::GetReloadAnim( void )
{
    return CAL30_RELOAD;
}

int C30CAL::GetDrawAnim( void )
{
    return CAL30_DOWNTOUP;
}

int C30CAL::GetIdleAnim( void )
{
    return -1;
}

int C30CAL::GetUpToDownAnim( void )
{
    return m_iClip < CAL30_UPIDLE1 ? CAL30_UPTODOWN : CAL30_UPTODOWNEMPTY;
}

int C30CAL::GetDownToUpAnim( void )
{
    return m_iClip < CAL30_UPIDLE1 ? CAL30_DOWNTOUP : CAL30_DOWNTOUPEMPTY;
}

class C30CALAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_CAL30].ammo_maxclip, "ammo_66mm_us", WpnInfo[WEAPON_CAL30].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_30cal, C30CALAmmoClip )