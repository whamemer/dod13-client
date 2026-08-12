//
// handgrenade_ex.cpp
//
// implementation of CHandGrenadeEx class
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

LINK_ENTITY_TO_CLASS( weapon_handgrenade_ex, CHandGrenadeEx )

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

void CHandGrenadeEx::Spawn( void )
{
    Precache();
    m_iId = WEAPON_HANDGRENADEX;
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_HANDGRENADEX].wmodel );
    m_iDefaultAmmo = 1;
    FallInit();
}

void CHandGrenadeEx::Precache( void )
{
    PRECACHE_SOUND( "weapons/grenthrow.wav" );
}

int CHandGrenadeEx::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_agrensex";
    p->iMaxAmmo1 = 1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = -1;
    p->iSlot = 4;
    p->iPosition = 3;
    p->iFlags = ITEM_FLAG_GRENADE;
    p->iId = m_iId = WEAPON_HANDGRENADEX;
    p->iWeight = 100;
    return 1;
}

BOOL CHandGrenadeEx::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    m_flReleaseThrow = -1.0f;
    m_flStartThrow = 0.0f;
    return FALSE;
}

BOOL CHandGrenadeEx::CanHolster( void )
{
    return FALSE;
}

void CHandGrenadeEx::Holster( int skiplocal )
{
    // Nothing.
}

void CHandGrenadeEx::PrimaryAttack( void )
{
    StartThrow( FALSE );
}

void CHandGrenadeEx::SecondaryAttack( void )
{
    StartThrow( TRUE );
}

void CHandGrenadeEx::StartThrow( BOOL bUnderhand )
{
    if( m_flTimeToExplode < gpGlobals->time )
        DropGren();

    if( m_flStartThrow == 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
    {
        m_flReleaseThrow = 0.0f;
        m_flStartThrow = 1.0f;
        m_bUnderhand = bUnderhand;
        SendWeaponAnim( HANDGRENADE_EX_PINPULL, UseDecrement() != FALSE );

        if( m_flTimeToExplode >= gpGlobals->time )
            m_flReleaseThrow = gpGlobals->time + 0.01f;
    }
}

// WHAMER: TODO
void CHandGrenadeEx::DropGren( void )
{

}

void CHandGrenadeEx::SetDmgTime( float time )
{
    m_flTimeToExplode = time;
}

void CHandGrenadeEx::WeaponIdle( void )
{
    if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
    {
        RetireWeapon();
        m_pPlayer->ClearHasWeapon( WEAPON_HANDGRENADEX );
        SetThink( &CBasePlayerItem::DestroyItem );
        pev->nextthink - gpGlobals->time + 0.1f;
        m_flStartThrow = 0.0f;
        return;
    }

    if( m_flTimeToExplode < gpGlobals->time )
        DropGren();
    
    if( m_flTimeWeaponIdle < gpGlobals->time )
    {
        if( m_flReleaseThrow == -1.0f )
        {
            m_flTimeWeaponIdle = gpGlobals->time + 1.0f;
            SendWeaponAnim( HANDGRENADE_EX_IDLE, UseDecrement() != FALSE );
        }
    }

    if( m_flReleaseThrow <= gpGlobals->time )
    {
        if( m_flStartThrow == 1.0f )
        {
            if( m_flReleaseThrow != -1.0f && m_flTimeToExplode >= gpGlobals->time )
            {
                if( m_bUnderhand )
                    m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
                else
                    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
            
                m_flStartThrow = 2.0f;
                m_flReleaseThrow = gpGlobals->time + 0.3f;
            }
        }
        else if( m_flStartThrow == 2.0f && m_flReleaseThrow != -1.0f && m_flTimeToExplode >= gpGlobals->time )
        {
            DropGren();
        }
    }
}

BOOL CHandGrenadeEx::CanDeploy( void )
{
    return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != FALSE;
}