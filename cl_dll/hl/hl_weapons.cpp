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
class CCOLT               g_Colt;
class CLUGER              g_Luger;
class CGarand             g_Garand;
class CScopedKar          g_ScopedKar;
class CThompson           g_Thompson;
class CSPRING             g_Spring;
class CKAR                g_KAR;
class CBAR                g_BAR;
class CMP40               g_MP40;
class CMP44               g_MP44;
class CMG42               g_MG42;
class C30CAL              g_30CAL;
class CMG34               g_MG34;
class CAmerKnife          g_AmerKnife;
class CGerKnife           g_GerKnife;
class CSpade              g_Spade;
class CM1Carbine          g_M1Carbine;
class CGreaseGun          g_GreaseGun;
class CFG42               g_FG42;
class CK43                g_K43;
class CENFIELD            g_Enfield;
class CSTEN               g_Sten;
class CBREN               g_Bren;
class CWEBLEY             g_Webley;
class CBazooka            g_Bazooka;
class CPschreck           g_Pschreck;
class CPIAT               g_PIAT;
class CHandGrenade        g_HandGrenade;
class CStickGrenade       g_StickGrenade;
class CHandGrenadeEx      g_HandGrenadeEx;
class CStickGrenadeEx     g_StickGrenadeEx;

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
WHAMER: TODO: rework
=====================
*/
BOOL CBasePlayerWeapon::DefaultReload( int iClipSize, int iAnim, float fDelay, int body )
{
	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		return FALSE;

	int j = Q_min( iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] );

	if( j == 0 )
		return FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	//!!UNDONE -- reload sound goes here !!!
	SendWeaponAnim( iAnim, UseDecrement(), body );

	m_fInReload = TRUE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
	return TRUE;
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
WHAMER: TODO: rework
=====================
*/
BOOL CBasePlayerWeapon::DefaultDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt,
	const char *szAnimReloadExt, int skiplocal, int body )
{
	if( !CanDeploy() )
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );

	SendWeaponAnim( iAnim, skiplocal, body );

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
	// need vgui2 support
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
WHAMER: TODO: rework
=====================
*/
void CBasePlayerWeapon::ItemPostFrame( void )
{
	if( ( m_fInReload ) && ( m_pPlayer->m_flNextAttack <= 0.0f ) )
	{
#if 1
		// complete the reload. 
		ItemInfo itemInfo;
		memset( &itemInfo, 0, sizeof( itemInfo ) );
		GetItemInfo( &itemInfo );

		int j = Q_min( itemInfo.iMaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] );

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
#else
		m_iClip += 10;
#endif
		m_fInReload = FALSE;
	}

	if( ( m_pPlayer->pev->button & IN_ATTACK2 ) && ( m_flNextSecondaryAttack <= 0.0f ) )
	{
		if( pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()] )
		{
			m_fFireOnEmpty = TRUE;
		}

		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if( ( m_pPlayer->pev->button & IN_ATTACK ) && ( m_flNextPrimaryAttack <= 0.0f ) )
	{
		if( ( m_iClip == 0 && pszAmmo1() ) || ( iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] ) )
		{
			m_fFireOnEmpty = TRUE;
		}

		PrimaryAttack();
	}
	else if( m_pPlayer->pev->button & IN_RELOAD && iMaxClip() != WEAPON_NOCLIP && !m_fInReload )
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if( !( m_pPlayer->pev->button & ( IN_ATTACK | IN_ATTACK2 ) ) )
	{
		// no fire buttons down
		m_fFireOnEmpty = FALSE;

		// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if( m_iClip == 0 && !( iFlags() & ITEM_FLAG_NOAUTORELOAD ) && m_flNextPrimaryAttack <= 0.0f )
		{
			Reload();
			return;
		}

		WeaponIdle( );
		return;
	}

	// catch all
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

int CBasePlayerWeapon::ZoomIn( void )
{
	if( m_pPlayer->pev->fuser2 != g_lastFOV )
		return 0;
	
	SetScopeId( m_iId );
	g_lastFOV = 20.0f;
	return 0;
}

int CBasePlayerWeapon::ZoomOut( void )
{
	if( g_iVuser1z || m_pPlayer->pev->fuser2 != g_lastFOV )
		return 0;

	g_lastFOV = 0.0f;
	SetScopeId( WEAPON_NONE );
	return 0;
}

int CBasePlayerWeapon::GetFOV( void ) 
{ 
	return g_lastFOV; 
}

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

void CBasePlayerWeapon::ThinkZoomOut( void )
{
	m_pPlayer->m_iFOV = ZoomOut();
	UpdateZoomSpeed();
}

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

float CBasePlayerWeapon::flAim( float accuracyFactor, CBasePlayer *pOther )
{
	return accuracyFactor;
}

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
	int i;
	int buttonsChanged;
	CBasePlayerWeapon *pWeapon = NULL;
	CBasePlayerWeapon *pCurrent;
	weapon_data_t nulldata = {0}, *pfrom, *pto;
	static int lasthealth;

	HUD_InitClientWeapons();

	// Get current clock
	gpGlobals->time = time;

	if( to->client.health <= 0.0f )
		gHUD.i_Recoil = 0;

	if( gHUD.m_iRoundState != 1 )
		gHUD.i_Recoil = 0;

	// Fill in data based on selected weapon
	// FIXME, make this a method in each weapon?  where you pass in an entity_state_t *?
	switch( from->client.m_iId )
	{
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
		case WEAPON_MP40:
			pWeapon = &g_MP40;
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
		case WEAPON_MG42:
			pWeapon = &g_MG42;
			break;
		case WEAPON_MG34:
			pWeapon = &g_MG34;
			break;
		case WEAPON_CAL30:
			pWeapon = &g_30CAL;
			break;
		case WEAPON_AMERKNIFE:
			pWeapon = &g_AmerKnife;
			break;
		case WEAPON_GERKNIFE:
			pWeapon = &g_GerKnife;
			break;
		case WEAPON_SPADE:
			pWeapon = &g_Spade;
			break;
		case WEAPON_M1CARBINE:
			pWeapon = &g_M1Carbine;
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
		case WEAPON_HANDGRENADE:
			pWeapon = &g_HandGrenade;
			break;
		case WEAPON_HANDGRENADEX:
			pWeapon = &g_HandGrenadeEx;
			break;
		case WEAPON_STICKGRENADE:
			pWeapon = &g_StickGrenade;
			break;
		case WEAPON_STICKGRENADEX:
			pWeapon = &g_StickGrenadeEx;
			break;
	}

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	g_finalstate = to;

	// If we are running events/etc. go ahead and see if we
	//  managed to die between last frame and this one
	// If so, run the appropriate player killed or spawn function
	if( g_runfuncs )
	{
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

	// We are not predicting the current weapon, just bow out here.
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
		pCurrent->m_flNextPrimaryAttack	= pfrom->m_flNextPrimaryAttack;
		pCurrent->m_flNextSecondaryAttack = pfrom->m_flNextSecondaryAttack;
		pCurrent->m_flTimeWeaponIdle = pfrom->m_flTimeWeaponIdle;
		pCurrent->m_iWeaponState = pfrom->m_iWeaponState;
		pCurrent->m_flWeaponHeat = pfrom->fuser1;
		pCurrent->pev->fuser1 = pfrom->fuser1;
		pCurrent->m_flStartThrow = pfrom->fuser2;
		pCurrent->m_flReleaseThrow = pfrom->fuser3;
		pCurrent->m_chargeReady = pfrom->iuser1;
		pCurrent->m_fInAttack = pfrom->iuser2;
		pCurrent->m_fireState = pfrom->iuser3;

		pCurrent->m_iSecondaryAmmoType = (int)from->client.vuser3[2];
		pCurrent->m_iPrimaryAmmoType = (int)from->client.vuser4[0];
		player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType] = (int)from->client.vuser4[1];
		player.m_rgAmmo[pCurrent->m_iSecondaryAmmoType] = (int)from->client.vuser4[2];
	}

	g_iWeaponFlags = pWeapon->m_iWeaponState;
	g_flWeaponHeat = pWeapon->m_flWeaponHeat;

	// For random weapon events, use this seed to seed random # generator
	player.random_seed = random_seed;

	// Get old buttons from previous state.
	player.m_afButtonLast = from->playerstate.oldbuttons;

	// Which buttsons chave changed
	buttonsChanged = ( player.m_afButtonLast ^ cmd->buttons );	// These buttons have changed this frame

	// Debounced button codes for pressed/released
	// The changed ones still down are "pressed"
	player.m_afButtonPressed =  buttonsChanged & cmd->buttons;	
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
	player.m_flNextAmmoBurn = from->client.fuser2;
	player.m_flAmmoStartCharge = from->client.fuser3;

	gHUD.m_vecVelocity = player.pev->velocity;

	player.m_rgAmmo[1] = (int)from->client.ammo_shells;

	if( gEngfuncs.GetLocalPlayer() )
	{
		player.pev->origin = from->client.origin;
		player.pev->v_angle = v_angles;
		player.pev->angles = gEngfuncs.GetLocalPlayer()->angles;
	}

	player.pev->fuser2 = from->client.fuser2;

	// Point to current weapon object
	if( from->client.m_iId )
	{
		player.m_pActiveItem = g_pWpns[from->client.m_iId];
	}

	g_finalstate = to;

	// Don't go firing anything if we have died.
	// Or if we don't have a weapon model deployed
	if( ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) && 
		 !CL_IsDead() && player.pev->viewmodel && !g_iUser1 )
	{
		if( player.m_flNextAttack <= 0 )
		{
			pWeapon->ItemPostFrame();
		}
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

	// Assume that we are not going to switch weapons
	to->client.m_iId = from->client.m_iId;

	// Now see if we issued a changeweapon command ( and we're not dead )
	if( cmd->weaponselect && ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) )
	{
		// Switched to a different weapon?
		if( from->weapondata[cmd->weaponselect].m_iId == cmd->weaponselect )
		{
			CBasePlayerWeapon *pNew = g_pWpns[cmd->weaponselect];
			if( pNew && ( pNew != pWeapon ) )
			{
				// Put away old weapon
				if( player.m_pActiveItem )
					player.m_pActiveItem->Holster();

				player.m_pLastItem = player.m_pActiveItem;
				player.m_pActiveItem = pNew;

				// Deploy new weapon
				if( player.m_pActiveItem )
				{
					player.m_pActiveItem->Deploy();
				}

				// Update weapon id so we can predict things correctly.
				to->client.m_iId = cmd->weaponselect;
			}
		}
	}

	// Copy in results of prediction code
	to->client.viewmodel = player.pev->viewmodel;
	to->client.fov = player.pev->fov;
	to->client.weaponanim = player.pev->weaponanim;
	to->client.m_flNextAttack = player.m_flNextAttack;
	to->client.fuser2 = player.m_flNextAmmoBurn;
	to->client.fuser3 = player.m_flAmmoStartCharge;
	to->client.maxspeed = player.pev->maxspeed;
	to->client.ammo_shells = player.m_rgAmmo[1];

	// Make sure that weapon animation matches what the game .dll is telling us
	//  over the wire ( fixes some animation glitches )
	if( g_runfuncs && ( HUD_GetWeaponAnim() != to->client.weaponanim ) )
		HUD_SendWeaponAnim( to->client.weaponanim, 1 );

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		pCurrent = g_pWpns[i];

		pto = &to->weapondata[i];

		if( !pCurrent )
		{
			memset( pto, 0, sizeof(weapon_data_t) );
			continue;
		}

		pto->m_fInReload = pCurrent->m_fInReload;
		pto->m_fInSpecialReload = pCurrent->m_fInSpecialReload;
		pto->m_iClip = pCurrent->m_iClip; 
		pto->m_flNextPrimaryAttack = pCurrent->m_flNextPrimaryAttack;
		pto->m_flNextSecondaryAttack = pCurrent->m_flNextSecondaryAttack;
		pto->m_flTimeWeaponIdle = pCurrent->m_flTimeWeaponIdle;
		pto->fuser1 = pCurrent->pev->fuser1;
		pto->fuser2 = pCurrent->m_flStartThrow;
		pto->fuser3 = pCurrent->m_flReleaseThrow;
		pto->iuser1 = pCurrent->m_chargeReady;
		pto->iuser2 = pCurrent->m_fInAttack;
		pto->iuser3 = pCurrent->m_fireState;

		if( to->client.m_iId == pCurrent->m_iId )
		{
			gHUD.g_iClip = pCurrent->m_iClip;
			g_flNextPrimaryAttack = pto->m_flNextPrimaryAttack;
			g_flNextSecondaryAttack = pto->m_flNextSecondaryAttack;
		}

		pto->m_iWeaponState = pCurrent->m_iWeaponState;

		// Decrement weapon counters, server does this at same time ( during post think, after doing everything else )
		pto->m_flNextReload -= cmd->msec / 1000.0f;
		pto->m_fNextAimBonus -= cmd->msec / 1000.0f;
		pto->m_flNextPrimaryAttack -= cmd->msec / 1000.0f;
		pto->m_flNextSecondaryAttack -= cmd->msec / 1000.0f;
		pto->m_flTimeWeaponIdle -= cmd->msec / 1000.0f;
		pto->fuser1 -= cmd->msec / 1000.0f;

		to->client.vuser3[2] = pCurrent->m_iSecondaryAmmoType;
		to->client.vuser4[0] = pCurrent->m_iPrimaryAmmoType;
		to->client.vuser4[1] = player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType];
		to->client.vuser4[2] = player.m_rgAmmo[pCurrent->m_iSecondaryAmmoType];

		if( pto->m_fNextAimBonus < -1.0f )
		{
			pto->m_fNextAimBonus = -1.0f;
		}

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

		if( pto->fuser1 < -0.001f )
		{
			pto->fuser1 = -0.001f;
		}
	}

	// m_flNextAttack is now part of the weapons, but is part of the player instead
	to->client.m_flNextAttack -= cmd->msec / 1000.0f;
	if( to->client.m_flNextAttack < -0.001f )
	{
		to->client.m_flNextAttack = -0.001f;
	}

	to->client.fuser2 -= cmd->msec / 1000.0f;
	if( to->client.fuser2 < -0.001f )
	{
		to->client.fuser2 = -0.001f;
	}

	to->client.fuser3 -= cmd->msec / 1000.0f;
	if( to->client.fuser3 < -0.001f )
	{
		to->client.fuser3 = -0.001f;
	}

	// Store off the last position from the predicted state.
	HUD_SetLastOrg();

	gHUD.g_NextAttack = to->client.m_flNextAttack -= cmd->msec / 1000.0f;
	previousorigin = g_finalstate->playerstate.origin + g_finalstate->client.view_ofs;

	// Wipe it so we can't use it after this frame
	g_finalstate = NULL;
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
	g_clorg = Vector( *o );
	g_clang = Vector( *a );
}

void DoD_GetOrientation( float *o, float *a )
{
	o = Vector( g_clorg );
	a = Vector( g_clang );
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

#if CLIENT_WEAPONS
	if( cl_lw && cl_lw->value )
	{
		HUD_WeaponsPostThink( from, to, cmd, time, random_seed );
	}
	else
#endif
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