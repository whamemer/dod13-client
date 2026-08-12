//
// mg42.cpp
//
// implementation of CMG42 class
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

LINK_ENTITY_TO_CLASS( weapon_mg42, CMG42 )

enum MG42_e
{
    MG42_UPIDLE = 0,
    MG42_UPIDLE8,
    MG42_UPIDLE7,
    MG42_UPIDLE6,
    MG42_UPIDLE5,
    MG42_UPIDLE4,
    MG42_UPIDLE3,
    MG42_UPIDLE2,
    MG42_UPIDLE1,
    MG42_UPIDLEEMPTY,
    MG42_DOWNIDLE,
    MG42_DOWNIDLE8,
    MG42_DOWNIDLE7,
    MG42_DOWNIDLE6,
    MG42_DOWNIDLE5,
    MG42_DOWNIDLE4,
    MG42_DOWNIDLE3,
    MG42_DOWNIDLE2,
    MG42_DOWNIDLE1,
    MG42_DOWNIDLEEMPTY,
    MG42_DOWNTOUP,
    MG42_DOWNTOUP8,
    MG42_DOWNTOUP7,
    MG42_DOWNTOUP6,
    MG42_DOWNTOUP5,
    MG42_DOWNTOUP4,
    MG42_DOWNTOUP3,
    MG42_DOWNTOUP2,
    MG42_DOWNTOUP1,
    MG42_DOWNTOUPEMPTY,
    MG42_UPTODOWN,
    MG42_UPTODOWN8,
    MG42_UPTODOWN7,
    MG42_UPTODOWN6,
    MG42_UPTODOWN5,
    MG42_UPTODOWN4,
    MG42_UPTODOWN3,
    MG42_UPTODOWN2,
    MG42_UPTODOWN1,
    MG42_UPTODOWNEMPTY,
    MG42_UPSHOOT,
    MG42_UPSHOOT8,
    MG42_UPSHOOT7,
    MG42_UPSHOOT6,
    MG42_UPSHOOT5,
    MG42_UPSHOOT4,
    MG42_UPSHOOT3,
    MG42_UPSHOOT2,
    MG42_UPSHOOT1,
    MG42_DOWNSHOOT,
    MG42_DOWNSHOOT8,
    MG42_DOWNSHOOT7,
    MG42_DOWNSHOOT6,
    MG42_DOWNSHOOT5,
    MG42_DOWNSHOOT4,
    MG42_DOWNSHOOT3,
    MG42_DOWNSHOOT2,
    MG42_DOWNSHOOT1,
    MG42_RELOAD
};

void CMG42::Spawn( void )
{
    CBipodWeapon::Spawn( WEAPON_MG42 );
}

void CMG42::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_MG42].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_MG42].wmodel );

    PRECACHE_SOUND( "weapons/weaponempty.wav" );
    PRECACHE_SOUND( "weapons/chainpull.wav" );
    PRECACHE_SOUND( "weapons/bulletchain.wav" );
    PRECACHE_SOUND( "weapons/clampdown.wav" );
    PRECACHE_SOUND( "weapons/mgbolt.wav" );
    PRECACHE_SOUND( "weapons/mg42_shoot.wav" );
    PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );
    PRECACHE_SOUND( "weapons/mgoverheat.wav" );

    m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/mg42.sc" );
    CBipodWeapon::Precache();
}

int CMG42::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_66mm_ger";
    p->iMaxAmmo1 = WpnInfo[WEAPON_MG42].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_MG42].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_66MM_GER_BIPOD;
    p->iId = m_iId = WEAPON_MG42;
    p->iBulletId = BULLET_PLAYER_MG42;
    p->flRecoil = 15.0f;
    p->iWeight = WpnInfo[WEAPON_MG42].misc_weight;
    return 1;
}

int CMG42::GetReloadAnim( void )
{
    return MG42_RELOAD;
}

int CMG42::GetDrawAnim( void )
{
    return GetDownToUpAnim();
}

int CMG42::GetIdleAnim( void )
{
    return -1;
}

int CMG42::GetUpToDownAnim( void )
{
    return m_iClip < MG42_UPIDLE1 ? MG42_UPTODOWN : MG42_UPTODOWNEMPTY;
}

int CMG42::GetDownToUpAnim( void )
{
    return m_iClip < MG42_UPIDLE1 ? MG42_DOWNTOUP : MG42_DOWNTOUPEMPTY;
}

class CMG42AmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_MG42].ammo_maxclip, "ammo_66mm_ger", WpnInfo[WEAPON_MG42].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_mg42, CMG42AmmoClip )