//
// stickgrenade_ex.cpp
//
// implementation of CStickGrenadeEx class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"

#include "dod_shared.h"

extern p_wpninfo_s *WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_stickgrenade_ex, CStickGrenadeEx )

void CStickGrenadeEx::Spawn( void )
{
    Precache();
    m_iId = WEAPON_STICKGRENADEX;
    SET_MODEL( ENT( pev ), WpnInfo[WEAPON_STICKGRENADEX].wmodel );
    m_iDefaultAmmo = 1;
    FallInit();
}

void CStickGrenadeEx::Precache( void )
{
    PRECACHE_SOUND( "weapons/grenthrow.wav" );
}

int CStickGrenadeEx::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_ggrensex";
    p->iMaxAmmo1 = 1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = -1;
    p->iSlot = 4;
    p->iPosition = 4;
    p->iFlags = ITEM_FLAG_GRENADE;
    p->iId = m_iId = WEAPON_STICKGRENADEX;
    p->iWeight = 100;
    return 1;
}

BOOL CStickGrenadeEx::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    m_flReleaseThrow = -1.0f;
    m_flStartThrow = 0.0f;
    return FALSE;
}

void CStickGrenadeEx::PrimaryAttack( void )
{
    StartThrow( FALSE );
}

void CStickGrenadeEx::SecondaryAttack( void )
{
    StartThrow( TRUE );
}

void CStickGrenadeEx::StartThrow( BOOL bUnderhand )
{
    if( m_flTimeToExplode < gpGlobals->time )
        DropGren();

    if( m_flStartThrow == 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
    {
        m_flReleaseThrow = 0.0f;
        m_flStartThrow = 1.0f;
        m_bUnderhand = bUnderhand;
        SendWeaponAnim( STICKGRENADE_EX_PINPULL, UseDecrement() != FALSE );

        if( m_flTimeToExplode >= gpGlobals->time )
            m_flReleaseThrow = gpGlobals->time + 0.01f;
    }
}

// WHAMER: TODO
void CStickGrenadeEx::DropGren( void )
{

}

void CStickGrenadeEx::WeaponIdle( void )
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
            SendWeaponAnim( STICKGRENADE_EX_IDLE, UseDecrement() != FALSE );
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

BOOL CStickGrenadeEx::CanDeploy( void )
{
    return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != FALSE;
}