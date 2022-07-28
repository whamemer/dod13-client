#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum LUGER_e
{
    LUGER_IDLE,
    LUGER_IDLE2,
    LUGER_IDLE3,
    LUGER_SHOOT,
    LUGER_SHOOT_EMPTY,
    LUGER_RELOAD_EMPTY,
    LUGER_RELOAD,
    LUGER_DRAW,
    LUGER_IDLE_EMPTY
};

LINK_ENTITY_TO_CLASS( weapon_luger, CLUGER );
LINK_ENTITY_TO_CLASS( ammo_luger, CLUGER );

void CLUGER::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_luger.mdl" );
    m_iDefaultAmmo = LUGER_DEFAULT_GIVE;

    CPistol::Spawn( WEAPON_LUGER );
}

void CLUGER::Precache( void )
{
    PRECACHE_MODEL( "models/v_luger.mdl" );
    PRECACHE_MODEL( "models/w_luger.mdl" );
    PRECACHE_MODEL( "models/p_luger.mdl" );

    PRECACHE_SOUND( "weapons/luger_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );

    m_usFireLuger = PRECACHE_EVENT( 1, "events/weapons/luger.sc" );
}

int CLUGER::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_12mm";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _12MM_MAX_CARRY;
    p->iMaxAmmo2 = -1;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_PISTOL;
    p->iMaxClip = LUGER_MAX_CLIP;
	p->iId = m_iId = WEAPON_LUGER;
    p->iBulletId = BULLET_PLAYER_LUGER;
	p->iWeight = LUGER_WEIGHT;

	return 1;
}

BOOL CLUGER::Deploy( void )
{
    return DefaultDeploy( "models/v_luger.mdl", "models/p_luger.mdl", LUGER_DRAW, "luger" );
}

void CLUGER::PrimaryAttack( void )
{
    // TODO: WHAMER
}

BOOL CLUGER::CanHolster( void )
{
    return TRUE;
}

void CLUGER::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
    {
        if( m_iClip )
        {
            SendWeaponAnim( LUGER_IDLE_EMPTY );
        }
        else
        {
            SendWeaponAnim( LUGER_IDLE2 );
        }
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
    }
}

void CLugerAmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CLugerAmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_LUGERCLIP_GIVE, "ammo_12mm", _12MM_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}