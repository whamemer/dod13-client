//
// pschreck.cpp
//
// implementation of CPschreck class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"
#include "hud.h"

extern struct p_wpninfo_s *WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_pschreck, CPschreck )

enum PSCHRECK_e
{
    PSCHRECK_IDLE = 0,
    PSCHRECK_DRAW,
    PSCHRECK_AIMED,
    PSCHRECK_FIRE,
    PSCHRECK_RAISE,
    PSCHRECK_LOWER,
    PSCHRECK_RELOAD_AIMED,
    PSCHRECK_RELOAD_IDLE
};

void CPschreck::Reload( void )
{
    BOOL iResult;

    if( m_iClip < 0 )
    {
        if( m_iWeaponState |= WPNSTATE_ROCKET_SLOW )
        {
            iResult = DefaultReload( WpnInfo[WEAPON_PSCHRECK].ammo_maxclip, PSCHRECK_RELOAD_AIMED, WpnInfo[WEAPON_PSCHRECK].anim_reloadtime );

            if( !iResult )
                return;
            
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_reloadtime;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_reloadtime;
        }
        else
        {
            iResult = DefaultReload( WpnInfo[WEAPON_PSCHRECK].ammo_maxclip, PSCHRECK_RELOAD_IDLE, WpnInfo[WEAPON_PSCHRECK].anim_reloadtime );

            if( !iResult )
                return;
            
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_reloadtime;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_reloadtime;
        }

        if( m_iWeaponState & WPNSTATE_ROCKET_SLOW )
        {
            UnSlow();
            m_iWeaponState &= ~WPNSTATE_ROCKET_SLOW;
        }
    }
}

void CPschreck::Spawn( void )
{
    Precache();
    m_iId = WEAPON_PSCHRECK;
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_PSCHRECK].wmodel );
    m_iDefaultAmmo = WpnInfo[WEAPON_PSCHRECK].ammo_default;
    FallInit();
    m_iClip = WpnInfo[WEAPON_PSCHRECK].ammo_maxclip;
}

void CPschreck::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_PSCHRECK].wmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_PSCHRECK].vmodel );

    PRECACHE_SOUND( "items/ammopickup.wav" );
    PRECACHE_SOUND( "weapons/rocketfire1.wav" );
    PRECACHE_SOUND( "weapons/glauncher.wav" );

    m_usFirePschreck = PRECACHE_EVENT( 1, "events/weapons/pschreck.sc" );
}

int CPschreck::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "rockets";
    p->iMaxAmmo1 = WpnInfo[WEAPON_PSCHRECK].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 1;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_ROCKET;
    p->iId = m_iId = WEAPON_PSCHRECK;
    p->iWeight = 20;
    return 1;
}

BOOL CPschreck::Deploy( void )
{
    m_iWeaponState &= WPNSTATE_ROCKET_SLOW;
    float idleTime = WpnInfo[WEAPON_PSCHRECK].anim_drawtime;

    return TimedDeploy( WpnInfo[WEAPON_PSCHRECK].vmodel, WpnInfo[WEAPON_PSCHRECK].pmodel, PSCHRECK_DRAW, WpnInfo[WEAPON_PSCHRECK].szAnimExt, 
        WpnInfo[WEAPON_PSCHRECK].szAnimReloadExt, idleTime, idleTime, 0 );
}

BOOL CPschreck::CanHolster( void )
{
    return m_iWeaponState;
}

void CPschreck::Holster( int skiplocal )
{
    ClientSetSensitivity( 0 );
    m_iWeaponState & WPNSTATE_ROCKET_SLOW;
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
    CBasePlayerWeapon::Holster( skiplocal );
}

void CPschreck::PrimaryAttack( void )
{
    if( m_iClip < 0 )
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
        if( m_pPlayer->pev->waterlevel > 1 )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
        }
        else if( m_iWeaponState |= WPNSTATE_ROCKET_SLOW )
        {
            m_pPlayer->m_iWeaponVolume = 1000;
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFirePschreck, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );
            --m_iClip;

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_firedelay;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.17f;
        }
        else
        {
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_firedelay;
        }
    }
}

void CPschreck::SecondaryAttack( void )
{
    if( m_pPlayer->pev->waterlevel <= 1 )
    {
        if( m_iWeaponState & WPNSTATE_ROCKET_SLOW )
        {
            Raise();
            return;
        }

        Lower();
    }

    if( m_iWeaponState |= WPNSTATE_ROCKET_SLOW )
    {
        Lower();
    }
}

void CPschreck::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        if( m_iWeaponState |= WPNSTATE_ROCKET_SLOW )
            SendWeaponAnim( PSCHRECK_AIMED );
        else
            SendWeaponAnim( PSCHRECK_IDLE );
        
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0f;
    }
}

void CPschreck::Raise( void )
{
    SendWeaponAnim( PSCHRECK_RAISE );
    ReSlow();

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_down2uptime;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_down2uptime;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_down2uptime;
}

void CPschreck::Lower( void )
{
    SendWeaponAnim( PSCHRECK_LOWER );
    UnSlow();

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_up2downtime;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_up2downtime;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PSCHRECK].anim_up2downtime;
}

void CPschreck::UnSlow( void )
{
    m_iWeaponState & WPNSTATE_ROCKET_SLOW;
    ClientSetSensitivity( 0 );
}

void CPschreck::ReSlow( void )
{
    m_iWeaponState &= ~WPNSTATE_ROCKET_SLOW;
    ClientSetSensitivity( 1 );
}

class CPschreckAmmo : CBasePlayerAmmo
{
    void Spawn( void )
    {
        Precache();
        SET_MODEL( ENT( pev ), "models/w_pschreck_rocket.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    void Precahce( void )
    {
        PRECACHE_MODEL( "models/w_pschreck_rocket.mdl" );
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( PSCHRECK_AIMED, "rockets", PSCHRECK_LOWER ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_pschreck, CPschreckAmmo )