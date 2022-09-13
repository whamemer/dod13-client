/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
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

extern struct p_wpninfo_s P_WpnInfo[];

void CPistol::Spawn( int weapon_id )
{
    m_iId = weapon_id;
    SET_MODEL( ENT( pev ), P_WpnInfo[m_iId].wmodel );

    m_iDefaultAmmo = P_WpnInfo[m_iId].ammo_default;

    FallInit();
    Precache();
}

BOOL CPistol::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();

    m_pPlayer->CheckPlayerSpeed();

    float idleTime = P_WpnInfo[m_iId].anim_drawtime;
    int iAnim = GetDrawAnim();

    return TimedDeploy( P_WpnInfo[m_iId].vmodel, P_WpnInfo[m_iId].pmodel, iAnim, P_WpnInfo[m_iId].szAnimExt, P_WpnInfo[m_iId].szAnimReloadExt, idleTime, idleTime, 0 );
}

BOOL CPistol::CanHolster( void )
{
    return TRUE;
}

void CPistol::Reload( void )
{
    float fDelay = P_WpnInfo[m_iId].anim_reloadtime;
    int iAnim = GetReloadAnim();

    DefaultReload( P_WpnInfo[m_iId].ammo_maxclip, iAnim, fDelay );
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
        if( m_iClip <= 0 )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
            m_fInAttack = 1;
        }
        else
        {
            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

            ItemInfo sz[40];
            GetItemInfo( sz );

            float flSpread = 0.035f;
            int iBulletType;
            entvars_t *pevAttacker = m_pPlayer->pev;
            int shared_rand = m_pPlayer->random_seed;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            vec3_t vecDirShooting = gpGlobals->v_forward;

            CBaseEntity::FireBulletsNC( (Vector *)m_pPlayer, vecSrc, vecDirShooting, flSpread, 8192.0f, iBulletType, 3, 0, pevAttacker, shared_rand );

            HUD_PlaybackEvent( 1, ENT( m_pPlayer->pev ), m_iFireEvent, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + P_WpnInfo[m_iId].anim_firedelay;
            m_fInAttack = 1;
            m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0f, 15.0f ) + UTIL_WeaponTimeBase();

            RemoveStamina( 0.0f, m_pPlayer );
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