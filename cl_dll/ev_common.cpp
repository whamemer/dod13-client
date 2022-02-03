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

#define IS_FIRSTPERSON_SPEC ( g_iUser1 == OBS_IN_EYE || ( g_iUser1 && ( gHUD.m_Spectator.m_pip->value == INSET_IN_EYE ) ) )
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
	// check if we are in some way in first person spec mode
	if( IS_FIRSTPERSON_SPEC )
		return ( g_iUser2 == idx );
	else
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
		if( EV_IsLocal( idx ) && !IS_FIRSTPERSON_SPEC )
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

/*
=================
EV_PSchreckSmoke & EV_BazookaSmoke

Need Particleman
=================
*/
void EV_BazookaSmoke( cl_entity_t *ent )
{
	int ccw, speeds[8];
	float rf;
	Vector temp, p_org, p_normal, forward, right, up;
	model_s *pSprite;
	CDoDParticle *pParticle;

	if( ent )
	{
		temp.x = ent->angles.x;
		temp.y = ent->angles.y;
		temp.z = -ent->angles.x;

		pSprite = (struct model_s *)gEngfuncs.GetSpritePointer( SPR_Load( "sprites/bazookapuff.spr" ) );

		if( pSprite )
		{
			ccw = 1;

			for( int i; i != 8; ++i )
			{
				if( ( i & 1 ) != 0 )
				{
					p_org.x = forward.x * 30.0f + up.x * 16.0f + ent->origin.x;
					p_org.y = forward.y * 30.0f + up.y * 16.0f + ent->origin.y;
					p_org.z = forward.z * 30.0f + up.z * 16.0f + ent->origin.z;
				}
				else
				{
					ccw = 1 - ccw;

					p_org.x = up.x * 16.0f + ent->origin.x - forward.x * 30.0f;
					p_org.y = up.y * 16.0f + ent->origin.y - forward.y * 30.0f;
					p_org.z = up.z * 16.0f + ent->origin.z - forward.z * 30.0f;
				}

				memset( &p_normal, 0, sizeof( p_normal ) );
				pParticle = CDodParticle::Create( &p_org, &p_normal, pSprite, 90.0f, 80.0f, "dod_particle", 1 );

				if( !pParticle )
					break;
				
				pParticle->m_iCollisionFlags |= iFlag;
				pParticle->m_iRendermode = 5;
				pParticle->m_iRenderFlags |= iFlag;
				pParticle->m_flFadeSpeed = 0.7f;
				pParticle->m_flScaleSpeed = 1.1f;
				pParticle->m_flDampingTime = 0.5f;
				pParticle->m_iFrame = 0;
				pParticle->m_iFramerate = 2;
				pParticle->m_vColor.x = 200.0f;
				pParticle->m_vColor.y = 200.0f;
				pParticle->m_vColor.z = 200.0f;
				pParticle->m_vVelocity.x = forward.x * speeds[i] + up.x * gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
				pParticle->m_vVelocity.y = forward.y * speeds[i] + up.y * gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
				pParticle->m_vVelocity.z = forward.z * speeds[i] + up.z * gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );

				if( !ccw )
				{
					rf = -1.0f;
				}
				else
				{
					rf = 1.0f;
				}

				rf = pParticle->m_vAVelocity.z;
				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 5.0f;
				CDoDParticle::AddGlobalWind( pParticle );
			}
		}
		else
		{
			gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", "sprites/bazookapuff.spr" );
		}
	}
}

void EV_PSchreckSmoke( cl_entity_t *ent )
{
	EV_BazookaSmoke( ent );
}

/*
=================
EV_GetDefaultShellInfo

Determine where to eject shells from
=================
*/
void EV_GetDefaultShellInfo( event_args_t *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale )
{
	int i;
	vec3_t view_ofs;
	float fR, fU;

	int idx;

	idx = args->entindex;

	VectorClear( view_ofs );
	view_ofs[2] = DEFAULT_VIEWHEIGHT;

	if( EV_IsPlayer( idx ) )
	{
		if( EV_IsLocal( idx ) )
		{
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight( view_ofs );
		}
		else if( args->ducking == 1 )
		{
			view_ofs[2] = VEC_DUCK_VIEW;
		}
	}

	fR = gEngfuncs.pfnRandomFloat( 50, 70 );
	fU = gEngfuncs.pfnRandomFloat( 100, 150 );

	for( i = 0; i < 3; i++ )
	{
		ShellVelocity[i] = velocity[i] + right[i] * fR + up[i] * fU + forward[i] * 25;
		ShellOrigin[i] = origin[i] + view_ofs[i] + up[i] * upScale + forward[i] * forwardScale + right[i] * rightScale;
	}
}

/*
=================
EV_MuzzleFlash

Flag weapon/view model for muzzle flash
=================
*/
void EV_MuzzleFlash( void )
{
	// Add muzzle flash to current weapon model
	cl_entity_t *ent = GetViewEntity();
	if( !ent )
	{
		return;
	}

	// Or in the muzzle flash
	ent->curstate.effects |= EF_MUZZLEFLASH;
}
