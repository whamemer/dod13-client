//
// stickgrenade.cpp
//
// implementation of CStickGrenade class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

extern p_wpninfo_s *WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_stickgrenade, CStickGrenade )

void CStickGrenade::Spawn( void )
{
    Precache();
    m_iId = WEAPON_STICKGRENADE;
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_STICKGRENADE].wmodel );
    m_iDefaultAmmo = 1;
    FallInit();
}

void CStickGrenade::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_STICKGRENADE].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_STICKGRENADE].wmodel );

    PRECACHE_SOUND( "weapons/grenthrow.wav" );
}

int CStickGrenade::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_ggrens";
    p->iMaxAmmo1 = 5;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = -1;
    p->iSlot = 4;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_GRENADE;
    p->iId = m_iId = WEAPON_STICKGRENADE;
    p->iWeight = 1;
    return 1;
}

BOOL CStickGrenade::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    m_flReleaseThrow = -1.0f;
    SendWeaponAnim( STICKGRENADE_DRAW, UseDecrement() != FALSE );
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.85f;
    return FALSE;
}

BOOL CStickGrenade::CanHolster( void )
{
    return m_flStartThrow == 0.0f;
}

// WHAMER: TODO
void CStickGrenade::DropGren( void )
{
    if( m_flStartThrow != 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
    {
        SendWeaponAnim( STICKGRENADE_THROW, UseDecrement() != FALSE );

        if( m_bUnderhand )
            m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
        else
            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        TraceResult tr;
        Vector vecSrc, vecThrow;

        float fl = m_pPlayer->pev->classname;

        UTIL_TraceLine( vecSrc, vecThrow + Vector( 0, 0, 0 ), ignore_monsters, ENT( pev ), &tr );

        if( tr.flFraction < 1.0f )
            tr.vecEndPos = vecSrc;

        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
        m_flStartThrow = 0.0f;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
    }
}

void CStickGrenade::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

    if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
    {
        SendWeaponAnim( HANDGRENADE_HOLSTER, UseDecrement() != FALSE );
    }
    else
    {
        m_pPlayer->ClearHasWeapon( WEAPON_STICKGRENADE );
        SetThink( &CBasePlayerItem::DestroyItem );
        pev->nextthink = gpGlobals->time + 0.1f;
    }
    EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM, 0, 100 );
}

void CStickGrenade::PrimaryAttack( void )
{
    StartThrow( FALSE );
}

void CStickGrenade::SecondaryAttack( void )
{
    StartThrow( TRUE );
}

void CStickGrenade::StartThrow( BOOL bUnderhand )
{
    if( m_flStartThrow == 0.0f )
    {
        m_flReleaseThrow = 0.0f;
        m_flStartThrow = 1.0f;
        m_bUnderhand = bUnderhand;
        SendWeaponAnim( STICKGRENADE_PINPULL, UseDecrement() != FALSE );
        m_flTimeWeaponIdle = gpGlobals->time + 0.1f;
    }
}

// WHAMER: TODO
void CStickGrenade::WeaponIdle( void )
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

BOOL CStickGrenade::CanDeploy( void )
{
    return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != FALSE;
}