/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_enfield, CENFIELD )
LINK_ENTITY_TO_CLASS( weapon_scopedenfield, CENFIELD )

extern p_wpninfo_s P_WpnInfo[];
extern float flBoltHideXHair;

void CENFIELD::Spawn( void )
{
	Precache();

	if( m_iWeaponState & WPNSTATE_SCOPED )
		SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_SCOPEDENFIELD].wmodel );
	else
		SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_ENFIELD].wmodel );

	m_iId = WEAPON_ENFIELD;

	m_iDefaultAmmo = P_WpnInfo[WEAPON_ENFIELD].ammo_default;

	FallInit();
}

void CENFIELD::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_ENFIELD].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_ENFIELD].wmodel );

	m_usFireEnfield = PRECACHE_EVENT( 1, "events/weapons/enfield.sc" );

	PRECACHE_SOUND( "weapons/enfield_shoot.wav" );
	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( "weapons/boltforward.wav" );
	PRECACHE_SOUND( "weapons/boltback.wav" );
	PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

	PRECACHE_MODEL( P_WpnInfo[WEAPON_SCOPEDENFIELD].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_SCOPEDENFIELD].wmodel );

	PRECACHE_SOUND( "weapons/enfieldsniper_shoot.wav" );

	m_usFireScopedEnfield = PRECACHE_EVENT( 1, "events/weapons/scopedenfield.sc" );
}

int CENFIELD::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_16mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_ENFIELD].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_ENFIELD].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_ENFIELD;
	p->iWeight = P_WpnInfo[WEAPON_ENFIELD].misc_weight;

	return 1;
}

int CENFIELD::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return TRUE;

	return FALSE;
}

BOOL CENFIELD::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_SCOPEDENFIELD].anim_drawtime;

		return DefaultDeploy( P_WpnInfo[WEAPON_SCOPEDENFIELD].vmodel, P_WpnInfo[WEAPON_SCOPEDENFIELD].pmodel, ENFIELD_DRAW, P_WpnInfo[WEAPON_SCOPEDENFIELD].szAnimExt, P_WpnInfo[WEAPON_SCOPEDENFIELD].szAnimReloadExt, 0 );
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_ENFIELD].anim_drawtime;

	return DefaultDeploy( P_WpnInfo[WEAPON_ENFIELD].vmodel, P_WpnInfo[WEAPON_ENFIELD].pmodel, ENFIELD_DRAW, P_WpnInfo[WEAPON_ENFIELD].szAnimExt, P_WpnInfo[WEAPON_ENFIELD].szAnimReloadExt, 0 );
}

BOOL CENFIELD::CanHolster( void )
{
	return TRUE;
}

void CENFIELD::PrimaryAttack( void )
{
	float flSpread;
	vec3_t vecSrc;
	vec3_t vecAiming;
	vec3_t vecNewShot;

	if( PlayerIsWaterSniping() )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4f;
		return;
	}

	if( m_fInAttack )
		return;

	if( m_iClip <= 0 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		m_fInAttack = 1;
		return;
	}

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = 600;

	m_iClip--;

	flSpread = P_WpnInfo[WEAPON_ENFIELD].base_accuracy;

	if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		if( GetFOV() <= 0 )
			flSpread = P_WpnInfo[WEAPON_SCOPEDENFIELD].base_accuracy;
		else
			flSpread = P_WpnInfo[WEAPON_SCOPEDENFIELD].base_accuracy2;
	}

	if( m_pPlayer->pev->velocity.Length() > 45.0f )
		flSpread += P_WpnInfo[WEAPON_ENFIELD].accuracy_penalty;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_ENFIELD, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireScopedEnfield, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, 0, 0, m_iClip == 0, GetFOV() > 0 );

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_SCOPEDENFIELD].anim_firedelay;
	}
	else
	{
		PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireEnfield, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, 0, 0, m_iClip == 0, 0 );

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_ENFIELD].anim_firedelay;
	}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_ENFIELD].anim_firedelay;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	m_fInAttack = 1;

	flBoltHideXHair = P_WpnInfo[WEAPON_KAR].anim_firedelay;

	if( ( m_iWeaponState & WPNSTATE_SCOPED ) && GetFOV() > 0 )
	{
		if( m_iClip <= 0 )
			SetThink( &CBasePlayerWeapon::ThinkZoomOut );
		else
			SetThink( &CENFIELD::ThinkZoomOutIn );

		pev->nextthink = gpGlobals->time + 0.5f;
	}

	RemoveStamina( 2.0f, m_pPlayer );
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
		RifleMeleeAttack( m_pPlayer, 25 );

		m_pPlayer->SetAnimation( PLAYER_ATTACK2 );

		PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireEnfield, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, 1, 0, m_iClip == 0, 0 );

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1f;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;
	}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2f;
}

void CENFIELD::Reload( void )
{
	float fDelay;

	if( m_iWeaponState & WPNSTATE_SCOPED )
	{
		SetThink( NULL );

		if( GetFOV() > 0 )
		{
			m_pPlayer->m_iFOV = ZoomOut();
			UpdateZoomSpeed();
		}

		fDelay = P_WpnInfo[WEAPON_SCOPEDENFIELD].anim_reloadtime;
	}
	else
	{
		fDelay = P_WpnInfo[WEAPON_ENFIELD].anim_reloadtime;
	}

	DefaultReload( P_WpnInfo[WEAPON_ENFIELD].ammo_maxclip, ENFIELD_RELOAD, fDelay );
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

	SetThink( NULL );

	CBasePlayerWeapon::Holster( skiplocal );
}

void CENFIELD::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( ENFIELD_IDLE, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	}
}

int CENFIELD::Classify( void )
{
	return ( m_iWeaponState & WPNSTATE_SCOPED ) == 0 ? CLASS_RIFLE : CLASS_SCOPED_RIFLE;
}

class CEnfieldAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_ENFIELD].ammo_maxclip, "ammo_16mm", P_WpnInfo[WEAPON_ENFIELD].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_enfield, CEnfieldAmmoClip )
