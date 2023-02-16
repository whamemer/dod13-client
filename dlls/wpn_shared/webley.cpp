//
// webley.cpp
//
// implementation of CWEBLEY class
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

LINK_ENTITY_TO_CLASS( weapon_webley, CWEBLEY )

enum WEBLEY_e
{
    WEBLEY_IDLE = 0,
    WEBLEY_SHOOT,
    WEBLEY_RELOAD,
    WEBLEY_DRAW
};

void CWEBLEY::Spawn( void )
{
    Precache();
    m_iId = WEAPON_WEBLEY;
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_WEBLEY].wmodel );
    m_iDefaultAmmo = WpnInfo[WEAPON_WEBLEY].ammo_default;
    FallInit();
}

void CWEBLEY::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_WEBLEY].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_WEBLEY].wmodel );

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
    p->iMaxAmmo1 = WpnInfo[WEAPON_WEBLEY].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_WEBLEY].ammo_maxclip;
    p->iSlot = 1;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_PISTOL;
    p->iId = m_iId = WEAPON_WEBLEY;
    p->iWeight = WpnInfo[WEAPON_WEBLEY].misc_weight;
    return 1;
}

void CWEBLEY::PrimaryAttack( void )
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
            Vector vecSrc = m_pPlayer->GetGunPosition();

            CBaseEntity::FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_WEBLEY, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireWebley, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_WEBLEY].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

BOOL CWEBLEY::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_pPlayer->CheckPlayerSpeed();

    return TimedDeploy( WpnInfo[WEAPON_WEBLEY].vmodel, WpnInfo[WEAPON_WEBLEY].pmodel, WEBLEY_DRAW, WpnInfo[WEAPON_WEBLEY].szAnimExt, 
        WpnInfo[WEAPON_WEBLEY].szAnimReloadExt, WpnInfo[WEAPON_WEBLEY].anim_drawtime, 1.0f, 0 );
}

void CWEBLEY::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_WEBLEY].ammo_maxclip, WEBLEY_RELOAD, WpnInfo[WEAPON_WEBLEY].anim_reloadtime );
}

void CWEBLEY::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( WEBLEY_IDLE );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
    }
}

class CWebleyAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_WEBLEY].ammo_maxclip, "ammo_12mm", WpnInfo[WEAPON_WEBLEY].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_webley, CWebleyAmmoClip )