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

#include "usercmd.h"
#include "entity_state.h"
#include "demo_api.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

#include "../hud_iface.h"
#include "../com_weapons.h"
#include "../demo.h"

extern globalvars_t *gpGlobals;
extern float flBoltHideXHair;

extern int g_iUser1;
extern int g_iUser3;
extern int g_iVuser1x;
extern int g_iVuser1z;

extern void ClientSetSensitivity( int sens );

// Pool of client side entities/entvars_t
static entvars_t ev[MAX_WEAPONS];
static int num_ents = 0;

// The entity we'll use to represent the local client
static CBasePlayer player;

// Local version of game .dll global variables ( time, etc. )
static globalvars_t Globals;

static CBasePlayerWeapon *g_pWpns[MAX_WEAPONS];

static int g_gaitseq = 0;
static int g_rseq = 0;
static vec3_t g_clorg;
static vec3_t g_clang;

float g_flNextPrimaryAttack = 0.0f;
float g_flNextSecondaryAttack = 0.0f;

extern int g_iWeaponFlags;
extern float g_flWeaponHeat;

extern cvar_t *cl_autoreload;

vec3_t previousorigin;

// DoD Weapon placeholder entities.
CCOLT g_Colt;
CLUGER g_Luger;
CGarand g_Garand;
CScopedKar g_ScopedKar;
CThompson g_Thompson;
CSPRING g_Spring;
CKAR g_KAR;
CBAR g_BAR;
CMP40 g_MP40;
CMP44 g_MP44;
CMG42 g_MG42;
C30CAL g_30CAL;
CMG34 g_MG34;
CAmerKnife g_AmerKnife;
CGerKnife g_GerKnife;
CSpade g_Spade;
CM1Carbine g_M1Carbine;
CGreaseGun g_GreaseGun;
CFG42 g_FG42;
CK43 g_K43;
CENFIELD g_Enfield;
CSTEN g_Sten;
CBREN g_Bren;
CWEBLEY g_Webley;
CBazooka g_Bazooka;
CPschreck g_Pschreck;
CPIAT g_PIAT;
CHandGrenade g_HandGrenade;
CStickGrenade g_StickGrenade;
CHandGrenadeEx g_HandGrenadeEx;
CStickGrenadeEx g_StickGrenadeEx;

/*
======================
AlertMessage

Print debug messages to console
======================
*/
void AlertMessage( ALERT_TYPE atype, const char *szFmt, ... )
{
	va_list argptr;
	static char string[1024];

	va_start( argptr, szFmt );
	vsprintf( string, szFmt, argptr );
	va_end( argptr );

	gEngfuncs.Con_Printf( "cl:  " );
	gEngfuncs.Con_Printf( string );
}

//Returns if it's multiplayer.
//Mostly used by the client side weapons.
bool bIsMultiplayer( void )
{
	return gEngfuncs.GetMaxClients() == 1 ? 0 : 1;
}

//Just loads a v_ model.
void LoadVModel( char *szViewModel, CBasePlayer *m_pPlayer )
{
	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );
}

/*
=====================
HUD_PrepEntity

Links the raw entity to an entvars_s holder.  If a player is passed in as the owner, then
we set up the m_pPlayer field.
=====================
*/
void HUD_PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner, int ammotype )
{
	memset( &ev[num_ents], 0, sizeof(entvars_t) );
	pEntity->pev = &ev[num_ents++];

	pEntity->Precache();
	pEntity->Spawn();

	if( pWeaponOwner )
	{
		ItemInfo info;

		( (CBasePlayerWeapon *)pEntity )->m_pPlayer = pWeaponOwner;
		( (CBasePlayerWeapon *)pEntity )->current_ammo = &pWeaponOwner->m_rgAmmo[ammotype];

		( (CBasePlayerWeapon *)pEntity )->GetItemInfo( &info );

		g_pWpns[info.iId] = (CBasePlayerWeapon *)pEntity;
	}
}

/*
=====================
CBaseEntity::Killed

If weapons code "kills" an entity, just set its effects to EF_NODRAW
=====================
*/
void CBaseEntity::Killed( entvars_t *pevAttacker, int iGib )
{
	pev->effects |= EF_NODRAW;
}

/*
=====================
CBasePlayerWeapon::CanDeploy
=====================
*/
BOOL CBasePlayerWeapon::CanDeploy( void )
{
	BOOL bHasAmmo = 0;
	ItemInfo info;

	GetItemInfo( &info );

	if( info.pszAmmo1 && info.iMaxAmmo1 != -1 )
	{
		if( current_ammo )
			bHasAmmo = *current_ammo;
		else
			bHasAmmo = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;

		if( info.pszAmmo2 )
			bHasAmmo |= m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0;

		if( m_iClip <= 0 )
			return bHasAmmo != 0;
	}

	return TRUE;
}

/*
=====================
CBasePlayerWeapon::DefaultDeploy
=====================
*/
BOOL CBasePlayerWeapon::DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, char *szAnimReloadExt, int skiplocal )
{
	if( !CanDeploy() )
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );

	SendWeaponAnim( iAnim, 1 );

	flBoltHideXHair = 0.5f;

	HUD_SetWeaponModel( szWeaponModel - gpGlobals->pStringBase );

	m_pPlayer->m_flNextAttack = 0.5f;
	m_flTimeWeaponIdle = 1.0f;

	return TRUE;
}

/*
=====================
CBasePlayerWeapon::TimedDeploy
=====================
*/
BOOL CBasePlayerWeapon::TimedDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, char *szAnimReloadExt, float idleTime, float attackTime, int skiplocal )
{
	if( !CanDeploy() )
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );

	SendWeaponAnim( iAnim, 1 );

	m_pPlayer->m_flNextAttack = attackTime;
	flBoltHideXHair = attackTime;
	m_flTimeWeaponIdle = idleTime;

	return TRUE;
}

/*
=====================
CBasePlayerWeapon::PlayEmptySound
=====================
*/
BOOL CBasePlayerWeapon::PlayEmptySound( void )
{
	if( m_iPlayEmptySound )
	{
		HUD_PlaySound( "weapons/357_cock1.wav", 0.8f );
		m_iPlayEmptySound = 0;
		return 0;
	}

	return 0;
}

/*
=====================
CBasePlayerWeapon::ResetEmptySound
=====================
*/
void CBasePlayerWeapon::ResetEmptySound( void )
{
	m_iPlayEmptySound = 1;
}

/*
=====================
CBasePlayerWeapon::Holster

Put away weapon
=====================
*/
void CBasePlayerWeapon::Holster( int skiplocal /* = 0 */ )
{
	m_fInReload = FALSE; // cancel any reload in progress.
	m_fInAttack = 0;

	m_pPlayer->pev->viewmodel = 0;

	ClientSetSensitivity( 0 );
}

/*
=====================
CBasePlayerWeapon::SendWeaponAnim

Animate weapon model
=====================
*/
void CBasePlayerWeapon::SendWeaponAnim( int iAnim, int skiplocal )
{
	m_pPlayer->pev->weaponanim = iAnim;

	HUD_SendWeaponAnim( iAnim, 0 );
}

/*
=====================
CBasePlayerWeapon::PostMortarValue
=====================
*/
void CBasePlayerWeapon::PostMortarValue( float value )
{
	HUD_PostMortarValue( value );
}

/*
=====================
CBasePlayerWeapon::SendMortarFireCommand
=====================
*/
void CBasePlayerWeapon::SendMortarFireCommand( char *c )
{
	gEngfuncs.pfnServerCmd( c );
}

/*
=====================
CBasePlayerWeapon::DefaultReload
=====================
*/
BOOL CBasePlayerWeapon::DefaultReload( int iClipSize, int iAnim, float fDelay )
{
	ItemInfo ii;
	int *pAmmo;
	int j;

	GetItemInfo( &ii );

	pAmmo = current_ammo;

	if( !pAmmo )
		pAmmo = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

	if( *pAmmo <= 0 )
		return FALSE;

	if( m_iId == WEAPON_ENFIELD )
		j = Q_min( 2 * ii.iMaxClip - m_iClip, *pAmmo );
	else
		j = Q_min( ii.iMaxClip - m_iClip, *pAmmo );

	if( j == 0 )
		return FALSE;

	if( *pAmmo <= m_iClip )
		return FALSE;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + fDelay;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + fDelay;
	flBoltHideXHair = UTIL_WeaponTimeBase() + fDelay;

	SendWeaponAnim( iAnim, UseDecrement() != 0 );

	m_fInReload = TRUE;

	HUD_SetAutoReloadComplete( false );

	if( ii.iFlags & ITEM_FLAG_HEAT )
		m_flWeaponHeat = 0.0f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + Q_max( 3.0f, fDelay );

	return TRUE;
}

/*
=====================
CBasePlayerWeapon::ItemPostFrame

Handles weapon firing, reloading, etc.
=====================
*/
void CBasePlayerWeapon::ItemPostFrame( void )
{
	ItemInfo info;
	int *pAmmo;
	ItemInfo ii;
	int j;
	CRocketWeapon *pRocketWpn;
	bool bAutoreload;

	GetItemInfo( &info );

	pAmmo = current_ammo;

	if( !pAmmo )
		pAmmo = &m_pPlayer->m_rgAmmo[1];

	if( m_fInReload && m_flNextPrimaryAttack <= 0.0f && *pAmmo > 0 )
	{
		if( Classify() == CLASS_ROCKET )
		{
			if( m_iWeaponState & WPNSTATE_ROCKET_SLOW )
			{
				pRocketWpn = ( CRocketWeapon * )this;

				m_iWeaponState &= ~WPNSTATE_ROCKET_SLOW;
				pRocketWpn->ReSlow();
			}
		}

		// complete the reload.
		if( m_iId == WEAPON_ENFIELD )
			j = Q_min( 5, *pAmmo );
		else
			j = Q_min( iMaxClip(), *pAmmo );

		// Add them to the clip
		m_iClip = j;
		*pAmmo -= j;

		m_fInReload = FALSE;
		m_fFireOnEmpty = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
	}

	if( m_fInAttack && !( m_pPlayer->pev->button & IN_ATTACK ) )
		m_fInAttack = 0;

	GetItemInfo( &ii );

	if( ii.iFlags & ITEM_FLAG_HEAT )
	{
		m_flWeaponHeat = g_flWeaponHeat;

		if( g_flWeaponHeat < 0.0f )
			m_flWeaponHeat = 0.0f;
	}

	if( ( m_pPlayer->pev->button & IN_ATTACK2 ) && m_flNextSecondaryAttack <= 0.0f )
	{
		if( pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()] )
			m_fFireOnEmpty = TRUE;

		if( HUD_GetRoundState() == 1 )
			SecondaryAttack();

		m_pPlayer->pev->button &= ~IN_ATTACK2;

		if( ShouldWeaponIdle() )
			WeaponIdle();
	}
	else if( ( m_pPlayer->pev->button & IN_ATTACK ) && m_flNextPrimaryAttack <= 0.0f )
	{
		if( ( !m_iClip && info.pszAmmo1 ) || ( info.iMaxClip == -1 && !*pAmmo ) )
			m_fFireOnEmpty = TRUE;

		if( HUD_GetRoundState() == 1 )
			PrimaryAttack();

		HUD_SetRecoil( 1 );

		if( ShouldWeaponIdle() )
			WeaponIdle();
	}
	else if( ( m_pPlayer->pev->button & IN_RELOAD ) && iMaxClip() != WEAPON_NOCLIP && !m_fInReload && HUD_IsAutoReloadComplete() )
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		if( m_pPlayer->pev->waterlevel <= 2 )
			Reload();

		if( ShouldWeaponIdle() )
			WeaponIdle();
	}
	else if( !( m_pPlayer->pev->button & ( IN_ATTACK | IN_ATTACK2 ) ) )
	{
		// no fire buttons down
		m_fFireOnEmpty = FALSE;

		HUD_SetRecoil( 0 );

		bAutoreload = cl_autoreload->value > 0.0f;

		if( bAutoreload && HUD_IsAutoReloadComplete() && !m_iClip && !( info.iFlags & ITEM_FLAG_NOAUTORELOAD ) && m_flNextPrimaryAttack < 0.0f
			&& ( ( g_iUser3 != STANCE_PRONE_DEPLOYED && g_iVuser1x != SANDBAG_DEPLOYED ) || Classify() != CLASS_MACHINEGUN )
			&& m_pPlayer->pev->waterlevel <= 2 )
		{
			Reload();
			return;
		}

		WeaponIdle();
	}
	else
	{
		if( ShouldWeaponIdle() )
			WeaponIdle();
	}
}

/*
=====================
SetScopeId
=====================
*/
static void SetScopeId( int id )
{
	if( !g_iVuser1z )
	{
		HUD_SetScope( id );
	}
}

/*
=====================
CBasePlayerWeapon::ChangeFOV
=====================
*/
int CBasePlayerWeapon::ChangeFOV( int fov )
{
	if( g_iVuser1z )
		return g_lastFOV;

	if( m_pPlayer->pev->fuser2 != g_lastFOV )
		return 0;

	if( g_lastFOV <= 0 )
	{
		g_lastFOV = fov;
		SetScopeId( m_iId );
	}
	else
	{
		g_lastFOV = 0;
		SetScopeId( 0 );
	}

	return g_lastFOV;
}

/*
=====================
CBasePlayerWeapon::ZoomIn
=====================
*/
int CBasePlayerWeapon::ZoomIn( void )
{
	if( m_pPlayer->pev->fuser2 != g_lastFOV )
		return 0;

	SetScopeId( m_iId );

	g_lastFOV = 20;

	return 0;
}

/*
=====================
CBasePlayerWeapon::ZoomOut
=====================
*/
int CBasePlayerWeapon::ZoomOut( void )
{
	if( g_iVuser1z )
		return 0;

	if( m_pPlayer->pev->fuser2 != g_lastFOV )
		return 0;

	g_lastFOV = 0;

	SetScopeId( 0 );

	return 0;
}

/*
=====================
CBasePlayerWeapon::UpdateZoomSpeed
=====================
*/
void CBasePlayerWeapon::UpdateZoomSpeed( void )
{
	/* Nothing. */
}

/*
=====================
CBasePlayerWeapon::GetFOV
=====================
*/
int CBasePlayerWeapon::GetFOV( void )
{
	return g_lastFOV;
}

/*
=====================
CBasePlayerWeapon::PlayerIsWaterSniping
=====================
*/
bool CBasePlayerWeapon::PlayerIsWaterSniping( void )
{
	if( HUD_GetWaterLevel() == 3 )
		return true;

	if( HUD_GetWaterLevel() > 0 )
		return g_iUser3 == STANCE_PRONE;

	return false;
}

/*
=====================
CBasePlayerWeapon::ThinkZoomOut
=====================
*/
void CBasePlayerWeapon::ThinkZoomOut( void )
{
	m_pPlayer->m_iFOV = ZoomOut();

	UpdateZoomSpeed();
}

/*
=====================
CBasePlayerWeapon::ThinkZoomIn
=====================
*/
void CBasePlayerWeapon::ThinkZoomIn( void )
{
	if( m_pPlayer->pev->velocity.Length() <= 45.0f )
	{
		if( !( m_pPlayer->pev->button & IN_ATTACK ) )
		{
			m_pPlayer->m_iFOV = ZoomIn();

			UpdateZoomSpeed();
		}
	}
}

/*
=====================
CBasePlayerWeapon::flAim
=====================
*/
float CBasePlayerWeapon::flAim( float accuracyFactor, CBasePlayer *pOther )
{
	return accuracyFactor;
}

/*
=====================
CBasePlayerWeapon::Aim
=====================
*/
Vector CBasePlayerWeapon::Aim( float accuracyFactor, CBasePlayer *pOther, int shared_rand )
{
	float targetx = UTIL_SharedRandomFloat( shared_rand, accuracyFactor, 0.0f );
	float targety = UTIL_SharedRandomFloat( shared_rand, accuracyFactor, 0.0f );
	float targetz = UTIL_SharedRandomFloat( shared_rand, accuracyFactor, 0.0f );

	return Vector( targetx, targety, targetz );
}

/*
=====================
CBasePlayerWeapon::RemoveStamina
=====================
*/
void CBasePlayerWeapon::RemoveStamina( float removeAmount, CBasePlayer *pOther )
{
	/* Nothing. */
}

/*
=====================
CBasePlayerWeapon::Classify
=====================
*/
int CBasePlayerWeapon::Classify( void )
{
	return CLASS_NONE;
}

/*
=====================
CBasePlayerWeapon::GetRoundState
=====================
*/
int CBasePlayerWeapon::GetRoundState( void )
{
	return HUD_GetRoundState();
}

/*
=====================
CBasePlayer::SelectItem

  Switch weapons
=====================
*/
void CBasePlayer::SelectItem( const char *pstr )
{
	/* Nothing. */
}

/*
=====================
CBasePlayer::SelectLastItem
=====================
*/
void CBasePlayer::SelectLastItem( void )
{
	if( !m_pLastItem )
	{
		return;
	}

	if( m_pActiveItem && !m_pActiveItem->CanHolster() )
	{
		return;
	}

	// FIX, this needs to queue them up and delay
	if( m_pActiveItem )
		m_pActiveItem->Holster();

	CBasePlayerItem *pTemp = m_pActiveItem;
	m_pActiveItem = m_pLastItem;
	m_pLastItem = pTemp;
	m_pActiveItem->Deploy();
}

/*
=====================
CBasePlayer::Killed
=====================
*/
void CBasePlayer::Killed( entvars_t *pevAttacker, int iGib )
{
	// Holster weapon immediately, to allow it to cleanup
	if( m_pActiveItem )
		m_pActiveItem->Holster();
}

/*
=====================
CBasePlayer::Spawn
=====================
*/
void CBasePlayer::Spawn( void )
{
	if( m_pActiveItem )
	{
		if( m_pActiveItem->m_iId == WEAPON_M1CARBINE || m_pActiveItem->m_iId == WEAPON_GREASEGUN )
			m_pActiveItem->SpawnDeploy();
		else
			m_pActiveItem->Deploy();
	}
}

/*
=====================
CBasePlayer::IsInMGDeploy
=====================
*/
bool CBasePlayer::IsInMGDeploy( void )
{
	return g_iUser3 == STANCE_PRONE_DEPLOYED || g_iVuser1x == SANDBAG_DEPLOYED;
}

/*
=====================
CBasePlayer::IsProneDeployed
=====================
*/
bool CBasePlayer::IsProneDeployed( void )
{
	return g_iUser3 == STANCE_PRONE_DEPLOYED;
}

/*
=====================
CBasePlayer::IsSandbagDeployed
=====================
*/
bool CBasePlayer::IsSandbagDeployed( void )
{
	return g_iVuser1x == SANDBAG_DEPLOYED;
}

/*
=====================
UTIL_TraceLine

Don't actually trace, but act like the trace didn't hit anything.
=====================
*/
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
	memset( ptr, 0, sizeof(*ptr) );
	ptr->flFraction = 1.0f;
}

/*
=====================
UTIL_ParticleBox

For debugging, draw a box around a player made out of particles
=====================
*/
void UTIL_ParticleBox( CBasePlayer *player, float *mins, float *maxs, float life, unsigned char r, unsigned char g, unsigned char b )
{
	int i;
	vec3_t mmin, mmax;

	for( i = 0; i < 3; i++ )
	{
		mmin[i] = player->pev->origin[i] + mins[i];
		mmax[i] = player->pev->origin[i] + maxs[i];
	}

	gEngfuncs.pEfxAPI->R_ParticleBox( (float *)&mmin, (float *)&mmax, 5.0, 0, 255, 0 );
}

/*
=====================
UTIL_ParticleBoxes

For debugging, draw boxes for other collidable players
=====================
*/
void UTIL_ParticleBoxes( void )
{
	int idx;
	physent_t *pe;
	cl_entity_t *player;
	vec3_t mins, maxs;

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	player = gEngfuncs.GetLocalPlayer();
	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( player->index - 1 );

	for( idx = 1; idx < 100; idx++ )
	{
		pe = gEngfuncs.pEventAPI->EV_GetPhysent( idx );
		if( !pe )
			break;

		if( pe->info >= 1 && pe->info <= gEngfuncs.GetMaxClients() )
		{
			mins = pe->origin + pe->mins;
			maxs = pe->origin + pe->maxs;

			gEngfuncs.pEfxAPI->R_ParticleBox( (float *)&mins, (float *)&maxs, 0, 0, 255, 2.0 );
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

/*
=====================
UTIL_ParticleLine

For debugging, draw a line made out of particles
=====================
*/
void UTIL_ParticleLine( CBasePlayer *player, float *start, float *end, float life, unsigned char r, unsigned char g, unsigned char b )
{
	gEngfuncs.pEfxAPI->R_ParticleLine( start, end, r, g, b, life );
}

/*
=====================
CBasePlayerWeapon::PrintState

Dump a few of the more important fields to a log file
=====================
*/
void CBasePlayerWeapon::PrintState( void )
{
	COM_Log( "c:\\hl.log", "%.4f ", gpGlobals->time );
	COM_Log( "c:\\hl.log", "%.4f ", m_pPlayer->m_flNextAttack );
	COM_Log( "c:\\hl.log", "%.4f ", m_flNextPrimaryAttack );
	COM_Log( "c:\\hl.log", "%.4f ", m_flTimeWeaponIdle - gpGlobals->time );
	COM_Log( "c:\\hl.log", "%i ", m_iClip );
}

/*
=====================
HUD_InitClientWeapons

Set up weapons, player and functions needed to run weapons code client-side.
=====================
*/
void HUD_InitClientWeapons( void )
{
	static int initialized = 0;
	if( initialized )
		return;

	initialized = 1;

	// Set up pointer ( dummy object )
	gpGlobals = &Globals;

	// Fill in current time ( probably not needed )
	gpGlobals->time = gEngfuncs.GetClientTime();

	// Handled locally
	g_engfuncs.pfnSetClientMaxspeed = HUD_SetMaxSpeed;
	g_engfuncs.pfnPlaybackEvent = HUD_PlaybackEvent;
	g_engfuncs.pfnAlertMessage = AlertMessage;

	// Pass through to engine
	g_engfuncs.pfnPrecacheEvent = gEngfuncs.pfnPrecacheEvent;

	// Fake functions
	g_engfuncs.pfnNameForFunction = stub_NameForFunction;
	g_engfuncs.pfnPrecacheModel = stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound = stub_PrecacheSound;
	g_engfuncs.pfnSetModel = stub_SetModel;
	g_engfuncs.pfnRandomFloat = gEngfuncs.pfnRandomFloat;
	g_engfuncs.pfnRandomLong = gEngfuncs.pfnRandomLong;

	// Allocate a slot for the local player
	HUD_PrepEntity( &player, NULL, 0 );

	// Allocate slot(s) for each weapon that we are going to be predicting
	HUD_PrepEntity( &g_Colt, &player, 1 );
	HUD_PrepEntity( &g_Luger, &player, 1 );
	HUD_PrepEntity( &g_Garand, &player, 1 );
	HUD_PrepEntity( &g_ScopedKar, &player, 1 );
	HUD_PrepEntity( &g_Thompson, &player, 1 );
	HUD_PrepEntity( &g_MP44, &player, 1 );
	HUD_PrepEntity( &g_MP40, &player, 1 );
	HUD_PrepEntity( &g_Spring, &player, 1 );
	HUD_PrepEntity( &g_KAR, &player, 1 );
	HUD_PrepEntity( &g_BAR, &player, 1 );
	HUD_PrepEntity( &g_MG42, &player, 1 );
	HUD_PrepEntity( &g_MG34, &player, 1 );
	HUD_PrepEntity( &g_30CAL, &player, 1 );
	HUD_PrepEntity( &g_AmerKnife, &player, 1 );
	HUD_PrepEntity( &g_GerKnife, &player, 1 );
	HUD_PrepEntity( &g_Spade, &player, 1 );
	HUD_PrepEntity( &g_M1Carbine, &player, 1 );
	HUD_PrepEntity( &g_GreaseGun, &player, 1 );
	HUD_PrepEntity( &g_FG42, &player, 1 );
	HUD_PrepEntity( &g_K43, &player, 1 );
	HUD_PrepEntity( &g_Enfield, &player, 1 );
	HUD_PrepEntity( &g_Sten, &player, 1 );
	HUD_PrepEntity( &g_Bren, &player, 1 );
	HUD_PrepEntity( &g_Webley, &player, 1 );
	HUD_PrepEntity( &g_Bazooka, &player, 1 );
	HUD_PrepEntity( &g_Pschreck, &player, 1 );
	HUD_PrepEntity( &g_PIAT, &player, 1 );
	HUD_PrepEntity( &g_HandGrenade, &player, 5 );
	HUD_PrepEntity( &g_StickGrenade, &player, 5 );
	HUD_PrepEntity( &g_HandGrenadeEx, &player, 5 );
	HUD_PrepEntity( &g_StickGrenadeEx, &player, 5 );
}

/*
=====================
HUD_GetLastOrg

Retruns the last position that we stored for egon beam endpoint.
=====================
*/
void HUD_GetLastOrg( float *org )
{
	int i;

	// Return last origin
	for( i = 0; i < 3; i++ )
	{
		org[i] = previousorigin[i];
	}
}

/*
=====================
HUD_SetLastOrg

Remember our exact predicted origin so we can draw the egon to the right position.
=====================
*/
void HUD_SetLastOrg( void )
{
	int i;

	// Offset final origin by view_offset
	for( i = 0; i < 3; i++ )
	{
		previousorigin[i] = g_finalstate->playerstate.origin[i] + g_finalstate->client.view_ofs[i];
	}
}

/*
=====================
HUD_WeaponsPostThink

Run Weapon firing code on client
=====================
*/
void HUD_WeaponsPostThink( local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed )
{
	int i;
	int buttonsChanged;
	CBasePlayerWeapon *pWeapon = NULL;
	CBasePlayerWeapon *pCurrent;
	weapon_data_t nulldata, *pfrom, *pto;
	static int lasthealth;

	memset( &nulldata, 0, sizeof( nulldata ) );

	HUD_InitClientWeapons();

	// Get current clock
	gpGlobals->time = time;

	if( to->client.health <= 0 )
		HUD_SetRecoil( 0 );

	if( HUD_GetRoundState() != 1 )
		HUD_SetRecoil( 0 );

	// Fill in data based on selected weapon
	switch( from->client.m_iId )
	{
	case WEAPON_AMERKNIFE:
		pWeapon = &g_AmerKnife;
		break;
	case WEAPON_GERKNIFE:
		pWeapon = &g_GerKnife;
		break;
	case WEAPON_COLT:
		pWeapon = &g_Colt;
		break;
	case WEAPON_LUGER:
		pWeapon = &g_Luger;
		break;
	case WEAPON_GARAND:
		pWeapon = &g_Garand;
		break;
	case WEAPON_SCOPEDKAR:
		pWeapon = &g_ScopedKar;
		break;
	case WEAPON_THOMPSON:
		pWeapon = &g_Thompson;
		break;
	case WEAPON_MP44:
		pWeapon = &g_MP44;
		break;
	case WEAPON_SPRING:
		pWeapon = &g_Spring;
		break;
	case WEAPON_KAR:
		pWeapon = &g_KAR;
		break;
	case WEAPON_BAR:
		pWeapon = &g_BAR;
		break;
	case WEAPON_MP40:
		pWeapon = &g_MP40;
		break;
	case WEAPON_HANDGRENADE:
		pWeapon = &g_HandGrenade;
		break;
	case WEAPON_STICKGRENADE:
		pWeapon = &g_StickGrenade;
		break;
	case WEAPON_STICKGRENADEX:
		pWeapon = &g_StickGrenadeEx;
		break;
	case WEAPON_HANDGRENADEX:
		pWeapon = &g_HandGrenadeEx;
		break;
	case WEAPON_MG42:
		pWeapon = &g_MG42;
		break;
	case WEAPON_CAL30:
		pWeapon = &g_30CAL;
		break;
	case WEAPON_SPADE:
		pWeapon = &g_Spade;
		break;
	case WEAPON_M1CARBINE:
		pWeapon = &g_M1Carbine;
		break;
	case WEAPON_MG34:
		pWeapon = &g_MG34;
		break;
	case WEAPON_GREASEGUN:
		pWeapon = &g_GreaseGun;
		break;
	case WEAPON_FG42:
		pWeapon = &g_FG42;
		break;
	case WEAPON_K43:
		pWeapon = &g_K43;
		break;
	case WEAPON_ENFIELD:
		pWeapon = &g_Enfield;
		break;
	case WEAPON_STEN:
		pWeapon = &g_Sten;
		break;
	case WEAPON_BREN:
		pWeapon = &g_Bren;
		break;
	case WEAPON_WEBLEY:
		pWeapon = &g_Webley;
		break;
	case WEAPON_BAZOOKA:
		pWeapon = &g_Bazooka;
		break;
	case WEAPON_PSCHRECK:
		pWeapon = &g_Pschreck;
		break;
	case WEAPON_PIAT:
		pWeapon = &g_PIAT;
		break;
	}

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	if( !pWeapon )
		return;

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		pCurrent = g_pWpns[i];
		if( !pCurrent )
		{
			continue;
		}

		pfrom = &from->weapondata[i];

		pCurrent->m_fInReload = pfrom->m_fInReload;
		pCurrent->m_fInSpecialReload = pfrom->m_fInSpecialReload;
		pCurrent->m_iClip = pfrom->m_iClip;
		pCurrent->m_flNextPrimaryAttack = pfrom->m_flNextPrimaryAttack;
		pCurrent->m_flNextSecondaryAttack = pfrom->m_flNextSecondaryAttack;
		pCurrent->m_flTimeWeaponIdle = pfrom->m_flTimeWeaponIdle;
		pCurrent->m_iWeaponState = pfrom->m_iWeaponState;
		pCurrent->m_flWeaponHeat = pfrom->fuser1;
	}

	g_iWeaponFlags = pWeapon->m_iWeaponState;
	g_flWeaponHeat = pWeapon->m_flWeaponHeat;

	// For random weapon events, use this seed to seed random # generator
	player.random_seed = random_seed;

	// Get old buttons from previous state.
	player.m_afButtonLast = from->playerstate.oldbuttons;

	// Which buttons chave changed
	buttonsChanged = ( player.m_afButtonLast ^ cmd->buttons );	// These buttons have changed this frame

	// Debounced button codes for pressed/released
	// The changed ones still down are "pressed"
	player.m_afButtonPressed = buttonsChanged & cmd->buttons;
	// The ones not down are "released"
	player.m_afButtonReleased = buttonsChanged & ( ~cmd->buttons );

	// Set player variables that weapons code might check/alter
	player.pev->button = cmd->buttons;

	player.pev->velocity = from->client.velocity;
	player.pev->flags = from->client.flags;

	player.pev->deadflag = from->client.deadflag;
	player.pev->waterlevel = from->client.waterlevel;
	player.pev->maxspeed = from->client.maxspeed;
	player.pev->punchangle = from->client.punchangle;
	player.pev->fov = from->client.fov;
	player.pev->weaponanim = from->client.weaponanim;
	player.pev->viewmodel = from->client.viewmodel;
	player.m_flNextAttack = from->client.m_flNextAttack;

	HUD_SetVelocity( player.pev->velocity );

	player.m_rgAmmo[1] = from->client.ammo_shells;

	// Point to current weapon object
	if( gEngfuncs.GetLocalPlayer() )
	{
		player.pev->origin = from->client.origin;
		player.pev->v_angle = v_angles;
		player.pev->angles = gEngfuncs.GetLocalPlayer()->angles;
	}

	player.pev->fuser2 = from->client.fuser2;

	if( from->client.m_iId )
		player.m_pActiveItem = g_pWpns[from->client.m_iId];

	// Don't go firing anything if we have died or are spectating
	// Or if we don't have a weapon model deployed
	g_finalstate = to;

	if( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) && !CL_IsDead() && player.pev->viewmodel && !g_iUser1 && player.m_flNextAttack <= 0.0f )
	{
		pWeapon->ItemPostFrame();
	}

	// Assume that we are not going to switch weapons
	if( g_runfuncs )
	{
		// Make sure that the weapon is deployed/holstered when the player is respawned/killed
		if( to->client.health <= 0 && lasthealth > 0 )
		{
			player.Killed( NULL, 0 );
		}
		else if( to->client.health > 0 && lasthealth <= 0 )
		{
			player.Spawn();
		}

		lasthealth = to->client.health;
	}

	player.PostThink();

	// Assume that we are not going to switch weapons
	to->client.m_iId = from->client.m_iId;

	// Now see if we issued a changeweapon command ( and we're not dead )
	if( cmd->weaponselect && ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) && cmd->weaponselect == from->weapondata[cmd->weaponselect].m_iId )
	{
		CBasePlayerWeapon *pNew = g_pWpns[cmd->weaponselect];

		if( pNew && ( pNew != pWeapon ) )
		{
			BOOL canHolster = player.m_pActiveItem->CanHolster() != 0;

			// Put away old weapon
			if( canHolster && player.m_pActiveItem )
				player.m_pActiveItem->Holster();

			player.m_pLastItem = player.m_pActiveItem;
			player.m_pActiveItem = pNew;

			// Deploy new weapon
			if( canHolster )
				player.m_pActiveItem->Deploy();

			// Update weapon id so we can predict things correctly.
			to->client.m_iId = cmd->weaponselect;
		}
	}

	// Copy in results of prediction code
	to->client.viewmodel = player.pev->viewmodel;
	to->client.fov = player.pev->fov;
	to->client.weaponanim = player.pev->weaponanim;
	to->client.m_flNextAttack = player.m_flNextAttack;
	to->client.maxspeed = player.pev->maxspeed;
	to->client.ammo_shells = player.m_rgAmmo[1];
	to->client.fuser2 = player.pev->fuser2;

	// Make sure that weapon animation matches what the game .dll is telling us
	//  over the wire ( fixes some animation glitches )
	if( g_runfuncs && ( HUD_GetWeaponAnim() != to->client.weaponanim ) )
	{
		// Force a fixed anim down to viewmodel
		HUD_SendWeaponAnim( to->client.weaponanim, 1 );
	}

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		pCurrent = g_pWpns[i];

		pto = &to->weapondata[i];

		if( !pCurrent )
		{
			memset( pto, 0, sizeof( weapon_data_t ) );
			continue;
		}

		pto->m_fInReload = pCurrent->m_fInReload;
		pto->m_iClip = pCurrent->m_iClip;
		pto->m_flNextPrimaryAttack = pCurrent->m_flNextPrimaryAttack;
		pto->m_flNextSecondaryAttack = pCurrent->m_flNextSecondaryAttack;

		if( to->client.m_iId == pCurrent->m_iId )
		{
			HUD_SetClip( pCurrent->m_iClip );

			pto->m_flNextPrimaryAttack = pto->m_flNextPrimaryAttack;
			pto->m_flNextSecondaryAttack = pto->m_flNextSecondaryAttack;

			g_flNextPrimaryAttack = pCurrent->m_flNextPrimaryAttack;
			g_flNextSecondaryAttack = pCurrent->m_flNextSecondaryAttack;
		}

		pto->m_flTimeWeaponIdle = pCurrent->m_flTimeWeaponIdle;
		pto->m_iWeaponState = pCurrent->m_iWeaponState;

		// Decrement weapon counters, server does this at same time ( during post think, after doing everything else )
		pto->m_flNextReload -= cmd->msec / 1000.0;
		pto->m_flNextPrimaryAttack -= cmd->msec / 1000.0;
		pto->m_flNextSecondaryAttack -= cmd->msec / 1000.0;
		pto->m_flTimeWeaponIdle -= cmd->msec / 1000.0;

		if( pto->m_flNextPrimaryAttack < -1.0 )
		{
			pto->m_flNextPrimaryAttack = -1.0;
		}

		if( pto->m_flNextSecondaryAttack < -0.001 )
		{
			pto->m_flNextSecondaryAttack = -0.001;
		}

		if( pto->m_flTimeWeaponIdle < -0.001 )
		{
			pto->m_flTimeWeaponIdle = -0.001;
		}

		if( pto->m_flNextReload < -0.001 )
		{
			pto->m_flNextReload = -0.001;
		}
	}

	// m_flNextAttack is now part of the weapons, but is part of the player instead
	to->client.m_flNextAttack -= cmd->msec / 1000.0;
	if( to->client.m_flNextAttack < -0.001 )
	{
		to->client.m_flNextAttack = -0.001;
	}

	HUD_SetNextAttack( to->client.m_flNextAttack );

	// Store off the last position from the predicted state.
	HUD_SetLastOrg();

	// Wipe it so we can't use it after this frame
	g_finalstate = NULL;
}

/*
=====================
DoD_GetSequence
=====================
*/
void DoD_GetSequence( int *seq, int *gaitseq )
{
	*seq = g_rseq;
	*gaitseq = g_gaitseq;
}

/*
=====================
DoD_SetSequence
=====================
*/
void DoD_SetSequence( int seq, int gaitseq )
{
	g_rseq = seq;
	g_gaitseq = gaitseq;
}

/*
=====================
DoD_SetOrientation
=====================
*/
void DoD_SetOrientation( Vector o, Vector a )
{
	g_clorg = o;
	g_clang = a;
}

/*
=====================
DoD_GetOrientation
=====================
*/
void DoD_GetOrientation( float *o, float *a )
{
	int i;

	for( i = 0; i < 3; i++ )
	{
		o[i] = g_clorg[i];
		a[i] = g_clang[i];
	}
}

/*
=====================
HUD_PostRunCmd

Client calls this during prediction, after it has moved the player and updated any info changed into to->
time is the current client clock based on prediction
cmd is the command that caused the movement, etc
runfuncs is 1 if this is the first time we've predicted this command.  If so, sounds and effects should play, otherwise, they should
be ignored
=====================
*/
void _DLLEXPORT HUD_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed )
{
	g_runfuncs = runfuncs;

	HUD_WeaponsPostThink( from, to, cmd, time, random_seed );

	if( g_runfuncs )
	{
		DoD_SetSequence( to->playerstate.sequence, to->playerstate.gaitsequence );
		DoD_SetOrientation( to->playerstate.origin, cmd->viewangles );
	}
}

/*
=====================
CBaseEntity::FireBulletsNC

Compute the random bullet spread offset (shared_rand is the usercmd random seed).
=====================
*/
Vector CBaseEntity::FireBulletsNC( Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	float x, y, z;

	if( pevAttacker )
	{
		// Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat( shared_rand, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + 1, -0.5, 0.5 );
		y = UTIL_SharedRandomFloat( shared_rand + 2, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + 3, -0.5, 0.5 );
	}
	else
	{
		do
		{
			x = RANDOM_FLOAT( -0.5, 0.5 ) + RANDOM_FLOAT( -0.5, 0.5 );
			y = RANDOM_FLOAT( -0.5, 0.5 ) + RANDOM_FLOAT( -0.5, 0.5 );
			z = x * x + y * y;
		} while( z > 1 );
	}

	return Vector( x * flSpread, y * flSpread, 0 );
}
