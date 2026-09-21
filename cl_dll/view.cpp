//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// view/refresh setup functions

#include "hud.h"
#include "cl_util.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"

#include "entity_state.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "in_defs.h" // PITCH YAW ROLL
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"
#include "screenfade.h"
#include "shake.h"
#include "hltv.h"
#include "view.h"
#include "dod_shared.h"

// Spectator Mode
extern "C" 
{
	float vecNewViewAngles[3];
	int iHasNewViewAngles;
	float vecNewViewOrigin[3];
	int iHasNewViewOrigin;
	int iIsSpectator;
}

extern "C" 
{
	int CL_IsThirdPerson( void );
	void CL_CameraOffset( float *ofs );

	void DLLEXPORT V_CalcRefdef( struct ref_params_s *pparams );

	void PM_ParticleLine( float *start, float *end, int pcolor, float life, float vert );
	int PM_GetVisEntInfo( int ent );
	int PM_GetPhysEntInfo( int ent );
	void InterpolateAngles( float * start, float * end, float * output, float frac );
	float PM_SplineFraction( float value, float scale );
	void NormalizeAngles( float * angles );
	float Distance( const float * v1, const float * v2 );
	float AngleBetweenVectors(  const float * v1,  const float * v2 );

	float vJumpOrigin[3];
	float vJumpAngles[3];
}

void V_DropPunchAngle( float frametime, float *ev_punchangle );
void VectorAngles( const float *forward, float *angles );

#include "r_studioint.h"
#include "com_model.h"

extern engine_studio_api_t IEngineStudio;

/*
The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.
*/

extern cvar_t	*cl_forwardspeed;
extern cvar_t	*chase_active;
extern cvar_t	*scr_ofsx, *scr_ofsy, *scr_ofsz;
extern cvar_t	*cl_vsmoothing;
extern cvar_t	*cl_viewbob;
extern Vector   dead_viewangles;

#define	CAM_MODE_RELAX		1
#define CAM_MODE_FOCUS		2

vec3_t v_origin, v_angles, v_cl_angles, v_sim_org, v_lastAngles;
float v_frametime, v_lastDistance;	
float v_cameraRelaxAngle = 5.0f;
float v_cameraFocusAngle = 35.0f;
int v_cameraMode = CAM_MODE_FOCUS;
qboolean v_resetCamera = 1;

vec3_t v_client_aimangles;
vec3_t g_ev_punchangle;

cvar_t	*scr_ofsx;
cvar_t	*scr_ofsy;
cvar_t	*scr_ofsz;

cvar_t	*v_centermove;
cvar_t	*v_centerspeed;

cvar_t	*cl_bobcycle;
cvar_t	*cl_bob;
cvar_t	*cl_bobup;
cvar_t	*cl_waterdist;
cvar_t	*cl_chasedist;

// These cvars are not registered (so users can't cheat), so set the ->value field directly
// Register these cvars in V_Init() if needed for easy tweaking
cvar_t	v_iyaw_cycle		= {"v_iyaw_cycle", "2", 0, 2};
cvar_t	v_iroll_cycle		= {"v_iroll_cycle", "0.5", 0, 0.5};
cvar_t	v_ipitch_cycle		= {"v_ipitch_cycle", "1", 0, 1};
cvar_t	v_iyaw_level		= {"v_iyaw_level", "0.3", 0, 0.3};
cvar_t	v_iroll_level		= {"v_iroll_level", "0.1", 0, 0.1};
cvar_t	v_ipitch_level		= {"v_ipitch_level", "0.3", 0, 0.3};

float	v_idlescale;  // used by TFC for concussion grenade effect
float	v_modeloffscreen;

//=============================================================================
void V_GetDoDMapPosition( float *origin, float *angles )
{
	vec3_t forward;
	vec3_t zScaledTarget;
	float scale;

	zScaledTarget.z = 0.0f;

	angles[1] = 0.0f;
	angles[2] = 0.0f;
	angles[0] = 90.0f;

	zScaledTarget.x = gHUD.m_Spectator.m_mapOrigin.x;
	zScaledTarget.y = gHUD.m_Spectator.m_mapOrigin.y;

	AngleVectors( angles, forward, NULL, NULL );
	VectorNormalize( forward );

	float flZoom = gHUD.m_Spectator.m_mapZoom;

	if( flZoom <= 0.0f )
	{
		flZoom = 1.0f;
	}

	scale = -( 4096.0f / flZoom );

	VectorMA( zScaledTarget, scale, forward, origin );
}

float V_CalcBob( struct ref_params_s *pparams )
{
	static double bobtime = 0.0;
	static float bob = 0.0f;
	static float lasttime = 0.0f;
	float cycle;

	if( pparams->onground == -1 || pparams->time == lasttime )
	{
		return bob;
	}

	lasttime = pparams->time;
	bobtime += pparams->frametime;

	float flBobCycle = cl_bobcycle->value;

	if( flBobCycle <= 0.0f )
	{
		flBobCycle = 0.46f;
	}

	cycle = bobtime - ( int ) ( bobtime / flBobCycle ) * flBobCycle;
	cycle /= flBobCycle;

	float flBobUp = cl_bobup->value;

	if( cycle < flBobUp )
	{
		cycle = M_PI_F * cycle / flBobUp;
	}
	else
	{
		cycle = M_PI_F + M_PI_F * ( cycle - flBobUp ) / ( 1.0f - flBobUp );
	}

	float flSpeedFactor = sqrt( pparams->simvel[0] * pparams->simvel[0] + pparams->simvel[1] * pparams->simvel[1] ) * cl_bob->value;
	float flFinalBob = sin( cycle ) * ( flSpeedFactor * 0.7f ) + 0.3f * flSpeedFactor;

	if( flFinalBob >= 4.0f )
	{
		bob = 4.0f;
	}
	else if( flFinalBob <= -7.0f )
	{
		bob = -7.0f;
	}
	else
	{
		bob = flFinalBob;
	}

	return bob;
}

/*
===============
V_CalcRoll
Used by view and sv_user
===============
*/
float V_CalcRoll( vec3_t angles, vec3_t velocity, float rollangle, float rollspeed )
{
	float sign;
	float side;
	float value;
	vec3_t forward, right, up;

	AngleVectors( angles, forward, right, up );

	side = DotProduct( velocity, right );
	sign = side < 0.0f ? -1.0f : 1.0f;
	side = fabs( side );

	value = rollangle;
	if( side < rollspeed )
	{
		side = side * value / rollspeed;
	}
	else
	{
		side = value;
	}
	return side * sign;
}

typedef struct pitchdrift_s
{
	float pitchvel;
	int nodrift;
	float driftmove;
	double laststop;
} pitchdrift_t;

static pitchdrift_t pd;

/*
===============
V_DriftPitch

Moves the client pitch angle towards idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.
===============
*/
void V_StartPitchDrift( void )
{
	double laststop = pd.laststop;

	if( laststop != gEngfuncs.GetClientTime() && ( pd.nodrift || pd.pitchvel == 0.0 ) )
	{
		pd.driftmove = 0.0f;
		pd.pitchvel = v_centerspeed->value;
		pd.nodrift = 0;
	}
}

void V_StopPitchDrift( void )
{
	pd.nodrift = 1;
	pd.pitchvel = 0.0f;
	pd.laststop = gEngfuncs.GetClientTime();
}

void V_DriftPitch( struct ref_params_s *pparams )
{
	usercmd_t *cmd;
	float flMoveAbs;
	float flDelta;
	float flStep;

	if( gEngfuncs.IsNoClipping() || !pparams->onground || pparams->demoplayback || pparams->spectator )
	{
		pd.driftmove = 0.0f;
		pd.pitchvel = 0.0f;
	}
	else if( pd.nodrift )
	{
		cmd = pparams->cmd;
		flMoveAbs = fabs( cmd->forwardmove );

		if( cl_forwardspeed->value <= flMoveAbs )
		{
			pd.driftmove += pparams->frametime;
		}
		else
		{
			pd.driftmove = 0.0f;
		}

		if( pd.driftmove > v_centermove->value )
		{
			if( pd.laststop != gEngfuncs.GetClientTime() && ( pd.nodrift || pd.pitchvel == 0.0f ) )
			{
				pd.driftmove = 0.0f;
				pd.pitchvel = v_centerspeed->value;
				pd.nodrift = false;
			}
		}
	}
	else
	{
		float flCurrentPitch = pparams->cl_viewangles[0];
		flDelta = pparams->idealpitch - flCurrentPitch;

		if( flDelta == 0.0f )
		{
			pd.pitchvel = 0.0f;
		}
		else
		{
			flStep = pparams->frametime * pd.pitchvel;
			pd.pitchvel += pparams->frametime * v_centerspeed->value;

			if( flDelta <= 0.0f )
			{
				if( flDelta < 0.0f )
				{
					float flAbsDelta = -flDelta;
					float flFinalMove = ( flStep <= flAbsDelta ) ? flStep : flAbsDelta;

					if( flStep > flAbsDelta )
					{
						pd.pitchvel = 0.0f;
					}

					pparams->cl_viewangles[0] = flCurrentPitch - flFinalMove;
				}
			}
			else
			{
				float flFinalMove = ( flStep <= flDelta ) ? flStep : flDelta;

				if( flStep > flDelta )
				{
					pd.pitchvel = 0.0f;
				}

				pparams->cl_viewangles[0] = flCurrentPitch + flFinalMove;
			}
		}
	}
}

/* 
============================================================================== 
						VIEW RENDERING 
============================================================================== 
*/ 

/*
==================
V_CalcGunAngle
==================
*/
void V_CalcGunAngle( struct ref_params_s *pparams )
{
	cl_entity_t *viewent;

	viewent = gEngfuncs.GetViewModel();
	if( !viewent )
		return;

	viewent->angles[YAW] = pparams->viewangles[YAW] + pparams->crosshairangle[YAW];
	viewent->angles[PITCH] = -pparams->viewangles[PITCH] + pparams->crosshairangle[PITCH] * 0.25f;
	viewent->angles[ROLL] -= v_idlescale * sin( pparams->time * v_iroll_cycle.value ) * v_iroll_level.value;
	viewent->angles[PITCH] -= v_idlescale * sin( pparams->time * v_ipitch_cycle.value ) * ( v_ipitch_level.value * 0.5f );
	viewent->angles[YAW] -= v_idlescale * sin( pparams->time * v_iyaw_cycle.value ) * v_iyaw_level.value;

	VectorCopy( viewent->angles, viewent->curstate.angles );
	VectorCopy( viewent->angles, viewent->latched.prevangles );
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle( struct ref_params_s *pparams )
{
	pparams->viewangles[ROLL] += v_idlescale * sin( pparams->time * v_iroll_cycle.value ) * v_iroll_level.value;
	pparams->viewangles[PITCH] += v_idlescale * sin( pparams->time * v_ipitch_cycle.value ) * v_ipitch_level.value;
	pparams->viewangles[YAW] += v_idlescale * sin( pparams->time * v_iyaw_cycle.value ) * v_iyaw_level.value;
}

/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll( struct ref_params_s *pparams )
{
	float side;
	cl_entity_t *viewentity;

	viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
	if( !viewentity )
		return;

	side = V_CalcRoll( viewentity->angles, pparams->simvel, pparams->movevars->rollangle, pparams->movevars->rollspeed );

	pparams->viewangles[ROLL] += side;

	if( pparams->health <= 0 && ( pparams->viewheight[2] != 0 ) )
	{
		// only roll the view if the player is dead and the viewheight[2] is nonzero 
		// this is so deadcam in multiplayer will work.
		pparams->viewangles[ROLL] = 80;	// dead view angle
		return;
	}
}

/*
==================
V_CalcIntermissionRefdef

==================
*/
void V_CalcIntermissionRefdef( struct ref_params_s *pparams )
{
	cl_entity_t *view;
	float old;

	if( pparams->maxclients > 1 )
	{
		gEngfuncs.Cvar_SetValue( "scr_ofsx", 0.0f );
		gEngfuncs.Cvar_SetValue( "scr_ofsy", 0.0f );
		gEngfuncs.Cvar_SetValue( "scr_ofsz", 0.0f );
	}

	// view is the weapon model (only visible from inside body )
	view = gEngfuncs.GetViewModel();

	VectorCopy( pparams->simorg, pparams->vieworg );
	VectorCopy( pparams->cl_viewangles, pparams->viewangles );

	if( view )
		view->model = NULL;

	// allways idle in intermission
	old = v_idlescale;
	v_idlescale = 1;

	V_AddIdle( pparams );

	if( gEngfuncs.IsSpectateOnly() )
	{
		// in HLTV we must go to 'intermission' position by ourself
		VectorCopy( gHUD.m_Spectator.m_cameraOrigin, pparams->vieworg );
		VectorCopy( gHUD.m_Spectator.m_cameraAngles, pparams->viewangles );
	}

	v_idlescale = old;

	v_cl_angles = pparams->cl_viewangles;
	v_origin = pparams->vieworg;
	v_angles = pparams->viewangles;
}

#define ORIGIN_BACKUP 64
#define ORIGIN_MASK ( ORIGIN_BACKUP - 1 )

typedef struct 
{
	float Origins[ORIGIN_BACKUP][3];
	float OriginTime[ORIGIN_BACKUP];

	float Angles[ORIGIN_BACKUP][3];
	float AngleTime[ORIGIN_BACKUP];

	int CurrentOrigin;
	int CurrentAngle;
} viewinterp_t;

/*
==================
V_CalcRefdef

==================
*/
extern float DoDGunGoOnOffScreen( float v_modeloffscreen, ref_params_s *pparams );

extern int g_iOnlyClientDraw;

void V_CalcNormalRefdef( struct ref_params_s *pparams )
{
	cl_entity_t *view;
	int i;
	float waterOffset;
	static viewinterp_t ViewInterp;
	static float oldz;
	static float lasttime;
	cl_entity_t *pwater;

	int waterEntity;
	float t, fraction, fRate, steptime;
	static float lastorg[3];
	int foundidx;
	double frac, dt;
	cl_entity_t *viewentity;

	vec3_t angles;
	vec3_t camAngles, camForward, camRight, camUp;
	vec3_t vSide;
	vec3_t delta;
	vec3_t neworg;

	pparams->onlyClientDraw = 0;

	V_DriftPitch( pparams );

	bool bRenderMap = false;

	if( gEngfuncs.IsSpectateOnly() )
	{
		gEngfuncs.GetEntityByIndex( g_iUser2 );

		if( pparams->nextView )
			bRenderMap = true;
	}
	else
	{
		gEngfuncs.GetLocalPlayer();

		if( pparams->nextView )
			bRenderMap = true;
	}

	if( bRenderMap )
	{
		if( pparams->nextView == 1 )
		{
			int map_x, map_y, map_w, map_h;
			v_sim_org = pparams->simorg;

			gHUD.GetMapBounds( map_x, map_y, map_w, map_h );

			pparams->nextView = 0;
			pparams->onlyClientDraw = 1;
			g_iOnlyClientDraw = 1;

			pparams->viewangles[0] = 90.0f;
			pparams->viewangles[1] = 0.0f;
			pparams->viewangles[2] = 0.0f;

			pparams->viewport[0] = map_x;
			pparams->viewport[1] = map_y;
			pparams->viewport[2] = map_w;
			pparams->viewport[3] = map_h;

			delta[0] = gHUD.m_Spectator.m_mapOrigin.x;
			delta[1] = gHUD.m_Spectator.m_mapOrigin.y;
			delta[2] = 0.0f;

			AngleVectors( pparams->viewangles, neworg, NULL, NULL );
			VectorNormalize( neworg );

			float scalec = -( 4096.0f / gHUD.m_Spectator.m_mapZoom );
			VectorMA( delta, scalec, neworg, pparams->vieworg );
		}
		return;
	}

	if( gHUD.GetMinimapState() > 0 )
		pparams->nextView = 1;

	view = gEngfuncs.GetViewModel();
	float bob = V_CalcBob( pparams );

	VectorCopy( pparams->simorg, pparams->vieworg );
	VectorCopy( pparams->right, vSide );
	VectorScale( vSide, bob, vSide );

	if( pparams->health <= 0 )
	{
		VectorCopy( dead_viewangles, pparams->viewangles );
	}
	else
	{
		VectorCopy( pparams->cl_viewangles, pparams->viewangles );
	}

	pparams->vieworg[0] = pparams->vieworg[0] + vSide[0] + pparams->viewheight[0];
	pparams->vieworg[1] = pparams->vieworg[1] + vSide[1] + pparams->viewheight[1];
	pparams->vieworg[2] = pparams->vieworg[2] + vSide[2] + pparams->viewheight[2];

	gEngfuncs.V_CalcShake();
	gEngfuncs.V_ApplyShake( pparams->vieworg, pparams->viewangles, 1.0f );

	pparams->vieworg[0] += 1.0f / 32.0f;
	pparams->vieworg[1] += 1.0f / 32.0f;
	pparams->vieworg[2] += 1.0f / 32.0f;
	
	waterOffset = 0;

	if( pparams->waterlevel >= 2 )
	{
		int contents, waterDist;
		vec3_t point;
		waterDist = cl_waterdist->value;

		if( pparams->hardware )
		{
			waterEntity = gEngfuncs.PM_WaterEntity( pparams->simorg );
			if( waterEntity >= 0 && waterEntity < pparams->max_entities )
			{
				pwater = gEngfuncs.GetEntityByIndex( waterEntity );
				if( pwater && ( pwater->model != NULL ) )
				{
					waterDist += ( pwater->curstate.scale * 16.0f );
				}
			}
		}

		VectorCopy( pparams->vieworg, point );

		if( pparams->waterlevel == 2 )
		{
			point[2] -= waterDist;

			bool bBreakLoop = false;

			for( i = 0; i < waterDist && !bBreakLoop; i++ )
			{
				contents = gEngfuncs.PM_PointContents( point, NULL );
				if( contents > CONTENTS_WATER )
				{
					bBreakLoop = true;
				}
				else
				{
					point[2] += 1.0f;
				}
			}
			waterOffset = ( point[2] + waterDist ) - pparams->vieworg[2];
		}
		else
		{
			point[2] += waterDist;
			bool bBreakLoop = false;

			for( i = 0; i < waterDist && !bBreakLoop; i++ )
			{
				contents = gEngfuncs.PM_PointContents( point, NULL );

				if( contents <= CONTENTS_WATER )
					bBreakLoop = true;
				else
					point[2] -= 1.0f;
			}
			waterOffset = ( point[2] - waterDist ) - pparams->vieworg[2];
		}
	}

	pparams->vieworg[2] += waterOffset;

	V_CalcViewRoll( pparams );
	V_AddIdle( pparams );

	if( pparams->health <= 0 )
	{
		VectorCopy( dead_viewangles, angles );
	}
	else
	{
		VectorCopy( pparams->cl_viewangles, angles );
	}

	AngleVectors( angles, pparams->forward, pparams->right, pparams->up );

	if( pparams->maxclients <= 1 )
	{
		for( i = 0; i < 3; i++ )
		{
			pparams->vieworg[i] += scr_ofsx->value * pparams->forward[i] + scr_ofsy->value * pparams->right[i] + scr_ofsz->value * pparams->up[i];
		}
	}

	if( CL_IsThirdPerson() )
	{
		vec3_t ofs;
		ofs[0] = ofs[1] = ofs[2] = 0.0f;
		CL_CameraOffset( ( float * ) &ofs );

		VectorCopy( ofs, camAngles );
		camAngles[ROLL] = 0;

		AngleVectors( camAngles, camForward, camRight, camUp );

		for( i = 0; i < 3; i++ )
		{
			pparams->vieworg[i] += -ofs[2] * camForward[i];
		}
	}

	if( pparams->health <= 0 )
	{
		VectorCopy( dead_viewangles, view->angles );
	}
	else
	{
		VectorCopy( pparams->cl_viewangles, view->angles );
	}

	V_CalcGunAngle( pparams );

	VectorCopy( pparams->simorg, view->origin );
	view->origin[2] += waterOffset;
	VectorAdd( view->origin, pparams->viewheight, view->origin );

	gEngfuncs.V_ApplyShake( view->origin, view->angles, 0.45f );

	for( i = 0; i < 3; i++ )
	{
		view->origin[i] += bob * 0.4f * pparams->forward[i];
	}
	view->origin[2] += bob;

	view->angles[YAW] -= bob * 0.5f;
	view->angles[ROLL] -= bob * 1.0f;
	view->angles[PITCH] -= bob * 0.3f;

	gHUD.SetWaterLevel( pparams->waterlevel );

	if( gHUD.IsInMortarDeploy() )
	{
		t = gEngfuncs.GetClientTime() - gHUD.GetMortarDeployTime();
		if( t <= 1.0f )
		{
			fraction = PM_SplineFraction( t, 1.0f );
			view->curstate.angles[0] = -( ( 1.0f - fraction ) * pparams->viewangles[0] );
		}
		else
		{
			view->curstate.angles[0] = 0.0f;
		}
		view->curstate.vuser1[0] = view->curstate.vuser1[1] = view->curstate.vuser1[2] = 0.0f;
	}
	else
	{
		t = gEngfuncs.GetClientTime() - gHUD.GetMortarUnDeployTime();
		if( t >= 1.0f )
		{
			v_modeloffscreen = DoDGunGoOnOffScreen( v_modeloffscreen, pparams );

			delta[0] = pparams->up[0] + pparams->forward[0] + pparams->up[0];
			delta[1] = pparams->up[1] + ( pparams->forward[1] + pparams->up[1] );
			delta[2] = ( pparams->forward[2] + pparams->up[2] ) + pparams->up[2];

			VectorScale( delta, v_modeloffscreen * -0.7f, view->curstate.vuser1 );
		}
		else if( t <= 1.0f )
		{
			fraction = PM_SplineFraction( t, 1.0f );
			view->curstate.angles[0] = -( fraction * pparams->viewangles[0] );
		}
	}

	view->origin[0] += view->curstate.vuser1[0];
	view->origin[1] += view->curstate.vuser1[1];
	float flFinalGunZ = view->curstate.vuser1[2] + view->origin[2] - 1.0f;
	view->origin[2] = flFinalGunZ;

	if( pparams->viewsize == 110.0f )        view->origin[2] = flFinalGunZ + 1.0f;
	else if( pparams->viewsize == 100.0f )   view->origin[2] = flFinalGunZ + 2.0f;
	else if( pparams->viewsize == 90.0f )    view->origin[2] = flFinalGunZ + 1.0f;
	else if( pparams->viewsize == 80.0f )    view->origin[2] = flFinalGunZ + 0.5f;

	VectorAdd( pparams->viewangles, pparams->punchangle, pparams->viewangles );
	VectorAdd( pparams->viewangles, ( float * ) &g_ev_punchangle, pparams->viewangles );

	float flLen = VectorNormalize( g_ev_punchangle );
	fRate = flLen * 0.5f + 10.0f;
	float flDecay = flLen - fRate * pparams->frametime;

	if( flDecay <= 0.0f ) flDecay = 0.0f;
	VectorScale( g_ev_punchangle, flDecay, g_ev_punchangle );

	if( !pparams->smoothing && pparams->onground && pparams->simorg[2] - oldz > 0.0f )
	{
		steptime = pparams->time - lasttime;
		if( steptime < 0 ) steptime = 0;

		oldz += steptime * 150.0f;
		if( oldz > pparams->simorg[2] )			oldz = pparams->simorg[2];
		if( pparams->simorg[2] - oldz > 18.0f )	oldz = pparams->simorg[2] - 18.0f;
		pparams->vieworg[2] += oldz - pparams->simorg[2];
		view->origin[2] += oldz - pparams->simorg[2];
	}
	else
	{
		oldz = pparams->simorg[2];
	}

	{
		VectorSubtract( pparams->simorg, lastorg, delta );

		if( Length( delta ) != 0.0f )
		{
			VectorCopy( pparams->simorg, ViewInterp.Origins[ViewInterp.CurrentOrigin & ORIGIN_MASK] );
			ViewInterp.OriginTime[ViewInterp.CurrentOrigin & ORIGIN_MASK] = pparams->time;
			ViewInterp.CurrentOrigin++;

			VectorCopy( pparams->simorg, lastorg );
		}
	}

	if( cl_vsmoothing && cl_vsmoothing->value && ( pparams->smoothing && ( pparams->maxclients > 1 ) ) )
	{
		if( cl_vsmoothing->value < 0.0f ) gEngfuncs.Cvar_SetValue( "cl_vsmoothing", 0.0f );

		t = pparams->time - cl_vsmoothing->value;

		bool bFoundFrame = false;
		for( i = 1; i < ORIGIN_MASK && !bFoundFrame; i++ )
		{
			foundidx = ViewInterp.CurrentOrigin - 1 - i;
			if( ViewInterp.OriginTime[foundidx & ORIGIN_MASK] <= t )
			{
				bFoundFrame = true;
			}
		}

		if( bFoundFrame && ViewInterp.OriginTime[foundidx & ORIGIN_MASK] != 0.0f )
		{
			int nextidx = ( foundidx + 1 ) & ORIGIN_MASK;
			int curidx = foundidx & ORIGIN_MASK;

			dt = ViewInterp.OriginTime[nextidx] - ViewInterp.OriginTime[curidx];
			if( dt > 0.0 )
			{
				frac = ( t - ViewInterp.OriginTime[curidx] ) / dt;
				if( frac > 1.0 ) frac = 1.0;

				VectorSubtract( ViewInterp.Origins[nextidx], ViewInterp.Origins[curidx], delta );
				VectorMA( ViewInterp.Origins[curidx], frac, delta, neworg );

				if( Length( delta ) < 64.0f )
				{
					VectorSubtract( neworg, pparams->simorg, delta );
					VectorAdd( pparams->simorg, delta, pparams->simorg );
					VectorAdd( pparams->vieworg, delta, pparams->vieworg );
					VectorAdd( view->origin, delta, view->origin );
				}
			}
		}
	}

	v_angles = pparams->viewangles;
	v_client_aimangles = pparams->cl_viewangles;
	v_lastAngles = pparams->viewangles;

	if( CL_IsThirdPerson() )
	{
		VectorCopy( camAngles, pparams->viewangles );
	}

	if( pparams->viewentity > pparams->maxclients )
	{
		viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
		if( viewentity )
		{
			VectorCopy( viewentity->origin, pparams->vieworg );
			VectorCopy( viewentity->angles, pparams->viewangles );
			v_angles = pparams->viewangles;
		}
	}

	VectorCopy( view->origin, view->curstate.origin );
	VectorCopy( view->origin, view->latched.prevorigin );
	VectorCopy( view->angles, view->curstate.angles );
	VectorCopy( view->angles, view->latched.prevangles );

	if( gHUD.IsInMortarDeploy() )
	{
		t = gEngfuncs.GetClientTime() - gHUD.GetMortarDeployTime();
		if( t < 1.0f )
		{
			fraction = PM_SplineFraction( t, 1.0f );
			pparams->viewangles[0] = ( 1.0f - fraction ) * pparams->viewangles[0];
		}
		else
		{
			pparams->viewangles[0] = 0.0f;
		}
	}
	else
	{
		t = gEngfuncs.GetClientTime() - gHUD.GetMortarUnDeployTime();
		if( t < 1.0f )
		{
			fraction = PM_SplineFraction( t, 1.0f );
			pparams->viewangles[0] = fraction * pparams->viewangles[0];
		}
	}

	lasttime = pparams->time;
	v_angles[0] = pparams->viewangles[0];

	v_origin = pparams->vieworg;
	v_cl_angles = pparams->cl_viewangles;
	v_angles[1] = pparams->viewangles[1];
	v_angles[2] = pparams->viewangles[2];

	g_iOnlyClientDraw = 0;
}

void V_SmoothInterpolateAngles( float * startAngle, float * endAngle, float * finalAngle, float degreesPerSec )
{
	float absd, frac, d, threshhold;

	NormalizeAngles( startAngle );
	NormalizeAngles( endAngle );

	for( int i = 0; i < 3; i++ )
	{
		d = endAngle[i] - startAngle[i];

		if( d > 180.0f )
		{
			d -= 360.0f;
		}
		else if( d < -180.0f )
		{
			d += 360.0f;
		}

		absd = fabs( d );

		if( absd > 0.01f )
		{
			frac = degreesPerSec * v_frametime;

			threshhold= degreesPerSec / 4.0f;

			if( absd < threshhold )
			{
				float h = absd / threshhold;
				h *= h;
				frac *= h;  // slow down last degrees
			}

			if( frac > absd )
			{
				finalAngle[i] = endAngle[i];
			}
			else
			{
				if( d > 0.0f )
					finalAngle[i] = startAngle[i] + frac;
				else
					finalAngle[i] = startAngle[i] - frac;
			}
		}
		else
		{
			finalAngle[i] = endAngle[i];
		}
	}

	NormalizeAngles( finalAngle );
}

// Get the origin of the Observer based around the target's position and angles
void V_GetChaseOrigin( float * angles, float * origin, float distance, float * returnvec )
{
	vec3_t vecEnd;
	vec3_t forward;
	vec3_t vecStart;
	pmtrace_t *trace = 0;
	int maxLoops = 8;

	int ignoreent = -1;	// first, ignore no entity

	cl_entity_t *ent = NULL;

	// Trace back from the target using the player's view angles
	AngleVectors( angles, forward, NULL, NULL );

	VectorScale( forward, -1, forward );

	VectorCopy( origin, vecStart );

	VectorMA( vecStart, distance , forward, vecEnd );

	while( maxLoops > 0 )
	{
		trace = gEngfuncs.PM_TraceLine( vecStart, vecEnd, PM_TRACELINE_PHYSENTSONLY, 2, ignoreent );

		// WARNING! trace->ent is is the number in physent list not the normal entity number

		if( trace->ent <= 0)
			break;	// we hit the world or nothing, stop trace

		ent = gEngfuncs.GetEntityByIndex( PM_GetPhysEntInfo( trace->ent ) );

		if( ent == NULL )
			break;

		// hit non-player solid BSP , stop here
		if( ent->curstate.solid == SOLID_BSP && !ent->player ) 
			break;

		// if close enought to end pos, stop, otherwise continue trace
		if( Distance( trace->endpos, vecEnd ) < 1.0f )
		{
			break;
		}
		else
		{
			ignoreent = trace->ent;	// ignore last hit entity
			VectorCopy( trace->endpos, vecStart );
		}

		maxLoops--;
	}
	assert( trace );
/*	if( ent )
	{
		gEngfuncs.Con_Printf( "Trace loops %i , entity %i, model %s, solid %i\n",(8-maxLoops),ent->curstate.number, ent->model->name , ent->curstate.solid );
	} */

	VectorMA( trace->endpos, 4, trace->plane.normal, returnvec );

	v_lastDistance = Distance( trace->endpos, origin );	// real distance without offset
}

void V_GetDeathCam( cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin )
{
	float newAngle[3]; float newOrigin[3];

	float distance = 168.0f;

	v_lastDistance += v_frametime * 96.0f;	// move unit per seconds back

	if( v_resetCamera )
		v_lastDistance = 64.0f;

	if( distance > v_lastDistance )
		distance = v_lastDistance;

	VectorCopy( ent1->origin, newOrigin );

	if( ent1->player )
		newOrigin[2] += 17.0f; // head level of living player

	// get new angle towards second target
	if( ent2 )
	{
		VectorSubtract( ent2->origin, ent1->origin, newAngle );
		VectorAngles( newAngle, newAngle );
		newAngle[0] = -newAngle[0];
	}
	else
	{
		// if no second target is given, look down to dead player
		newAngle[0] = 90.0f;
		newAngle[1] = 0.0f;
		newAngle[2] = 0.0f;
	}

	// and smooth view
	V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );
	
	V_GetChaseOrigin( angle, newOrigin, distance, origin );

	VectorCopy( angle, v_lastAngles );
}

/*
==================
V_GetSingleTargetCam
==================
*/
void V_GetSingleTargetCam( cl_entity_t *ent1, float *angle, float *origin )
{
	int flags;
	qboolean deadPlayer;
	float dfactor;
	float distance;

	vec3_t newAngle;
	vec3_t newOrigin;

	flags = gHUD.m_Spectator.m_iObserverFlags;
	deadPlayer = ent1->player && ( ent1->curstate.solid == SOLID_NOT );

	dfactor = ( flags & DRC_FLAG_DRAMATIC ) ? -1.0f : 1.0f;
	distance = 112.0f + ( 16.0f * dfactor );

	if( flags & DRC_FLAG_FINAL )
	{
		distance *= 2.0f;
	}
	else if( deadPlayer )
	{
		distance *= 1.5f;
	}

	v_lastDistance += v_frametime * 32.0f;

	if( distance > v_lastDistance )
	{
		distance = v_lastDistance;
	}

	VectorCopy( ent1->origin, newOrigin );

	if( ent1->player )
	{
		if( deadPlayer )
			newOrigin[PITCH] += 2.0f;
		else
			newOrigin[PITCH] += 17.0f;
	}
	else
	{
		newOrigin[PITCH] += 8.0f;
	}

	VectorCopy( ent1->angles, newAngle );

	V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );
	V_GetChaseOrigin( angle, newOrigin, distance, origin );
}

float MaxAngleBetweenAngles( float *a1, float *a2 )
{
	float d, maxd = 0.0f;

	NormalizeAngles( a1 );
	NormalizeAngles( a2 );

	for( int i = 0; i < 3; i++ )
	{
		d = a2[i] - a1[i];
		if( d > 180.0f )
		{
			d -= 360.0f;
		}
		else if( d < -180.0f )
		{	
			d += 360.0f;
		}

		d = fabs( d );

		if( d > maxd )
			maxd = d;
	}

	return maxd;
}

void V_GetDoubleTargetsCam( cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin )
{
	float newAngle[3], newOrigin[3], tempVec[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;

	float dfactor = ( flags & DRC_FLAG_DRAMATIC ) ? -1.0f : 1.0f;

	float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;

	// go away in final scenes or if player just died
	if( flags & DRC_FLAG_FINAL )
		distance *= 2.0f;	

	// let v_lastDistance float smoothly away
	v_lastDistance += v_frametime * 32.0f;	// move unit per seconds back

	if( distance > v_lastDistance )
		distance = v_lastDistance;

	VectorCopy( ent1->origin, newOrigin );

	if( ent1->player )
		newOrigin[2] += 17.0f; // head level of living player
	else
		newOrigin[2] += 8.0f;	// object, tricky, must be above bomb in CS

	// get new angle towards second target
	VectorSubtract( ent2->origin, ent1->origin, newAngle );

	VectorAngles( newAngle, newAngle );
	newAngle[0] = -newAngle[0];

	// set angle diffrent in Dramtaic scenes
	newAngle[0] += 12.5f * dfactor; // lower angle if dramatic

	if( flags & DRC_FLAG_SIDE )
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	float d = MaxAngleBetweenAngles( v_lastAngles, newAngle );

	if( ( d < v_cameraFocusAngle) && ( v_cameraMode == CAM_MODE_RELAX ) )
	{
		// difference is to small and we are in relax camera mode, keep viewangles
		VectorCopy( v_lastAngles, newAngle );
	}
	else if( ( d < v_cameraRelaxAngle ) && ( v_cameraMode == CAM_MODE_FOCUS ) )
	{
		// we catched up with our target, relax again
		v_cameraMode = CAM_MODE_RELAX;
	}
	else
	{
		// target move too far away, focus camera again
		v_cameraMode = CAM_MODE_FOCUS;
	}

	// and smooth view, if not a scene cut
	if( v_resetCamera || ( v_cameraMode == CAM_MODE_RELAX ) )
	{
		VectorCopy( newAngle, angle );
	}
	else
	{
		V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 180.0f );
	}

	V_GetChaseOrigin( newAngle, newOrigin, distance, origin );

	// move position up, if very close at target
	if( v_lastDistance < 64.0f )
		origin[2] += 16.0f * ( 1.0f - ( v_lastDistance / 64.0f ) );

	// calculate angle to second target
	VectorSubtract( ent2->origin, origin, tempVec );
	VectorAngles( tempVec, tempVec );
	tempVec[0] = -tempVec[0];
}

void V_GetDirectedChasePosition(cl_entity_t *ent1, cl_entity_t *ent2,float *angle, float *origin)
{
	if( v_resetCamera )
	{
		v_lastDistance = 4096.0f;
		// v_cameraMode = CAM_MODE_FOCUS;
	}

	if( ( ent2 == (cl_entity_t*)0xFFFFFFFF ) || ( ent1->player && ( ent1->curstate.solid == SOLID_NOT ) ) )
	{
		// we have no second target or player just died
		V_GetSingleTargetCam( ent1, angle, origin );
	}
	else if( ent2 )
	{
		// keep both target in view
		V_GetDoubleTargetsCam( ent1, ent2, angle, origin );
	}
	else
	{
		// second target disappeard somehow (dead)

		// keep last good viewangle
		float newOrigin[3];

		int flags = gHUD.m_Spectator.m_iObserverFlags;

		float dfactor = ( flags & DRC_FLAG_DRAMATIC ) ? -1.0f : 1.0f;

		float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;

		// go away in final scenes or if player just died
		if( flags & DRC_FLAG_FINAL )
			distance *= 2.0f;	

		// let v_lastDistance float smoothly away
		v_lastDistance+= v_frametime * 32.0f;	// move unit per seconds back

		if( distance > v_lastDistance )
			distance = v_lastDistance;

		VectorCopy( ent1->origin, newOrigin );

		if( ent1->player )
			newOrigin[2] += 17.0f; // head level of living player
		else
			newOrigin[2] += 8.0f;	// object, tricky, must be above bomb in CS

		V_GetChaseOrigin( angle, newOrigin, distance, origin );
	}

	VectorCopy( angle, v_lastAngles );
}

void V_GetChasePos( int target, float *cl_angles, float *origin, float *angles )
{
	cl_entity_t *ent = NULL;

	if( target ) 
	{
		ent = gEngfuncs.GetEntityByIndex( target );
	}
	
	if( !ent )
	{
		// just copy a save in-map position
		VectorCopy( vJumpAngles, angles );
		VectorCopy( vJumpOrigin, origin );
		return;
	}

	if( ent->index == gEngfuncs.GetLocalPlayer()->index )
	{
		if( g_iUser3 )
		{
			V_GetDeathCam( ent, gEngfuncs.GetEntityByIndex( g_iUser3 ), angles, origin );
		}
		else
		{
			V_GetDeathCam( ent, NULL, angles, origin );
		}
	}
	else if( gHUD.m_Spectator.m_autoDirector->value )
	{
		if( g_iUser3 )
			V_GetDirectedChasePosition( ent, gEngfuncs.GetEntityByIndex( g_iUser3 ),
				angles, origin );
		else
			V_GetDirectedChasePosition( ent, (cl_entity_t*)0xFFFFFFFF,
				angles, origin );
	}
	else
	{
		if( cl_angles == NULL )	// no mouse angles given, use entity angles ( locked mode )
		{
			VectorCopy( ent->angles, angles);
			angles[0] *= -1.0f;
		}
		else
			VectorCopy( cl_angles, angles );

		VectorCopy( ent->origin, origin );

		origin[2] += 28.0f; // DEFAULT_VIEWHEIGHT - some offset

		V_GetChaseOrigin( angles, origin, cl_chasedist->value, origin );
	}

	v_resetCamera = false;	
}

void V_ResetChaseCam()
{
	v_resetCamera = true;
}

/*
==================
V_GetInEyePos
==================
*/
void V_GetInEyePos( int target, float *origin, float *angles )
{
	if( !target )
	{
		VectorCopy( vJumpAngles, angles );
		VectorCopy( vJumpOrigin, origin );
		return;
	}

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex( target );

	if( !ent )
		return;

	VectorCopy( ent->origin, origin );
	VectorCopy( ent->angles, angles );

	angles[PITCH] *= -3.0f;

	if( ent->curstate.solid == SOLID_NOT )
	{
		angles[ROLL] = 80.0f;
		origin[2] += -8.0f;
	}
	else if( ent->curstate.usehull == 1 )
	{
		origin[2] += 18.0f;
	}
	else
	{
		origin[2] += 22.0f;
	}
}

void V_GetMapFreePosition( float *cl_angles, float *origin, float *angles )
{
	vec3_t forward;
	vec3_t zScaledTarget;

	VectorCopy( cl_angles, angles );

	// modify angles since we don't wanna see map's bottom
	angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );

	zScaledTarget[0] = gHUD.m_Spectator.m_mapOrigin[0];
	zScaledTarget[1] = gHUD.m_Spectator.m_mapOrigin[1];
	zScaledTarget[2] = gHUD.m_Spectator.m_mapOrigin[2] * ( ( 90.0f - angles[0] ) / 90.0f );

	AngleVectors( angles, forward, NULL, NULL );

	VectorNormalize( forward );

	VectorMA( zScaledTarget, -( 4096.0f / gHUD.m_Spectator.m_mapZoom ), forward, origin );
}

/*
==================
V_GetMapChasePosition
==================
*/
void V_GetMapChasePosition( int target, float *cl_angles, float *origin, float *angles )
{
	vec3_t forward;

	if( target )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( target );

		if( gHUD.m_Spectator.m_autoDirector->value )
		{
			V_GetChasePos( target, cl_angles, origin, angles );
			VectorCopy( ent->curstate.origin, origin );
			angles[0] = 45.0f;
		}
		else
		{
			VectorCopy( cl_angles, angles );
			VectorCopy( ent->origin, origin );

			angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );
		}
	}
	else
	{
		VectorCopy( cl_angles, angles );
		angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );
	}

	origin[2] *= ( ( 90.0f - angles[0] ) / 90.0f );
	angles[2] = 0.0f;

	AngleVectors( angles, forward, NULL, NULL );

	VectorNormalize( forward );

	VectorMA( origin, -1536.0f, forward, origin );
}

extern p_wpninfo_s WpnInfo[];

/*
==================
V_FindViewModelByWeaponModel
==================
*/
int V_FindViewModelByWeaponModel( int weaponindex )
{
	struct model_s *weaponModel;
	int len;

	weaponModel = IEngineStudio.GetModelByIndex( weaponindex );

	if( !weaponModel || !weaponModel->name )
		return 0;

	len = strlen( weaponModel->name );

	int i = 1;

	while( i < 41 )
	{
		if( WpnInfo[i].pmodel != NULL )
		{
			if( _strnicmp( weaponModel->name, WpnInfo[i].pmodel, len ) == 0 )
			{
				return gEngfuncs.pEventAPI->EV_FindModelIndex( WpnInfo[i].vmodel );
			}
		}
		i++;
	}

	return 0;
}

/*
==================
V_CalcSpectatorRefdef

==================
*/
extern int EV_GetWeaponBody( void );

void V_CalcSpectatorRefdef( struct ref_params_s *pparams )
{
	static vec3_t velocity = { 0.0f, 0.0f, 0.0f };

	static int lastWeaponModelIndex = 0;
	static int lastViewModelIndex = 0;

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex( g_iUser2 );

	pparams->onlyClientDraw = false;
	g_iOnlyClientDraw = 0;

	// refresh position
	VectorCopy( pparams->simorg, v_sim_org );

	// get old values
	VectorCopy( pparams->cl_viewangles, v_cl_angles );
	VectorCopy( pparams->viewangles, v_angles );
	VectorCopy( pparams->vieworg, v_origin );

	if( ( g_iUser1 == OBS_IN_EYE || gHUD.m_Spectator.m_pip->value == 2.0f ) && ent )
	{
		// calculate player velocity
		float timeDiff = ent->curstate.msg_time - ent->prevstate.msg_time;

		if( timeDiff > 0.0f )
		{
			vec3_t distance;
			VectorSubtract( ent->prevstate.origin, ent->curstate.origin, distance );
			VectorScale( distance, 1.0f / timeDiff, distance );

			velocity[0] = velocity[0] * 0.9f + distance[0] * 0.1f;
			velocity[1] = velocity[1] * 0.9f + distance[1] * 0.1f;
			velocity[2] = velocity[2] * 0.9f + distance[2] * 0.1f;

			VectorCopy( velocity, pparams->simvel );
		}

		// predict missing client data and set weapon model ( in HLTV mode )
		if( gEngfuncs.IsSpectateOnly() || gHUD.m_Spectator.m_pip->value == 2.0f )
		{
			V_GetInEyePos( g_iUser2, pparams->simorg, pparams->cl_viewangles );

			pparams->health = 1;

			cl_entity_t *gunModel = gEngfuncs.GetViewModel();

			if( lastWeaponModelIndex != ent->curstate.weaponmodel )
			{
				// weapon model changed
				lastWeaponModelIndex = ent->curstate.weaponmodel;
				lastViewModelIndex = V_FindViewModelByWeaponModel( lastWeaponModelIndex );

				if( lastViewModelIndex )
				{
					int WeaponBody = EV_GetWeaponBody();
					gEngfuncs.pfnWeaponAnim( 0, WeaponBody );
				}
				else
				{
					// model not found
					gunModel->model = NULL;
					lastWeaponModelIndex = lastViewModelIndex = 0;
				}
			}

			if( lastViewModelIndex )
			{
				gunModel->model = IEngineStudio.GetModelByIndex( lastViewModelIndex );
				gunModel->curstate.modelindex = lastViewModelIndex;
				gunModel->curstate.frame = 0.0f;
				gunModel->curstate.colormap = 0;
				gunModel->index = g_iUser2;
			}
			else
			{
				gunModel->model = NULL;
			}
		}
		else
		{
			// only get viewangles from entity
			VectorCopy( ent->angles, pparams->cl_viewangles );
			pparams->cl_viewangles[PITCH] *= -3.0f;	// see CL_ProcessEntityUpdate()
		}
	}
	else if( ent && gHUD.m_Spectator.m_pip->value != 2.0f )
	{
		VectorCopy( ent->angles, pparams->cl_viewangles );
		pparams->cl_viewangles[PITCH] *= -3.0f;
	}

	v_frametime = pparams->frametime;

	if( pparams->nextView == 0 )
	{
		if( g_iUser1 == OBS_CHASE_LOCKED )
		{
			V_GetChasePos( g_iUser2, NULL, v_origin, v_angles );
		}
		else if( g_iUser1 == OBS_CHASE_FREE )
		{
			V_GetChasePos( g_iUser2, v_cl_angles, v_origin, v_angles );
		}
		else if( g_iUser1 == OBS_ROAMING )
		{
			VectorCopy( v_cl_angles, v_angles );
			VectorCopy( v_sim_org, v_origin );
		}
		else if( g_iUser1 == OBS_IN_EYE )
		{
			V_CalcNormalRefdef( pparams );
		}

		if( gHUD.GetMinimapState() > 0 )
		{
			pparams->nextView = 1;	// force a second renderer view
		}
	}
	else
	{
		if( pparams->nextView == 1 )
		{
			int map_x, map_y, map_w, map_h;
			gHUD.GetMapBounds( map_x, map_y, map_w, map_h );

			pparams->viewangles[PITCH] = 90.0f;
			pparams->viewangles[YAW] = 0.0f;
			pparams->viewangles[ROLL] = 0.0f;

			pparams->viewport[0] = map_x;
			pparams->viewport[1] = map_y;
			pparams->viewport[2] = map_w;
			pparams->viewport[3] = map_h;

			pparams->nextView = 0;
			pparams->onlyClientDraw = true;
			g_iOnlyClientDraw = 1;

			vec3_t veca;
			veca[0] = gHUD.m_Spectator.m_mapOrigin.x;
			veca[1] = gHUD.m_Spectator.m_mapOrigin.y;
			veca[2] = 0.0f;

			vec3_t distance;
			AngleVectors( pparams->viewangles, distance, NULL, NULL );
			VectorNormalize( distance );

			float scalec = -( 4096.0f / gHUD.m_Spectator.m_mapZoom );
			VectorMA( veca, scalec, distance, pparams->vieworg );
		}
	}

	// write back new values into pparams
	VectorCopy( v_cl_angles, pparams->cl_viewangles );
	VectorCopy( v_angles, pparams->viewangles );
	VectorCopy( v_origin, pparams->vieworg );
}

/*
==================
V_CalcRefdef

==================
*/
int g_ionground;

void DLLEXPORT V_CalcRefdef( struct ref_params_s *pparams )
{
	if( !IEngineStudio.IsHardware() )
	{
		cl_entity_t *viewent = gEngfuncs.GetViewModel();

		if( viewent )
		{
			viewent->curstate.rendermode = kRenderTransAlpha;
		}
	}

	g_ionground = pparams->onground;

	// intermission / finale rendering
	if( pparams->intermission )
	{
		V_CalcIntermissionRefdef( pparams );
	}
	else if( pparams->spectator || g_iUser1 )	// g_iUser true if in spectator mode
	{
		V_CalcSpectatorRefdef( pparams );
	}
	else if( !pparams->paused )
	{
		V_CalcNormalRefdef( pparams );
	}
}

/*
=============
V_DropPunchAngle

=============
*/
void V_DropPunchAngle( float frametime, float *ev_punchangle )
{
	float len;

	len = VectorNormalize( ev_punchangle );
	len -= ( 10.0f + len * 0.5f ) * (float)frametime;
	len = max( len, 0.0f );
	VectorScale( ev_punchangle, len, ev_punchangle );
}

/*
=============
V_PunchAxis

Client side punch effect
=============
*/
void V_PunchAxis( int axis, float punch )
{
	g_ev_punchangle[axis] = punch;
}

extern bool DoDAreWeAttacking( void );

void V_DoDCenterViewCommand( void )
{
	if( !DoDAreWeAttacking() )
	{
		V_StartPitchDrift();
	}
}

/*
=============
V_Init
=============
*/
void V_Init( void )
{
	gEngfuncs.pfnAddCommand( "centerview", V_DoDCenterViewCommand );

	scr_ofsx = gEngfuncs.pfnRegisterVariable( "scr_ofsx", "0", 0 );
	scr_ofsy = gEngfuncs.pfnRegisterVariable( "scr_ofsy", "0", 0 );
	scr_ofsz = gEngfuncs.pfnRegisterVariable( "scr_ofsz", "0", 0 );

	v_centermove = gEngfuncs.pfnRegisterVariable( "v_centermove", "0.15", 0 );
	v_centerspeed = gEngfuncs.pfnRegisterVariable( "v_centerspeed", "500", 0 );

	cl_bobcycle = gEngfuncs.pfnRegisterVariable( "cl_bobcycle", "0.8", 0 );
	cl_bob = gEngfuncs.pfnRegisterVariable( "cl_bob", "0.005", FCVAR_CLIENTDLL );

	cl_bobup = gEngfuncs.pfnRegisterVariable( "cl_bobup", "0.5", 0 );
	cl_waterdist = gEngfuncs.pfnRegisterVariable( "cl_waterdist", "4", 0 );
	cl_chasedist = gEngfuncs.pfnRegisterVariable( "cl_chasedist", "112", 0 );
}

/*
====================
CalcFov
====================
*/
float CalcFov( float fov_x, float width, float height )
{
	float a;
	float x;
	float flTanValue;

	if( fov_x < 1.0f )
	{
		flTanValue = 0.99999999f;
	}
	else if( fov_x <= 179.0f )
	{
		flTanValue = tan( ( fov_x / 360.0f ) * M_PI_F );
	}
	else
	{
		flTanValue = 0.99999999f;
	}

	x = width / flTanValue;

	a = atan( height / x );

	a = a * 360.0f / M_PI_F;

	return a;
}
