//
// thompson.cpp
//
// implementation of CThompson class
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

LINK_ENTITY_TO_CLASS( weapon_thompson, CThompson )

enum THOMPSON_e
{
    THOMPSON_IDLE = 0,
    THOMPSON_RELOAD,
    THOMPSON_DRAW,
    THOMPSON_SHOOT1,
    THOMPSON_SHOOT2,
    THOMPSON_IDLE_EMPTY,
    THOMPSON_PRONE_BACK,
    THOMPSON_PRONE_FORWARD,
    THOMPSON_PRONE_STILL
};

void CThompson::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_THOMPSON].wmodel );
    m_iId = WEAPON_THOMPSON;
    m_iDefaultAmmo = WpnInfo[WEAPON_THOMPSON].ammo_default;
    FallInit();
}

void CThompson::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_THOMPSON].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_THOMPSON].wmodel );

    PRECACHE_SOUND( "weapons/reloadgarand.wav" );
    PRECACHE_SOUND( "weapons/thompson_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

    m_usFireThompson = PRECACHE_EVENT( 1, "events/weapons/thompson.sc" );
}

int CThompson::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_44mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_THOMPSON].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_THOMPSON].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_THOMPSON;
    p->iWeight = WpnInfo[WEAPON_THOMPSON].misc_weight;
    return 1;
}

int CThompson::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CThompson::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_THOMPSON].anim_drawtime;
    return DefaultDeploy( WpnInfo[WEAPON_THOMPSON].vmodel, WpnInfo[WEAPON_THOMPSON].pmodel, THOMPSON_DRAW, WpnInfo[WEAPON_THOMPSON].szAnimExt, 
        WpnInfo[WEAPON_THOMPSON].szAnimReloadExt, 0 );
}

BOOL CThompson::CanHolster( void )
{
    return TRUE;
}

void CThompson::PrimaryAttack( void )
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
                flSpread = WpnInfo[WEAPON_THOMPSON].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_THOMPSON, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireThompson, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_THOMPSON].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CThompson::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_THOMPSON].ammo_maxclip, THOMPSON_RELOAD, WpnInfo[WEAPON_THOMPSON].anim_reloadtime );
}

void CThompson::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( m_iClip == 0 ? THOMPSON_IDLE_EMPTY : THOMPSON_IDLE, UseDecrement() != FALSE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CThompson::Classify( void )
{
    return CLASS_AUTO_RIFLE;
}

class CThompsonAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_THOMPSON].ammo_maxclip, "ammo_44mm", WpnInfo[WEAPON_THOMPSON].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_thompson, CThompsonAmmoClip )