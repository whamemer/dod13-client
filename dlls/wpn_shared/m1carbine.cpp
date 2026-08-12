//
// m1carbine.cpp
//
// implementation of CM1Carbine class
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

LINK_ENTITY_TO_CLASS( weapon_m1carbine, CM1Carbine )

enum M1CARBINE_e
{
    M1CARBINE_IDLE = 0,
    M1CARBINE_RELOAD,
    M1CARBINE_DRAW,
    M1CARBINE_SHOOT,
    M1CARBINE_FAST_DRAW
};

void CM1Carbine::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_M1CARBINE].wmodel );
    m_iId = WEAPON_M1CARBINE;
    m_iDefaultAmmo = WpnInfo[WEAPON_M1CARBINE].ammo_default;
    FallInit();
}

int CM1Carbine::AddToPlayer( CBasePlayer *pPlayer )
{
    if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
        SET_MODEL( ENT( pev ), WpnInfo[WEAPON_M1CARBINE].wmodel );
		return TRUE;
	}
	return FALSE;
}

void CM1Carbine::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_M1CARBINE].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_M1CARBINE].wmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_FOLDINGCARBINE].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_FOLDINGCARBINE].wmodel );

    PRECACHE_SOUND( "weapons/carbine_shoot.wav" );
    PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

    m_usFireM1Carbine = PRECACHE_EVENT( 1, "events/weapons/m1carbine.sc" );
}

int CM1Carbine::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_66mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_M1CARBINE].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_M1CARBINE].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_M1CARBINE;
    p->iWeight = WpnInfo[WEAPON_M1CARBINE].misc_weight;
    return 1;
}

BOOL CM1Carbine::SpawnDeploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
    return DefaultDeploy( WpnInfo[WEAPON_M1CARBINE].vmodel, WpnInfo[WEAPON_M1CARBINE].pmodel, M1CARBINE_DRAW, WpnInfo[WEAPON_M1CARBINE].szAnimExt, 
        WpnInfo[WEAPON_M1CARBINE].szAnimReloadExt, 0 );
}

BOOL CM1Carbine::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
    return DefaultDeploy( WpnInfo[WEAPON_M1CARBINE].vmodel, WpnInfo[WEAPON_M1CARBINE].pmodel, M1CARBINE_DRAW, WpnInfo[WEAPON_M1CARBINE].szAnimExt, 
        WpnInfo[WEAPON_M1CARBINE].szAnimReloadExt, 0 );
}

BOOL CM1Carbine::CanHolster( void )
{
    return TRUE;
}

void CM1Carbine::PrimaryAttack( void )
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
                flSpread = WpnInfo[WEAPON_M1CARBINE].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_M1CARBINE, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireM1Carbine, 0.0f, g_vecZero, g_vecZero, 0, 0, m_pPlayer->pev->punchangle.x, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_M1CARBINE].anim_firedelay;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_M1CARBINE].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.05f;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CM1Carbine::SecondaryAttack( void )
{
    // Nothing.
}

void CM1Carbine::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_M1CARBINE].ammo_maxclip, M1CARBINE_RELOAD, WpnInfo[WEAPON_M1CARBINE].anim_reloadtime );
}

void CM1Carbine::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( M1CARBINE_IDLE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CM1Carbine::Classify( void )
{
    return CLASS_RIFLE;
}

class CM1CarbineAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_M1CARBINE].ammo_maxclip, "ammo_66mm", WpnInfo[WEAPON_M1CARBINE].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_k43, CM1CarbineAmmoClip )