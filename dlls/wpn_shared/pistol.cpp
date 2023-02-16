//
// pistol.cpp
//
// implementation of CPistol class
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

void CPistol::Spawn( int weapon_id )
{
    Precache();
    m_iId = weapon_id;
    SET_MODEL( ENT( pev ), WpnInfo[m_iId].wmodel );
    m_iDefaultAmmo = WpnInfo[m_iId].ammo_default;
    FallInit();
}

BOOL CPistol::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();

    m_pPlayer->CheckPlayerSpeed();

    float idleTime = WpnInfo[m_iId].anim_drawtime;
    int iAnim = GetDrawAnim();

    return TimedDeploy( WpnInfo[m_iId].vmodel, WpnInfo[m_iId].pmodel, iAnim, WpnInfo[m_iId].szAnimExt, WpnInfo[m_iId].szAnimReloadExt, idleTime, idleTime, 0 );
}

BOOL CPistol::CanHolster( void )
{
    return TRUE;
}

void CPistol::Reload( void )
{
    float fDelay = WpnInfo[m_iId].anim_reloadtime;
    int iAnim = GetReloadAnim();

    DefaultReload( WpnInfo[m_iId].ammo_maxclip, iAnim, fDelay );
}

void CPistol::PrimaryAttack( void )
{
    if( PlayerIsWaterSniping() )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
    }
    else if( !m_fInAttack )
    {
        if( m_iClip < 0 )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
            m_fInAttack = TRUE;
        }
        else
        {
            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

            ItemInfo sz[40];
            GetItemInfo( sz );

            float flSpread;

            if( m_pPlayer->pev->origin.Length() > 45.0f )
                flSpread = flSpread + WpnInfo[m_iId].accuracy_penalty;

            int iBulletType;
            Vector vecSrc = m_pPlayer->GetGunPosition();

            CBaseEntity::FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, iBulletType, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_iFireEvent, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[m_iId].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + WpnInfo[m_iId].anim_firedelay;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0f, 15.0f ) + UTIL_WeaponTimeBase();

            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CPistol::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        int iAnim = GetIdleAnim();
        SendWeaponAnim( iAnim );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.0f;
    }
}