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

#include "dod_shared.h"

extern p_wpninfo_s *WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_handgrenade, CHandGrenade )

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
    vec3_t eyes, vecSrc;
    TraceResult tr;

    if( m_flStartThrow != 0.0f && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
    {
        SendWeaponAnim( HANDGRENADE_THROW, UseDecrement() != FALSE );

        if( m_bUnderhand )
            m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
        else
            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        eyes = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
        vecSrc = eyes + gpGlobals->v_forward * 16.0f;

        UTIL_TraceLine( eyes, vecSrc, ignore_monsters, m_pPlayer->pev->pContainingEntity, &tr );

        if( tr.flFraction < 1.0f )
            vecSrc = tr.vecEndPos;

        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
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

void CHandGrenade::WeaponIdle( void )
{
	vec3_t eyes;
	float time;
	TraceResult tr;

	time = gpGlobals->time;

	if( m_flReleaseThrow == 0.0f )
		m_flReleaseThrow = time;

	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
	{
		if( m_flReleaseThrow > 0.0f && m_flStartThrow == 0.0f && time > m_flTimeWeaponIdle )
		{
			m_flStartThrow = 0.0f;
			RetireWeapon();
		}
		return;
	}

	if( m_flTimeWeaponIdle > time )
		return;

	if( m_flTimeWeaponIdle != time && m_flStartThrow == 1.0f && m_flReleaseThrow != 0.0f )
	{
		m_flStartThrow = time + 0.5f;

		if( m_bUnderhand )
			m_pPlayer->SetAnimation( PLAYER_ROLLGRENADE );
		else
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if( angThrow.x >= 0.0f )
			angThrow.x = angThrow.x * 1.11f - 10.0f;
		else
			angThrow.x = angThrow.x * 0.89f - 10.0f;

		flVel = ( 90.0f - angThrow.x ) * 4.0f;

		if( flVel > 600.0f )
			flVel = 600.0f;

		UTIL_MakeVectors( angThrow );

		float flPlayerSpeedBonus = DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );
		vecThrow = gpGlobals->v_forward * ( flPlayerSpeedBonus + flVel );
		return;
	}

	if( time > m_flStartThrow && m_flStartThrow != 0.0f && m_flStartThrow != 1.0f )
	{
		angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if( angThrow.x < 0.0f )
			angThrow.x = angThrow.x * 0.89 - 10.0f;
		else
			angThrow.x = angThrow.x * 1.11 - 10.0f;

		UTIL_MakeVectors( angThrow );

		eyes = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
		vecSrc = eyes + gpGlobals->v_forward * 16.0f;

		if( m_bUnderhand )
		{
			vecSrc = eyes + gpGlobals->v_forward * 16.0f;
			vecThrow = gpGlobals->v_forward * 300.0f;
		}

		UTIL_TraceLine( eyes, vecSrc, ignore_monsters, m_pPlayer->pev->pContainingEntity, &tr );

		if( tr.flFraction < 1.0f )
			vecSrc = tr.vecEndPos;

		EMIT_SOUND( m_pPlayer->pev->pContainingEntity, CHAN_WEAPON, "weapons/grenthrow.wav", 0.8f, ATTN_NORM );
		SendWeaponAnim( HANDGRENADE_THROW, UseDecrement() != FALSE );

		m_flStartThrow = 0.0f;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		return;
	}

	if( m_flStartThrow == 0.0f )
	{
		if( m_flReleaseThrow < 0.0f )
		{
			m_flTimeWeaponIdle = time + RANDOM_FLOAT( 5.0f, 7.0f );

			SendWeaponAnim( HANDGRENADE_DRAW, UseDecrement() != FALSE );
			return;
		}

		if( m_flReleaseThrow > 0.0f && time > m_flTimeWeaponIdle )
		{
			m_flTimeWeaponIdle = time + RANDOM_FLOAT( 5.0f, 7.0f );;

			SendWeaponAnim( HANDGRENADE_IDLE, UseDecrement() != FALSE );
			m_flReleaseThrow = -1.0f;
		}
	}
}

BOOL CHandGrenade::CanDeploy( void )
{
    return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != FALSE;
}