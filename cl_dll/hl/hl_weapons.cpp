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
extern float flBoltHideXHair;

static entvars_s ev[64];
static int num_ents;
static CBasePlayer player;
static globalvars_t Globals;
static CBasePlayerWeapon *g_pWpns[64];
static int g_gaitseq;
static vec3_t g_clang;

float g_flNextPrimaryAttack, g_flNextSecondaryAttack;

extern int g_iWeaponFlags;
extern float g_flWeaponHeat;
extern cvar_t *cl_autoreload;

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

CCOLT               g_Colt;
CLUGER              g_Luger;
CGarand             g_Garand;
CScopedKar          g_ScopedKar;
CThompson           g_Thompson;
CSPRING             g_Spring;
CKAR                g_KAR;
CBAR                g_BAR;
CMP40               g_MP40;
CMP44               g_MP44;
CMG42               g_MG42;
C30CAL              g_30CAL;
CMG34               g_MG34;
CAmerKnife          g_AmerKnife;
CGerKnife           g_GerKnife;
CSpade              g_Spade;
CM1Carbine          g_M1Carbine;
CGreaseGun          g_GreaseGun;
CFG42               g_FG42;
CK43                g_K43;
CENFIELD            g_Enfield;
CSTEN               g_Sten;
CBREN               g_Bren;
CWEBLEY             g_Webley;
CBazooka            g_Bazooka;
CPschreck           g_Pschreck;
CPIAT               g_PIAT;
CHandGrenade        g_HandGrenade;
CStickGrenade       g_StickGrenade;
CHandGrenadeEx      g_HandGrenadeEx;
CStickGrenadeEx     g_StickGrenadeEx;

void AlertMessage( ALERT_TYPE atype, const char *szFmt, ... )
{

}

bool bIsMultiplayer( void )
{
    
}

void LoadVModel( const char *szViewModel, CBasePlayer *m_pPlayer )
{

}

void HUD_PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner, int ammotype )
{

}

void CBaseEntity::Killed( entvars_t *pevAttacker, int iGib )
{

}

BOOL CBasePlayerWeapon::CanDeploy( void ) 
{

}

BOOL CBasePlayerWeapon::DefaultDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, const char *szAnimReloadExt, int skiplocal )
{

}

BOOL CBasePlayerWeapon::TimedDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, char *szAnimReloadExt, float idleTime, float attackTime, int skiplocal )
{

}

BOOL CBasePlayerWeapon::PlayEmptySound( void )
{

}

void CBasePlayerWeapon::Holster( int skiplocal )
{

}

void CBasePlayerWeapon::SendWeaponAnim( int iAnim, int skiplocal )
{

}

void CBasePlayerWeapon::PostMortarValue( float value )
{

}

void CBasePlayerWeapon::SendMortarFireCommand( char *c )
{

}

BOOL CBasePlayerWeapon::DefaultReload( int iClipSize, int iAnim, float fDelay )
{

}

void CBasePlayerWeapon::ItemPostFrame( void )
{

}

void SetScopeId( int id )
{

}

int CBasePlayerWeapon::ChangeFOV( int fov )
{

}

int CBasePlayerWeapon::ZoomIn( void )
{

}

int CBasePlayerWeapon::ZoomOut( void )
{

}

void CBasePlayerWeapon::UpdateZoomSpeed( void )
{

}

int CBasePlayerWeapon::GetFOV( void )
{

}

bool CBasePlayerWeapon::PlayerIsWaterSniping( void )
{

}

void CBasePlayerWeapon::ThinkZoomOut( void )
{

}

void CBasePlayerWeapon::ThinkZoomIn( void )
{

}

float CBasePlayerWeapon::flAim( float accuracyFactor, CBasePlayer *pOther )
{

}

Vector CBasePlayerWeapon::Aim( float accuracyFactor, CBasePlayer *pOther, int shared_rand )
{

}

void CBasePlayerWeapon::RemoveStamina( float removeAmount, CBasePlayer *pOther )
{

}

int CBasePlayerWeapon::Classify( void )
{

}

int CBasePlayerWeapon::GetRoundState( void )
{

}

void CBasePlayer::SelectItem( const char *pstr )
{

}

void CBasePlayer::SelectLastItem( void )
{

}

void CBasePlayer::Killed( entvars_t *pevAttacker, int iGib )
{

}

void CBasePlayer::Spawn( void )
{

}

bool CBasePlayer::IsInMGDeploy( void )
{

}

bool CBasePlayer::IsProneDeployed( void )
{

}

bool CBasePlayer::IsSandbagDeployed( void )
{

}

void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{

}

void UTIL_ParticleBox( CBasePlayer *player, float *mins, float *maxs, float life, unsigned char r, unsigned char g, unsigned char b )
{

}

void UTIL_ParticleBoxes( void )
{

}

void UTIL_ParticleLine( CBasePlayer *player, float *start, float *end, float life, unsigned char r, unsigned char g, unsigned char b )
{

}

void CBasePlayerWeapon::PrintState( void )
{

}

void HUD_InitClientWeapons( void )
{

}

void HUD_GetLastOrg( float *org )
{

}

void HUD_SetLastOrg( void )
{

}

void HUD_WeaponsPostThink( local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed )
{

}

void DoD_GetSequence( int *seq, int *gaitseq )
{

}

void DoD_SetSequence( int seq, int gaitseq )
{

}

void DoD_SetOrientation( Vector o, Vector a )
{

}

void DoD_GetOrientation( float *o, float *a )
{

}

void _DLLEXPORT HUD_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed )
{

}

Vector CBaseEntity::FireBulletsNC( Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{

}