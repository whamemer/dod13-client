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

LINK_ENTITY_TO_CLASS( weapon_spring, CSPRING )

extern p_wpninfo_s P_WpnInfo[];
extern float flBoltHideXHair;

void CSPRING::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_SPRING].wmodel );
	m_iId = WEAPON_SPRING;

	m_iDefaultAmmo = P_WpnInfo[WEAPON_SPRING].ammo_default;

	FallInit();
}

void CSPRING::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_SPRING].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_SPRING].wmodel );

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
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_SPRING].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_SPRING].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_SPRING;
	p->iWeight = P_WpnInfo[WEAPON_SPRING].misc_weight;

	return 1;
}

int CSPRING::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return TRUE;

	return FALSE;
}

BOOL CSPRING::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_SPRING].anim_drawtime;

	return DefaultDeploy( P_WpnInfo[WEAPON_SPRING].vmodel, P_WpnInfo[WEAPON_SPRING].pmodel, SPRING_DRAW, P_WpnInfo[WEAPON_SPRING].szAnimExt, P_WpnInfo[WEAPON_SPRING].szAnimReloadExt, 0 );
}

BOOL CSPRING::CanHolster( void )
{
	return TRUE;
}

void CSPRING::PrimaryAttack( void )
{
	float flSpread;
	vec3_t vecSrc;
	vec3_t vecAiming;
	vec3_t vecNewShot;

	if( PlayerIsWaterSniping() )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
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

	if( GetFOV() > 0 )
		flSpread = P_WpnInfo[m_iId].base_accuracy2;
	else
		flSpread = P_WpnInfo[m_iId].base_accuracy;

	if( m_pPlayer->pev->velocity.Length() > 45.0f )
		flSpread += P_WpnInfo[m_iId].accuracy_penalty;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_SPRING, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireSpring, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, 0, 0, m_iClip == 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_SPRING].anim_firedelay;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_SPRING].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;

	m_fInAttack = 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_SPRING].anim_firedelay;

	flBoltHideXHair = P_WpnInfo[WEAPON_KAR].anim_firedelay;

	if( GetFOV() > 0 )
	{
		if( m_iClip <= 0 )
			SetThink( &CSPRING::ThinkZoomOut );
		else
			SetThink( &CSPRING::ThinkZoomOutIn );

		pev->nextthink = gpGlobals->time + 0.5f;
	}

	RemoveStamina( 2.0f, m_pPlayer );
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
	{
		m_pPlayer->m_iFOV = ZoomOut();
		UpdateZoomSpeed();
	}

	SetThink( NULL );

	DefaultReload( P_WpnInfo[WEAPON_SPRING].ammo_maxclip, SPRING_RELOAD, P_WpnInfo[WEAPON_SPRING].anim_reloadtime );
}

void CSPRING::ThinkZoomOutIn( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

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
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	SetThink( NULL );

	CBasePlayerWeapon::Holster( skiplocal );
}

void CSPRING::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( SPRING_IDLE, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	}
}

int CSPRING::Classify( void )
{
	return CLASS_SCOPED_RIFLE;
}

class CSpringAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_SPRING].ammo_maxclip, "ammo_22mm", P_WpnInfo[WEAPON_SPRING].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_spring, CSpringAmmoClip )
