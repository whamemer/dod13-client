/*#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
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

// Pool of client side entities/entvars_t
static entvars_t ev[32];
static int num_ents = 0;

// The entity we'll use to represent the local client
static CBasePlayer player;

// Local version of game .dll global variables ( time, etc. )
static globalvars_t Globals; 

static CBasePlayerWeapon *g_pWpns[32];

float g_flApplyVel = 0.0;
int g_irunninggausspred = 0;

vec3_t previousorigin;

// HLDM Weapon placeholder entities.
C30CAL g_30CAL;
CAmerKnife g_AmerKnife;
CBAR g_BAR;
CBazooka g_Bazooka;
CBREN g_Bren;
CCOLT g_Colt;
CENFIELD g_Enfield;
CFG42 g_FG42;
CGarand g_Garand;
CGerKnife g_GerKnife;
CGreaseGun g_GreaseGun;
CHandGrenade g_HandGrenade;
CHandGrenadeEx g_HandGrenadeEx;
CK43 g_K43;
CKAR g_KAR;
CLuger g_Luger;
CM1Carbine g_M1Carbine;
CMG34 g_MG34;
CMG42 g_MG42;
CMP40 g_MP40;
CMP44 g_MP44;
CPIAT g_PIAT;
CPschreck g_Pschreck;
CScopedKar g_ScopedKar;
CSpade g_Spade;
CSPRING g_Spring;
CSTEN g_Sten;
CStickGrenade g_StickGrenade;
CStickGrenadeEx g_StickGrenadeEx;
CThompson g_Thompson;
CWEBLEY g_Webley;

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

bool CBasePlayer::IsInMGDeploy( void )
{

}

bool CBasePlayer::IsProneDeployed( void )
{

}

Vector CBasePlayerWeapon::Aim( float accuracyFactor, CBasePlayer *pOther, int shared_rand )
{

}

int CBasePlayerWeapon::ChangeFOV( int fov )
{

}


int CBasePlayerWeapon::Classify( void )
{

}

int CBasePlayerWeapon::GetFOV( void )
{

}

int CBasePlayerWeapon::GetRoundState( void )
{

}

bool CBasePlayerWeapon::PlayerIsWaterSniping( void )
{

}

void CBasePlayerWeapon::PostMortarValue( float value )
{

}

void CBasePlayerWeapon::SendMortarFireCommand( char *c )
{

}

void CBasePlayerWeapon::ThinkZoomIn( void )
{

}

void CBasePlayerWeapon::ThinkZoomOut( void )
{
    
}

BOOL CBasePlayerWeapon::TimedDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, char *szAnimReloadExt, float idleTime, float attackTime, int skiplocal )
{

}

void CBasePlayerWeapon::UpdateZoomSpeed( void )
{

}

int CBasePlayerWeapon::ZoomIn( void )
{

}

int CBasePlayerWeapon::ZoomOut( void )
{
    
}

float CBasePlayerWeapon::flAim( float accuracyFactor, CBasePlayer *pOther )
{

}

void DoD_GetOrientation( float *o, float *a )
{

}

void DoD_GetSequence( int *seq, int *gaitseq )
{

}

void DoD_SetOrientation( Vector *p_o, Vector *p_a )
{

}

void DoD_SetSequence( int seq, int gaitseq )
{

}

void SetScopeId( int Id )
{

}


void HUD_SetLastOrg( void )
{
	int i;

	// Offset final origin by view_offset
	for( i = 0; i < 3; i++ )
	{
		previousorigin[i] = g_finalstate->playerstate.origin[i] + g_finalstate->client.view_ofs[i];
	}
}

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
	HUD_PrepEntity( &g_30CAL, &player );
	HUD_PrepEntity( &g_AmerKnife, &player );
	HUD_PrepEntity( &g_BAR, &player );
	HUD_PrepEntity( &g_Bazooka, &player );
	HUD_PrepEntity( &g_Bren, &player );
	HUD_PrepEntity( &g_Colt, &player );
	HUD_PrepEntity( &g_Enfield, &player );
	HUD_PrepEntity( &g_FG42, &player );
	HUD_PrepEntity( &g_Garand, &player );
	HUD_PrepEntity( &g_GerKnife, &player );
	HUD_PrepEntity( &g_GreaseGun, &player );
	HUD_PrepEntity( &g_HandGrenade, &player );
	HUD_PrepEntity( &g_HandGrenadeEx, &player );
	HUD_PrepEntity( &g_K43, &player );
    HUD_PrepEntity( &g_KAR, &player );
    HUD_PrepEntity( &g_Luger, &player );
    HUD_PrepEntity( &g_M1Carbine, &player );
    HUD_PrepEntity( &g_MG34, &player );
    HUD_PrepEntity( &g_MG42, &player );
    HUD_PrepEntity( &g_MP40, &player );
    HUD_PrepEntity( &g_MP44, &player );
    HUD_PrepEntity( &g_PIAT, &player );
    HUD_PrepEntity( &g_Pschreck, &player );
    HUD_PrepEntity( &g_ScopedKar, &player );
    HUD_PrepEntity( &g_Spade, &player );
    HUD_PrepEntity( &g_Spring, &player );
    HUD_PrepEntity( &g_Sten, &player );
    HUD_PrepEntity( &g_StickGrenade, &player );
    HUD_PrepEntity( &g_StickGrenadeEx, &player );
    HUD_PrepEntity( &g_Thompson, &player );
    HUD_PrepEntity( &g_Webley, &player );
}

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

	// Fill in data based on selected weapon
	// FIXME, make this a method in each weapon?  where you pass in an entity_state_t *?
	switch( from->client.m_iId )
	{
		case WEAPON_AMER_KNIFE:
			pWeapon = &g_AmerKnife;
			break;
		case WEAPON_GER_KNIFE:
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

	for( i = 0; i < 32; i++ )
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
	}

	int g_iWeaponFlags = pWeapon->m_iWeaponState;
	float g_flWeaponHeat = pWeapon->m_flWeaponHeat;

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
	player.pev->fov = from->client.fov;
	player.pev->weaponanim = from->client.weaponanim;
	player.pev->viewmodel = from->client.viewmodel;
	player.m_flNextAttack = from->client.m_flNextAttack;
	player.m_flNextAmmoBurn = from->client.fuser2;
	player.m_flAmmoStartCharge = from->client.fuser3;
}
*/