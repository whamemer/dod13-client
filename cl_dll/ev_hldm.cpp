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

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"
#include "pm_shared.h"
#include "cvardef.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <string.h>

#include "r_studioint.h"
#include "com_model.h"

#include "dod_shared.h"
#include "weapons.h"

//extern IParticleMan *g_pParticleMan;

static float fl_TimeMusicLeft;

static int tracerCount[32];
float total_time;

extern char *sHelmetModels[7];

extern cvar_t *cl_dynamiclights;
cvar_t *cl_numshotrubble;

extern float g_flWeaponHeat;

extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;

extern "C"
{
void EV_TrainPitchAdjust( struct event_args_s *args );
void EV_FireColt( struct event_args_s *args );
void EV_FireLuger( struct event_args_s *args );
void EV_FireM1Carbine( struct event_args_s *args );
void EV_FireGarand( struct event_args_s *args );
void EV_FireScopedKar( struct event_args_s *args );
void EV_FireThompson( struct event_args_s *args );
void EV_FireMP44( struct event_args_s *args );
void EV_FireSpring( struct event_args_s *args );
void EV_FireKar( struct event_args_s *args );
void EV_FireMP40( struct event_args_s *args );
void EV_Knife( struct event_args_s *args );
void EV_FireBAR( struct event_args_s *args );
void EV_FireMG42( struct event_args_s *args );
void EV_FireMG34( struct event_args_s *args );
void EV_Fire30CAL( struct event_args_s *args );
void EV_FireGreaseGun( struct event_args_s *args );
void EV_FireFG42( struct event_args_s *args );
void EV_FireK43( struct event_args_s *args );
void EV_FireEnfield( struct event_args_s *args );
void EV_FireSten( struct event_args_s *args );
void EV_FireBren( struct event_args_s *args );
void EV_FireWebley( struct event_args_s *args );
void EV_FireScopedEnfield( struct event_args_s *args );
void EV_FireBazooka( struct event_args_s *args );
void EV_FirePschreck( struct event_args_s *args );
void EV_FirePIAT( struct event_args_s *args );
void EV_FireMortar( struct event_args_s *args );
void EV_FireMelee( struct event_args_s *args );
void EV_Pain( struct event_args_s *args );
void EV_Smoke( struct event_args_s *args );
void EV_BloodSprite( struct event_args_s *args );
void EV_BloodStream( struct event_args_s *args );
void EV_BulletTracers( struct event_args_s *args );
void EV_BubbleTrails( struct event_args_s *args );
void EV_Bubbles( struct event_args_s *args );
void EV_Explosion( struct event_args_s *args );
void EV_SparkShower( struct event_args_s *args );
void EV_PlayWhizz( struct event_args_s *args );
void EV_USVoice( struct event_args_s *args );
void EV_GERVoice( struct event_args_s *args );
void EV_BodyDamage( struct event_args_s *args );
void EV_RoundReleaseSound( struct event_args_s *args );
void EV_DoDCamera( struct event_args_s *args );
void EV_PopHelmet( struct event_args_s *args );
void EV_RoundReset( struct event_args_s *args );
void EV_Overheat( struct event_args_s *args );
void EV_RocketTrail( struct event_args_s *args );
void EV_MortarShell( struct event_args_s *args );
}

enum GUNTYPE_e
{
    PISTOL = 1,
    RIFLE = 2,
    SUBMACHINE = 3,
    MG = 4
};

int EV_GetWeaponBody( void )
{
	int team = g_iTeamNumber;

	if( g_iUser1 == OBS_IN_EYE )
		team = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
	
	if( team == 1 )
		return gHUD.m_bBritish;
	
	return 2;
}

void EV_ResetAnimationEvents( int index )
{
	cl_entity_t *ent = gEngfuncs.GetEntityByIndex( index );
	
	ent->baseline.iuser1 = 0;
	ent->baseline.iuser2 = 0;
	ent->baseline.iuser3 = 0;
	ent->baseline.iuser4 = 0;
}

float EV_HLDM_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType )
{

}

void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, float *vecSrc, float *vecEnd, int iBulletType, char *decalName )
{

}

void EV_RoundReset( event_args_t *args )
{

}

void RemoveBody( TEMPENTITY *te, float frametime, float current_time )
{

}

void HitBody( TEMPENTITY *ent, struct pmtrace_s *ptr )
{

}

void CreateCorpse( Vector vOrigin, Vector vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody )
{

}

void EV_BasicPuff(pmtrace_t *pTrace, float scale)
{

}

void EV_CreteRubble( pmtrace_t *pTrace, float fScale )
{

}

void CreateSpark( Vector origin, Vector vNormal, const char *szSpriteName )
{

}

void EV_MetalHit( pmtrace_t *pTrace )
{

}

void EV_DirtHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_SandHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_GrassHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_WoodChips( pmtrace_t *pTrace, float fScale )
{

}

void EV_GlassShards( pmtrace_t *pTrace, float fScale )
{

}

void EV_WaterHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_LeavesHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_SnowHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_TileHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_PlaySurfaceHitSound( pmtrace_t *pTrace, int iBulletType, char cSurfaceType )
{

}

void EV_HLDM_DoDSurfaceFX( pmtrace_t *pTrace, int iBulletType, char cSurfaceType )
{

}

void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType, float *vecSrc, float *vecEnd )
{

}

int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount, float *tracerOrigin )
{

}

void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount )
{

}

float EV_HLDM_WaterHeight( Vector position, float minz, float maxz )
{

}

int EV_HLDM_WaterEntryPoint( pmtrace_t *pTrace, float *vecSrc, float *vecResult )
{

}

void EV_HLDM_BubbleTrails( Vector from, Vector to, int count )
{

}

void EV_FireMelee( event_args_t *args )
{
	int idx, iAnim, bPlayWiff, bPlayHit;

	vec3_t origin;

	idx = args->entindex;
	iAnim = args->iparam1;
	bPlayWiff = args->bparam1;
	bPlayHit = args->bparam2;

	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim , EV_GetWeaponBody() );
	}

	if( bPlayWiff )
	{
		gEngfuncs.pfnRandomLong( 0, 1 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	if( bPlayHit )
	{
		gEngfuncs.pfnRandomLong( 0, 2 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_hit1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	EV_ResetAnimationEvents( idx );
}

void EV_FireColt( event_args_t *args )
{
	int idx, empty;
	
	vec3_t vecSpread;

	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	vecSpread.x = flSpread_x;
	vecSpread.y = flSpread_y;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin, angles, velocity;

	vec3_t ShellVelocity, ShellOrigin;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( empty == 0 ? COLT_SHOOT : COLT_SHOOT_EMPTY, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_COLT );
	}
	EV_MuzzleFlash( idx, PISTOL );

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/colt_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_COLT, 0, &tracerCount[idx-1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireLuger( event_args_t *args )
{
	int idx, empty;
	
	vec3_t vecSpread;
	
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	vecSpread.x = flSpread_x;
	vecSpread.y = flSpread_y;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin, angles, velocity;

	vec3_t ShellVelocity, ShellOrigin;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( LUGER_SHOOT_EMPTY - empty == 0, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_LUGER );
	}
	EV_MuzzleFlash( idx, PISTOL );

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/luger_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_LUGER, 0, &tracerCount[idx-1] );

	EV_ResetAnimationEvents( idx );
}

int MG42_DownAnims[9], MG42_UpAnims[9];;

void EV_FireMG42( event_args_t *args )
{

}

void EV_FireMG34( event_args_t *args )
{

}

int ThirtyCal_DownAnims[9], ThirtyCal_UpAnims[9];;

void EV_Fire30CAL( event_args_t *args )
{

}

void EV_FireGarand( event_args_t *args )
{

}

void EV_FireM1Carbine( event_args_t *args )
{

}

void EV_FireScopedKar( event_args_t *args )
{

}

void EV_FireThompson( event_args_t *args )
{

}

void EV_FireMP44( event_args_t *args )
{

}

void EV_FireGreaseGun( event_args_t *args )
{

}

void EV_FireFG42( event_args_t *args )
{

}

void EV_FireK43( event_args_t *args )
{

}

void EV_FireEnfield( event_args_t *args )
{

}

void EV_FireSten( event_args_t *args )
{

}

void EV_FireBren( event_args_t *args )
{

}

void EV_FireWebley( event_args_t *args )
{
	int idx, empty;
	
	vec3_t vecSpread;
	
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	vecSpread.x = flSpread_x;
	vecSpread.y = flSpread_y;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin, angles, velocity;

	vec3_t ShellVelocity, ShellOrigin;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( WEBLEY_SHOOT, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_WEBLEY );
	}
	EV_MuzzleFlash( idx, PISTOL );

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/webley_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_WEBLEY, 0, &tracerCount[idx-1] );

	EV_ResetAnimationEvents( idx );
}

void EV_PopHelmet( event_args_t *args )
{

}

void EV_Knife( event_args_t *args )
{
	int idx, g_iSwing;

	vec3_t origin;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	if( gEngfuncs.pfnRandomFloat( 0.0f, 1.0f ) == 1.0f )
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash2.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 94.0f + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f ) );
	else
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 94.0f + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f ) );

	if( EV_IsLocal( idx ) )
	{
		switch( ( g_iSwing++ ) % 2 )
		{
			case 0:
				gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH1, EV_GetWeaponBody() );
				break;
			case 1:
				gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH2, EV_GetWeaponBody() );
				break;
		}
	}
}

void EV_FireMP40( event_args_t *args )
{

}

void EV_FireSpring( event_args_t *args )
{

}

void EV_FireKar( event_args_t *args )
{

}

void EV_FireBAR( event_args_t *args )
{

}

void EV_FireBazooka( event_args_t *args )
{

}

void EV_FirePschreck( event_args_t *args )
{

}

void EV_FirePIAT( event_args_t *args )
{

}

void EV_FireMortar( event_args_t *args )
{
    
}

void EV_FireScopedEnfield( event_args_t *args )
{

}

void EV_PlayWhizz( event_args_t *args )
{

}

void EV_DoDCamera( event_args_t *args )
{

}

void EV_SparkShower( event_args_t *args )
{

}

void P_ExplosionFlash( event_args_t *args )
{

}

void PlayExplosionSound( float *origin )
{

}

void PlayMortarExplosionSound( float *origin )
{

}

void EV_BaseExplosion( event_args_t *args )
{

}

void EV_WaterExplosion( event_args_t *args )
{

}

void EV_DirtTrailCallback( tempent_s *ent, float frametime, float currenttime )
{

}

void EV_DirtRubble( event_args_t *args )
{

}

void EV_DirtExplosion( event_args_t *args )
{

}

void EV_SandExplosion( event_args_t *args )
{

}

void EV_Explosion( event_args_t *args )
{

}

void EV_Bubbles( event_args_t *args )
{

}

void EV_BubbleTrails( event_args_t *args )
{

}

void EV_BulletTracers( event_args_t *args )
{

}

void EV_BloodStream( event_args_t *args )
{

}

void EV_BloodSprite( event_args_t *args )
{

}

void EV_Smoke( event_args_t *args )
{

}

void EV_Pain( event_args_t *args )
{

}

void EV_USVoice( event_args_t *args )
{

}

void EV_GERVoice( event_args_t *args )
{

}

void EV_BodyDamage( event_args_t *args )
{

}

void EV_RoundReleaseSound( event_args_t *args )
{

}

void EV_TrainPitchAdjust( event_args_t *args )
{

}

int EV_TFC_IsAllyTeam( int iTeam1, int iTeam2 )
{

}

void P_Rubble( event_args_t *args )
{

}

void P_ExplosionSmoke( event_args_t *args )
{

}

char EV_FindHitTexture( float *start, float *end )
{

}

char EV_TexNameToType( char *pTextureName )
{

}

void CreateFlyingRubble( Vector origin, bool bLargeRubble, float vVelocityx, float vVelocityy, float vVelocityz, float fSize, int iTextureType, Vector vNormal )
{

}

void EV_RocketTrailCallback( tempent_s *ent, float frametime, float currenttime )
{

}

void EV_RocketTrail( event_args_t *args )
{

}

void EV_MortarShellCallback( tempent_s *ent, float frametime, float currenttime )
{

}

void EV_MortarShell( event_args_t *args )
{

}

void EV_OverheatCallback( tempent_s *ent, float frametime, float currenttime )
{

}

void EV_Overheat( event_args_t *args )
{

}