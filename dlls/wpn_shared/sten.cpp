//
// sten.cpp
//
// implementation of CSTEN class
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

LINK_ENTITY_TO_CLASS( weapon_sten, CSTEN )

enum STEN_e
{
    STEN_IDLE = 0,
    STEN_RELOAD,
    STEN_DRAW,
    STEN_SHOOT
};

void CSTEN::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_STEN].wmodel );
    m_iId = WEAPON_STEN;
    m_iDefaultAmmo = WpnInfo[WEAPON_STEN].ammo_default;
    FallInit();
}

void CSTEN::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_STEN].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_STEN].wmodel );

    PRECACHE_SOUND( "weapons/reloadgarand.wav" );
    PRECACHE_SOUND( "weapons/sten_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

    m_usFireSten = PRECACHE_EVENT( 1, "events/weapons/sten.sc" );
}

int CSTEN::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_44mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_STEN].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_STEN].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_STEN;
    p->iWeight = WpnInfo[WEAPON_STEN].misc_weight;
    return 1;
}

int CSTEN::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CSTEN::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75f;
    return DefaultDeploy( WpnInfo[WEAPON_STEN].vmodel, WpnInfo[WEAPON_STEN].pmodel, STEN_DRAW, WpnInfo[WEAPON_STEN].szAnimExt, WpnInfo[WEAPON_STEN].szAnimReloadExt, 0 );
}

BOOL CSTEN::CanHolster( void )
{
    return TRUE;
}

void CSTEN::PrimaryAttack( void )
{
    float flSpread;

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
            m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
            --m_iClip;

            if( m_pPlayer->pev->origin.Length() > 45.0f )
                flSpread = WpnInfo[WEAPON_STEN].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_STEN, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireSten, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_STEN].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CSTEN::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_STEN].ammo_maxclip, STEN_RELOAD, WpnInfo[WEAPON_STEN].anim_reloadtime );
}

void CSTEN::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( STEN_IDLE, UseDecrement() != FALSE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CSTEN::Classify( void )
{
    return CLASS_AUTO_RIFLE;
}

class CSTENAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_STEN].ammo_maxclip, "ammo_44mm", WpnInfo[WEAPON_STEN].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_sten, CSTENAmmoClip )