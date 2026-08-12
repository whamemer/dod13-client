//
// scoped98k.cpp
//
// implementation of CScopedKar class
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

LINK_ENTITY_TO_CLASS( weapon_scopedkar, CScopedKar )

enum SCOPEDKAR_e
{
    SCOPEDKAR_IDLE = 0,
    SCOPEDKAR_SHOOT,
    SCOPEDKAR_RELOAD,
    SCOPEDKAR_DRAW,
    SCOPEDKAR_STABSLASH,
    SCOPEDKAR_STAB,
    SCOPEDKAR_LOWER_FOR_ZOOM
};

void CScopedKar::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_SCOPEDKAR].wmodel );
    m_iId = WEAPON_SCOPEDKAR;
    m_iDefaultAmmo = WpnInfo[WEAPON_SCOPEDKAR].ammo_default;
    FallInit();
}

void CScopedKar::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_SCOPEDKAR].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_SCOPEDKAR].wmodel );

    PRECACHE_SOUND( "weapons/kar_shoot.wav" );
    PRECACHE_SOUND( "weapons/boltforward.wav" );
    PRECACHE_SOUND( "weapons/boltback.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

    m_usFireScopedKar = PRECACHE_EVENT( 1, "events/weapons/scopedkar.sc" );
}

int CScopedKar::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_16mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_SCOPEDKAR].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_SCOPEDKAR].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_SCOPEDKAR;
    p->iWeight = WpnInfo[WEAPON_SCOPEDKAR].misc_weight;
    return 1;
}

int CScopedKar::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CScopedKar::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SCOPEDKAR].anim_drawtime;
    return DefaultDeploy( WpnInfo[WEAPON_SCOPEDKAR].vmodel, WpnInfo[WEAPON_SCOPEDKAR].pmodel, SCOPEDKAR_DRAW, WpnInfo[WEAPON_SCOPEDKAR].szAnimExt, 
        WpnInfo[WEAPON_SCOPEDKAR].szAnimReloadExt, 0 );
}

BOOL CScopedKar::CanHolster( void )
{
    return TRUE;
}

void CScopedKar::PrimaryAttack( void )
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
            flSpread = WpnInfo[WEAPON_SCOPEDKAR].base_accuracy;

            if( m_pPlayer->pev->origin.Length() > 45.0f )
                flSpread += WpnInfo[WEAPON_SCOPEDKAR].accuracy_penalty;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_SCOPEDKAR, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireScopedKar, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, GetFOV() > 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SCOPEDKAR].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SCOPEDKAR].anim_firedelay;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            flBoltHideXHair = WpnInfo[WEAPON_SCOPEDKAR].anim_firedelay;

            if( GetFOV() > 0 )
            {
                if( m_iClip < 0 )
                    SetThink( &CBasePlayerWeapon::ThinkZoomOut );
                else
                    SetThink( &CScopedKar::ThinkZoomOutIn );

                pev->nextthink = gpGlobals->time + 0.5f;
            }
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CScopedKar::SecondaryAttack( void )
{
    m_pPlayer->m_iFOV = ChangeFOV( 20 );
    UpdateZoomSpeed();
    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2f;
}

void CScopedKar::Reload( void )
{
    if( GetFOV() > 0 )
    {
        m_pPlayer->m_iFOV = ZoomOut();
        UpdateZoomSpeed();
    }

    DefaultReload( WpnInfo[WEAPON_SCOPEDKAR].ammo_maxclip, SCOPEDKAR_RELOAD, WpnInfo[WEAPON_SCOPEDKAR].anim_reloadtime );
}

void CScopedKar::ThinkZoomOutIn( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    SetThink( &CBasePlayerWeapon::ThinkZoomIn );
    pev->nextthink = gpGlobals->time + 1.1f;
}

void CScopedKar::Holster( int skiplocal )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
    CBasePlayerWeapon::Holster( skiplocal );
}

void CScopedKar::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( SCOPEDKAR_IDLE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CScopedKar::Classify( void )
{
    return CLASS_SCOPE_RIFLE;
}

class CScopedKarAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_SCOPEDKAR].ammo_maxclip, "ammo_22mm", WpnInfo[WEAPON_SCOPEDKAR].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_spring, CScopedKarAmmoClip )