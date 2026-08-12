//
// enfield.cpp
//
// implementation of CENFIELD class
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

LINK_ENTITY_TO_CLASS( weapon_enfield, CENFIELD )
LINK_ENTITY_TO_CLASS( weapon_scopedenfield, CENFIELD )

enum ENFIELD_e
{
    ENFIELD_IDLE = 0,
    ENFIELD_SHOOT,
    ENFIELD_RELOAD,
    ENFIELD_DRAW,
    ENFIELD_RELOADLONG,
    ENFIELD_SWING
};

enum SCOPEDENFIELD_e
{
    SCOPEDENFIELD_IDLE = 0,
    SCOPEDENFIELD_SHOOT,
    SCOPEDENFIELD_RELOAD,
    SCOPEDENFIELD_DRAW,
    SCOPEDENFIELD_RELOADLONG
};

void CENFIELD::Spawn( void )
{
    Precache();

    if( m_iWeaponState & WPNSTATE_SCOPED )
        SET_MODEL( ENT( pev ), WpnInfo[WEAPON_SCOPEDENFIELD].wmodel );
    else
        SET_MODEL( ENT( pev ), WpnInfo[WEAPON_ENFIELD].wmodel );

    m_iId = WEAPON_ENFIELD;
    m_iDefaultAmmo = WpnInfo[WEAPON_ENFIELD].ammo_default;
    FallInit();
}

void CENFIELD::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_ENFIELD].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_ENFIELD].wmodel );

    m_usFireEnfield = PRECACHE_EVENT( 1, "events/weapons/enfield.sc" );

    PRECACHE_SOUND( "weapons/enfield_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/boltforward.wav" );
    PRECACHE_SOUND( "weapons/boltback.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

    PRECACHE_MODEL( WpnInfo[WEAPON_SCOPEDENFIELD].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_SCOPEDENFIELD].wmodel );

    PRECACHE_SOUND( "weapons/enfieldsniper_shoot.wav" );

    m_usFireScopedEnfield = PRECACHE_EVENT( 1, "events/weapons/scopedenfield.sc" );
}

int CENFIELD::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_16mm";
    p->iMaxAmmo1 = WpnInfo[WEAPON_ENFIELD].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WpnInfo[WEAPON_ENFIELD].ammo_maxclip;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_RIFLE;
    p->iId = m_iId = WEAPON_ENFIELD;
    p->iWeight = WpnInfo[WEAPON_ENFIELD].misc_weight;
    return 1;
}

int CENFIELD::AddToPlayer( CBasePlayer *pPlayer )
{
    return CBasePlayerWeapon::AddToPlayer( pPlayer ) != 0;
}

BOOL CENFIELD::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();

    if( m_iWeaponState & WPNSTATE_SCOPED )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SCOPEDENFIELD].anim_drawtime;
        return DefaultDeploy( WpnInfo[WEAPON_SCOPEDENFIELD].vmodel, WpnInfo[WEAPON_SCOPEDENFIELD].pmodel, SCOPEDENFIELD_DRAW, 
            WpnInfo[WEAPON_SCOPEDENFIELD].szAnimExt, WpnInfo[WEAPON_SCOPEDENFIELD].szAnimReloadExt, 0 );
    }
    else
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_ENFIELD].anim_drawtime;
        return DefaultDeploy( WpnInfo[WEAPON_ENFIELD].vmodel, WpnInfo[WEAPON_ENFIELD].pmodel, ENFIELD_DRAW, 
            WpnInfo[WEAPON_ENFIELD].szAnimExt, WpnInfo[WEAPON_ENFIELD].szAnimReloadExt, 0 );
    }
}

BOOL CENFIELD::CanHolster( void )
{
    return TRUE;
}

void CENFIELD::PrimaryAttack( void )
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
            flSpread = WpnInfo[WEAPON_ENFIELD].base_accuracy;

            if( m_iWeaponState & WPNSTATE_SCOPED )
            {
                if( GetFOV() <= 0 )
                    flSpread = WpnInfo[WEAPON_SCOPEDENFIELD].base_accuracy;
                else
                    flSpread = WpnInfo[WEAPON_SCOPEDENFIELD].base_accuracy2;
            }

            if( m_pPlayer->pev->origin.Length() > 45.0f )
                flSpread += WpnInfo[WEAPON_ENFIELD].accuracy_penalty;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            FireBulletsNC( vecSrc, gpGlobals->v_forward, flSpread, 8192.0f, BULLET_PLAYER_ENFIELD, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

            if( m_iWeaponState & WPNSTATE_SCOPED )
            {
                PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireScopedEnfield, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, GetFOV() > 0 );
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_SCOPEDENFIELD].anim_firedelay;
            }
            else
            {
                PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireEnfield, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_ENFIELD].anim_firedelay;
            }

            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + WpnInfo[WEAPON_ENFIELD].anim_firedelay;
            m_fInAttack = TRUE;
            m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            flBoltHideXHair = WpnInfo[WEAPON_ENFIELD].anim_firedelay;

            if( m_iWeaponState & WPNSTATE_SCOPED || GetFOV() > 0 )
            {
                if( m_iClip < 0 )
                    SetThink( &CBasePlayerWeapon::ThinkZoomOut );
                else
                    SetThink( &CENFIELD::ThinkZoomOutIn );

                pev->nextthink = gpGlobals->time + 0.5f;
            }
            RemoveStamina( 1.0f, m_pPlayer );
        }
    }
}

void CENFIELD::SecondaryAttack( void )
{
    if( m_iWeaponState & WPNSTATE_SCOPED )
    {
        m_pPlayer->m_iFOV = ChangeFOV( 20 );
        UpdateZoomSpeed();
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
    }
    else
    {
        RifleMeleeAttack( m_pPlayer, WEAPON_ENFIELD );
        m_pPlayer->SetAnimation( PLAYER_ATTACK2 );
        PLAYBACK_EVENT_FULL( 1, ENT( m_pPlayer->pev ), m_usFireEnfield, 0.0f, g_vecZero, g_vecZero, 0, 0, 1, 0, m_iClip == 0, 0 );
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1f;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;
    }
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2f;
}

void CENFIELD::Reload( void )
{
    float fAnimReloadTime;

    if( m_iWeaponState & WPNSTATE_SCOPED )
    {
        if( GetFOV() > 0 )
        {
            m_pPlayer->m_iFOV = ZoomOut();
            UpdateZoomSpeed();
        }

        fAnimReloadTime = WpnInfo[WEAPON_SCOPEDENFIELD].anim_reloadtime;
    }
    else
    {
        fAnimReloadTime = WpnInfo[WEAPON_ENFIELD].anim_reloadtime;
    }
    DefaultReload( WpnInfo[WEAPON_ENFIELD].ammo_maxclip, ENFIELD_RELOAD, fAnimReloadTime );
}

void CENFIELD::ThinkZoomOutIn( void )
{
    if( m_iWeaponState & WPNSTATE_SCOPED )
    {
        m_pPlayer->m_iFOV = ZoomOut();
        UpdateZoomSpeed();
        SetThink( &CBasePlayerWeapon::ThinkZoomIn );
        pev->nextthink = gpGlobals->time + 1.1f;
    }
}

void CENFIELD::Holster( int skiplocal )
{
    if( m_iWeaponState & WPNSTATE_SCOPED )
    {
        m_pPlayer->m_iFOV = ZoomOut();
        UpdateZoomSpeed();
    }
    CBasePlayerWeapon::Holster( skiplocal );
}

void CENFIELD::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( ENFIELD_IDLE );
        m_flTimeWeaponIdle = RANDOM_FLOAT( 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
    }
}

int CENFIELD::Classify( void )
{
    return m_iWeaponState & WPNSTATE_SCOPED ? CLASS_RIFLE : CLASS_SCOPE_RIFLE;
}

class CEnfieldAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( WpnInfo[WEAPON_ENFIELD].ammo_maxclip, "ammo_16mm", WpnInfo[WEAPON_ENFIELD].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_enfield, CEnfieldAmmoClip )