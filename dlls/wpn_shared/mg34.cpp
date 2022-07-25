#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum MG34_e
{
    MG34_DOWNIDLE,
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

LINK_ENTITY_TO_CLASS( weapon_mg34, CMG34 );
LINK_ENTITY_TO_CLASS( ammo_mg34, CMG34 );

void CMG34::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_mg34.mdl" );
    m_iDefaultAmmo = MG34_DEFAULT_GIVE;

    CBipodWeapon::Spawn( WEAPON_MG34 );
}

void CMG34::Precache( void )
{
    PRECACHE_MODEL( "models/v_mg34.mdl" );
    PRECACHE_MODEL( "models/w_mg34.mdl" );
    PRECACHE_MODEL( "models/p_mg34.mdl" );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/clampdown.wav" );
    PRECACHE_SOUND( "weapons/mgbolt.wav" );
    PRECACHE_SOUND( "weapons/mg34_shoot.wav" );

    m_usFireMG34 = PRECACHE_EVENT( 1, "events/weapons/mg34.sc" );
}

int CMG34::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_66mm";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _66MM_MAX_CARRY;
    p->iMaxAmmo2 = -1;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_MG34;
    p->iMaxClip = MG34_MAX_CLIP;
	p->iId = m_iId = WEAPON_MG34;
    p->iBulletId = BULLET_PLAYER_MG34;
    p->flRecoil = 15.0f;
	p->iWeight = MG34_WEIGHT;

	return 1;
}

BOOL CMG34::Deploy( void )
{
    return DefaultDeploy( "models/v_mg34.mdl", "models/p_mg34.mdl", BREN_UP_DRAW, "mg34" );
}

int CMG34::Classify( void )
{
    return CLASS_MACHINEGUN;
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
    if( CBipodWeapon::IsDeployed() )
        return m_iClip <= MG34_DOWNIDLE;
    else
        return ( m_iClip <= MG34_DOWNIDLE ) + MG34_UPIDLE;
}

int CMG34::GetUpToDownAnim( void )
{
    return ( m_iClip <= MG34_DOWNIDLE ) + MG34_UPTODOWN;
}

int CMG34::GetDownToUpAnim( void )
{
    return ( m_iClip <= MG34_DOWNIDLE ) + MG34_DOWNTOUP;
}

void CMG34AmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CMG34AmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_MG34CLIP_GIVE, "ammo_66mm", _66MM_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}