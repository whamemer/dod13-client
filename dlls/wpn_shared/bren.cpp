#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum BREN_e
{
    BREN_UP_IDLE,
    BREN_UP_RELOAD,
    BREN_UP_DRAW,
    BREN_UP_SHOOT,
    BREN_UP_TO_DOWN,
    BREN_DOWN_IDLE,
    BREN_DOWN_RELOAD,
    BREN_DOWN_SHOOT,
    BREN_DOWN_TO_UP
};

LINK_ENTITY_TO_CLASS( weapon_bren, CBREN );
LINK_ENTITY_TO_CLASS( ammo_bren, CBREN );

void CBREN::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_bren.mdl" );
    m_iDefaultAmmo = BREN_DEFAULT_GIVE;

    CBipodWeapon::Spawn( WEAPON_BREN );
}

void CBREN::Precache( void )
{
    PRECACHE_MODEL( "models/v_bren.mdl" );
    PRECACHE_MODEL( "models/w_bren.mdl" );
    PRECACHE_MODEL( "models/p_bren.mdl" );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/bren_shoot.wav" );

    m_usFireBren = PRECACHE_EVENT( 1, "events/weapons/bren.sc" );
}

int CBREN::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_55mm";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _55MM_MAX_CARRY;
    p->iMaxAmmo2 = -1;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_BREN;
    p->iMaxClip = BREN_MAX_CLIP;
	p->iId = m_iId = WEAPON_BREN;
    p->iBulletId = BULLET_PLAYER_BREN;
    p->flRecoil = 2.0f;
	p->iWeight = BREN_WEIGHT;

	return 1;
}

BOOL CBREN::Deploy( void )
{
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

    return DefaultDeploy( "models/v_bren.mdl", "models/p_bren.mdl", BREN_UP_DRAW, "bren" );
}

int CBREN::Classify( void )
{
    return CLASS_MACHINEGUN;
}

int CBREN::GetReloadAnim( void )
{
    return !CBipodWeapon::IsDeployed() ? BREN_UP_RELOAD : BREN_DOWN_RELOAD;
}

int CBREN::GetDrawAnim( void )
{
    return BREN_UP_DRAW;
}

int CBREN::GetIdleAnim( void )
{
    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
        return !CBipodWeapon::IsDeployed() ? BREN_UP_IDLE : BREN_DOWN_IDLE;
    else
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

void CBRENAmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CBRENAmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_BRENCLIP_GIVE, "ammo_55mm", _55MM_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}