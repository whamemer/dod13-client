//
// handgrenade.cpp
//
// implementation of CHandGrenade class
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

LINK_ENTITY_TO_CLASS( weapon_handgrenade, CHandGrenade )

enum handgrenade_e
{
    HANDGRENADE_IDLE = 0,
    HANDGRENADE_DRAW,
    HANDGRENADE_PINPULL,
    HANDGRENADE_HOLSTER,
    HANDGRENADE_THROW,
    HANDGRENADE_EX_IDLE,
    HANDGRENADE_EX_DRAW,
    HANDGRENADE_EX_PINPULL,
    HANDGRENADE_EX_THROW
};

void CHandGrenade::Spawn( void )
{
    Precache();
    m_iId = WEAPON_HANDGRENADE;
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_HANDGRENADE].wmodel );
    m_iDefaultAmmo = 1;
    FallInit();
}

void CHandGrenade::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_HANDGRENADE].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_HANDGRENADE].wmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_MILLSGRENADE].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_MILLSGRENADE].wmodel );

    PRECACHE_SOUND( "weapons/grenthrow.wav" );
}

int CHandGrenade::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_agrens";
    p->iMaxAmmo1 = 3;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = -1;
    p->iSlot = 4;
    p->iPosition = 1;
    p->iFlags = ITEM_FLAG_GRENADE;
    p->iId = m_iId = WEAPON_HANDGRENADE;
    p->iWeight = 1;
    return 1;
}

BOOL CHandGrenade::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    m_flReleaseThrow = -1.0f;
    SendWeaponAnim( HANDGRENADE_DRAW, UseDecrement() != FALSE );
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.85f;
    return FALSE;
}

BOOL CHandGrenade::CanHolster( void )
{
    return m_flStartThrow == 0.0f;
}

void CHandGrenade::DropGren( void )
{
    if( m_flStartThrow != 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
    {
        SendWeaponAnim( HANDGRENADE_THROW, UseDecrement() != FALSE );

        if( m_bUnderhand )
            m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
        else
            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        
        TraceResult tr;

        /* WHAMER: TODO
        m_pPlayer = this->m_pPlayer;
        p_classname = &m_pPlayer->pev->classname;
        v5 = p_classname[4] + p_classname[95];
        v6 = p_classname[3] + p_classname[94];
        v7 = p_classname[2] + p_classname[93];
        vecStart.x = v7;
        v8 = v7;
        vecStart.y = v6;
        vecStart.z = v5;
        v9 = v5 + gpGlobals->v_forward.z * 16.0;
        v10 = v6 + gpGlobals->v_forward.y * 16.0;
        vecEnd.x = v8 + 16.0 * gpGlobals->v_forward.x;
        vecEnd.y = v10;
        vecEnd.z = v9;
        UTIL_TraceLine(&vecStart, &vecEnd, ignore_monsters, m_pPlayer->pev->pContainingEntity, &ptr);
        if ( ptr.flFraction < 1.0 )
            vecEnd = ptr.vecEndPos;*/
        
        --m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
        m_flStartThrow = 0.0f;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
    }
}

void CHandGrenade::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

    if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
    {
        SendWeaponAnim( HANDGRENADE_HOLSTER, UseDecrement() != FALSE );
    }
    else
    {
        m_pPlayer->ClearHasWeapon( WEAPON_HANDGRENADE );
        SetThink( &CBasePlayerItem::DestroyItem );
        pev->nextthink = gpGlobals->time + 0.1f;
    }
    EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM, 0, 100 );
}

void CHandGrenade::PrimaryAttack( void )
{
    StartThrow( FALSE );
}

void CHandGrenade::SecondaryAttack( void )
{
    StartThrow( TRUE );
}

void CHandGrenade::StartThrow( BOOL bUnderhand )
{
    if( m_flStartThrow == 0.0f )
    {
        m_flReleaseThrow = 0.0f;
        m_flStartThrow = 1.0f;
        m_bUnderhand = bUnderhand;
        SendWeaponAnim( HANDGRENADE_PINPULL, UseDecrement() != FALSE );
        m_flTimeWeaponIdle = gpGlobals->time + 0.1f;
    }
}

// WHAMER: TODO
void CHandGrenade::WeaponIdle( void )
{
    if( m_flReleaseThrow == 0.0f ) m_flReleaseThrow = gpGlobals->time;
    
    /*if( ( m_pPlayer->m_pGoalEnt + m_iPrimaryAmmoType + 284 ) <= 0 )
    {

    }*/
    
    if( m_flTimeWeaponIdle > gpGlobals->time )
        return;
    
    if( m_flTimeWeaponIdle > gpGlobals->time && m_flStartThrow == 1.0f )
    {
        if( m_flReleaseThrow != 0.0f )
        {
            m_flStartThrow = gpGlobals->time + 0.5f;

            if( m_bUnderhand )
                m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
            else
                m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
            

        }
    }
}

BOOL CHandGrenade::CanDeploy( void )
{
    return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != FALSE;
}