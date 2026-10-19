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

LINK_ENTITY_TO_CLASS( weapon_thompson, CThompson )

extern p_wpninfo_s P_WpnInfo[];

void CThompson::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_THOMPSON].wmodel );
	m_iId = WEAPON_THOMPSON;

	m_iDefaultAmmo = P_WpnInfo[WEAPON_THOMPSON].ammo_default;

	FallInit();
}

void CThompson::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_THOMPSON].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_THOMPSON].wmodel );

	PRECACHE_SOUND( "weapons/reloadgarand.wav" );
	PRECACHE_SOUND( "weapons/thompson_shoot.wav" );
	PRECACHE_SOUND( "weapons/357_cock1.wav" );
	PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );
	PRECACHE_SOUND( "weapons/tommy_reload_clipout.wav" );
	PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

	m_usFireThompson = PRECACHE_EVENT( 1, "events/weapons/thompson.sc" );
}

int CThompson::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_44mm";
	p->iMaxAmmo1 = P_WpnInfo[WEAPON_THOMPSON].ammo_maxcarry;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P_WpnInfo[WEAPON_THOMPSON].ammo_maxclip;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_RIFLE;
	p->iId = m_iId = WEAPON_THOMPSON;
	p->iWeight = P_WpnInfo[WEAPON_THOMPSON].misc_weight;

	return 1;
}

int CThompson::AddToPlayer( CBasePlayer *pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
		return TRUE;

	return FALSE;
}

BOOL CThompson::Deploy( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_THOMPSON].anim_drawtime;

	return DefaultDeploy( P_WpnInfo[WEAPON_THOMPSON].vmodel, P_WpnInfo[WEAPON_THOMPSON].pmodel, THOMPSON_DRAW, P_WpnInfo[WEAPON_THOMPSON].szAnimExt, P_WpnInfo[WEAPON_THOMPSON].szAnimReloadExt, 0 );
}

BOOL CThompson::CanHolster( void )
{
	return TRUE;
}

void CThompson::PrimaryAttack( void )
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

	if( m_iClip <= 0 )
	{
		if( !m_fInAttack )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
			m_fInAttack = 1;
		}

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

	vecNewShot = m_pPlayer->FireBulletsNC( vecSrc, vecAiming, flSpread, 8192.0f, BULLET_PLAYER_THOMPSON, 3, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( 1, m_pPlayer->edict(), m_usFireThompson, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecNewShot.x, vecNewShot.y, 0, 0, m_iClip == 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_THOMPSON].anim_firedelay;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );

	RemoveStamina( 2.0f, m_pPlayer );
}

void CThompson::SecondaryAttack( void )
{
	/* Nothing. */
}

void CThompson::Reload( void )
{
	DefaultReload( P_WpnInfo[WEAPON_THOMPSON].ammo_maxclip, THOMPSON_RELOAD, P_WpnInfo[WEAPON_THOMPSON].anim_reloadtime );
}

void CThompson::WeaponIdle( void )
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		SendWeaponAnim( m_iClip == 0 ? THOMPSON_IDLE_EMPTY : THOMPSON_IDLE, UseDecrement() != 0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0f, 15.0f );
	}
}

int CThompson::Classify( void )
{
	return CLASS_SMG;
}

class CThompsonAmmoClip : public CBasePlayerAmmo
{
public:
	virtual void Spawn( void )
	{
		SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	virtual BOOL AddAmmo( CBaseEntity *pOther )
	{
		BOOL bResult = pOther->GiveAmmo( P_WpnInfo[WEAPON_THOMPSON].ammo_maxclip, "ammo_44mm", P_WpnInfo[WEAPON_THOMPSON].ammo_maxcarry ) != -1;

		if( bResult )
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_thompson, CThompsonAmmoClip )
