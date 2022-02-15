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
		{
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		}

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
		{
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		}

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

}

void EV_FireScopedKar( event_args_s *args )
{

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

}

void EV_FireMP40( event_args_s *args )
{

}

void EV_Knife( event_args_s *args )
{

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
		{
			g_iTeamNumber = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;
		}

		gEngfuncs.pEventAPI->EV_WeaponAnimation( WEBLEY_SHOOT, 2 );

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

}