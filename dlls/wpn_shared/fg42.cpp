#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum FG42_e
{
    FG42_UP_IDLE,
    FG42_UP_RELOAD,
    FG42_UP_DRAW,
    FG42_UP_SHOOT,
    FG42_UP_TO_DOWN,
    FG42_DOWN_IDLE,
    FG42_DOWN_RELOAD,
    FG42_DOWN_SHOOT,
    FG42_DOWN_TO_UP,
    FG42_LOWER_FOR_ZOOM
};

LINK_ENTITY_TO_CLASS( weapon_fg42, CFG42 );
LINK_ENTITY_TO_CLASS( weapon_scopedfg42, CFG42 );
LINK_ENTITY_TO_CLASS( ammo_fg42, CFG42 );

void CFG42::Spawn( void )
{
    Precache();

    SET_MODEL( ENT( pev ), "models/w_fg42.mdl" );

    m_iDefaultAmmo = FG42_DEFAULT_GIVE;
    m_iId = WEAPON_FG42;

    FallInit();
}

void CFG42::Precache( void )
{
    PRECACHE_MODEL( "models/v_fg42.mdl" );
    PRECACHE_MODEL( "models/w_fg42.mdl" );
    PRECACHE_MODEL( "models/p_fg42.mdl" );

    PRECACHE_MODEL( "models/v_scopedfg42.mdl" );
    PRECACHE_MODEL( "models/w_fg42s.mdl" );
    PRECACHE_MODEL( "models/p_fg42s.mdl" );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/fg42_shoot.wav" );

    m_usFireFG42 = PRECACHE_EVENT( 1, "events/weapons/fg42.sc" );
}

int CFG42::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_55mm";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _55MM_MAX_CARRY;
    p->iMaxAmmo2 = -1;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_BRENBAR;
    p->iMaxClip = FG42_MAX_CLIP;
    p->iId = m_iId = WEAPON_FG42;
    p->iBulletId = BULLET_PLAYER_FG42;
    p->iWeight = FG42_WEIGHT;

    return 1;
}

int CFG42::AddToPlayer( CBasePlayer *pPlayer )
{
    if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CFG42::Deploy( void )
{
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.93f;

    if( !m_iWeaponState )
        return DefaultDeploy( "models/v_fg42.mdl", "models/p_fg42.mdl", FG42_UP_DRAW, "fg42" );
    else
        return DefaultDeploy( "models/v_scopedfg42.mdl", "models/p_fg42s.mdl", FG42_UP_DRAW, "scopedenfg42" );
}

BOOL CFG42::CanHolster( void )
{
    return !CBasePlayer::IsInMGDeploy( m_pPlayer );;
}

void CFG42::PrimaryAttack( void )
{
    // TODO: WHAMER
}

void CFG42::SecondaryAttack( void )
{

}

void CFG42::Reload( void )
{

}

void CFG42::WeaponIdle( void )
{
    if( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
    {
        if( CBasePlayer::IsInMGDeploy( m_pPlayer ) )
            SendWeaponAnim( FG42_UP_IDLE );
        else
            SendWeaponAnim( FG42_DOWN_IDLE );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

BOOL CFG42::IsDeployed( void )
{
    return CBasePlayer::IsInMGDeploy( m_pPlayer );
}

void CFG42::Holster( int skiplocal )
{

}

int CFG42::Classify( void )
{
    return !m_iWeaponState ? CLASS_MACHINEGUN : CLASS_SCOPED_SNIPER;
}

void CEnfieldAmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CEnfieldAmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_ENFIELDCLIP_GIVE, "ammo_16mm", _16MM_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}