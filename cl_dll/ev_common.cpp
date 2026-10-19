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
// shared event functions

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"

#include "r_efx.h"

#include "eventscripts.h"
#include "event_api.h"
#include "pm_shared.h"

#include "tri.h"

extern cvar_t *cl_dynamiclights;

/*
=================
GetEntity

Return's the requested cl_entity_t
=================
*/
struct cl_entity_s *GetEntity( int idx )
{
	return gEngfuncs.GetEntityByIndex( idx );
}

/*
=================
GetViewEntity

Return's the current weapon/view model
=================
*/
struct cl_entity_s *GetViewEntity( void )
{
	return gEngfuncs.GetViewModel();
}

/*
=================
EV_CreateTracer

Creates a tracer effect
=================
*/
void EV_CreateTracer( float *start, float *end )
{
	gEngfuncs.pEfxAPI->R_TracerEffect( start, end );
}

/*
=================
EV_IsPlayer

Is the entity's index in the player range?
=================
*/
qboolean EV_IsPlayer( int idx )
{
	if( idx >= 1 && idx <= gEngfuncs.GetMaxClients() )
		return true;

	return false;
}

/*
=================
EV_IsLocal

Is the entity == the local player
=================
*/
qboolean EV_IsLocal( int idx )
{
	if( g_iUser1 == OBS_IN_EYE )
		return g_iUser2 == idx;

	return gEngfuncs.pEventAPI->EV_IsLocal( idx - 1 ) ? true : false;
}

/*
=================
EV_GetGunPosition

Figure out the height of the gun
=================
*/
void EV_GetGunPosition( event_args_t *args, float *pos, float *origin )
{
	int idx;
	vec3_t view_ofs;

	idx = args->entindex;

	VectorClear( view_ofs );
	view_ofs[2] = DEFAULT_VIEWHEIGHT;

	if( EV_IsPlayer( idx ) )
	{
		// in spec mode use entity viewheigh, not own
		if( EV_IsLocal( idx ) && g_iUser1 != OBS_IN_EYE )
		{
			// Grab predicted result for local player
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight( view_ofs );
		}
		else if( args->ducking == 1 )
		{
			view_ofs[2] = VEC_DUCK_VIEW;
		}
	}

	VectorAdd( origin, view_ofs, pos );
}

/*
=================
EV_EjectBrass

Bullet shell casings
=================
*/
void EV_EjectBrass( float *origin, float *velocity, float rotation, int model, int soundtype )
{
	vec3_t endpos;
	VectorClear( endpos );
	endpos[1] = rotation;
	gEngfuncs.pEfxAPI->R_TempModel( origin, velocity, endpos, 2.5, model, soundtype );
}

int speeds[8] = { 0, 0, -4, 4, -6, 6, -8, 8 };

/*
=================
EV_PSchreckSmoke

Exhaust of the launcher (nothing calls it in the original)
=================
*/
void EV_PSchreckSmoke( cl_entity_t *ent )
{
	EV_BazookaSmoke( ent );
}

/*
=================
EV_BazookaSmoke

Exhaust of the launcher (nothing calls it in the original, HUD_StudioEvent uses Event_BazookaSmoke)
=================
*/
void EV_BazookaSmoke( cl_entity_t *ent )
{
	vec3_t forward, right, up, temp;
	model_s *pSprite;
	int ccw;
	int i;

	if( ent )
	{
		temp = Vector( ent->angles );
		temp.z = -ent->angles.z;

		AngleVectors( temp, forward, right, up );

		pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/bazookapuff.spr" ) );

		if( pSprite )
		{
			ccw = 1;

			for( i = 0; i < 8; i++ )
			{
				vec3_t org;
				CDoDParticle *pParticle;
				int speed;
				float flRand;

				if( i & 1 )
				{
					org = forward * 30.0f + up * 16.0f + ent->origin;
				}
				else
				{
					ccw = 1 - ccw;
					org = up * 16.0f + ent->origin - forward * 30.0f;
				}

				speed = speeds[i];
				flRand = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );

				pParticle = CDoDParticle::Create( org, Vector( 0, 0, 0 ), pSprite, 90.0f, 80.0f, "dod_particle", true );

				if( !pParticle )
					break;

				pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_ANIMATEDIE );
				pParticle->m_iRendermode = kRenderTransAdd;
				pParticle->SetLightFlag( LIGHT_NONE );
				pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_PLANE );
				pParticle->m_flFadeSpeed = 0.7f;
				pParticle->m_flScaleSpeed = 1.1f;
				pParticle->m_flDampingTime = 0.5f;
				pParticle->m_iFrame = 0;
				pParticle->m_iFramerate = 2;
				pParticle->m_vColor = Vector( 200.0f, 200.0f, 200.0f );
				pParticle->m_vVelocity = forward * (float)speed + up * flRand;
				pParticle->m_vAVelocity.z = ccw ? 1.0f : -1.0f;
				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 5.0f;
				pParticle->AddGlobalWind();
			}
		}
		else
		{
			gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", "sprites/bazookapuff.spr" );
		}
	}
}

/*
=================
EV_GetDefaultShellInfo

Determine where to eject shells from
=================
*/
void EV_GetDefaultShellInfo( event_args_t *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale )
{
	/* Nothing. */
}

/*
=================
EV_MuzzleFlash

Dynamic light of the muzzle flash
=================
*/
void EV_MuzzleFlash( int idx, int guntype )
{
	cl_entity_t *ent;
	vec3_t vLightOrigin;
	bool bShowLight = false;

	if( EV_IsLocal( idx ) )
	{
		ent = GetViewEntity();
		VectorCopy( ent->attachment[0], vLightOrigin );
		bShowLight = true;
	}
	else
	{
		cl_entity_t *thisent = gEngfuncs.GetEntityByIndex( idx );

		if( thisent && thisent->curstate.messagenum >= gEngfuncs.GetLocalPlayer()->curstate.messagenum )
		{
			VectorCopy( thisent->curstate.origin, vLightOrigin );
			bShowLight = true;
		}
	}

	if( bShowLight && cl_dynamiclights->value > 0.0f )
	{
		dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

		VectorCopy( vLightOrigin, dl->origin );
		dl->radius = ( float )( 50 * guntype + 50 );
		dl->color.r = 248;
		dl->color.g = 255;
		dl->color.b = 120;
		dl->decay = ( float )( 50 * guntype + 600 );
		dl->die = gHUD.m_flTime + 1.0;
	}
}
