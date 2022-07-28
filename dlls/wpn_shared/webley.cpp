#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum WEBLEY_e
{
    WEBLEY_IDLE,
    WEBLEY_SHOOT,
    WEBLEY_RELOAD,
    WEBLEY_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_webley, CWEBLEY );
LINK_ENTITY_TO_CLASS( ammo_webley, CWEBLEY );

void CWEBLEY::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_webley.mdl" );
    m_iDefaultAmmo = WEBLEY_DEFAULT_GIVE;

    CPistol::Spawn( WEAPON_WEBLEY );
}

void CWEBLEY::Precache( void )
{
    PRECACHE_MODEL( "models/v_webley.mdl" );
    PRECACHE_MODEL( "models/w_webley.mdl" );
    PRECACHE_MODEL( "models/p_webley.mdl" );

    PRECACHE_SOUND( "weapons/webley_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/colt_reload_clipin.wav" );
    PRECACHE_SOUND( "weapons/colt_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/colt_draw_pullslide.wav" );

    m_usFireWebley = PRECACHE_EVENT( 1, "events/weapons/webley.sc" );
}

int CWEBLEY::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_12mm";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _12MM_MAX_CARRY;
    p->iMaxAmmo2 = -1;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_PISTOL;
    p->iMaxClip = WEBLEY_MAX_CLIP;
	p->iId = m_iId = WEAPON_WEBLEY;
    p->iBulletId = BULLET_PLAYER_LUGER;
	p->iWeight = WEBLEY_WEIGHT;

	return 1;
}

BOOL CWEBLEY::Deploy( void )
{
    return DefaultDeploy( "models/v_webley.mdl", "models/p_webley.mdl", WEBLEY_DRAW, "webley" );
}

void CWEBLEY::PrimaryAttack( void )
{
    // TODO: WHAMER
}

BOOL CWEBLEY::CanHolster( void )
{
    return TRUE;
}

void CWEBLEY::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
    {
        if( m_iClip )
        {
            SendWeaponAnim( WEBLEY_IDLE );
        }
        else
        {
            SendWeaponAnim( WEBLEY_SHOOT );
        }
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
    }
}

void CWEBLEYAmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CWEBLEYAmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_WEBLEYCLIP_GIVE, "ammo_12mm", _12MM_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}