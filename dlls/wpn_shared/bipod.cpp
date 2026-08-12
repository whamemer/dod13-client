//
// bipod.cpp
//
// implementation of CBipodWeapon class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"
#include "pm_shared.h"

extern struct p_wpninfo_s *WpnInfo;

void CBipodWeapon::Spawn( int weapon_id )
{
    Precache();
    m_iId = weapon_id;
    SET_MODEL( ENT( pev ), WpnInfo[weapon_id].wmodel );
    m_iDefaultAmmo = WpnInfo[m_iId].ammo_default;
    FallInit();
}

void CBipodWeapon::Precache( void )
{
    m_iOverheatEvent = g_engfuncs.pfnPrecacheEvent( 1, "events/effects/overheat.sc" );
}

int CBipodWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CBipodWeapon::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

    int iAnim = GetDrawAnim();
    return CBasePlayerWeapon::DefaultDeploy( WpnInfo[m_iId].vmodel, WpnInfo[m_iId].pmodel, iAnim, WpnInfo[m_iId].szAnimExt, WpnInfo[m_iId].szAnimReloadExt, 0 );
}

BOOL CBipodWeapon::CanHolster( void )
{
    return IsDeployed();
}

void CBipodWeapon::Holster( int skiplocal )
{
    CBasePlayerWeapon::Holster( skiplocal );
}

void CBipodWeapon::PrimaryAttack( void )
{
    float flSpread;

    if( PlayerIsWaterSniping() )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
        return;
    }

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
        ItemInfo bipod[40];
        GetItemInfo( bipod );

        if( m_fInAttack )
            return;
        
        if( m_flWeaponHeat >= 99.0f )
        {
            m_fInAttack = TRUE;
            return;
        }

        if( m_flWeaponHeat >= 95.0f )
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_iOverheatEvent, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        if( IsDeployed() )
            flSpread = WpnInfo[m_iId].base_accuracy2;
        else
            flSpread = WpnInfo[m_iId].base_accuracy;

        if( !IsDeployed() )
        {
            if( m_pPlayer->pev->origin.Length() > 45.0f )
                flSpread = flSpread + WpnInfo[m_iId].accuracy_penalty;
        }

        int iBulletType;
        Vector vecSrc = m_pPlayer->GetGunPosition();

        FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, iBulletType, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
        m_iClip -= 1;

        if( m_iId == WEAPON_MG42 )
        {
            m_iClip -= 2;
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, iBulletType, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed + 5 );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_iFireEvent, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[m_iId].anim_firedelay;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[m_iId].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + WpnInfo[m_iId].anim_firedelay;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();

            if( !IsDeployed() )
                RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CBipodWeapon::CoolThink( void )
{
    m_flWeaponHeat -= 0.6f;

    if( m_flWeaponHeat <= 0.0f )
    {
        m_flWeaponHeat = 0.0f;
        ResetThink();
    }
    else
    {
        pev->nextthink = gpGlobals->time + 0.1f;
    }
}

void CBipodWeapon::SecondaryAttack( void )
{
    if ( m_pPlayer->pev->waterlevel <= 0 || m_pPlayer->pev->iuser3 == 2 )
    {
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[m_iId].anim_down2uptime;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[m_iId].anim_down2uptime;

        if( m_pPlayer->pev->iuser3 == 1 || m_pPlayer->pev->vuser1.x == 1.0f )
            SendWeaponAnim( GetUpToDownAnim() );
        else if( m_pPlayer->pev->iuser3 == 2 || m_pPlayer->pev->vuser1.x == 2.0f )
            SendWeaponAnim( GetDownToUpAnim() );
    }
}

void CBipodWeapon::ForceUndeploy( void )
{
    SendWeaponAnim( GetDownToUpAnim() );
}

void CBipodWeapon::Reload( void )
{
    float flReloadTime;
    int iAnim = GetReloadAnim();

    if( m_iId == WEAPON_BREN )
    {
        if( IsDeployed() )
        {
            flReloadTime = WpnInfo[m_iId].anim_reloadtime2;
            DefaultReload( WpnInfo[m_iId].ammo_maxclip, iAnim, flReloadTime );
        }
    }

    flReloadTime = WpnInfo[m_iId].anim_reloadtime;
    DefaultReload( WpnInfo[m_iId].ammo_maxclip, iAnim, flReloadTime );
}

void CBipodWeapon::WeaponIdle( void )
{
    ResetEmptySound();
    int iAnim = GetIdleAnim();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
        if( m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() )
            if( m_flNextSecondaryAttack <= UTIL_WeaponTimeBase() )
            {
                if( iAnim >= 0 )
                    SendWeaponAnim( iAnim );
            }
}

bool CBipodWeapon::IsDeployed( void )
{
    return IsDeployed();
}

float CBipodWeapon::GetBipodSpread( void )
{
    if( IsDeployed() )
        return WpnInfo[m_iId].base_accuracy2;
    else
        return WpnInfo[m_iId].base_accuracy;
}