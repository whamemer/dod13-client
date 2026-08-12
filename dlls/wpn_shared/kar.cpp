//
// kar.cpp
//
// implementation of CKAR class
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
extern float flBoltHideXHair;

LINK_ENTITY_TO_CLASS( weapon_kar, CKAR )

enum KAR_e
{
    KAR_IDLE = 0,
    KAR_SHOOT,
    KAR_RELOAD,
    KAR_DRAW,
    KAR_SWING,
    KAR_SLASH
};

void CKAR::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_KAR].wmodel );
    m_iId = WEAPON_KAR;
    m_iDefaultAmmo = WpnInfo[WEAPON_KAR].ammo_default;
    FallInit();
}

void CKAR::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_KAR].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_KAR].wmodel );

    PRECACHE_SOUND( "weapons/kar_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/boltforward.wav" );
    PRECACHE_SOUND( "weapons/boltback.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );
    PRECACHE_SOUND( "weapons/knifeswing.wav" );
    PRECACHE_SOUND( "weapons/knifeswing2.wav" );
    PRECACHE_SOUND( "weapons/knife_slash1.wav" );
    PRECACHE_SOUND( "weapons/knife_slash2.wav" );
    PRECACHE_SOUND( "weapons/knife_hit1.wav" );
    PRECACHE_SOUND( "weapons/knife_hit2.wav" );
    PRECACHE_SOUND( "weapons/knife_hit3.wav" );
    PRECACHE_SOUND( "weapons/cbar_hit1.wav" );
    PRECACHE_SOUND( "weapons/cbar_hit2.wav" );

    m_usFireKar = PRECACHE_EVENT( 1, "events/weapons/kar.sc" );
}

int CKAR::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_16mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_KAR].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_KAR].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_KAR;
    p->iWeight = WpnInfo[WEAPON_KAR].misc_weight;
    return 1;
}

int CKAR::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CKAR::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_KAR].anim_drawtime;
    return DefaultDeploy( WpnInfo[WEAPON_KAR].vmodel, WpnInfo[WEAPON_KAR].pmodel, KAR_DRAW, WpnInfo[WEAPON_KAR].szAnimExt, WpnInfo[WEAPON_KAR].szAnimReloadExt, 0 );
}

BOOL CKAR::CanHolster( void )
{
    return TRUE;
}

void CKAR::PrimaryAttack( void )
{
    float flSpread;

    if( PlayerIsWaterSniping() )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4f;
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
                flSpread = WpnInfo[WEAPON_KAR].base_accuracy + 0.1f;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_KAR, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireKar, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_KAR].anim_firedelay;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_KAR].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            flBoltHideXHair = WpnInfo[WEAPON_KAR].anim_firedelay;
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CKAR::SecondaryAttack( void )
{
    RifleMeleeAttack( m_pPlayer, WEAPON_KAR );
    m_pPlayer->SetAnimation( PLAYER_ATTACK2 );
    PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireKar, 0.0f, g_vecZero, g_vecZero, 0, 0, 1, 0, m_iClip == 0, 0 );
    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8f;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8f;
}

void CKAR::Reload( void )
{
    DefaultReload( WpnInfo[WEAPON_KAR].ammo_maxclip, KAR_RELOAD, WpnInfo[WEAPON_KAR].anim_reloadtime );
}

void CKAR::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( KAR_IDLE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CKAR::Classify( void )
{   
    return CLASS_RIFLE;
}

class CKarAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_KAR].ammo_maxclip, "ammo_16mm", WpnInfo[WEAPON_KAR].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_kar, CKarAmmoClip )