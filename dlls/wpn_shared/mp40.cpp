//
// mp40.cpp
//
// implementation of CMP40 class
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

LINK_ENTITY_TO_CLASS( weapon_mp40, CMP40 )

enum MP40_e
{
    MP40_IDLE = 0,
    MP40_RELOAD,
    MP40_DRAW,
    MP40_SHOOT1,
    MP40_SHOOT2,
    MP40_IDLEEMPTY
};

void CMP40::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_MP40].wmodel );
    m_iId = WEAPON_MP40;
    m_iDefaultAmmo = WpnInfo[WEAPON_MP40].ammo_default;
    FallInit();
}

void CMP40::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_MP40].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_MP40].wmodel );

    PRECACHE_SOUND( "weapons/weaponempty.wav" );
    PRECACHE_SOUND( "weapons/mp44_shoot.wav" );
    PRECACHE_SOUND( "weapons/mp40_shoot.wav" );

    m_usFireMP40 = PRECACHE_EVENT( 1, "events/weapons/mp40.sc" );
}

int CMP40::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_44mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_MP40].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_MP40].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_MP40;
    p->iWeight = WpnInfo[WEAPON_MP40].misc_weight;
    return 1;
}

int CMP40::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CMP40::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_MP40].anim_drawtime;
    return DefaultDeploy( WpnInfo[WEAPON_MP40].vmodel, WpnInfo[WEAPON_MP40].pmodel, MP40_DRAW, WpnInfo[WEAPON_MP40].szAnimExt, 
        WpnInfo[WEAPON_MP40].szAnimReloadExt, 0 );
}

BOOL CMP40::CanHolster( void )
{
    return TRUE;
}

void CMP40::PrimaryAttack( void )
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
                flSpread = WpnInfo[WEAPON_MP40].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_MP40, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireMP40, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_MP40].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CMP40::SecondaryAttack( void )
{
    // Nothing.
}

void CMP40::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_MP40].ammo_maxclip, MP40_RELOAD, WpnInfo[WEAPON_MP40].anim_reloadtime );
}

void CMP40::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( MP40_IDLE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CMP40::Classify( void )
{
    return CLASS_AUTO_RIFLE;
}

class CMP40AmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_MP40].ammo_maxclip, "ammo_44mm", WpnInfo[WEAPON_MP40].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_mp40, CMP40AmmoClip )