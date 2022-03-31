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

static int g_tracerCount[32];

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

	gEngfuncs.pfnAngleVectors( angles, forward, right, up );

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

	gEngfuncs.pfnAngleVectors( angles, forward, right, up );

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
	int prl;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

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
			prl = 4;

			if( empty )
			{
				prl = gEngfuncs.pfnRandomLong( 0, 2 ) + 1;
			}

			if( g_iUser1 == OBS_IN_EYE )
				g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

			if( g_iTeamNumber == 1 )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( prl, gHUD.m_bBritish );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( prl, 2 );
			
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

	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( 1, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( 1, 2 );
		
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

}

void EV_FireMP44( event_args_s *args )
{

}

void EV_FireSpring( event_args_s *args )
{

}

void EV_FireKar( event_args_s *args )
{
	int idx, FireModea;

	vec3_t origin, angles;

	idx = args->entindex;
	FireModea = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	if( FireModea == 1 )
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
					gEngfuncs.pEventAPI->EV_WeaponAnimation( 5, gHUD.m_bBritish );
				else
					gEngfuncs.pEventAPI->EV_WeaponAnimation( 5, 2 );
				return;
			}
		}
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( !EV_IsLocal( idx ) )
			return;

		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( 4, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( 4, 2 );
		return;
	}
	if( EV_IsLocal( idx ) )
	{
		if( g_iUser1 == OBS_IN_EYE )
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		
		if( g_iTeamNumber == 1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( 1, gHUD.m_bBritish );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( 1, 2 );
		
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

}

void EV_FireMG42( event_args_s *args )
{

}

void EV_FireMG34( event_args_s *args )
{

}

void EV_Fire30CAL( event_args_s *args )
{

}

void EV_FireGreaseGun( event_args_s *args )
{

}

void EV_FireFG42( event_args_s *args )
{

}

void EV_FireK43( event_args_s *args )
{

}

void EV_FireEnfield( event_args_s *args )
{

}

void EV_FireSten( event_args_s *args )
{

}

void EV_FireBren( event_args_s *args )
{

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

	gEngfuncs.pfnAngleVectors( angles, forward, right, up );

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

}

void EV_FireBazooka( event_args_s *args )
{

}

void EV_FirePschreck( event_args_s *args )
{

}

void EV_FirePIAT( event_args_s *args )
{

}

void EV_FireMortar( event_args_s *args )
{

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

}

void EV_Smoke( struct event_args_s *args )
{

}
void EV_BloodSprite( struct event_args_s *args )
{

}

void EV_BloodStream( struct event_args_s *args )
{

}

void EV_BulletTracers( struct event_args_s *args )
{

}

void EV_BubbleTrails( struct event_args_s *args )
{

}

void EV_Bubbles( struct event_args_s *args )
{

}

void EV_Explosion( struct event_args_s *args )
{

}

void EV_SparkShower( struct event_args_s *args )
{

}

void EV_PlayWhizz( struct event_args_s *args )
{

}

void EV_USVoice( struct event_args_s *args )
{

}

void EV_GERVoice( struct event_args_s *args )
{

}

void EV_BodyDamage( struct event_args_s *args )
{

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