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
// bazooka.cpp
//
// implementation of CBazooka class
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

extern struct p_wpninfo_s *P_WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_bazooka, CBazooka )

enum BAZOOKA_e
{
    BAZOOKA_IDLE = 0,
    BAZOOKA_DRAW,
    BAZOOKA_AIMED,
    BAZOOKA_FIRE,
    BAZOOKA_RAISE,
    BAZOOKA_LOWER,
    BAZOOKA_RELOAD_AIMED,
    BAZOOKA_RELOAD_IDLE
};

void CBazooka::Reload( void )
{
    BOOL iResult;

    if( m_iClip <= 0 )
    {
        if( m_iWeaponState != 0 )
            iResult = DefaultReload( P_WpnInfo[WEAPON_BAZOOKA].ammo_maxclip, BAZOOKA_RELOAD_AIMED, P_WpnInfo[WEAPON_BAZOOKA].anim_reloadtime );
        else
            iResult = DefaultReload( P_WpnInfo[WEAPON_BAZOOKA].ammo_maxclip, BAZOOKA_RELOAD_IDLE, P_WpnInfo[WEAPON_BAZOOKA].anim_reloadtime );
    }

    if( iResult )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_reloadtime;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_reloadtime;

        if( m_iWeaponState == 0 )
        {
            UnSlow();
            m_iWeaponState = 2;
        }
    }
}

void CBazooka::Spawn( void )
{
    Precache();
    m_iId = WEAPON_BAZOOKA;
    SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_BAZOOKA].wmodel );
    m_iDefaultAmmo = P_WpnInfo[WEAPON_BAZOOKA].ammo_default;
    FallInit();
    m_iClip = P_WpnInfo[WEAPON_BAZOOKA].ammo_maxclip;
}

void CBazooka::Precache( void )
{
    PRECACHE_MODEL( P_WpnInfo[WEAPON_BAZOOKA].wmodel );
    PRECACHE_MODEL( P_WpnInfo[WEAPON_BAZOOKA].vmodel );

    PRECACHE_SOUND( "items/ammopickup.wav" );
    PRECACHE_SOUND( "weapons/rocketfire1.wav" );
    PRECACHE_SOUND( "weapons/glauncher.wav" );
    PRECACHE_SOUND( "weapons/bazookareloadgetrocket.wav" );
    PRECACHE_SOUND( "weapons/bazookareloadrocketin.wav" );
    PRECACHE_SOUND( "weapons/bazookareloadshovehome.wav" );

    m_usFireBazooka = PRECACHE_EVENT( 1, "events/weapons/bazooka.sc" );
}

int CBazooka::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "rockets";
    p->iMaxAmmo1 = P_WpnInfo[WEAPON_BAZOOKA].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 1;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_ROCKET;
    p->iId = WEAPON_BAZOOKA;
    p->iWeight = 20;
    return 1;
}

BOOL CBazooka::Deploy( void )
{
    m_iWeaponState = 1;
    float idleTime = P_WpnInfo[WEAPON_BAZOOKA].anim_drawtime;

    return TimedDeploy( P_WpnInfo[WEAPON_BAZOOKA].vmodel, P_WpnInfo[WEAPON_BAZOOKA].pmodel, BAZOOKA_DRAW, P_WpnInfo[WEAPON_BAZOOKA].szAnimExt, 
        P_WpnInfo[WEAPON_BAZOOKA].szAnimReloadExt, idleTime, idleTime, 0 );
}

BOOL CBazooka::CanHolster( void )
{
    return !m_iWeaponState;
}

void CBazooka::Holster( int skiplocal )
{
    ClientSetSensitivity( 0 );
    m_iWeaponState = 1;
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
    CBasePlayerWeapon::Holster( skiplocal );
}

void CBazooka::PrimaryAttack( void )
{
    if( m_iClip <= 0 )
    {
        if( !m_fInAttack )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
        }
        else if( m_iWeaponState != 0 )
        {
            m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireBazooka, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );
            --m_iClip;

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_reloadtime;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
        }
        else
        {
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_reloadtime;
        }
    }
}

void CBazooka::SecondaryAttack( void )
{
    if( m_pPlayer->pev->waterlevel <= 1 )
    {
        if( m_iWeaponState == 0 )
        {
            Raise();
            return;
        }

        Lower();
    }

    if( m_iWeaponState != 0 )
    {
        Lower();
    }
}

void CBazooka::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        if( m_iWeaponState != 0 )
            SendWeaponAnim( BAZOOKA_AIMED );
        else
            SendWeaponAnim( BAZOOKA_IDLE );
        
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0f;
    }
}

void CBazooka::Raise( void )
{
    SendWeaponAnim( BAZOOKA_RAISE );
    ReSlow();

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_down2uptime;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_down2uptime;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_down2uptime;
}

void CBazooka::Lower( void )
{
    SendWeaponAnim( BAZOOKA_LOWER );
    UnSlow();

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_up2downtime;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_up2downtime;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_BAZOOKA].anim_up2downtime;
}

void CBazooka::UnSlow( void )
{
    m_iWeaponState = 0;
    ClientSetSensitivity( 0 );
}

void CBazooka::ReSlow( void )
{
    m_iWeaponState = 1;
    ClientSetSensitivity( 1 );
}

class CBazookaAmmo : CBasePlayerAmmo
{
    void Spawn( void )
    {
        Precache();
        SET_MODEL( ENT( pev ), "models/w_bazooka_rocket.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    void Precahce( void )
    {
        PRECACHE_MODEL( "models/w_bazooka_rocket.mdl" );
        PRECACHE_SOUND( "items/ammopickup.wav" );
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( BAZOOKA_AIMED, "rockets", BAZOOKA_LOWER ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_bazooka, CBazookaAmmo )