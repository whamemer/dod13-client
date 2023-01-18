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
// luger.cpp
//
// implementation of CLUGER class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"

extern struct p_wpninfo_s *P_WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_luger, CLUGER )

enum LUGER_e
{
    LUGER_IDLE = 0,
    LUGER_IDLE2,
    LUGER_IDLE3,
    LUGER_SHOOT,
    LUGER_SHOOT_EMPTY,
    LUGER_RELOAD_EMPTY,
    LUGER_RELOAD,
    LUGER_DRAW,
    LUGER_IDLE_EMPTY
};

void CLUGER::Spawn( void )
{
    Precache();
    m_iId = WEAPON_LUGER;
    SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_LUGER].wmodel );
    m_iDefaultAmmo = P_WpnInfo[WEAPON_LUGER].ammo_default;
    FallInit();
}

void CLUGER::Precache( void )
{
    PRECACHE_MODEL( P_WpnInfo[WEAPON_LUGER].vmodel );
    PRECACHE_MODEL( P_WpnInfo[WEAPON_LUGER].wmodel );

    PRECACHE_SOUND( "weapons/webley_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );

    m_usFireLuger = PRECACHE_EVENT( 1, "events/weapons/luger.sc" );
}

int CLUGER::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_12mm";
    p->iMaxAmmo1 = P_WpnInfo[WEAPON_LUGER].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = P_WpnInfo[WEAPON_LUGER].ammo_maxclip;
    p->iSlot = 1;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_PISTOL;
    p->iId = WEAPON_LUGER;
    p->iWeight = P_WpnInfo[WEAPON_LUGER].misc_weight;
    return 1;
}

void CLUGER::PrimaryAttack( void )
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

            float flSpread;
            Vector vecSrc = m_pPlayer->GetGunPosition();

            CBaseEntity::FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_LUGER, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireLuger, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_LUGER].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_fInAttack = 1;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            
            RemoveStamina( m_pPlayer->GetStamina(), m_pPlayer );
        }
    }
}

BOOL CLUGER::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();

    m_pPlayer->CheckPlayerSpeed();

    float idleTime = P_WpnInfo[m_iId].anim_drawtime;

    return DefaultDeploy( P_WpnInfo[WEAPON_LUGER].vmodel, P_WpnInfo[WEAPON_LUGER].pmodel, LUGER_DRAW, P_WpnInfo[WEAPON_LUGER].szAnimExt, P_WpnInfo[WEAPON_LUGER].szAnimReloadExt, 0 );
}

void CLUGER::Reload( void )
{
    DefaultReload( P_WpnInfo[WEAPON_LUGER].ammo_maxclip, LUGER_RELOAD - m_iClip == 0, P_WpnInfo[WEAPON_LUGER].anim_reloadtime );
}

void CLUGER::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( LUGER_IDLE_EMPTY * m_iClip <= 0 );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
    }
}

class CLugerAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( P_WpnInfo[WEAPON_LUGER].ammo_maxclip, "ammo_12mm", P_WpnInfo[WEAPON_LUGER].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_luger, CLugerAmmoClip )