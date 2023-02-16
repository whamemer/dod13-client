//
// piat.cpp
//
// implementation of CPIAT class
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

LINK_ENTITY_TO_CLASS( weapon_piat, CPIAT )

enum PIAT_e
{
    PIAT_IDLE = 0,
    PIAT_IDLE_EMPTY,
    PIAT_DRAW,
    PIAT_DRAW_EMPTY,
    PIAT_AIMED,
    PIAT_AIMED_EMPTY,
    PIAT_FIRE,
    PIAT_RAISE,
    PIAT_RAISE_EMPTY,
    PIAT_LOWER,
    PIAT_LOWER_EMPTY,
    PIAT_RELOAD_IDLE,
    PIAT_RELOAD_AIMED
};

void CPIAT::Reload( void )
{
    BOOL iResult;

    if( m_iClip < 0 )
    {
        if( m_iWeaponState |= WPNSTATE_ROCKET_SLOW )
            iResult = DefaultReload( WpnInfo[WEAPON_PIAT].ammo_maxclip, PIAT_RELOAD_AIMED, WpnInfo[WEAPON_PIAT].anim_reloadtime );
        else
            iResult = DefaultReload( WpnInfo[WEAPON_PIAT].ammo_maxclip, PIAT_RELOAD_IDLE, WpnInfo[WEAPON_PIAT].anim_reloadtime );
    }

    if( iResult )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_reloadtime;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_reloadtime;

        if( m_iWeaponState & WPNSTATE_ROCKET_SLOW )
        {
            UnSlow();
            m_iWeaponState &= ~WPNSTATE_ROCKET_SLOW;
        }
    }
}

void CPIAT::Spawn( void )
{
    Precache();
    m_iId = WEAPON_PIAT;
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_PIAT].wmodel );
    m_iDefaultAmmo = WpnInfo[WEAPON_PIAT].ammo_default;
    FallInit();
    m_iClip = WpnInfo[WEAPON_PIAT].ammo_maxclip;
}

void CPIAT::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_PIAT].wmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_PIAT].vmodel );

    PRECACHE_SOUND( "items/ammopickup.wav" );
    PRECACHE_SOUND( "weapons/rocketfire1.wav" );
    PRECACHE_SOUND( "weapons/glauncher.wav" );

    m_usFirePIAT = PRECACHE_EVENT( 1, "events/weapons/piat.sc" );
}

int CPIAT::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "rockets";
    p->iMaxAmmo1 = WpnInfo[WEAPON_PIAT].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 1;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_ROCKET;
    p->iId = m_iId = WEAPON_PIAT;
    p->iWeight = 20;
    return 1;
}

BOOL CPIAT::Deploy( void )
{
    m_iWeaponState &= WPNSTATE_ROCKET_SLOW;
    float idleTime = WpnInfo[WEAPON_PIAT].anim_drawtime;
    int iAnim = m_iClip != PIAT_IDLE_EMPTY + PIAT_DRAW;

    return TimedDeploy( WpnInfo[WEAPON_PIAT].vmodel, WpnInfo[WEAPON_PIAT].pmodel, iAnim, WpnInfo[WEAPON_PIAT].szAnimExt, 
        WpnInfo[WEAPON_PIAT].szAnimReloadExt, idleTime, idleTime, 0 );
}

BOOL CPIAT::CanHolster( void )
{
    return m_iWeaponState;
}

void CPIAT::Holster( int skiplocal )
{
    ClientSetSensitivity( 0 );
    m_iWeaponState & WPNSTATE_ROCKET_SLOW;
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
    CBasePlayerWeapon::Holster( skiplocal );
}

void CPIAT::PrimaryAttack( void )
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
            m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFirePIAT, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );
            --m_iClip;

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_firedelay;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.17f;
        }
        else
        {
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_firedelay;
        }
    }
}

void CPIAT::SecondaryAttack( void )
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

void CPIAT::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        if( m_iWeaponState |= WPNSTATE_ROCKET_SLOW )
            SendWeaponAnim( m_iClip != PIAT_IDLE_EMPTY + PIAT_AIMED );
        else
            SendWeaponAnim( m_iClip != PIAT_IDLE_EMPTY );
        
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0f;
    }
}

void CPIAT::Raise( void )
{
    SendWeaponAnim( m_iClip != PIAT_IDLE_EMPTY + PIAT_RAISE );
    ReSlow();

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_down2uptime;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_down2uptime;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_down2uptime;
}

void CPIAT::Lower( void )
{
    SendWeaponAnim( m_iClip != PIAT_IDLE_EMPTY + PIAT_LOWER );
    UnSlow();

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_up2downtime;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_up2downtime;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_PIAT].anim_up2downtime;
}

void CPIAT::UnSlow( void )
{
    m_iWeaponState & WPNSTATE_ROCKET_SLOW;
    ClientSetSensitivity( 0 );
}

void CPIAT::ReSlow( void )
{
    m_iWeaponState &= ~WPNSTATE_ROCKET_SLOW;
    ClientSetSensitivity( 1 );
}

class CPIATAmmo : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        Precache();
        SET_MODEL( ENT( pev ), "models/w_piat_rocket.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    void Precahce( void )
    {
        PRECACHE_MODEL( "models/w_piat_rocket.mdl" );
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( PIAT_DRAW, "rockets", PIAT_AIMED_EMPTY ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_piat, CPIATAmmo )