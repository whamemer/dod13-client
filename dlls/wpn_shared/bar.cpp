#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum BAR_e
{
    BAR_UP_IDLE = 0,
    BAR_UP_RELOAD = 1,
    BAR_UP_DRAW = 2,
    BAR_UP_SHOOT = 3,
    BAR_UP_TO_DOWN = 4,
    BAR_DOWN_IDLE = 5,
    BAR_DOWN_RELOAD = 6,
    BAR_DOWN_SHOOT = 7,
    BAR_DOWN_TO_UP = 8
};

LINK_ENTITY_TO_CLASS( weapon_bar, CBAR );
LINK_ENTITY_TO_CLASS( ammo_bar, CBAR );

void CMG34::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_bar.mdl" );
    m_iDefaultAmmo = BAR_DEFAULT_GIVE;

    CBipodWeapon::Spawn( WEAPON_BAR );
}

void CBAR::Precache( void )
{
    PRECACHE_MODEL( "models/v_bar.mdl" );
    PRECACHE_MODEL( "models/w_bar.mdl" );
    PRECACHE_MODEL( "models/p_barbu.mdl" );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/bar_shoot.wav" );

    m_usFireBAR = PRECACHE_EVENT( 1, "events/weapons/bar.sc" );
}

int CBAR::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_55mm";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _55MM_MAX_CARRY;
    p->iMaxAmmo2 = -1;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_BRENBAR;
    p->iMaxClip = BAR_MAX_CLIP;
	p->iId = m_iId = WEAPON_BAR;
    p->iBulletId = BULLET_PLAYER_BAR;
    p->flRecoil = 2.0f;
	p->iWeight = BAR_WEIGHT;

	return 1;
}

BOOL CBAR::Deploy( void )
{
    return DefaultDeploy( "models/v_bar.mdl", "models/p_barbu.mdl", BAR_UP_DRAW, "bar" );
}

int CBAR::Classify( void )
{
    return CLASS_MACHINEGUN;
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
    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
        return !CBipodWeapon::IsDeployed() ? BAR_DOWN_IDLE : BAR_UP_IDLE;
    else
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

void CBARAmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CBARAmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_BARCLIP_GIVE, "ammo_55mm", _55MM_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}