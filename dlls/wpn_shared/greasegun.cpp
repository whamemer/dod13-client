//
// greasegun.cpp
//
// implementation of CGreaseGun class
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

LINK_ENTITY_TO_CLASS( weapon_greasegun, CGreaseGun )

enum GREASEGUN_e 
{
    GREASEGUN_IDLE = 0,
    GREASEGUN_DRAW,
    GREASEGUN_SHOOT1,
    GREASEGUN_SHOOT2,
    GREASEGUN_IDLE_EMPTY,
    GREASEGUN_FAST_DRAW
};

void CGreaseGun::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_GREASEGUN].wmodel );
    m_iId = WEAPON_GREASEGUN;
    m_iDefaultAmmo = WpnInfo[WEAPON_GREASEGUN].ammo_default;
    FallInit();
}

void CGreaseGun::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_GREASEGUN].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_GREASEGUN].wmodel );

    PRECACHE_SOUND( "weapons/greasegun_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );

    m_usFireGreaseGun = PRECACHE_EVENT( 1, "events/weapons/greasegun.sc" );
}

int CGreaseGun::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_44mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_GREASEGUN].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_GREASEGUN].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_GREASEGUN;
    p->iWeight = WpnInfo[WEAPON_GREASEGUN].misc_weight;
    return 1;
}

int CGreaseGun::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CGreaseGun::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
    return DefaultDeploy( WpnInfo[WEAPON_GREASEGUN].vmodel, WpnInfo[WEAPON_GREASEGUN].pmodel, GREASEGUN_FAST_DRAW, WpnInfo[WEAPON_GREASEGUN].szAnimExt, 
        WpnInfo[WEAPON_GREASEGUN].szAnimReloadExt, 0 );
}

BOOL CGreaseGun::SpawnDeploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    return TimedDeploy( WpnInfo[WEAPON_GREASEGUN].vmodel, WpnInfo[WEAPON_GREASEGUN].pmodel, GREASEGUN_RELOAD, WpnInfo[WEAPON_GREASEGUN].szAnimExt, 
        WpnInfo[WEAPON_GREASEGUN].szAnimReloadExt, 1.6f, 1.6f, 1 );
}

BOOL CGreaseGun::CanHolster( void )
{
    return TRUE;
}

void CGreaseGun::PrimaryAttack( void )
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
                flSpread = WpnInfo[WEAPON_GREASEGUN].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_GREASEGUN, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireGreaseGun, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_GREASEGUN].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CGreaseGun::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_GREASEGUN].ammo_maxclip, GREASEGUN_DRAW, WpnInfo[WEAPON_GREASEGUN].anim_reloadtime );
}

void CGreaseGun::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( GREASEGUN_FAST_DRAW * ( m_iClip < 0 ) );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CGreaseGun::Classify( void )
{
    return CLASS_AUTO_RIFLE;
}

class CGreaseGunAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_GREASEGUN].ammo_maxclip, "ammo_44mm", WpnInfo[WEAPON_GREASEGUN].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_greasegun, CGreaseGunAmmoClip )