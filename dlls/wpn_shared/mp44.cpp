//
// mp44.cpp
//
// implementation of CMP44 class
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

LINK_ENTITY_TO_CLASS( weapon_mp44, CMP44 )

enum MP44_e
{
    MP44_IDLE = 0,
    MP44_RELOAD,
    MP44_DRAW,
    MP44_SHOOT1
};

void CMP44::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_MP44].wmodel );
    m_iId = WEAPON_MP44;
    m_iDefaultAmmo = WpnInfo[WEAPON_MP44].ammo_default;
    FallInit();
}

void CMP44::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_MP44].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_MP44].wmodel );

    PRECACHE_SOUND( "weapons/weaponempty.wav" );
    PRECACHE_SOUND( "weapons/mp44_shoot.wav" );

    m_usFireMP44 = PRECACHE_EVENT( 1, "events/weapons/mp44.sc" );
}

int CMP44::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_55mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_MP44].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_MP44].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_MP44;
    p->iWeight = WpnInfo[WEAPON_MP44].misc_weight;
    return 1;
}

int CMP44::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CMP44::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_MP44].anim_drawtime;
    return DefaultDeploy( WpnInfo[WEAPON_MP44].vmodel, WpnInfo[WEAPON_MP44].pmodel, MP40_DRAW, WpnInfo[WEAPON_MP44].szAnimExt, 
        WpnInfo[WEAPON_MP44].szAnimReloadExt, 0 );
}

BOOL CMP44::CanHolster( void )
{
    return TRUE;
}

void CMP44::PrimaryAttack( void )
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
                flSpread = WpnInfo[WEAPON_MP44].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_MP40, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireMP44, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_MP44].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CMP44::SecondaryAttack( void )
{
    // Nothing.
}

void CMP44::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_MP44].ammo_maxclip, MP40_RELOAD, WpnInfo[WEAPON_MP44].anim_reloadtime );
}

void CMP44::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( MP44_IDLE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CMP44::Classify( void )
{
    return CLASS_AUTO_RIFLE;
}

class CMP44AmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_MP40].ammo_maxclip, "ammo_55mm", WpnInfo[WEAPON_MP40].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_mp44, CMP44AmmoClip )