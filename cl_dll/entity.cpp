//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Client side entity management functions

#include <memory.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_types.h"
#include "studio_event.h" // def. of mstudioevent_t
#include "r_efx.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"	
#include "pm_shared.h"
#include "particleman.h"
#include "particleman_internal.h"
#include "tri.h"

void Game_AddObjects( void );

extern vec3_t v_origin;

int g_iAlive = 1;
int g_iDeadFlag = 0;

extern int g_iWeaponBits2, g_iMovetype, g_iEffects;
extern float g_fStamina, g_flGravity;

extern "C"
{
	int DLLEXPORT HUD_AddEntity( int type, struct cl_entity_s *ent, const char *modelname );
	void DLLEXPORT HUD_CreateEntities( void );
	void DLLEXPORT Event_EjectBrassP( const struct cl_entity_s *entity, int shelltype );
	void DLLEXPORT Event_EjectBrassV( const struct cl_entity_s *entity, int shelltype );
	void DLLEXPORT Event_MuzzleFlash( const struct cl_entity_s *entity, int options );
	void DLLEXPORT Event_MuzzleSmoke( const struct cl_entity_s *entity, int options );
	void DLLEXPORT Event_BazookaSmoke( const struct cl_entity_s *entity );
	void DLLEXPORT Event_PIATSmoke( const struct cl_entity_s *entity );
	void DLLEXPORT HUD_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
	void DLLEXPORT HUD_TxferLocalOverrides( struct entity_state_s *state, const struct clientdata_s *client );
	void DLLEXPORT HUD_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src );
	void DLLEXPORT HUD_TxferPredictionData ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
	void DLLEXPORT HUD_TempEntUpdate( double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int ( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ), void ( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
	struct cl_entity_s DLLEXPORT *HUD_GetUserEntity( int index );
}

extern cvar_t *cl_dmshowplayers;
extern cvar_t *cl_dmshowobjects;
extern cvar_t *cl_dmshowgrenades;

/*
========================
HUD_AddEntity
	Return 0 to filter entity from visible list for rendering
========================
*/
int DLLEXPORT HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname)
{
	if( !modelname || modelname[0] == '\0' )
	{
		return 1;
	}

	int mapstate = gHUD.GetMinimapState();

	if( gEngfuncs.IsSpectateOnly() || g_iUser1 || mapstate > 0 )
	{
		if( g_iTeamNumber == 1 || gEngfuncs.IsSpectateOnly() )
		{
			const char *hg = strstr( modelname, "w_grenade" );
			if( hg && ( !mapstate || cl_dmshowgrenades->value > 0.0f ) )
				gHUD.m_Spectator.AddOverviewEntityToList( gHUD.m_Spectator.m_hsprGrenade, ent, gEngfuncs.GetClientTime() - 1.0 );

			const char *m = strstr( modelname, "w_mills" );
			if( m && ( !mapstate || cl_dmshowgrenades->value > 0.0f ) )
				gHUD.m_Spectator.AddOverviewEntityToList( gHUD.m_Spectator.m_hsprGrenade, ent, gEngfuncs.GetClientTime() - 1.0 );
		}

		if( g_iTeamNumber == 2 || gEngfuncs.IsSpectateOnly() )
		{
			const char *s = strstr( modelname, "w_stick" );
			if( s && ( !mapstate || cl_dmshowgrenades->value > 0.0f ) )
			{
				gHUD.m_Spectator.AddOverviewEntityToList( gHUD.m_Spectator.m_hsprStick, ent, gEngfuncs.GetClientTime() - 1.0 );
			}
		}

		const char *t = strstr( modelname, "tnt" );
		if( t && ( !mapstate || cl_dmshowobjects->value > 0.0f ) )
			gHUD.m_Spectator.AddOverviewEntityToList( gHUD.m_Spectator.m_hsprTnT, ent, gEngfuncs.GetClientTime() - 1.0 );

		const char *b = strstr( modelname, "bangalore" );
		if( b && ( !mapstate || cl_dmshowobjects->value > 0.0f ) )
			gHUD.m_Spectator.AddOverviewEntityToList( gHUD.m_Spectator.m_hsprBanglr, ent, gEngfuncs.GetClientTime() - 1.0 );

		const char *m_ptr = strstr( modelname, "markernull" );
		if( m_ptr )
		{
			HSPRITE pSpr = gHUD.m_Spectator.GetMarkerSPR( ent->curstate.body );
			int marker = ( pSpr != NULL );

			if( marker )
				gHUD.m_Spectator.AddOverviewEntityToList( pSpr, ent, gEngfuncs.GetClientTime() + 0.2 );
		}

		const char *c = strstr(modelname, "hltv_");
		if( c && ( ent->curstate.rendermode != kRenderTransTexture || ent->curstate.renderamt ) )
		{
			if( !mapstate || cl_dmshowobjects->value > 0.0f )
			{
				char file[76];
				char sz[128];

				strncpy( file, c, sizeof( file ) - 5 );
				file[sizeof( file ) - 5] = '\0';

				strcat( file, ".spr" );
				sprintf( sz, "sprites/%s", file );

				gHUD.m_Spectator.m_hsprCustom = gEngfuncs.pfnSPR_Load( sz );
				gHUD.m_Spectator.AddOverviewEntityToList( gHUD.m_Spectator.m_hsprCustom, ent, gEngfuncs.GetClientTime() - 1.0 );
			}
		}
	}

	if( g_iUser1 || ( mapstate && cl_dmshowplayers->value > 0.0f ))
	{
		gHUD.m_Spectator.AddOverviewEntity( type, ent, modelname );

		if( ( g_iUser1 != OBS_IN_EYE && gHUD.m_Spectator.m_pip->value != 2.0f ) || ( ent->index != g_iUser2 ) )
			return 1;
	}

	return 0;
}

/*
=========================
HUD_TxferLocalOverrides

The server sends us our origin with extra precision as part of the clientdata structure, not during the normal
playerstate update in entity_state_t.  In order for these overrides to eventually get to the appropriate playerstate
structure, we need to copy them into the state structure at this point.
=========================
*/
void DLLEXPORT HUD_TxferLocalOverrides( struct entity_state_s *state, const struct clientdata_s *client )
{
	VectorCopy( client->origin, state->origin );

	// Spectator
	state->iuser1 = client->iuser1;
	state->iuser2 = client->iuser2;

	// Duck prevention
	state->iuser3 = client->iuser3;

	// Fire prevention
	state->iuser4 = client->iuser4;

	state->fuser4 = client->fuser4;
	state->vuser1 = client->vuser1;
}

/*
=========================
HUD_ProcessPlayerState

We have received entity_state_t for this player over the network.  We need to copy appropriate fields to the
playerstate structure
=========================
*/
void DLLEXPORT HUD_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src )
{
	// Copy in network data
	VectorCopy( src->origin, dst->origin );
	VectorCopy( src->angles, dst->angles );

	VectorCopy( src->velocity, dst->velocity );

	dst->frame					= src->frame;
	dst->modelindex				= src->modelindex;
	dst->skin					= src->skin;
	dst->effects				= src->effects;
	dst->weaponmodel			= src->weaponmodel;
	dst->movetype				= src->movetype;
	dst->sequence				= src->sequence;
	dst->animtime				= src->animtime;
	
	dst->solid					= src->solid;
	
	dst->rendermode				= src->rendermode;
	dst->renderamt				= src->renderamt;	
	dst->rendercolor.r			= src->rendercolor.r;
	dst->rendercolor.g			= src->rendercolor.g;
	dst->rendercolor.b			= src->rendercolor.b;
	dst->renderfx				= src->renderfx;

	dst->framerate				= src->framerate;
	dst->body					= src->body;

	memcpy( &dst->controller[0], &src->controller[0], 4 * sizeof(byte) );
	memcpy( &dst->blending[0], &src->blending[0], 2 * sizeof(byte) );

	VectorCopy( src->basevelocity, dst->basevelocity );

	dst->friction				= src->friction;
	dst->gravity				= src->gravity;
	dst->gaitsequence			= src->gaitsequence;
	dst->spectator				= src->spectator;
	dst->usehull				= src->usehull;
	dst->playerclass			= src->playerclass;
	dst->team					= src->team;
	dst->colormap				= src->colormap;

	if( gEngfuncs.IsSpectateOnly() )
	{
		g_PlayerExtraInfo[dst->number].teamnumber = src->team;
		g_PlayerExtraInfo[dst->number].playerclass = src->playerclass;
	}

	dst->iuser1 = src->iuser1;
	dst->iuser2 = src->iuser2;
	dst->iuser3 = src->iuser3;
	dst->iuser4 = src->iuser4;
	dst->fuser4 = src->fuser4;

	VectorCopy( src->vuser1, dst->vuser1 );

	g_PlayerExtraInfo[dst->number].health = src->health;

	// Save off some data so other areas of the Client DLL can get to it
	cl_entity_s *player = gEngfuncs.GetLocalPlayer();	// Get the local player's index

	if( dst->number == player->index )
	{
		g_iPlayerClass = dst->playerclass;
		g_iTeamNumber = dst->team;

		g_iUser1 = src->iuser1;
		g_iUser2 = src->iuser2;

		if( g_iUser3 == 3 )
		{
			if( src->iuser3 != 3 )
			{
				gHUD.SetMortarUnDeployTime();
			}
		}
		else
		{
			if( src->iuser3 == 3 )
			{
				gHUD.SetMortarDeployTime();
			}
		}

		g_iUser3 = src->iuser3;
	}

	g_iUser3 = src->iuser3;
	g_iWeaponBits2 = src->iuser4;
	g_iVuser1x = src->vuser1[0];
	g_iVuser1z = src->vuser1[1];
	g_iMovetype = src->movetype;
	g_iEffects = src->effects;
	g_fStamina = src->fuser4;
}

/*
=========================
HUD_TxferPredictionData

Because we can predict an arbitrary number of frames before the server responds with an update, we need to be able to copy client side prediction data in
 from the state that the server ack'd receiving, which can be anywhere along the predicted frame path ( i.e., we could predict 20 frames into the future and the server ack's
 up through 10 of those frames, so we need to copy persistent client-side only state from the 10th predicted frame to the slot the server
 update is occupying.
=========================
*/
void DLLEXPORT HUD_TxferPredictionData( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, 
										const struct clientdata_s *ppcd, struct weapon_data_s *wd, 
										const struct weapon_data_s *pwd )
{
	ps->oldbuttons = pps->oldbuttons;
	ps->flFallVelocity = pps->flFallVelocity;
	ps->iStepLeft = pps->iStepLeft;
	ps->playerclass = pps->playerclass;

	pcd->viewmodel = ppcd->viewmodel;
	pcd->m_iId = ppcd->m_iId;
	pcd->ammo_shells = ppcd->ammo_shells;
	pcd->ammo_rockets = ppcd->ammo_rockets;
	pcd->m_flNextAttack = ppcd->m_flNextAttack;
	pcd->fov = ppcd->fov;
	pcd->weaponanim = ppcd->weaponanim;
	pcd->tfstate = ppcd->tfstate;
	pcd->maxspeed = ppcd->maxspeed;

	pcd->deadflag = ppcd->deadflag;

	bool bIsAlive = true;

	if( ppcd->iuser1 == 0 )
	{
		bIsAlive = ( ppcd->deadflag == 0 );
	}
	g_iAlive = bIsAlive;
	g_iDeadFlag = ppcd->deadflag;

	pcd->iuser1 = ppcd->iuser1;
	pcd->iuser2 = ppcd->iuser2;
	pcd->iuser3 = ppcd->iuser3;
	pcd->iuser4 = ppcd->iuser4;

	if( gEngfuncs.IsSpectateOnly() )
	{
		pcd->iuser1 = g_iUser1;
		pcd->iuser2 = g_iUser2;
		pcd->iuser3 = g_iUser3;
	}

	pcd->fuser2 = ppcd->fuser2;
	pcd->fuser4 = ppcd->fuser4;

	VectorCopy( ppcd->vuser1, pcd->vuser1 );

	memcpy( wd, pwd, MAX_WEAPONS * sizeof( weapon_data_t ) );
}

/*
=========================
HUD_CreateEntities
	
Gives us a chance to add additional entities to the render this frame
=========================
*/
void DLLEXPORT HUD_CreateEntities( void )
{
	// e.g., create a persistent cl_entity_t somewhere.
	// Load an appropriate model into it ( gEngfuncs.CL_LoadModel )
	// Call gEngfuncs.CL_CreateVisibleEntity to add it to the visedicts list

	Game_AddObjects();
	GetClientVoiceHud()->CreateEntities();
	gHUD.m_StatusBar.CreateEntities();
}

/*
=========================
Event_EjectBrassP

=========================
*/
extern cvar_t *cl_bulletejects;
extern vec3_t v_angles;

void DLLEXPORT Event_EjectBrassP( const struct cl_entity_s *entity, int shelltype )
{
	vec3_t endpos, forward, right, up, velocity;
	vec3_t temp = entity->angles;
	int shellmodel = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shells.mdl" );
	TEMPENTITY *te = NULL;

	if( cl_bulletejects->value != 0.0f && shelltype <= 3 )
	{
		VectorNormalize( temp );
		AngleVectors( temp, forward, right, up );
		VectorScale( up, 100.0f, velocity );

		endpos[0] = 0.0f;
		endpos[1] = entity->angles[1];
		endpos[2] = 0.0f;

		te = gEngfuncs.pEfxAPI->R_TempModel( (float*)&entity->attachment[1], velocity, endpos, 2.5f, shellmodel, 1);
	}

	if( te )
		te->entity.curstate.body = shelltype;
}

/*
=========================
Event_EjectBrassV

=========================
*/
void DLLEXPORT Event_EjectBrassV( const struct cl_entity_s *entity, int shelltype )
{
	vec3_t endpos, forward, right, up, velocity, temp;
	int shellmodel = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shells.mdl" );
	TEMPENTITY *te = NULL;

	if( cl_bulletejects->value != 0.0f && shelltype <= 3 )
	{
		AngleVectors( v_angles, forward, right, up );
		VectorScale( right, gEngfuncs.pfnRandomLong( 60, 80 ), velocity );
		VectorScale( up, gEngfuncs.pfnRandomLong( 60, 80 ), temp );
		velocity = velocity + temp;

		endpos[0] = 0.0f;
		endpos[1] = entity->angles[1];
		endpos[2] = 0.0f;

		te = gEngfuncs.pEfxAPI->R_TempModel( ( float * ) &entity->attachment[1], velocity, endpos, 2.5, shellmodel, 1 );
	}

	if( te )
		te->entity.curstate.body = shelltype;
}

/*
=========================
Event_MuzzleFlash

=========================
*/
void DLLEXPORT Event_MuzzleFlash( const struct cl_entity_s *entity, int options )
{
	vec3_t origin, forward, right, up;
	vec3_t temp, org, normal;

	static char *sprname = "sprites/muzzleflash2.spr";
	static char *mgsprname = "sprites/muzzleflash1.spr";

	int flashtype;
	CDoDParticle *pParticle;
	model_s *pMGSprite;
	model_s *pSprite;

	AngleVectors( entity->angles, forward, right, up );

	float scale = ( float ) ( options / 10 ) * 0.1f;
	flashtype = ( options % 10 ) % 4;

	float finalScale = ( scale != 0.0f ) ? scale : 0.5f;

	if( flashtype == 3 )
	{
		HSPRITE hSpr1 = gEngfuncs.pfnSPR_Load( mgsprname );
		pMGSprite = (model_s *)gEngfuncs.GetSpritePointer( hSpr1 );
	}

	HSPRITE hSpr2 = gEngfuncs.pfnSPR_Load( sprname );
	pSprite = (model_s *)gEngfuncs.GetSpritePointer( hSpr2 );

	if( !pSprite )
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", sprname );
		return;
	}

	forward.z = -forward.z;
	origin = entity->attachment[0];

	VectorScale( &forward.x, 12.0f, &temp.x );
	memset( &normal, 0, sizeof( normal ) );
	org = origin + temp;

	float size1 = finalScale * 20.0f;
	pParticle = pParticle->Create( &org, &normal, pSprite, size1, 255.0f, "dod_particle", 1 );

	if( pParticle )
	{
		pParticle->m_vVelocity = { 0.0f, 0.0f, 0.0f };
		pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 0.0001f;
		pParticle->m_iRendermode = kRenderTransAdd;
		pParticle->m_vColor = { 255.0f, 255.0f, 255.0f };

		pParticle->SetCullFlag( CULL_FRUSTUM_SPHERE | CULL_PVS );
		pParticle->SetLightFlag( LIGHT_NONE );
	}

	VectorScale( &forward.x, 8.0f, &temp.x );
	memset( &normal, 0, sizeof( normal ) );
	org = origin + temp;

	float size2 = finalScale * 40.0f;
	pParticle = pParticle->Create( &org, &normal, pSprite, size2, 255.0f, "dod_particle", 1 );

	if( pParticle )
	{
		pParticle->m_vVelocity = { 0.0f, 0.0f, 0.0f };
		pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 0.0001f;
		pParticle->m_iRendermode = kRenderTransAdd;
		pParticle->m_vColor = { 255.0f, 255.0f, 255.0f };
		pParticle->SetCullFlag( CULL_FRUSTUM_SPHERE | CULL_PVS );
		pParticle->SetLightFlag( LIGHT_NONE );
	}

	VectorScale( &forward.x, 4.0f, &temp.x );
	memset( &normal, 0, sizeof( normal ) );
	org = origin + temp;

	if( flashtype == 3 )
	{
		float sizeMG = finalScale * 120.0f;
		pParticle = pParticle->Create( &org, &normal, pMGSprite, sizeMG, 210.0f, "dod_particle", 1 );
	}
	else
	{
		float sizeNormal = finalScale * 60.0f;
		pParticle = pParticle->Create( &org, &normal, pSprite, sizeNormal, 255.0f, "dod_particle", 1 );

		if( pParticle )
			pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
	}

	if( pParticle )
	{
		pParticle->m_vVelocity = { 0.0f, 0.0f, 0.0f };
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 0.0001f;
		pParticle->m_iRendermode = kRenderTransAdd;
		pParticle->m_vColor = { 255.0f, 255.0f, 255.0f };
		pParticle->SetCullFlag( CULL_FRUSTUM_SPHERE | CULL_PVS );
		pParticle->SetLightFlag( LIGHT_NONE );
	}
}

/*
=========================
Event_MuzzleSmoke

=========================
*/
void DLLEXPORT Event_MuzzleSmoke( const struct cl_entity_s *entity, int options )
{
	vec3_t forward, right, up;
	vec3_t org, normal, temp;

	static char *sprname = "sprites/bazookapuff.spr";

	float scale;
	model_s *pSprite;
	CDoDParticle *pParticle;

	AngleVectors( entity->angles, forward, right, up );

	scale = ( float ) ( options / 10 ) * 0.1f;
	float finalScale = ( scale != 0.0f ) ? scale : 0.5f;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( sprname );
	pSprite = (model_s *)gEngfuncs.GetSpritePointer( hSprite );

	if( !pSprite )
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", sprname );
		return;
	}

	forward.z = -forward.z;

	float size = finalScale * 100.0f;

	for( int i = 1; i < 3; i++ )
	{
		org = entity->attachment[0];
		memset( &normal, 0, sizeof( normal ) );

		pParticle = pParticle->Create( &org, &normal, pSprite, size, 20.0f, "dod_particle", 1 );

		if( pParticle )
		{
			float speedMultiplier = ( float ) i * 100.0f;

			VectorScale( &forward.x, speedMultiplier, &pParticle->m_vVelocity.x );

			pParticle->m_iRendermode = kRenderTransAdd;
			pParticle->m_flDampingTime = 0.3f;
			pParticle->m_flDampingVelocity = 0.5f;
			pParticle->m_flFadeSpeed = 2.0f;
			pParticle->m_flScaleSpeed = 6.0f;
			pParticle->m_iFrame = 0;
			pParticle->m_iFramerate = 15;
			pParticle->m_vColor = { 200.0f, 200.0f, 200.0f };
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 5.0f;
			pParticle->SetCollisionFlags( TRI_COLLIDEKILL_ANIM | TRI_COLLIDEDAMP | TRI_COLLIDESLIDE );
			pParticle->SetCullFlag( CULL_FRUSTUM_SPHERE | CULL_PVS );
			pParticle->SetLightFlag( LIGHT_NONE );
			pParticle->AddGlobalWind();
		}
	}
}

/*
=========================
Event_BazookaSmoke

=========================
*/
void DLLEXPORT Event_BazookaSmoke( const struct cl_entity_s *entity )
{
	vec3_t org, normal;
	model_s *pSprite;
	float flScale;
	CDoDRocketTrail *pSmoke;


	pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/bazooka_smoke.spr" ) );

	if( pSprite )
	{
		flScale = gEngfuncs.pfnRandomFloat( 40.0f, 55.0f );

		normal[0] = 0.0f; normal[1] = 0.0f; normal[2] = 1.0f;

		for( int i = 0; i > 2; i++ )
		{
			org[0] = entity->attachment[i].x;
			org[1] = entity->attachment[i].y;
			org[2] = entity->attachment[i].z;

			pSmoke = pSmoke->Create( &org, &normal, pSprite, flScale, 200.0f, "dod_trailsmoke" );

			if( pSmoke )
			{
				pSmoke->SetCollisionFlags( TRI_WATERTRACE );

				pSmoke->m_flGravity = -0.01f;
				pSmoke->m_flSize = flScale;
				pSmoke->m_iRendermode = kRenderTransAlpha;

				pSmoke->m_vAVelocity[0] = 0.0f;
				pSmoke->m_vAVelocity[1] = 0.0f;
				pSmoke->m_vAVelocity[2] = gEngfuncs.pfnRandomFloat( -1.5f, 1.5f );

				pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );
				pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
				pSmoke->m_flScaleSpeed = 2.0f;
				pSmoke->m_vColor = { 25.0f, 25.0f, 25.0f };
				pSmoke->m_iFrame = gEngfuncs.pfnRandomFloat( 10.0f, 15.0f );
				pSmoke->m_bRocketTrail = true;
				pSmoke->m_flFadeSpeed = -1.0f;
				pSmoke->m_vVelocity = { 0.0f, 0.0f, 0.0f };

				pSmoke->SetCullFlag( CULL_FRUSTUM_SPHERE | CULL_PVS );
				pSmoke->SetLightFlag( LIGHT_NONE );
			}
		}
	}
}

/*
=========================
Event_PIATSmoke

=========================
*/
void DLLEXPORT Event_PIATSmoke( const struct cl_entity_s *entity )
{
	vec3_t org, normal;
	model_s *pSprite;
	float flScale;
	CDoDRocketTrail *pSmoke;

	pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/bazooka_smoke.spr" ) );

	if( pSprite )
	{
		flScale = gEngfuncs.pfnRandomFloat( 40.0f, 55.0f );

		normal[0] = 0.0f; 
		normal[1] = 0.0f; 
		normal[2] = 1.0f;

		org[0] = entity->attachment[0].x;
		org[1] = entity->attachment[0].y;
		org[2] = entity->attachment[0].z;

		pSmoke = pSmoke->Create( &org, &normal, pSprite, flScale, 200.0f, "dod_trailsmoke" );

		if( pSmoke )
		{
			pSmoke->SetCollisionFlags( TRI_WATERTRACE );
			pSmoke->m_flGravity = -0.01f;
			pSmoke->m_flSize = flScale;
			pSmoke->m_iRendermode = kRenderTransAlpha;

			pSmoke->m_vAVelocity[0] = 0.0f;
			pSmoke->m_vAVelocity[1] = 0.0f;
			pSmoke->m_vAVelocity[2] = gEngfuncs.pfnRandomFloat( -1.5f, 1.5f );

			pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );
			pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
			pSmoke->m_flScaleSpeed = 2.0f;
			pSmoke->m_vColor = { 25.0f, 25.0f, 25.0f };
			pSmoke->m_iFrame = gEngfuncs.pfnRandomFloat( 10.0f, 15.0f );
			pSmoke->m_bRocketTrail = true;
			pSmoke->m_flFadeSpeed = -1.0f;
			pSmoke->m_vVelocity = { 0.0f, 0.0f, 0.0f };

			pSmoke->SetCullFlag( CULL_FRUSTUM_SPHERE | CULL_PVS );
			pSmoke->SetLightFlag( LIGHT_NONE );
		}
	}
}

/*
=========================
HUD_StudioEvent

The entity's studio model description indicated an event was
fired during this frame, handle the event by it's tag ( e.g., muzzleflash, sound )
=========================
*/
extern pmodel_fx_t g_PModelFxInfo[65];

void DLLEXPORT HUD_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity )
{
	switch( event->event )
	{
	case 5001:
		gEngfuncs.pEfxAPI->R_MuzzleFlash( (float *)&entity->attachment[0], atoi( event->options ) );
		break;
	case 5011:
		gEngfuncs.pEfxAPI->R_MuzzleFlash( (float *)&entity->attachment[1], atoi( event->options ) );
		break;
	case 5021:
		gEngfuncs.pEfxAPI->R_MuzzleFlash( (float *)&entity->attachment[2], atoi( event->options ) );
		break;
	case 5031:
		gEngfuncs.pEfxAPI->R_MuzzleFlash( (float *)&entity->attachment[3], atoi( event->options ) );
		break;
	case 5002:
		gEngfuncs.pEfxAPI->R_SparkEffect( (float *)&entity->attachment[0], atoi( event->options ), -100, 100 );
		break;
	// Client side sound
	case 5004:		
		gEngfuncs.pfnPlaySoundByNameAtLocation( (char *)event->options, 1.0, (float *)&entity->attachment[0] );
		break;
	case 6001:
		Event_EjectBrassP( entity, atoi( event->options ) );
		break;
	case 6002:
		Event_EjectBrassV( entity, atoi( event->options ) );
		break;
	case 7000:
		if( entity->player )
		{
			int options = entity->index;
			g_PModelFxInfo[options].bSwitch = 1;
			g_PModelFxInfo[options].iSwitchSeq = entity->curstate.sequence;
			g_PModelFxInfo[options].iSwitchFrame = atoi( event->options ) ? atoi( event->options ) : -1;
		}
		break;
	case 7001:
		Event_BazookaSmoke( entity );
		break;
	case 7002:
		if( entity->player )
		{
			int options = entity->index;
			g_PModelFxInfo[options].bAnim = 1;
			g_PModelFxInfo[options].iAnimSeq = entity->curstate.sequence;
			g_PModelFxInfo[options].iAnimFrame = event->frame;
			g_PModelFxInfo[options].iAnimTargetSeq = atoi( event->options ) ? atoi( event->options ) : 1;
		}
		break;
	case 7003:
		Event_PIATSmoke( entity );
		break;
	default:
		break;
	}
}

/*
=================
CL_UpdateTEnts

Simulation and cleanup of temporary entities
=================
*/
void DLLEXPORT HUD_TempEntUpdate(
	double frametime,   // Simulation time
	double client_time, // Absolute time on client
	double cl_gravity,  // True gravity on client
	TEMPENTITY **ppTempEntFree,   // List of freed temporary ents
	TEMPENTITY **ppTempEntActive, // List 
	int		( *Callback_AddVisibleEntity )( cl_entity_t *pEntity ),
	void	( *Callback_TempEntPlaySound )( TEMPENTITY *pTemp, float damp ) )
{
	static int gTempEntFrame = 0;
	int			i;
	TEMPENTITY *pTemp, *pnext, *pprev;
	float		/*freq,*/ gravity, gravitySlow, life, fastFreq;

	g_flGravity = cl_gravity;

	vec3_t vAngles;
	gEngfuncs.GetViewAngles( ( float * ) vAngles );

	if( g_pParticleMan )
		g_pParticleMan->SetVariables( cl_gravity, vAngles );

	// Nothing to simulate
	if( !*ppTempEntActive )
		return;

	// in order to have tents collide with players, we have to run the player prediction code so
	// that the client has the player list. We run this code once when we detect any COLLIDEALL 
	// tent, then set this BOOL to true so the code doesn't get run again if there's more than
	// one COLLIDEALL ent for this update. (often are).
	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers( -1 );

	// !!!BUGBUG	-- This needs to be time based
	gTempEntFrame = ( gTempEntFrame + 1 ) & 31;

	pTemp = *ppTempEntActive;

	// !!! Don't simulate while paused....  This is sort of a hack, revisit.
	if( frametime <= 0 )
	{
		while( pTemp )
		{
			if( !( pTemp->flags & FTENT_NOMODEL ) )
			{
				Callback_AddVisibleEntity( &pTemp->entity );
			}
			pTemp = pTemp->next;
		}
		goto finish;
	}

	pprev = NULL;
	fastFreq = client_time * 5.5;
	gravity = -frametime * cl_gravity;
	gravitySlow = gravity * 0.5f;

	while( pTemp )
	{
		int active = 1;
		pnext = pTemp->next;

		if( !( pTemp->flags & FTENT_INFINITE ) )
		{
			life = pTemp->die - ( float ) client_time;

			if( life < 0 )
			{
				if( pTemp->flags & FTENT_FADEOUT )
				{
					if( pTemp->entity.curstate.rendermode == kRenderNormal )
						pTemp->entity.curstate.rendermode = kRenderTransTexture;
					pTemp->entity.curstate.renderamt = pTemp->entity.baseline.renderamt * ( 1 + life * pTemp->fadeSpeed );
					if( pTemp->entity.curstate.renderamt <= 0 )
						active = 0;
				}
				else
					active = 0;
			}

		}

		if( !active )		// Kill it
		{
			pTemp->next = *ppTempEntFree;
			*ppTempEntFree = pTemp;
			if( !pprev )	// Deleting at head of list
				*ppTempEntActive = pnext;
			else
				pprev->next = pnext;
		}
		else
		{
			pprev = pTemp;

			VectorCopy( pTemp->entity.origin, pTemp->entity.prevstate.origin );

			if( pTemp->flags & FTENT_SPARKSHOWER )
			{
				// Adjust speed if it's time
				// Scale is next think time
				if( client_time > pTemp->entity.baseline.scale )
				{
					// Show Sparks
					gEngfuncs.pEfxAPI->R_SparkEffect( pTemp->entity.origin, 8, -200, 200 );

					// Reduce life
					pTemp->entity.baseline.framerate -= 0.1f;

					if( pTemp->entity.baseline.framerate <= 0.0f )
					{
						pTemp->die = client_time;
					}
					else
					{
						// So it will die no matter what
						pTemp->die = client_time + 0.5;

						// Next think
						pTemp->entity.baseline.scale = client_time + 0.1;
					}
				}
			}
			else if( pTemp->flags & FTENT_PLYRATTACHMENT )
			{

				cl_entity_t *pClient = gEngfuncs.GetEntityByIndex( pTemp->clientIndex );
				int hitSound = pTemp->hitSound;

				if( hitSound != 0 )
				{
					VectorCopy( &pClient->angles[3 * hitSound], pTemp->entity.origin );
				}
				else
				{
					VectorAdd( pClient->origin, pTemp->tentOffset, pTemp->entity.origin );
				}
			}
			else if( pTemp->flags & FTENT_SINEWAVE )
			{
				pTemp->x += pTemp->entity.baseline.origin[0] * ( float ) frametime;
				pTemp->y += pTemp->entity.baseline.origin[1] * ( float ) frametime;

				pTemp->entity.origin[0] = pTemp->x + sin( pTemp->entity.baseline.origin[2] + client_time * pTemp->entity.prevstate.frame ) * ( 10 * pTemp->entity.curstate.framerate );
				pTemp->entity.origin[1] = pTemp->y + sin( pTemp->entity.baseline.origin[2] + fastFreq + 0.7f ) * ( 8 * pTemp->entity.curstate.framerate );
				pTemp->entity.origin[2] += pTemp->entity.baseline.origin[2] * frametime;
			}
			else if( pTemp->flags & FTENT_SPIRAL )
			{
				pTemp->entity.origin[0] += pTemp->entity.baseline.origin[0] * ( float ) frametime + 8 * sin( client_time * 20 + ( size_t ) pTemp );
				pTemp->entity.origin[1] += pTemp->entity.baseline.origin[1] * ( float ) frametime + 4 * sin( client_time * 30 + ( size_t ) pTemp );
				pTemp->entity.origin[2] += pTemp->entity.baseline.origin[2] * ( float ) frametime;
			}
			else
			{
				for( i = 0; i < 3; i++ )
					pTemp->entity.origin[i] += pTemp->entity.baseline.origin[i] * ( float ) frametime;
			}

			if( pTemp->flags & FTENT_SPRANIMATE )
			{
				pTemp->entity.curstate.frame += ( float ) frametime * pTemp->entity.curstate.framerate;
				if( pTemp->entity.curstate.frame >= pTemp->frameMax )
				{
					pTemp->entity.curstate.frame = pTemp->entity.curstate.frame - ( int ) ( pTemp->entity.curstate.frame );

					if( !( pTemp->flags & FTENT_SPRANIMATELOOP ) )
					{
						// this animating sprite isn't set to loop, so destroy it.
						pTemp->die = client_time;
						pTemp = pnext;
						continue;
					}
				}
			}
			else if( pTemp->flags & FTENT_SPRCYCLE )
			{
				pTemp->entity.curstate.frame += frametime * 10;
				if( pTemp->entity.curstate.frame >= pTemp->frameMax )
				{
					pTemp->entity.curstate.frame = pTemp->entity.curstate.frame - ( int ) ( pTemp->entity.curstate.frame );
				}
			}

			if( pTemp->flags & FTENT_ROTATE )
			{
				pTemp->entity.angles[0] += pTemp->entity.baseline.angles[0] * ( float ) frametime;
				pTemp->entity.angles[1] += pTemp->entity.baseline.angles[1] * ( float ) frametime;
				pTemp->entity.angles[2] += pTemp->entity.baseline.angles[2] * ( float ) frametime;

				VectorCopy( pTemp->entity.angles, pTemp->entity.latched.prevangles );
			}

			if( pTemp->flags & ( FTENT_COLLIDEALL | FTENT_COLLIDEWORLD ) && !( pTemp->flags & FTENT_BODYSTATIC ) )
			{
				vec3_t	traceNormal( 0.0f, 0.0f, 0.0f );
				float	traceFraction = 1;

				if( pTemp->flags & FTENT_COLLIDEALL )
				{
					pmtrace_t pmtrace;
					physent_t *pe;

					gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );

					gEngfuncs.pEventAPI->EV_PlayerTrace( pTemp->entity.prevstate.origin, pTemp->entity.origin, PM_STUDIO_BOX, -1, &pmtrace );

					if( pmtrace.fraction != 1 )
					{
						pe = gEngfuncs.pEventAPI->EV_GetPhysent( pmtrace.ent );

						if( !pmtrace.ent || ( pe->info != pTemp->clientIndex ) )
						{
							traceFraction = pmtrace.fraction;
							VectorCopy( pmtrace.plane.normal, traceNormal );

							if( pTemp->hitcallback )
							{
								( *pTemp->hitcallback )( pTemp, &pmtrace );
							}
						}
					}
				}
				else if( pTemp->flags & FTENT_COLLIDEWORLD )
				{
					pmtrace_t pmtrace;

					gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );

					if( pTemp->flags & FTENT_SPRANIMATELOOP )
					{
						vec3_t vVel, vEndPos;
						VectorCopy( pTemp->entity.baseline.origin, vVel );
						VectorNormalize( vVel );
						VectorMA( pTemp->entity.prevstate.origin, 36.0f, vVel, vEndPos );

						gEngfuncs.pEventAPI->EV_PlayerTrace( pTemp->entity.prevstate.origin, vEndPos, PM_WORLD_ONLY, -1, &pmtrace );
					}
					else
					{
						gEngfuncs.pEventAPI->EV_PlayerTrace( pTemp->entity.prevstate.origin, pTemp->entity.origin, PM_WORLD_ONLY, -1, &pmtrace );
					}

					if( pmtrace.fraction != 1 )
					{
						traceFraction = pmtrace.fraction;
						VectorCopy( pmtrace.plane.normal, traceNormal );

						if( pTemp->flags & FTENT_SPARKSHOWER )
						{
							// Chop spark speeds a bit more
							//
							VectorScale( pTemp->entity.baseline.origin, 0.6f, pTemp->entity.baseline.origin );

							if( Length( pTemp->entity.baseline.origin ) < 10 )
							{
								pTemp->entity.baseline.framerate = 0.0;
							}
						}

						if( pTemp->hitcallback )
						{
							( *pTemp->hitcallback )( pTemp, &pmtrace );
						}
					}
				}

				if( traceFraction != 1 )	// Decent collision now, and damping works
				{
					float  proj, damp;

					// Place at contact point
					VectorMA( pTemp->entity.prevstate.origin, traceFraction * ( float ) frametime, pTemp->entity.baseline.origin, pTemp->entity.origin );
					// Damp velocity
					damp = pTemp->bounceFactor;
					if( pTemp->flags & ( FTENT_GRAVITY | FTENT_SLOWGRAVITY ) )
					{
						damp *= 0.5f;
						if( traceNormal[2] > 0.9f )		// Hit floor?
						{
							if( pTemp->entity.baseline.origin[2] <= 0 && pTemp->entity.baseline.origin[2] >= gravity * 3 )
							{
								damp = 0;		// Stop
								pTemp->flags &= ~( FTENT_ROTATE | FTENT_GRAVITY | FTENT_SLOWGRAVITY | FTENT_COLLIDEWORLD | FTENT_SMOKETRAIL );
								pTemp->entity.angles[0] = 0;
								pTemp->entity.angles[2] = 0;
							}
						}
					}

					if( pTemp->hitSound )
					{
						Callback_TempEntPlaySound( pTemp, damp );
					}

					if( pTemp->flags & FTENT_COLLIDEKILL )
					{
						// die on impact
						pTemp->flags &= ~FTENT_FADEOUT;
						pTemp->die = client_time;
					}
					else
					{
						// Reflect velocity
						if( damp != 0 )
						{
							proj = DotProduct( pTemp->entity.baseline.origin, traceNormal );
							VectorMA( pTemp->entity.baseline.origin, -proj * 2, traceNormal, pTemp->entity.baseline.origin );
							// Reflect rotation (fake)

							pTemp->entity.angles[1] = -pTemp->entity.angles[1];
						}

						if( damp != 1 )
						{

							VectorScale( pTemp->entity.baseline.origin, damp, pTemp->entity.baseline.origin );
							VectorScale( pTemp->entity.angles, 0.9, pTemp->entity.angles );
						}
					}
				}
			}

			if( ( pTemp->flags & FTENT_FLICKER ) && gTempEntFrame == pTemp->entity.curstate.effects )
			{
				dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );
				VectorCopy( pTemp->entity.origin, dl->origin );
				dl->radius = 60;
				dl->color.r = 255;
				dl->color.g = 120;
				dl->color.b = 0;
				dl->die = client_time + 0.01;
			}

			if( pTemp->flags & FTENT_SMOKETRAIL )
			{
				gEngfuncs.pEfxAPI->R_RocketTrail( pTemp->entity.prevstate.origin, pTemp->entity.origin, 1 );
			}

			if( !( pTemp->flags & FTENT_BODYSTATIC ) )
			{
				if( pTemp->flags & FTENT_GRAVITY )
					pTemp->entity.baseline.origin[2] += gravity;
				else if( pTemp->flags & FTENT_SLOWGRAVITY )
					pTemp->entity.baseline.origin[2] += gravitySlow;
				else if( pTemp->flags & FTENT_CUSTOMGRAVITY )
					pTemp->entity.baseline.origin[2] += gravity * pTemp->entity.baseline.gravity;

				if( pTemp->flags & FTENT_CLIENTCUSTOM )
				{
					if( pTemp->callback )
					{
						( *pTemp->callback )( pTemp, frametime, client_time );
					}
				}

				if( pTemp->flags & FTENT_INFINITE )
				{
					pTemp->entity.model->mins = { -999.0f, -999.0f, -999.0f };
					pTemp->entity.model->maxs = { 999.0f, 999.0f, 999.0f };
					Callback_AddVisibleEntity( &pTemp->entity );
				}

				// Cull to PVS (not frustum cull, just PVS)
				if( !( pTemp->flags & FTENT_NOMODEL ) )
				{
					if( !Callback_AddVisibleEntity( &pTemp->entity ) )
					{
						if( !( pTemp->flags & FTENT_PERSIST ) )
						{
							pTemp->die = client_time;			// If we can't draw it this frame, just dump it.
							pTemp->flags &= ~FTENT_FADEOUT;	// Don't fade out, just die
						}
					}
				}
			}
			pTemp = pnext;
		}
	}
finish:
	// Restore state info
	gEngfuncs.pEventAPI->EV_PopPMStates();
}

/*
=================
HUD_GetUserEntity

If you specify negative numbers for beam start and end point entities, then
  the engine will call back into this function requesting a pointer to a cl_entity_t 
  object that describes the entity to attach the beam onto.

Indices must start at 1, not zero.
=================
*/
cl_entity_t DLLEXPORT *HUD_GetUserEntity( int index )
{
	return NULL;
}
