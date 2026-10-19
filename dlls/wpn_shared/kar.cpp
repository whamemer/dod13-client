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

LINK_ENTITY_TO_CLASS( weapon_kar, CKAR )

extern p_wpninfo_s P_WpnInfo[];
extern float flBoltHideXHair;

void CKAR::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_KAR].wmodel );
	m_iId = WEAPON_KAR;

	m_iDefaultAmmo = P_WpnInfo[WEAPON_KAR].ammo_default;

	FallInit();
}

void CKAR::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_KAR].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_KAR].wmodel );

	PRECACHE_SOUND( "weapons/kar_shoot.wav" );
	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( "weapons/boltforward.wav" );
	PRECACHE_SOUND( "weapons/boltback.wav" );
	PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );
	PRECACHE_SOUND( "weapons/knifeswing.wav" );
	PRECACHE_SOUND( "weapons/knifeswing2.wav" );
	PRECACHE_SOUND( "weapons/knife_slash1.wav" );
	PRECACHE_SOUND( "weapons/knife_slash2.wav" );
	PRECACHE_SOUND( "weapons/knife_hit1.wav" );
	PRECACHE_SOUND( "weapons/knife_hit2.wav" );
	PRECACHE_SOUND( "weapons/knife_hit3.wav" );
	PRECACHE_SOUND( "weapons/cbar_hit1.wav" );
	PRECACHE_SOUND( "weapons/cbar_hit2.wav" );

	m_usFireKar = PRECACHE_EVENT( 1, "events/weapons/kar.sc" );
}

int CKAR::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_16mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_KAR].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_KAR].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_KAR;
	p->iWeight = P_WpnInfo[WEAPON_KAR].misc_weight;

	return 1;
}

int CKAR::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return TRUE;

	return FALSE;
}

BOOL CKAR::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_KAR].anim_drawtime;

	return DefaultDeploy( P_WpnInfo[WEAPON_KAR].vmodel, P_WpnInfo[WEAPON_KAR].pmodel, KAR_DRAW, P_WpnInfo[WEAPON_KAR].szAnimExt, P_WpnInfo[WEAPON_KAR].szAnimReloadExt, 0 );
}

BOOL CKAR::CanHolster( void )
{
	return TRUE;
}

void CKAR::PrimaryAttack( void )
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

	flSpread = P_WpnInfo[m_iId].base_accuracy;

	if( m_pPlayer->pev->velocity.Length() > 45.0f )
		flSpread += P_WpnInfo[m_iId].accuracy_penalty;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_KAR, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireKar, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, 0, 0, m_iClip == 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_KAR].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_KAR].anim_firedelay;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	m_fInAttack = 1;

	flBoltHideXHair = P_WpnInfo[WEAPON_KAR].anim_firedelay;

	RemoveStamina( 2.0f, m_pPlayer );
}

void CKAR::SecondaryAttack( void )
{
	RifleMeleeAttack( m_pPlayer, 10 );

	m_pPlayer->SetAnimation( PLAYER_ATTACK2 );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireKar, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, 1, 0, m_iClip == 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8f;
}

void CKAR::Reload( void )
{
	DefaultReload( P_WpnInfo[WEAPON_KAR].ammo_maxclip, KAR_RELOAD, P_WpnInfo[WEAPON_KAR].anim_reloadtime );
}

void CKAR::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( KAR_IDLE, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	}
}

int CKAR::Classify( void )
{
	return CLASS_RIFLE;
}

class CKarAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_KAR].ammo_maxclip, "ammo_16mm", P_WpnInfo[WEAPON_KAR].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_kar, CKarAmmoClip )
