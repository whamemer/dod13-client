//
// fg42.cpp
//
// implementation of CFG42 class
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

LINK_ENTITY_TO_CLASS( weapon_fg42, CFG42 )
LINK_ENTITY_TO_CLASS( weapon_scopedfg42, CFG42 )

enum FG42_e
{
    FG42_UP_IDLE = 0,
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

void CFG42::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_FG42].wmodel );
    m_iId = WEAPON_FG42;
    m_iDefaultAmmo = WpnInfo[WEAPON_FG42].ammo_default;
    FallInit();
}

int CFG42::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

void CFG42::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_FG42].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_FG42].wmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_SCOPED_FG42].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_SCOPED_FG42].wmodel );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/fg42_shoot.wav" );

    m_usFireFG42 = PRECACHE_EVENT( 1, "events/weapons/fg42.sc" );
}

int CFG42::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_16mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_FG42].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_FG42].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_FG42;
    p->iWeight = WpnInfo[WEAPON_FG42].misc_weight;
    return 1;
}

BOOL CFG42::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.93f;

    if( m_iWeaponState & WPNSTATE_SCOPED )
        return DefaultDeploy( WpnInfo[WEAPON_SCOPED_FG42].vmodel, WpnInfo[WEAPON_SCOPED_FG42].pmodel, FG42_UP_DRAW, WpnInfo[WEAPON_SCOPED_FG42].szAnimExt, 
            WpnInfo[WEAPON_SCOPED_FG42].szAnimReloadExt, 0 );
    else
        return DefaultDeploy( WpnInfo[WEAPON_FG42].vmodel, WpnInfo[WEAPON_FG42].pmodel, FG42_UP_DRAW, WpnInfo[WEAPON_FG42].szAnimExt, WpnInfo[WEAPON_FG42].szAnimReloadExt, 0 );
}

void CFG42::PrimaryAttack( void )
{
    float flSpread;
    if( PlayerIsWaterSniping() )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
    }
    else if( m_iClip < 0 )
    {
        if( !m_fInAttack )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
            m_fInAttack = TRUE;
        }
    }
    else
    {
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        flSpread = WpnInfo[WEAPON_FG42].base_accuracy;

        if( m_iWeaponState & WPNSTATE_SCOPED )
        {
            if( GetFOV() <= 0 )
                flSpread = WpnInfo[WEAPON_FG42].base_accuracy;
            else
                flSpread = WpnInfo[WEAPON_FG42].base_accuracy2;
        }

        if( m_pPlayer->pev->origin.Length() > 45.0f )
            flSpread += WpnInfo[WEAPON_FG42].accuracy_penalty;

        if( m_iWeaponState & WPNSTATE_SCOPED )
            GetFOV();
        else
            IsDeployed();
            m_pPlayer->GetGunPosition();

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_FG42, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireFG42, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, GetFOV() > 0 );

            if( GetFOV() >= 0)
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_FG42].anim_firedelay;
            else
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SCOPED_FG42].anim_firedelay;
            
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();

            if( GetFOV() > 0 || m_iClip < 0 )
            {
                SetThink( &CBasePlayerWeapon::ThinkZoomOut );
                pev->nextthink = gpGlobals->time + 0.5f;
            }
            RemoveStamina( 1.0f, m_pPlayer );
    }
}

void CFG42::SecondaryAttack( void )
{
    if( m_iWeaponState & WPNSTATE_SCOPED )
    {
        m_pPlayer->m_iFOV = ChangeFOV( 20 );
        UpdateZoomSpeed();
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2f;
    }
    else
    {
        if( m_pPlayer->pev->iuser3 == 1 || m_pPlayer->pev->vuser1.x == 1.0f )
        {
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
            SendWeaponAnim( FG42_UP_TO_DOWN );
        }
        else if( m_pPlayer->pev->iuser3 == 2 || m_pPlayer->pev->vuser1.x == 2.0f )
        {
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.98f;
            SendWeaponAnim( FG42_DOWN_TO_UP );
        }
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3f;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3f;
    }
}

void CFG42::Reload( void )
{
    if( GetFOV() > 0 )
    {
        m_pPlayer->m_iFOV = ZoomOut();
        UpdateZoomSpeed();
    }

    if( IsDeployed() )
        DefaultReload( WpnInfo[WEAPON_FG42].ammo_maxclip, FG42_DOWN_RELOAD, WpnInfo[WEAPON_FG42].anim_reloadtime );
    else
        DefaultReload( WpnInfo[WEAPON_FG42].ammo_maxclip, FG42_UP_RELOAD, WpnInfo[WEAPON_FG42].anim_reloadtime );
}

void CFG42::Holster( int skiplocal )
{
    if( IsDeployed() )
        CBasePlayerWeapon::Holster( skiplocal );
}

BOOL CFG42::CanHolster( void )
{
    return IsDeployed();
}

BOOL CFG42::IsDeployed( void )
{
    return m_pPlayer->IsInMGDeploy();
}

void CFG42::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        if( IsDeployed() )
            SendWeaponAnim( FG42_UP_IDLE );
        else
            SendWeaponAnim( FG42_DOWN_IDLE );
        
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CFG42::Classify( void )
{
    return m_iWeaponState & WPNSTATE_SCOPED ? CLASS_MACHINEGUNS : CLASS_SCOPE_RIFLE;
}

class CFG42AmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_FG42].ammo_maxclip, "ammo_55mm", WpnInfo[WEAPON_FG42].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_fg42, CFG42AmmoClip )