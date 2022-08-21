#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"

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

#include "voice_status.h"

static int g_tracerCount[32];

extern "C"
{
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

void EV_FireColt( event_args_s *args )
{
	int idx, empty;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( empty == 0 ? COLT_SHOOT : COLT_SHOOT_EMPTY, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( empty == 0 ? COLT_SHOOT : COLT_SHOOT_EMPTY, 2 );

		gHUD.DoRecoil( WEAPON_COLT );
	}
	EV_MuzzleFlashDOD( idx, 1 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/colt_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_COLT, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireM1Carbine( struct event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( M1CARBINE_SHOOT, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( M1CARBINE_SHOOT, 2 );
		
		gHUD.DoRecoil( WEAPON_M1CARBINE );
	}
	EV_MuzzleFlashDOD( idx, 2 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/carbine_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_M1CARBINE, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireLuger( event_args_s *args )
{
	int idx, empty;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( LUGER_SHOOT_EMPTY - ( empty == 0 ) , gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( LUGER_SHOOT_EMPTY - ( empty == 0 ), 2 );

		gHUD.DoRecoil( WEAPON_LUGER );
	}
	EV_MuzzleFlashDOD( idx, 1 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/luger_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_LUGER, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireGarand( event_args_s *args )
{
	int idx, empty;
	int iAnim;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( args->iparam1 )
	{
		if( EV_IsLocal( idx ) )
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( GARAND_SMASH_EMPTY - ( empty == 0 ) , gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( GARAND_SMASH_EMPTY - ( empty == 0 ), 2 );
		}

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			iAnim = GARAND_SHOOT_EMPTY;

			if( empty )
			{
				iAnim = gEngfuncs.pfnRandomLong( 0, 2 ) + GARAND_SHOOT1;
			}

			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, 2 );
			
			gHUD.DoRecoil( WEAPON_GARAND );
		}
		EV_MuzzleFlashDOD( idx, 2 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/garand_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( !empty )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/garand_reload_clipding.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );
		
		EV_GetGunPosition( args, vecSrc, origin );

		VectorCopy( forward, vecAiming );

		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_GARAND, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );
	}

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireScopedKar( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( SCOPEDKAR_SHOOT, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( SCOPEDKAR_SHOOT, 2 );
		
		gHUD.DoRecoil( WEAPON_SCOPEDKAR );
	}
	EV_MuzzleFlashDOD( idx, 2 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/kar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_SCOPEDKAR, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireThompson( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + THOMPSON_SHOOT1, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + THOMPSON_SHOOT1, 2 );
		
		gHUD.DoRecoil( WEAPON_THOMPSON );
	}
	EV_MuzzleFlashDOD( idx, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/thompson_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_THOMPSON, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireMP44( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + MP44_SHOOT1, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + MP44_SHOOT1, 2 );
		
		gHUD.DoRecoil( WEAPON_MP44 );
	}
	EV_MuzzleFlashDOD( idx, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp44_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP44, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireSpring( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + SPRING_SHOOT1, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + SPRING_SHOOT1, 2 );
		
		gHUD.DoRecoil( WEAPON_SPRING );
	}
	EV_MuzzleFlashDOD( idx, 2 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/spring_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_SPRING, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireKar( event_args_s *args )
{
	int idx, pitch;

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( pitch == 1 )
	{
		if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 1 )
		{
			if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) <= 1 )
			{
				if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) )
					return;
			}
			else if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 2 )
			{
				if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 3 )
					return;
				
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing2.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

				if( !EV_IsLocal( idx ) )
					return;

				if( g_iUser1 == OBS_IN_EYE )
					g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

				if( g_iTeamNumber == 1 )
					gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SLASH, gHUD.m_bBritish );
				else
					gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SLASH, 2 );
				return;
			}
		}
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( !EV_IsLocal( idx ) )
			return;

		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SWING, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SWING, 2 );
		return;
	}
	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SHOOT, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SHOOT, 2 );
		
		gHUD.DoRecoil( WEAPON_KAR );
	}
	EV_MuzzleFlashDOD( idx, 2 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/kar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_KAR, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireMP40( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + MP40_SHOOT1, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + MP40_SHOOT1, 2 );
		
		gHUD.DoRecoil( WEAPON_MP40 );
	}
	EV_MuzzleFlashDOD( idx, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp40_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP40, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_Knife( event_args_s *args )
{
	int idx;
	int g_iSwing;

	vec3_t origin;

	idx = args->entindex;

	VectorCopy( args->origin, origin );


	if( gEngfuncs.pfnRandomFloat( 0.0f, 1.0f ) == 1.0f )
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash2.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 94.0f + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f ) );
	else
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 94.0f + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f ) );

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

		switch( ( g_iSwing++ ) % 2 )
		{
			case 0:
				if( g_iTeamNumber == 1 )
					gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH1, gHUD.m_bBritish );
				else
					gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH1, 2 );
				break;
			case 1:
				if( g_iTeamNumber == 1 )
					gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH2, gHUD.m_bBritish );
				else
					gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH2, 2 );
				break;
		}
	}
}

void EV_FireBAR( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( gHUD.IsInMGDeploy() )
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( BAR_DOWN_SHOOT, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( BAR_DOWN_SHOOT, 2 );
		}
		else
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( BAR_UP_SHOOT, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( BAR_UP_SHOOT, 2 );
		}
		gHUD.DoRecoil( WEAPON_BAR );
	}
	EV_MuzzleFlashDOD( idx, 4 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/bar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_BAR, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireMG42( event_args_s *args )
{
	int idx, pitch, empty;
	int shell;
	bool bCallAnim;

	int MG42_DownAnims[9] = { MG42_DOWNSHOOT, MG42_DOWNSHOOT8, MG42_DOWNSHOOT7, MG42_DOWNSHOOT6, MG42_DOWNSHOOT5, MG42_DOWNSHOOT4, MG42_DOWNSHOOT3, 
	MG42_DOWNSHOOT2, MG42_DOWNSHOOT1 }; 

	int MG42_UpAnims[9] = { MG42_UPSHOOT, MG42_UPSHOOT8, MG42_UPSHOOT7, MG42_UPSHOOT6, MG42_UPSHOOT5, MG42_UPSHOOT4, MG42_UPSHOOT3,
	MG42_UPSHOOT2, MG42_UPSHOOT1 };

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;
	empty = args->bparam1;

	bCallAnim = empty == 0;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shell_mg42.dll" );

	if( !EV_IsLocal )
	{
		if( !bCallAnim ) // TODO: Need rework - WHAMER
		{
			goto bCallAnimLabel1;
		}
		goto bCallAnimLabel2;
	}

	if( bCallAnim )
	{
		if( pitch <= MG42_UPIDLE1 )
			pitch = MG42_UPIDLE1;

		if( gHUD.IsInMGDeploy() )
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_DownAnims[pitch], gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_DownAnims[pitch], 2 );
		}
		else
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_UpAnims[pitch], gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_UpAnims[pitch], 2 );
		}
		gHUD.DoRecoil( WEAPON_MG42 );
bCallAnimLabel2:
		EV_MuzzleFlashDOD( idx, 4 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg42_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}
bCallAnimLabel1:
	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MG42, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireMG34( event_args_s *args )
{
	int idx, pitch;
	int iAnim;

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( gHUD.IsInMGDeploy() )
			iAnim = MG34_DOWNSHOOTEMPTY - ( pitch <= 0 == 0 );
		else
			iAnim = MG34_UPSHOOTEMPTY - ( pitch <= 0 == 0 );
		
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, 2 );

		gHUD.DoRecoil( WEAPON_MG34 );
	}
	EV_MuzzleFlashDOD( idx, 4 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg34_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MG34, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_Fire30CAL( event_args_s *args )
{
	int idx, pitch;

	int ThirtyCal_DownAnims[9] = { CAL30_DOWNSHOOT, CAL30_DOWNSHOOT8, CAL30_DOWNSHOOT7, CAL30_DOWNSHOOT6, CAL30_DOWNSHOOT5, CAL30_DOWNSHOOT4, CAL30_DOWNSHOOT3, 
	CAL30_DOWNSHOOT2, CAL30_DOWNSHOOT1 }; 

	int ThirtyCal_UpAnims[9] = { CAL30_UPSHOOT, CAL30_UPSHOOT8, CAL30_UPSHOOT7, CAL30_UPSHOOT6, CAL30_UPSHOOT5, CAL30_UPSHOOT4, CAL30_UPSHOOT3,
	CAL30_UPSHOOT2, CAL30_UPSHOOT1 };

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( pitch <= CAL30_UPIDLE1 )
			pitch = CAL30_UPIDLE1;

		if( gHUD.IsInMGDeploy() )
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_DownAnims[pitch], gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_DownAnims[pitch], 2 );
		}
		else
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_UpAnims[pitch], gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_UpAnims[pitch], 2 );
		}
		gHUD.DoRecoil( WEAPON_CAL30 );
	}
	EV_MuzzleFlashDOD( idx, 4 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/30cal_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_30CAL, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireGreaseGun( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + GREASEGUN_SHOOT1, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + GREASEGUN_SHOOT1, 2 );
		
		gHUD.DoRecoil( WEAPON_GREASEGUN );
	}
	EV_MuzzleFlashDOD( idx, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/greasegun_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_GREASEGUN, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireFG42( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( gHUD.IsInMGDeploy() )
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( FG42_DOWN_SHOOT, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( FG42_DOWN_SHOOT, 2 );
		}
		else
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( FG42_UP_SHOOT, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( FG42_UP_SHOOT, 2 );
		}
		gHUD.DoRecoil( WEAPON_FG42 );
	}
	EV_MuzzleFlashDOD( idx, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/fg42_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_FG42, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireK43( event_args_s *args )
{
	int idx, pitch;

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( pitch == 1 )
	{
		if( EV_IsLocal( idx ) )
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( K43_SMASH, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( K43_SMASH, 2 );

			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		}
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + K43_SHOOT1, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + K43_SHOOT1, 2 );

			gHUD.DoRecoil( WEAPON_K43 );
		}
		EV_MuzzleFlashDOD( idx, 2 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/k43_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		EV_GetGunPosition( args, vecSrc, origin );

		VectorCopy( forward, vecAiming );

		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_K43, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

		gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
		gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
		gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
		gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
	}
}

void EV_FireEnfield( event_args_s *args )
{
	int idx, pitch;

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( pitch == 1 )
	{
		if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 1 )
		{
			if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) <= 1 )
			{
				if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) )
					return;
			}
			else if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 2 )
			{
				if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 3 )
					return;
				
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing2.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

				if( !EV_IsLocal( idx ) )
					return;

				if( g_iUser1 == OBS_IN_EYE )
					g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

				if( g_iTeamNumber == 1 )
					gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SWING, gHUD.m_bBritish );
				else
					gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SWING, 2 );
				return;
			}
		}
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( !EV_IsLocal( idx ) )
			return;
	}
	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SHOOT, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SHOOT, 2 );
		
		gHUD.DoRecoil( WEAPON_ENFIELD );
	}
	EV_MuzzleFlashDOD( idx, 2 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/enfield_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_ENFIELD, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireSten( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + GREASEGUN_SHOOT1, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + GREASEGUN_SHOOT1, 2 );
		
		gHUD.DoRecoil( WEAPON_STEN );
	}
	EV_MuzzleFlashDOD( idx, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/sten_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_STEN, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireBren( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( gHUD.IsInMGDeploy() )
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( BREN_DOWN_SHOOT, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( BREN_DOWN_SHOOT, 2 );
		}
		else
		{
			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( BREN_UP_SHOOT, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( BREN_UP_SHOOT, 2 );
		}
		gHUD.DoRecoil( WEAPON_BREN );
	}
	EV_MuzzleFlashDOD( idx, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/breen_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_BREN, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireWebley( event_args_s *args )
{
	int idx, empty;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( WEBLEY_SHOOT , gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( WEBLEY_SHOOT , 2 );

		gHUD.DoRecoil( WEAPON_WEBLEY );
	}
	EV_MuzzleFlashDOD( idx, 1 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/webley_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_WEBLEY, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireScopedEnfield( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( SCOPEDENFIELD_SHOOT, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( SCOPEDENFIELD_SHOOT, 2 );
		
		gHUD.DoRecoil( WEAPON_ENFIELD );
	}
	EV_MuzzleFlashDOD( idx, 2 );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/enfieldsniper_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_ENFIELD, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireBazooka( event_args_s *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( BAZOOKA_FIRE, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( BAZOOKA_FIRE, 2 );
		
		gHUD.DoRecoil( WEAPON_BAZOOKA );
	}
	EV_MuzzleFlashDOD( idx, 4 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FirePschreck( event_args_s *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( PSCHRECK_FIRE, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( PSCHRECK_FIRE, 2 );
		
		gHUD.DoRecoil( WEAPON_PSCHRECK);
	}
	EV_MuzzleFlashDOD( idx, 4 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FirePIAT( event_args_s *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( PIAT_FIRE, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( PIAT_FIRE, 2 );
		
		gHUD.DoRecoil( WEAPON_PIAT);
	}
	EV_MuzzleFlashDOD( idx, 4 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireMortar( event_args_s *args )
{
	int idx;

	vec3_t origin;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( MORTAR_DEPLOYED_FIRE, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( MORTAR_DEPLOYED_FIRE, 2 );
	}
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mortar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_MuzzleFlashDOD( idx, 4 );

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_FireMelee( event_args_s *args )
{
	int pitch, empty, empty2, idx;

	vec3_t origin;

	pitch = args->iparam1;
	empty = args->bparam1;
	empty2 = args->bparam2;
	idx = args->entindex;

	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( pitch , gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( pitch , 2 );
	}

	if( empty )
	{
		gEngfuncs.pfnRandomLong( 0, 1 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	if( empty2 )
	{
		gEngfuncs.pfnRandomLong( 0, 2 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_hit1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser1 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser2 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser3 = 0;
	gEngfuncs.GetEntityByIndex( idx )->baseline.iuser4 = 0;
}

void EV_Pain( struct event_args_s *args )
{
	int idx;

	char szSound[32];

	idx = args->entindex;

	vec3_t origin;

	VectorCopy( args->origin, origin );

	sprintf( szSound, "player/damage%d.wav", gEngfuncs.pfnRandomLong( 1, 11 ) );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_VOICE, szSound, 1.0f, 2.0f, 0, 100 );
}

void EV_Smoke( struct event_args_s *args )
{
	vec3_t origin;
	vec3_t angles;

	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	// TODO: WHAMER
}
void EV_BloodSprite( struct event_args_s *args )
{
	vec3_t origin;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEfxAPI->R_BloodSprite( origin, args->iparam2, args->iparam1, args->fparam1, args->fparam2 );
}

void EV_BloodStream( struct event_args_s *args )
{
	vec3_t origin, angles, velocity;

	TEMPENTITY *pShotDust, *pBloodNarrow;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	pShotDust = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, gEngfuncs.pfnRandomFloat( 2.0f, 4.0f ), gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/shot-dust.spr" ), kRenderTransAlpha, kRenderFxNone, 0.34f, 1.0f, FTENT_FADEOUT );

	pShotDust->fadeSpeed = 4.0f;
	pShotDust->entity.curstate.renderamt = 800;
	pShotDust->entity.curstate.framerate = 15.0f;
	pShotDust->entity.curstate.rendercolor.r = -76;
	pShotDust->entity.curstate.rendercolor.g = -80;
	pShotDust->entity.curstate.rendercolor.b = -108;

	pBloodNarrow = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, 0.0f, gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/blood-narrow.spr" ), kRenderTransAlpha, kRenderFxNone, 0.34f, 1.0f, FTENT_FADEOUT );

	pBloodNarrow->fadeSpeed = 4.0f;
	pBloodNarrow->entity.curstate.renderamt = 800;
	pBloodNarrow->entity.curstate.framerate = 15.0f;
	pBloodNarrow->entity.curstate.rendercolor.r = 75;
	pBloodNarrow->entity.curstate.rendercolor.g = 0;
	pBloodNarrow->entity.curstate.rendercolor.b = 0;
}

void EV_BulletTracers( struct event_args_s *args )
{
	vec3_t origin, angles;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	gEngfuncs.pEfxAPI->R_TracerEffect( origin, angles );
}

void EV_BubbleTrails( struct event_args_s *args )
{
	vec3_t origin, angles;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	gEngfuncs.pEfxAPI->R_BubbleTrail( origin, angles, args->fparam1, args->iparam1, args->iparam2, args->fparam2 );
}

void EV_Bubbles( struct event_args_s *args )
{
	vec3_t origin, angles;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	
	gEngfuncs.pEfxAPI->R_Bubbles( origin, angles, args->fparam1, args->iparam1, args->iparam2, args->fparam2 );
}

void EV_Explosion( struct event_args_s *args )
{
	// TODO: WHAMER
}

void EV_SparkShower( struct event_args_s *args )
{
	vec3_t origin;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEfxAPI->R_SparkShower( origin );
}

void EV_PlayWhizz( struct event_args_s *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
		gEngfuncs.pfnPlaySoundByName( "player/whizz.wav", 1.0f );
}

void EV_USVoice( struct event_args_s *args )
{
	int iPlayer, pitch1, pitch2;

	vec3_t origin;

	iPlayer = args->entindex;
	pitch1 = args->iparam1;
	pitch2 = args->iparam2;

	VectorCopy( args->origin, origin );

	if( GetClientVoiceMgr()->IsPlayerBlocked( iPlayer ) )
	{
		if( pitch2 )
		{

		}
	}
}

void EV_GERVoice( struct event_args_s *args )
{

}

void EV_BodyDamage( struct event_args_s *args )
{
	int idx, pitch;

	vec3_t origin;

	idx = args->entindex;
	pitch = args->iparam1;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, "player/helmet_hit.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );
}

void EV_RoundReleaseSound( struct event_args_s *args )
{

}

void EV_DoDCamera( struct event_args_s *args )
{

}

void EV_PopHelmet( struct event_args_s *args )
{

}
void EV_RoundReset( struct event_args_s *args )
{

}

void EV_Overheat( struct event_args_s *args )
{

}

void EV_RocketTrail( struct event_args_s *args )
{

}

void EV_MortarShell( struct event_args_s *args )
{

}