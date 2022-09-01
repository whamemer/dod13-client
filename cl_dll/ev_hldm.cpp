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

static float fl_TimeMusicLeft;

static int tracerCount[32];
float total_time;

extern char *sHelmetModels[7];

extern cvar_t *cl_dynamiclights;
cvar_t *cl_numshotrubble;

extern float g_flWeaponHeat;

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

}

void EV_ResetAnimationEvents( int index )
{

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

}

void EV_FireColt( event_args_t *args )
{

}

void EV_FireLuger( event_args_t *args )
{

}

void EV_FireMG42( event_args_t *args )
{

}

void EV_FireMG34( event_args_t *args )
{

}

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

}

void EV_PopHelmet( event_args_t *args )
{

}

void EV_Knife( event_args_t *args )
{

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