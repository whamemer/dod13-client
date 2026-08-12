//
// k43.cpp
//
// implementation of CK43 class
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

LINK_ENTITY_TO_CLASS( weapon_k43, CK43 )

enum K43_e
{
    K43_IDLE,
    K43_SHOOT1,
    K43_SHOOT2,
    K43_RELOAD,
    K43_DRAW,
    K43_SHOOT_EMPTY,
    K43_SMASH
};

void CK43::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_K43].wmodel );
    m_iId = WEAPON_K43;
    m_iDefaultAmmo = WpnInfo[WEAPON_K43].ammo_default;
    FallInit();
}

int CK43::AddToPlayer( CBasePlayer *pPlayer )
{
    if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
        SET_MODEL( ENT( pev ), WpnInfo[WEAPON_K43].wmodel );
		return TRUE;
	}
	return FALSE;
}

void CK43::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_K43].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_K43].wmodel );
    PRECACHE_SOUND( "weapons/k43_shoot.wav" );
    m_usFireK43 = PRECACHE_EVENT( 1, "events/weapons/k43.sc" );
}

int CK43::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_66mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_K43].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_K43].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_K43;
    p->iWeight = WpnInfo[WEAPON_K43].misc_weight;
    return 1;
}

BOOL CGreaseGun::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    return TimedDeploy( WpnInfo[WEAPON_K43].vmodel, WpnInfo[WEAPON_K43].pmodel, K43_DRAW, WpnInfo[WEAPON_K43].szAnimExt, WpnInfo[WEAPON_K43].szAnimReloadExt, 0.8f, 0.8f, 1 );
}

BOOL CK43::CanHolster( void )
{
    return TRUE;
}

void CK43::PrimaryAttack( void )
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
                flSpread = WpnInfo[WEAPON_K43].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_K43, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireK43, 0.0f, g_vecZero, g_vecZero, 0, 0, m_pPlayer->pev->punchangle.x, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_K43].anim_firedelay;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_K43].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CK43::SecondaryAttack( void )
{
    RifleMeleeAttack( m_pPlayer, WEAPON_K43 );
    m_pPlayer->SetAnimation( PLAYER_BUT_SWING );
    PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireK43, 0.0f, g_vecZero, g_vecZero, 0, 0, 1, 0, m_iClip == 0, 0 );
    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1f;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;
}

void CK43::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_K43].ammo_maxclip, K43_RELOAD, WpnInfo[WEAPON_K43].anim_reloadtime );
}

void CK43::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( K43_IDLE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CK43::Classify( void )
{
    return CLASS_RIFLE;
}

class CK43AmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_K43].ammo_maxclip, "ammo_66mm", WpnInfo[WEAPON_K43].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_k43, CK43AmmoClip )