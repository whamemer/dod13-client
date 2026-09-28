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
#include "nodes.h"
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

#include "dod_shared.h"

extern globalvars_t *gpGlobals;
extern int g_iUser1;
extern float flBoltHideXHair;

// Pool of client side entities/entvars_t
static entvars_t ev[MAX_WEAPONS];
static int num_ents = 0;

// The entity we'll use to represent the local client
static CBasePlayer player;

// Local version of game .dll global variables ( time, etc. )
static globalvars_t Globals; 

static CBasePlayerWeapon *g_pWpns[MAX_WEAPONS];

static int g_gaitseq, g_rseq;
static vec3_t g_clang, g_clorg;

float g_flNextPrimaryAttack, g_flNextSecondaryAttack;
extern int g_iWeaponFlags;
extern float g_flWeaponHeat;

extern cvar_t *cl_autoreload;

float g_flApplyVel = 0.0;

vec3_t previousorigin;

enum e_ammo
{
	ammo_none = 0,
	ammo_shells,
	ammo_chem,
	ammo_battery,
	ammo_minigun,
	ammo_rocket,
	ammo_gauss
};

// HLDM Weapon placeholder entities.
static CCOLT               g_Colt;
static CLUGER              g_Luger;
static CGarand             g_Garand;
static CScopedKar          g_ScopedKar;
static CThompson           g_Thompson;
static CSPRING             g_Spring;
static CKAR                g_KAR;
static CBAR                g_BAR;
static CMP40               g_MP40;
static CMP44               g_MP44;
static CMG42               g_MG42;
static C30CAL              g_30CAL;
static CMG34               g_MG34;
static CAmerKnife          g_AmerKnife;
static CGerKnife           g_GerKnife;
static CSpade              g_Spade;
static CM1Carbine          g_M1Carbine;
static CGreaseGun          g_GreaseGun;
static CFG42               g_FG42;
static CK43                g_K43;
static CENFIELD            g_Enfield;
static CSTEN               g_Sten;
static CBREN               g_Bren;
static CWEBLEY             g_Webley;
static CBazooka            g_Bazooka;
static CPschreck           g_Pschreck;
static CPIAT               g_PIAT;
static CHandGrenade        g_HandGrenade;
static CStickGrenade       g_StickGrenade;
static CHandGrenadeEx      g_HandGrenadeEx;
static CStickGrenadeEx     g_StickGrenadeEx;

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
void LoadVModel( const char *szViewModel, CBasePlayer *m_pPlayer )
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
void HUD_PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner )
{
	memset( &ev[num_ents], 0, sizeof(entvars_t) );
	pEntity->pev = &ev[num_ents++];

	pEntity->Precache();
	pEntity->Spawn();

	if( pWeaponOwner )
	{
		ItemInfo info;

		( (CBasePlayerWeapon *)pEntity )->m_pPlayer = pWeaponOwner;

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
CBasePlayerWeapon::DefaultReload
=====================
*/
BOOL CBasePlayerWeapon::DefaultReload( int iClipSize, int iAnim, float fDelay, int body )
{
	ItemInfo ii;
	int *pAmmo;
	int j;

	GetItemInfo( &ii );

	pAmmo = current_ammo;

	if( !pAmmo )
		pAmmo = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

	j = *pAmmo;

	if( j > 0 )
	{
		if( m_iId == WEAPON_ENFIELD )
			j = 2 * ii.iMaxClip - m_iClip;
		else
			j = ii.iMaxClip - m_iClip;

		if( *pAmmo <= j )
		{
			if( *pAmmo > m_iClip )
			{
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + fDelay;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + fDelay;

				flBoltHideXHair = UTIL_WeaponTimeBase() + fDelay;

				SendWeaponAnim( iAnim, UseDecrement() != 0 );

				m_fInReload = TRUE;
				gHUD.m_bAutoReloadComplete = FALSE;

				if( ii.iFlags & ITEM_FLAG_HEAT )
					m_flWeaponHeat = 0.0f;

				float flDelay = 3.0f;

				if( fDelay >= 3.0f )
				{
					flDelay = fDelay;
				}

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flDelay;
				return TRUE;
			}
		}
	}

	return FALSE;
}

/*
=====================
CBasePlayerWeapon::CanDeploy

=====================
*/
BOOL CBasePlayerWeapon::CanDeploy( void ) 
{
	BOOL bHasAmmo = 0;

	if( !pszAmmo1() )
	{
		// this weapon doesn't use ammo, can always deploy.
		return TRUE;
	}

	if( pszAmmo1() )
	{
		bHasAmmo |= ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0 );
	}
	if( pszAmmo2() )
	{
		bHasAmmo |= ( m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0 );
	}
	if( m_iClip > 0 )
	{
		bHasAmmo |= 1;
	}
	if( !bHasAmmo )
	{
		return FALSE;
	}

	return TRUE;
}

/*
=====================
CBasePlayerWeapon::DefaultDeploy

=====================
*/
BOOL CBasePlayerWeapon::DefaultDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt,
	const char *szAnimReloadExt, int skiplocal, int body )
{
	if( !CanDeploy() )
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );

	SendWeaponAnim( iAnim, skiplocal, body );

	flBoltHideXHair = 0.5f;
	gHUD.g_pModel = szWeaponModel - gpGlobals->pStringBase;
	m_pPlayer->m_flNextAttack = 0.5f;
	m_flTimeWeaponIdle = 1.0f;
	return TRUE;
}

/*
=====================
CBasePlayerWeapon::TimedDeploy

=====================
*/
BOOL CBasePlayerWeapon::TimedDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, 
	const char *szAnimReloadExt, float idleTime, float attackTime, int skiplocal )
{
	if( !CanDeploy() )
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );

	SendWeaponAnim( iAnim, skiplocal, 0 );

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
	m_fInAttack = FALSE;
	m_pPlayer->pev->viewmodel = 0; 
	gHUD.m_iSensLevel = 0;
}

/*
=====================
CBasePlayerWeapon::SendWeaponAnim

Animate weapon model
=====================
*/
void CBasePlayerWeapon::SendWeaponAnim( int iAnim, int skiplocal, int body )
{
	m_pPlayer->pev->weaponanim = iAnim;

	HUD_SendWeaponAnim( iAnim, 0 );
}

/*
=====================
CBaseEntity::PostMortarValue

=====================
*/
void CBasePlayerWeapon::PostMortarValue( float value )
{
	// Nothing.
}

/*
=====================
CBaseEntity::SendMortarFireCommand

=====================
*/
void CBasePlayerWeapon::SendMortarFireCommand( char *c )
{
	gEngfuncs.pfnServerCmd( c );
}

/*
=====================
CBaseEntity::FireBulletsPlayer

Only produces random numbers to match the server ones.
=====================
*/
Vector CBaseEntity::FireBulletsNC ( Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	float x = 0.0f, y = 0.0f, z;

	if( pevAttacker == NULL )
	{
		do {
			x = RANDOM_FLOAT( -0.5f, 0.5f ) + RANDOM_FLOAT( -0.5f, 0.5f );
			y = RANDOM_FLOAT( -0.5f, 0.5f ) + RANDOM_FLOAT( -0.5f, 0.5f );
			z = x * x + y * y;
		} while( z > 1 );
	}
	else
	{
		x = UTIL_SharedRandomFloat( shared_rand, -0.5f, 0.5f ) + UTIL_SharedRandomFloat( shared_rand + 1 , -0.5f, 0.5f );
		y = UTIL_SharedRandomFloat( shared_rand + 2, -0.5f, 0.5f ) + UTIL_SharedRandomFloat( shared_rand + 3, -0.5f, 0.5f );
		// z = x * x + y * y;
	}
	return Vector( x * flSpread, y * flSpread, 0.0f );
}

/*
=====================
CBasePlayerWeapon::ItemPostFrame

Handles weapon firing, reloading, etc.
=====================
*/
void CBasePlayerWeapon::ItemPostFrame( void )
{
	int *pAmmo;
	int j;
	CRocketWeapon *pRocketWpn;
	bool bAutoreload;
	ItemInfo info, ii;

	GetItemInfo( &info );

	pAmmo = current_ammo;
	if( !pAmmo )
	{
		pAmmo = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
	}

	if( m_fInReload && m_flNextPrimaryAttack <= 0.0f && *pAmmo > 0 )
	{
		if( Classify() == CLASS_ROCKET )
		{
			if( m_iWeaponState & WPNSTATE_ROCKET_SLOW )
			{
				m_iWeaponState &= ~WPNSTATE_ROCKET_SLOW;
				pRocketWpn->ReSlow();
			}
		}

		if( m_iId == WEAPON_ENFIELD )
		{
			j = 5;
			if( *pAmmo <= 5 )
				j = *pAmmo;
		}
		else
		{
			j = CBasePlayerItem::ItemInfoArray[m_iId].iMaxClip;
			if( *pAmmo <= j )
				j = *pAmmo;
		}

		m_iClip = j;
		*pAmmo -= j;
		m_fInReload = FALSE;
		m_fFireOnEmpty = FALSE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
	}

	if( m_fInAttack && ( m_pPlayer->pev->button & IN_ATTACK ) == 0 )
		m_fInAttack = FALSE;

	GetItemInfo( &ii );

	if( ii.iFlags & ITEM_FLAG_HEAT )
	{
		m_flWeaponHeat = g_flWeaponHeat;
		if( g_flWeaponHeat < 0.0f )
			m_flWeaponHeat = 0.0f;
	}

	int button = m_pPlayer->pev->button;

	if( ( button & IN_ATTACK2 ) && m_flNextSecondaryAttack <= 0.0f )
	{
		if( CBasePlayerItem::ItemInfoArray[m_iId].pszAmmo2 && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()] )
			m_fFireOnEmpty = TRUE;

		if( gHUD.m_iRoundState == 1 )
			SecondaryAttack();

		m_pPlayer->pev->button &= ~IN_ATTACK2;

		if( ShouldWeaponIdle() )
			WeaponIdle();

		return;
	}

	if( ( button & IN_ATTACK ) && m_flNextPrimaryAttack <= 0.0f )
	{
		if( ( !m_iClip && info.pszAmmo1 ) || ( info.iMaxClip == -1 && !*pAmmo ) )
			m_fFireOnEmpty = TRUE;

		if( gHUD.m_iRoundState == 1 )
			PrimaryAttack();

		gHUD.i_Recoil = 1;

		if( ShouldWeaponIdle() )
			WeaponIdle();

		return;
	}

	if( ( button & IN_RELOAD ) && CBasePlayerItem::ItemInfoArray[m_iId].iMaxClip != -1 
		&& !m_fInReload && gHUD.m_bAutoReloadComplete )
	{
		if( m_pPlayer->pev->waterlevel <= 2 )
		{
			Reload();
			return;
		}
	}
	else
	{
		m_fFireOnEmpty = FALSE;
		gHUD.i_Recoil = 0;

		bAutoreload = ( cl_autoreload->value > 0.0f );

		if( bAutoreload
			&& gHUD.m_bAutoReloadComplete
			&& !m_iClip
			&& ( info.iFlags & ITEM_FLAG_NOAUTOSWITCHEMPTY ) == 0
			&& m_flNextPrimaryAttack < 0.0f
			&& ( g_iUser3 != 2 && g_iVuser1x != 2 || Classify() != CLASS_MACHINEGUNS )
			&& m_pPlayer->pev->waterlevel <= 2 )
		{
			Reload();
			return;
		}
	}

	if( ShouldWeaponIdle() )
	{
		WeaponIdle();
	}
}

/*
=====================
SetScopeId

=====================
*/
void SetScopeId( int id )
{
	if ( g_iVuser1z == 0 )
	{
		gHUD.m_Scope.SetScope( id );
		gHUD.m_Scope.m_iFlags |= HUD_ACTIVE;
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

	if( g_lastFOV <= 0.0f )
	{
		g_lastFOV = fov;
		gHUD.m_Scope.SetScope( m_iId );
	}
	else
	{
		g_lastFOV = 0.0f;
		gHUD.m_Scope.SetScope( WEAPON_NONE );
	}

	gHUD.m_Scope.m_iFlags |= HUD_ACTIVE;
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
	g_lastFOV = 20.0f;
	return 0;
}

/*
=====================
CBasePlayerWeapon::ZoomOut

=====================
*/
int CBasePlayerWeapon::ZoomOut( void )
{
	if( g_iVuser1z || m_pPlayer->pev->fuser2 != g_lastFOV )
		return 0;

	g_lastFOV = 0.0f;
	SetScopeId( WEAPON_NONE );
	return 0;
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
	int WaterLevel = gHUD.GetWaterLevel();

	if( WaterLevel != 3 )
	{
		if( WaterLevel > 0 )
			return g_iUser3 == true;
		return false;
	}
	return true;
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
void CBasePlayerWeapon::ThinkZoomIn()
{
	vec3_t length = m_pPlayer->pev->velocity;

	if( sqrt( length.x * length.x + length.y * length.y + length.z * length.z ) <= 45.0f )
	{
		if( ( m_pPlayer->pev->button & 1 ) == 0 )
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
Vector CBasePlayerWeapon::Aim( float accuracyFactor, CBasePlayer *pOther, unsigned int shared_rand )
{
	float targetx, targety, targetz;

	targetx = UTIL_SharedRandomFloat( shared_rand, accuracyFactor, 0.0 );
	targety = UTIL_SharedRandomFloat( shared_rand, accuracyFactor, 0.0 );
	targetz = UTIL_SharedRandomFloat( shared_rand, accuracyFactor, 0.0 );

	return Vector( targetx, targety, targetz );
}

/*
=====================
CBasePlayer::SelectItem

  Switch weapons
=====================
*/
void CBasePlayer::SelectItem( const char *pstr )
{
	if( !pstr )
		return;

	CBasePlayerItem *pItem = NULL;

	if( !pItem )
		return;

	if( pItem == m_pActiveItem )
		return;

	if( m_pActiveItem )
		m_pActiveItem->Holster();

	m_pLastItem = m_pActiveItem;
	m_pActiveItem = pItem;

	if( m_pActiveItem )
	{
		m_pActiveItem->Deploy();
	}
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

	if( m_pActiveItem )
		m_pActiveItem->Holster();

	CBasePlayerItem *pTemp = m_pActiveItem;
	m_pActiveItem = m_pLastItem;
	m_pLastItem = pTemp;
	m_pActiveItem->Deploy( );
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
		m_pActiveItem->Deploy( );
}

bool CBasePlayer::IsInMGDeploy( void )
{
	if( g_iUser3 != 2 )
		return g_iVuser1x == 2;

	return true;
}

bool CBasePlayer::IsProneDeployed( void )
{
	return g_iUser3 == 2;
}

bool CBasePlayer::IsSandbagDeployed( void )
{
	return g_iVuser1x == 2;
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

=====================
*/
void CBasePlayerWeapon::PrintState( void )
{
	COM_Log( "c:\\hl.log", "%.4f ", gpGlobals->time );
	COM_Log( "c:\\hl.log", "%.4f ", m_pPlayer->m_flNextAttack );
	COM_Log( "c:\\hl.log", "%.4f ", m_flNextPrimaryAttack );
	COM_Log( "c:\\hl.log", "%.4f ", m_flTimeWeaponIdle - gpGlobals->time );
	COM_Log( "c:\\hl.log", "%.4f ", m_iClip );
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

	// Fake functions
	g_engfuncs.pfnPrecacheModel = stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound = stub_PrecacheSound;
	g_engfuncs.pfnPrecacheEvent = stub_PrecacheEvent;
	g_engfuncs.pfnNameForFunction = stub_NameForFunction;
	g_engfuncs.pfnSetModel = stub_SetModel;
	g_engfuncs.pfnSetClientMaxspeed = HUD_SetMaxSpeed;

	// Handled locally
	g_engfuncs.pfnPlaybackEvent = HUD_PlaybackEvent;
	g_engfuncs.pfnAlertMessage = AlertMessage;

	// Pass through to engine
	g_engfuncs.pfnPrecacheEvent = gEngfuncs.pfnPrecacheEvent;
	g_engfuncs.pfnRandomFloat = gEngfuncs.pfnRandomFloat;
	g_engfuncs.pfnRandomLong = gEngfuncs.pfnRandomLong;

	// Allocate a slot for the local player
	HUD_PrepEntity( &player, NULL );

	// Allocate slot(s) for each weapon that we are going to be predicting
	HUD_PrepEntity( &g_Colt, &player );
	HUD_PrepEntity( &g_Luger, &player );
	HUD_PrepEntity( &g_Garand, &player );
	HUD_PrepEntity( &g_ScopedKar, &player );
	HUD_PrepEntity( &g_Thompson, &player );
	HUD_PrepEntity( &g_MP44, &player );
	HUD_PrepEntity( &g_MP40, &player );
	HUD_PrepEntity( &g_Spring, &player );
	HUD_PrepEntity( &g_KAR, &player );
	HUD_PrepEntity( &g_BAR, &player );
	HUD_PrepEntity( &g_MG42, &player );
	HUD_PrepEntity( &g_MG34, &player );
	HUD_PrepEntity( &g_30CAL, &player );
	HUD_PrepEntity( &g_AmerKnife, &player );
	HUD_PrepEntity( &g_GerKnife, &player );
	HUD_PrepEntity( &g_Spade, &player );
	HUD_PrepEntity( &g_M1Carbine, &player );
	HUD_PrepEntity( &g_GreaseGun, &player );
	HUD_PrepEntity( &g_FG42, &player );
	HUD_PrepEntity( &g_K43, &player );
	HUD_PrepEntity( &g_Enfield, &player );
	HUD_PrepEntity( &g_Sten, &player );
	HUD_PrepEntity( &g_Bren, &player );
	HUD_PrepEntity( &g_Webley, &player );
	HUD_PrepEntity( &g_Bazooka, &player );
	HUD_PrepEntity( &g_Pschreck, &player );
	HUD_PrepEntity( &g_PIAT, &player );
	HUD_PrepEntity( &g_HandGrenade, &player );
	HUD_PrepEntity( &g_StickGrenade, &player );
	HUD_PrepEntity( &g_HandGrenadeEx, &player );
	HUD_PrepEntity( &g_StickGrenadeEx, &player );
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
	CBasePlayerWeapon *pWeapon = NULL;
	CBasePlayerWeapon *pCurrent = NULL;
	CBasePlayerWeapon *pNew = NULL;
	weapon_data_t *pto;
	static int lasthealth;
	BOOL canHolster;

	weapon_data_t nulldata;
	memset( &nulldata, 0, sizeof( weapon_data_t ) );

	HUD_InitClientWeapons();

	gpGlobals->time = time;

	if( to->client.health <= 0.0f )
		gHUD.i_Recoil = 0;

	if( gHUD.m_iRoundState != 1 )
		gHUD.i_Recoil = 0;

	switch( from->client.m_iId )
	{
	case WEAPON_COLT:          pWeapon = &g_Colt; break;
	case WEAPON_LUGER:         pWeapon = &g_Luger; break;
	case WEAPON_GARAND:        pWeapon = &g_Garand; break;
	case WEAPON_SCOPEDKAR:     pWeapon = &g_ScopedKar; break;
	case WEAPON_THOMPSON:      pWeapon = &g_Thompson; break;
	case WEAPON_MP44:          pWeapon = &g_MP44; break;
	case WEAPON_MP40:          pWeapon = &g_MP40; break;
	case WEAPON_SPRING:        pWeapon = &g_Spring; break;
	case WEAPON_KAR:           pWeapon = &g_KAR; break;
	case WEAPON_BAR:           pWeapon = &g_BAR; break;
	case WEAPON_MG42:          pWeapon = &g_MG42; break;
	case WEAPON_MG34:          pWeapon = &g_MG34; break;
	case WEAPON_CAL30:         pWeapon = &g_30CAL; break;
	case WEAPON_AMERKNIFE:     pWeapon = &g_AmerKnife; break;
	case WEAPON_GERKNIFE:      pWeapon = &g_GerKnife; break;
	case WEAPON_SPADE:         pWeapon = &g_Spade; break;
	case WEAPON_M1CARBINE:     pWeapon = &g_M1Carbine; break;
	case WEAPON_GREASEGUN:     pWeapon = &g_GreaseGun; break;
	case WEAPON_FG42:          pWeapon = &g_FG42; break;
	case WEAPON_K43:           pWeapon = &g_K43; break;
	case WEAPON_ENFIELD:       pWeapon = &g_Enfield; break;
	case WEAPON_STEN:          pWeapon = &g_Sten; break;
	case WEAPON_BREN:          pWeapon = &g_Bren; break;
	case WEAPON_WEBLEY:        pWeapon = &g_Webley; break;
	case WEAPON_BAZOOKA:       pWeapon = &g_Bazooka; break;
	case WEAPON_PSCHRECK:      pWeapon = &g_Pschreck; break;
	case WEAPON_PIAT:          pWeapon = &g_PIAT; break;
	case WEAPON_HANDGRENADE:   pWeapon = &g_HandGrenade; break;
	case WEAPON_HANDGRENADEX:  pWeapon = &g_HandGrenadeEx; break;
	case WEAPON_STICKGRENADE:  pWeapon = &g_StickGrenade; break;
	case WEAPON_STICKGRENADEX: pWeapon = &g_StickGrenadeEx; break;
	}

	if( !pWeapon )
		return;

	for( int j = 0; j < MAX_WEAPONS; j++ )
	{
		pCurrent = g_pWpns[j];
		if( !pCurrent )
		{
			continue;
		}

		pCurrent->m_fInReload = from->weapondata[j].m_fInReload;
		pCurrent->m_fInSpecialReload = from->weapondata[j].m_fInSpecialReload;
		pCurrent->m_iClip = from->weapondata[j].m_iClip;
		pCurrent->m_flNextPrimaryAttack = from->weapondata[j].m_flNextPrimaryAttack;
		pCurrent->m_flNextSecondaryAttack = from->weapondata[j].m_flNextSecondaryAttack;
		pCurrent->m_flTimeWeaponIdle = from->weapondata[j].m_flTimeWeaponIdle;
		pCurrent->m_iWeaponState = from->weapondata[j].m_iWeaponState;
		pCurrent->m_flWeaponHeat = from->weapondata[j].fuser1;

	}

	g_iWeaponFlags = pWeapon->m_iWeaponState;
	g_flWeaponHeat = pWeapon->m_flWeaponHeat;

	player.random_seed = random_seed;
	player.m_afButtonLast = from->playerstate.oldbuttons;

	int buttons = cmd->buttons;
	player.m_afButtonPressed = ( buttons ^ player.m_afButtonLast ) & buttons;
	player.m_afButtonReleased = ( buttons ^ player.m_afButtonLast ) & ~cmd->buttons;

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

	gHUD.m_vecVelocity = player.pev->velocity;
	player.m_rgAmmo[1] = ( int ) from->client.ammo_shells;

	if( gEngfuncs.GetLocalPlayer() )
	{
		player.pev->origin = from->client.origin;
		player.pev->v_angle = v_angles;
		player.pev->angles = gEngfuncs.GetLocalPlayer()->angles;
	}

	player.pev->fuser2 = from->client.fuser2;

	if( from->client.m_iId )
	{
		player.m_pActiveItem = g_pWpns[from->client.m_iId];
	}

	g_finalstate = to;

	if( ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) && 
			!CL_IsDead() && player.pev->viewmodel && !g_iUser1
			&& player.m_flNextAttack <= 0 )
	{
			pWeapon->ItemPostFrame();
	}

	if( g_runfuncs )
	{
		if( to->client.health > 0.0f || lasthealth <= 0 )
		{
			if( to->client.health > 0.0f && lasthealth <= 0 && player.m_pActiveItem )
			{
				if( player.m_pActiveItem->m_iId == WEAPON_M1CARBINE || player.m_pActiveItem->m_iId == WEAPON_GREASEGUN )
					player.m_pActiveItem->SpawnDeploy();
				else
					player.m_pActiveItem->Deploy();

			}
		}
		else if( player.m_pActiveItem )
			player.m_pActiveItem->Holster();

		lasthealth = to->client.health;
	}

	player.PostThink();

	to->client.m_iId = from->client.m_iId;

	if( cmd->weaponselect && ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) )
	{
		if( from->weapondata[cmd->weaponselect].m_iId == cmd->weaponselect )
		{
			pNew = g_pWpns[cmd->weaponselect];

			if( pNew && ( pNew != pWeapon ) )
			{
				canHolster = player.m_pActiveItem->CanHolster() != FALSE;

				if( canHolster && player.m_pActiveItem )
				{
					player.m_pActiveItem->Holster();
					CBasePlayerItem *m_pActiveItem = player.m_pActiveItem;
					player.m_pActiveItem = pNew;
					player.m_pLastItem = m_pActiveItem;
				}
				else
				{
					player.m_pLastItem = player.m_pActiveItem;
					player.m_pActiveItem = pNew;

					if( !canHolster )
					{
						pNew->Deploy();
					}
				}

				to->client.m_iId = cmd->weaponselect;
			}
		}
	}

	to->client.viewmodel = player.pev->viewmodel;
	to->client.fov = player.pev->fov;
	to->client.weaponanim = player.pev->weaponanim;
	to->client.m_flNextAttack = player.m_flNextAttack;
	to->client.maxspeed = player.pev->maxspeed;
	to->client.ammo_shells = player.m_rgAmmo[1];
	to->client.fuser2 = player.pev->fuser2;

	if( g_runfuncs && ( HUD_GetWeaponAnim() != to->client.weaponanim ) )
		HUD_SendWeaponAnim( to->client.weaponanim, 1 );

	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		pCurrent = g_pWpns[i];
		pto = &to->weapondata[i];

		if( !pCurrent )
		{
			memset( pto, 0, sizeof(weapon_data_t) );
			continue;
		}

		pto->m_fInReload = pCurrent->m_fInReload;
		pto->m_iClip = pCurrent->m_iClip;

		float m_flNextPrimaryAttack = pCurrent->m_flNextPrimaryAttack;
		pto->m_flNextPrimaryAttack = m_flNextPrimaryAttack;

		float m_flNextSecondaryAttack = pCurrent->m_flNextSecondaryAttack;
		pto->m_flNextSecondaryAttack = m_flNextSecondaryAttack;

		if( to->client.m_iId == pCurrent->m_iId )
		{
			gHUD.g_iClip = pCurrent->m_iClip;
			m_flNextPrimaryAttack = pto->m_flNextPrimaryAttack;
			m_flNextSecondaryAttack = pto->m_flNextSecondaryAttack;
			g_flNextPrimaryAttack = pCurrent->m_flNextPrimaryAttack;
			g_flNextSecondaryAttack = pCurrent->m_flNextSecondaryAttack;
		}

		float m_flTimeWeaponIdle = pCurrent->m_flTimeWeaponIdle;
		pto->m_flTimeWeaponIdle = m_flTimeWeaponIdle;
		pto->m_iWeaponState = pCurrent->m_iWeaponState;

		float flFrameTime = ( float ) cmd->msec / 1000.0f;

		pto->m_flNextReload -= flFrameTime;
		pto->m_flNextPrimaryAttack -= flFrameTime;
		pto->m_flNextSecondaryAttack -= flFrameTime;
		pto->m_flTimeWeaponIdle -= flFrameTime;

		if( pto->m_flNextPrimaryAttack < -1.0f )
		{
			pto->m_flNextPrimaryAttack = -1.0f;
		}

		if( pto->m_flNextSecondaryAttack < -0.001f )
		{
			pto->m_flNextSecondaryAttack = -0.001f;
		}

		if( pto->m_flTimeWeaponIdle < -0.001f )
		{
			pto->m_flTimeWeaponIdle = -0.001f;
		}

		if( pto->m_flNextReload < -0.001f )
		{
			pto->m_flNextReload = -0.001f;
		}
	}

	float flFrameTime = ( float ) cmd->msec / 1000.0f;

	to->client.m_flNextAttack -= flFrameTime;
	if( to->client.m_flNextAttack < -0.001f )
	{
		to->client.m_flNextAttack = -0.001f;
	}

	gHUD.g_NextAttack = to->client.m_flNextAttack;

	previousorigin.x = to->playerstate.origin.x + to->client.view_ofs.x;
	previousorigin.y = to->playerstate.origin.y + to->client.view_ofs.y;
	previousorigin.z = to->playerstate.origin.z + to->client.view_ofs.z;

	g_finalstate = NULL;
	return;
}

void DoD_GetSequence( int *seq, int *gaitseq )
{
	*seq = g_rseq;
	*gaitseq = g_gaitseq;
}

void DoD_SetSequence( int seq, int gaitseq )
{
	g_rseq = seq;
	g_gaitseq = gaitseq;
}

void DoD_SetOrientation( vec3_t *o, vec3_t *a )
{
	if( o && a )
	{
		g_clorg[0] = ( *o )[0];
		g_clorg[1] = ( *o )[1];
		g_clorg[2] = ( *o )[2];

		g_clang[0] = ( *a )[0];
		g_clang[1] = ( *a )[1];
		g_clang[2] = ( *a )[2];
	}
}

void DoD_GetOrientation( float *o, float *a )
{
	vec3_t *pOutOrigin = ( vec3_t * ) o;
	vec3_t *pOutAngles = ( vec3_t * ) a;

	*pOutOrigin = g_clorg;
	*pOutAngles = g_clang;
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

	if( cl_lw && cl_lw->value )
	{
		HUD_WeaponsPostThink( from, to, cmd, time, random_seed );
	}
	else
	{
		to->client.fov = g_lastFOV;
	}

	if( g_runfuncs )
	{
		DoD_SetSequence( to->playerstate.gaitsequence, to->playerstate.sequence );
		DoD_SetOrientation( &to->playerstate.origin, &cmd->viewangles );
	}

	// All games can use FOV state
	g_lastFOV = to->client.fov;
}