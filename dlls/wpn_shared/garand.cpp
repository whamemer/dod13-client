//
// garand.cpp
//
// implementation of CGarand class
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

LINK_ENTITY_TO_CLASS( weapon_garand, CGarand )

enum GARAND_e
{
    GARAND_IDLE = 0,
    GARAND_SHOOT1,
    GARAND_SHOOT2,
    GARAND_SHOOT3,
    GARAND_SHOOT_EMPTY,
    GARAND_RELOAD,
    GARAND_DRAW,
    GARAND_IDLE_EMPTY,
    GARAND_DRAW_EMPTY,
    GARAND_SMASH,
    GARAND_SMASH_EMPTY
};

void CGarand::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_GARAND].wmodel );
    m_iId = WEAPON_GARAND;
    m_iDefaultAmmo = WpnInfo[WEAPON_GARAND].ammo_default;
    FallInit();
}

void CGarand::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_GARAND].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_GARAND].wmodel );

    PRECACHE_SOUND( "weapons/reloadgarand.wav" );
    PRECACHE_SOUND( "weapons/garand_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/garand_reload_clipding.wav" );
    PRECACHE_SOUND( "weapons/garand_reload_clipin.wav" );
    PRECACHE_SOUND( "weapons/butt_hit.wav" );

    m_usFireGarand = PRECACHE_EVENT( 1, "events/weapons/garand.sc" );
}

int CGarand::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_16mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_GARAND].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_GARAND].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_GARAND;
    p->iWeight = WpnInfo[WEAPON_GARAND].misc_weight;
    return 1;
}

int CGarand::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CGarand::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_GARAND].anim_drawtime;

    return DefaultDeploy( WpnInfo[WEAPON_GARAND].vmodel, WpnInfo[WEAPON_GARAND].pmodel, GARAND_DRAW, WpnInfo[WEAPON_GARAND].szAnimExt, WpnInfo[WEAPON_GARAND].szAnimReloadExt, 0 );
}

BOOL CGarand::CanHolster( void )
{
    return TRUE;
}

void CGarand::PrimaryAttack( void )
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
                flSpread = WpnInfo[WEAPON_GARAND].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_GARAND, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireGarand, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_GARAND].anim_firedelay;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_GARAND].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CGarand::SecondaryAttack( void )
{
    RifleMeleeAttack( m_pPlayer, WEAPON_GARAND );
    m_pPlayer->SetAnimation( PLAYER_BUT_SWING );
    PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireGarand, 0.0f, g_vecZero, g_vecZero, 0, 0, 1, 0, m_iClip == 0, 0 );
    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1f;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;
}

void CGarand::Reload( void )
{
    if( m_iClip < 0 )
        DefaultReload( WpnInfo[WEAPON_GARAND].ammo_maxclip, GARAND_RELOAD, WpnInfo[WEAPON_GARAND].anim_reloadtime );
}

void CGarand::WeaponIdle( void )
{
    int iAnim = GARAND_IDLE_EMPTY;
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        if( m_iClip > 0 )
            iAnim = GARAND_IDLE;
        
        SendWeaponAnim( iAnim );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CGarand::Classify( void )
{
    return CLASS_RIFLE;
}

class CGarandAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_GARAND].ammo_maxclip, "ammo_16mm", WpnInfo[WEAPON_GARAND].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_garand, CGarandAmmoClip )