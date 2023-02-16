//
// spring.cpp
//
// implementation of CSPRING class
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

LINK_ENTITY_TO_CLASS( weapon_spring, CSPRING )

enum SPRING_e
{
    SPRING_IDLE = 0,
    SPRING_SHOOT1,
    SPRING_SHOOT2,
    SPRING_RELOAD,
    SPRING_DRAW,
    SPRING_xxx,
    SPRING_LOWER_FOR_ZOOM
};

void CSPRING::Spawn( void )
{
    Precache();
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_SPRING].wmodel );
    m_iId = WEAPON_SPRING;
    m_iDefaultAmmo = WpnInfo[WEAPON_SPRING].ammo_default;
    FallInit();
}

void CSPRING::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_SPRING].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_SPRING].wmodel );

    PRECACHE_SOUND( "weapons/reloadgarand.wav" );
    PRECACHE_SOUND( "weapons/spring_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/garand_reload_clipin.wav" );

    m_usFireSpring = PRECACHE_EVENT( 1, "events/weapons/spring.sc" );
}

int CSPRING::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_22mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_SPRING].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_SPRING].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_SPRING;
    p->iWeight = WpnInfo[WEAPON_SPRING].misc_weight;
    return 1;
}

int CSPRING::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CSPRING::Deploy( void )
{
    ThinkZoomOut();
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SPRING].anim_drawtime;
    return DefaultDeploy( WpnInfo[WEAPON_SPRING].vmodel, WpnInfo[WEAPON_SPRING].pmodel, SPRING_DRAW, WpnInfo[WEAPON_SPRING].szAnimExt, WpnInfo[WEAPON_SPRING].szAnimReloadExt, 0 );
}

BOOL CSPRING::CanHolster( void )
{
    return TRUE;
}

void CSPRING::PrimaryAttack( void )
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
            flSpread = WpnInfo[WEAPON_SPRING].base_accuracy;

            if( m_pPlayer->pev->origin.Length() > 45.0f )
                flSpread += WpnInfo[WEAPON_SPRING].accuracy_penalty;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_SPRING, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );
            PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireSpring, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SPRING].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SPRING].anim_firedelay;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            flBoltHideXHair = WpnInfo[WEAPON_SPRING].anim_firedelay;

            if( GetFOV() > 0 )
            {
                if( m_iClip < 0 )
                    SetThink( &CSPRING::ThinkZoomOut );
                else
                    SetThink( &CSPRING::ThinkZoomOutIn );

                pev->nextthink = gpGlobals->time + 0.5f;
            }
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CSPRING::SecondaryAttack( void )
{
    m_pPlayer->m_iFOV = ChangeFOV( 20 );
    UpdateZoomSpeed();
    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2f;
}

void CSPRING::Reload( void )
{
    if( GetFOV() > 0 )
        ThinkZoomOut();

    DefaultReload( WpnInfo[WEAPON_SPRING].ammo_maxclip, SPRING_RELOAD, WpnInfo[WEAPON_SPRING].anim_reloadtime );
}

void CSPRING::ThinkZoomOutIn( void )
{
    ThinkZoomOut();
    SetThink( &CBasePlayerWeapon::ThinkZoomIn );
    pev->nextthink = gpGlobals->time + 1.4f;
}

void CSPRING::ThinkZoomOut( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();
}

void CSPRING::Holster( int skiplocal )
{
    ThinkZoomOut();
    CBasePlayerWeapon::Holster( skiplocal );
}

void CSPRING::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( SPRING_IDLE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CSPRING::Classify( void )
{
    return CLASS_SCOPE_RIFLE;
}

class CSpringAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_SPRING].ammo_maxclip, "ammo_22mm", WpnInfo[WEAPON_SPRING].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_spring, CSpringAmmoClip )