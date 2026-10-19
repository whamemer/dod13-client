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
//
// dod_camera.cpp
//
// the entities of the map that the client parses by itself (cameras, path_corners ...) and the cutscene camera settings
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "screenfade.h"
#include "vgui/idodviewport.h"

#include "dod_shared.h"

#include "dmc_bspfile.h"
#include "dod_camera.h"

#include "tri.h"

extern int g_iWeatherType;

extern float g_lastFOV;

static char *ParseSingleEnt( char *buffer, dod_bspparse_ent *pEnt, int *error );
static void DoD_SetKVPair( dod_bspparse_ent *pEnt, const char *key, const char *value );
static void Parse_dod_camera( dod_bspparse_ent *pEnt );
static void Parse_path_corner( dod_bspparse_ent *pEnt );
static void Parse_particle_shooter( dod_bspparse_ent *pEnt );
static void Parse_info_doddetect( dod_bspparse_ent *pEnt );
static void Parse_env_model( dod_bspparse_ent *pEnt );

static dod_camera_t s_cameras[256];
static int s_num_cameras;

void DoD_ProcessEnts( char *buffer )
{
	char token[1024];
	int error;
	dod_bspparse_ent entKVPairs;

	s_num_cameras = 0;

	gHUD.m_CEnvModel.RemoveAllModels();

	while( 1 )
	{
		buffer = gEngfuncs.COM_ParseFile( buffer, token );

		if( !buffer )
			break;

		if( token[0] != '{' )
		{
			gEngfuncs.Con_Printf( "DoD_ProcessEnts: found %s when expecting {\n", token );
			break;
		}

		error = 0;

		buffer = ParseSingleEnt( buffer, &entKVPairs, &error );

		if( error )
		{
			gEngfuncs.Con_Printf( "DoD_ProcessEnts: error parsing entities\n" );
			break;
		}

		if( !stricmp( entKVPairs.classname, "dod_camera" ) )
		{
			Parse_dod_camera( &entKVPairs );
		}
		else if( !stricmp( entKVPairs.classname, "path_corner" ) )
		{
			Parse_path_corner( &entKVPairs );
		}
		else if( !stricmp( entKVPairs.classname, "particle_shooter" ) )
		{
			Parse_particle_shooter( &entKVPairs );
		}
		else if( !stricmp( entKVPairs.classname, "info_doddetect" ) )
		{
			Parse_info_doddetect( &entKVPairs );
		}
		else if( !stricmp( entKVPairs.classname, "env_model" ) )
		{
			Parse_env_model( &entKVPairs );
		}
	}
}

static void Parse_dod_camera( dod_bspparse_ent *pEnt )
{
	int i;
	char *key;
	char *value;
	dod_camera_t *pCamera;

	if( s_num_cameras > 255 )
		return;

	pCamera = &s_cameras[s_num_cameras++];

	pCamera->type = DOD_CAMERA;

	strcpy( pCamera->classname, pEnt->classname );

	for( i = 0; i < pEnt->numpairs; i++ )
	{
		key = pEnt->kvPairs[i].szKeyName;
		value = pEnt->kvPairs[i].szValue;

		if( !stricmp( key, "target" ) )
		{
			strcpy( pCamera->target, value );
		}
		else if( !stricmp( key, "targetname" ) )
		{
			strcpy( pCamera->targetname, value );
		}
		else if( !stricmp( key, "cam_overlay" ) )
		{
			pCamera->overlay = atoi( value );
		}
		else if( !stricmp( key, "fov" ) )
		{
			pCamera->fov = atoi( value );
		}
		else if( !stricmp( key, "subtitle" ) )
		{
			pCamera->subtitle = atoi( value );
		}
		else if( !stricmp( key, "cam_fade" ) )
		{
			pCamera->fade = atoi( value );
		}
	}
}

static void Parse_path_corner( dod_bspparse_ent *pEnt )
{
	int i;
	char *key;
	char *value;
	dod_camera_t *pCamera;

	if( s_num_cameras > 255 )
		return;

	pCamera = &s_cameras[s_num_cameras++];

	pCamera->type = DOD_TARGET;

	strcpy( pCamera->classname, pEnt->classname );

	for( i = 0; i < pEnt->numpairs; i++ )
	{
		key = pEnt->kvPairs[i].szKeyName;
		value = pEnt->kvPairs[i].szValue;

		if( !stricmp( key, "target" ) )
		{
			strcpy( pCamera->target, value );
		}
		else if( !stricmp( key, "targetname" ) )
		{
			strcpy( pCamera->targetname, value );
		}
		else if( !stricmp( key, "cam_overlay" ) )
		{
			pCamera->overlay = atoi( value );
		}
		else if( !stricmp( key, "fov" ) )
		{
			pCamera->fov = atoi( value );
		}
		else if( !stricmp( key, "subtitle" ) )
		{
			pCamera->subtitle = atoi( value );
		}
		else if( !stricmp( key, "cam_fade" ) )
		{
			pCamera->fade = atoi( value );
		}
	}
}

static void Parse_particle_shooter( dod_bspparse_ent *pEnt )
{
	int i;
	char *key;
	char *value;
	particle_shooter_t shooter;

	memset( &shooter, 0, sizeof( shooter ) );

	for( i = 0; i < pEnt->numpairs; i++ )
	{
		key = pEnt->kvPairs[i].szKeyName;
		value = pEnt->kvPairs[i].szValue;

		if( !stricmp( key, "group_id" ) )
		{
			shooter.id = atoi( value );
		}
		else if( !stricmp( key, "num_particles" ) )
		{
			shooter.iNumParticles = atoi( value );
		}
		else if( !stricmp( key, "fire_delay" ) )
		{
			shooter.fFireDelay = (float)atof( value );
		}
		else if( !stricmp( key, "particle_life" ) )
		{
			shooter.fParticleLife = (float)atof( value );
		}
		else if( !stricmp( key, "spawnflags" ) )
		{
			shooter.iFlags = atoi( value );
		}
		else if( !stricmp( key, "origin" ) )
		{
			float x, y, z;

			sscanf( value, "%f %f %f\n", &x, &y, &z );

			shooter.vOrigin = Vector( x, y, z );
		}
		else if( !stricmp( key, "model" ) )
		{
			strncpy( shooter.szSprite, value, sizeof( shooter.szSprite ) );
		}
		else if( !stricmp( key, "colour_red" ) )
		{
			shooter.iColour[0] = atoi( value );
		}
		else if( !stricmp( key, "colour_green" ) )
		{
			shooter.iColour[1] = atoi( value );
		}
		else if( !stricmp( key, "colour_blue" ) )
		{
			shooter.iColour[2] = atoi( value );
		}
		else if( !stricmp( key, "size" ) )
		{
			shooter.fSize = (float)atof( value );
		}
		else if( !stricmp( key, "gravity" ) )
		{
			shooter.fGravity = (float)atof( value );
		}
		else if( !stricmp( key, "fade_speed" ) )
		{
			shooter.fFadeSpeed = (float)atof( value );
		}
		else if( !stricmp( key, "spin_speed" ) )
		{
			shooter.iSpinDegPerSec = atoi( value );
		}
		else if( !stricmp( key, "damping_time" ) || !stricmp( key, "damping_vel" ) )
		{
			// the original puts damping_vel in the damping time too
			shooter.fDampingTime = (float)atof( value );
		}
		else if( !stricmp( key, "vel_variance" ) )
		{
			shooter.fVariance = (float)atof( value );
		}
		else if( !stricmp( key, "brightness" ) )
		{
			shooter.fBrightness = (float)atof( value );
		}
		else if( !stricmp( key, "scale_speed" ) )
		{
			shooter.fScaleSpeed = (float)atof( value );
		}
		else if( !stricmp( key, "framerate" ) )
		{
			shooter.iFramerate = (int)atof( value );
		}
		else if( !stricmp( key, "rendermode" ) )
		{
			shooter.iRenderMode = atoi( value );
		}
	}

	if( shooter.fFireDelay < 0.1f )
		shooter.fFireDelay = 0.1f;

	gHUD.m_PShooter.AddParticleSystem( &shooter );
}

// the wind, the weather and the country of the allies
static void Parse_info_doddetect( dod_bspparse_ent *pEnt )
{
	int i;
	char *key;
	char *value;
	vec3_t vWind;

	vWind = Vector( 0.0f, 0.0f, 0.0f );

	for( i = 0; i < pEnt->numpairs; i++ )
	{
		key = pEnt->kvPairs[i].szKeyName;
		value = pEnt->kvPairs[i].szValue;

		if( !stricmp( key, "detect_wind_velocity_x" ) )
		{
			vWind.x = (float)atof( value );
		}
		else if( !stricmp( key, "detect_wind_velocity_y" ) )
		{
			vWind.y = (float)atof( value );
		}
		else if( !stricmp( key, "detect_weather_type" ) )
		{
			g_iWeatherType = atoi( value );
		}
		else if( !stricmp( key, "detect_allies_country" ) )
		{
			gHUD.m_bBritish = ( atoi( value ) == 1 );
		}
	}

	CDoDParticle::SetGlobalWind( vWind );
}

// the static env_models are drawn by the client only
static void Parse_env_model( dod_bspparse_ent *pEnt )
{
	int i;
	char *key;
	char *value;
	env_model_t model;

	memset( &model, 0, sizeof( model ) );

	for( i = 0; i < pEnt->numpairs; i++ )
	{
		key = pEnt->kvPairs[i].szKeyName;
		value = pEnt->kvPairs[i].szValue;

		// the targeted env_models are created by the server
		if( !stricmp( key, "targetname" ) )
			return;

		if( !stricmp( key, "spawnflags" ) )
		{
			model.spawnflags |= strtol( value, NULL, 10 );

			// initially off
			if( model.spawnflags & SF_ENVMODEL_OFF )
				return;
		}
		else if( !stricmp( key, "model" ) )
		{
			strncpy( model.szModel, value, sizeof( model.szModel ) );
		}
		else if( !stricmp( key, "body" ) )
		{
			model.iBody = strtol( value, NULL, 10 );
		}
		else if( !stricmp( key, "origin" ) )
		{
			sscanf( value, "%f %f %f\n", &model.vecOrigin.x, &model.vecOrigin.y, &model.vecOrigin.z );
		}
		else if( !stricmp( key, "angles" ) )
		{
			sscanf( value, "%f %f %f\n", &model.vecAngles.x, &model.vecAngles.y, &model.vecAngles.z );
		}
		else if( !stricmp( key, "m_iAction_On" ) )
		{
			if( strtol( value, NULL, 10 ) > 0 )
				model.spawnflags |= SF_ENVMODEL_ANIMATION;
		}
		else if( !stricmp( key, "m_iszSequence_On" ) )
		{
			strncpy( model.szSequence, value, sizeof( model.szSequence ) );
		}
	}

	gHUD.m_CEnvModel.AddEnvModel( &model );
}

// the entity lump of the bsp as a text, the caller has to free it
char *DoD_LoadEntityLump( char *filename )
{
	dheader_t header;
	int size;
	lump_t *curLump;
	char *buffer;
	byte *pBSPData;

	pBSPData = gEngfuncs.COM_LoadFile( filename, 5, 0 );

	if( !pBSPData )
		return NULL;

	header = *(dheader_t *)pBSPData;

	if( header.version != 29 && header.version != 30 )
	{
		gEngfuncs.Con_Printf( "DoD_LoadEntityLump:  Map [%s] has incorrect BSP version (%i should be %i).\n", filename, header.version, 30 );

		// The BSP data is not needed after the version check
		gEngfuncs.COM_FreeFile( pBSPData );
		return NULL;
	}

	curLump = &header.lumps[0];
	size = curLump->filelen;

	buffer = (char *)malloc( size + 1 );

	if( !buffer )
	{
		gEngfuncs.Con_Printf( "DoD_LoadEntityLump:  Couldn't allocate %i bytes\n", size + 1 );

		gEngfuncs.COM_FreeFile( pBSPData );
		return NULL;
	}

	memcpy( buffer, pBSPData + curLump->fileofs, size );
	buffer[size] = 0;

	gEngfuncs.COM_FreeFile( pBSPData );

	return buffer;
}

void DoD_LoadClientEnts( const char *map )
{
	char *buffer;
	char filename[260];

	strcpy( filename, map );

	buffer = DoD_LoadEntityLump( filename );

	if( buffer )
	{
		DoD_ProcessEnts( buffer );
		free( buffer );
	}
	else
	{
		gEngfuncs.Con_DPrintf( "ERROR - Couldn't load bsp to parse client ents - %s - Check working folder is set properly\n", filename );
	}
}

dod_camera_t *DoD_FindTarget( const char *name )
{
	int i;
	dod_camera_t *target;

	for( i = 0; i < 256; i++ )
	{
		target = &s_cameras[i];

		if( !stricmp( target->targetname, name ) )
			return target;
	}

	return NULL;
}

void DoD_ReadCameraSettings( const char *Targetname )
{
	dod_camera_t *target;
	int def_fov;

	target = DoD_FindTarget( Targetname );

	if( !target )
		return;

	// the widescreen bars belong to the VGUI2 viewport, there is none here
	switch( target->overlay )
	{
	case 1:
		gHUD.m_Scope.SetScope( WEAPON_SPRING );
		gHUD.m_Scope.m_iFlags |= HUD_ACTIVE;

		if( gDoDViewPortInterface )
			gDoDViewPortInterface->setWidescreenVisible( false );
		break;
	case 2:
		gHUD.m_Scope.SetScope( WEAPON_BINOC );
		gHUD.m_Scope.m_iFlags |= HUD_ACTIVE;

		if( gDoDViewPortInterface )
			gDoDViewPortInterface->setWidescreenVisible( false );
		break;
	case 3:
		gHUD.m_Scope.SetScope( 0 );

		if( gDoDViewPortInterface )
			gDoDViewPortInterface->setWidescreenVisible( true );
		break;
	default:
		gHUD.m_Scope.SetScope( 0 );

		if( gDoDViewPortInterface )
			gDoDViewPortInterface->setWidescreenVisible( false );
		break;
	}

	def_fov = target->fov;

	g_lastFOV = (float)def_fov;

	if( !def_fov )
		def_fov = 90;

	gHUD.SetFOV( def_fov );

	if( target->subtitle && gDoDViewPortInterface )
	{
		gDoDViewPortInterface->setWidescreenVisible( true );
		gDoDViewPortInterface->setWidescreenSubtitle( target->subtitle );
	}
}

void DoD_ScreenFades( int flags, float holdtime, int r, int g, int b, int a, int speed )
{
	screenfade_t sf;
	float endtime;

	gEngfuncs.pfnGetScreenFade( &sf );

	sf.fader = r;
	sf.fadeg = g;
	sf.fadeb = b;
	sf.fadeSpeed = (float)speed;
	sf.fadealpha = a;

	endtime = gEngfuncs.GetClientTime() + holdtime;

	sf.fadeFlags = flags;
	sf.fadeReset = endtime;
	sf.fadeTotalEnd = endtime;
	sf.fadeEnd = endtime;

	gEngfuncs.pfnSetScreenFade( &sf );
}

static char *ParseSingleEnt( char *buffer, dod_bspparse_ent *pEnt, int *error )
{
	char key[256];
	char token[1024];
	int n;

	memset( pEnt, 0, sizeof( dod_bspparse_ent ) );

	while( 1 )
	{
		buffer = gEngfuncs.COM_ParseFile( buffer, token );

		if( token[0] == '}' )
			return buffer;

		if( !buffer )
		{
			*error = 1;
			return buffer;
		}

		strcpy( key, token );

		// remove trailing spaces
		n = strlen( key );

		while( n && key[n - 1] == ' ' )
		{
			key[n - 1] = 0;
			n--;
		}

		buffer = gEngfuncs.COM_ParseFile( buffer, token );

		if( !buffer || token[0] == '}' )
		{
			*error = 1;
			return buffer;
		}

		DoD_SetKVPair( pEnt, key, token );
	}
}

static void DoD_SetKVPair( dod_bspparse_ent *pEnt, const char *key, const char *value )
{
	if( !strcmp( key, "classname" ) )
	{
		strcpy( pEnt->classname, value );
	}
	else if( pEnt->numpairs <= 127 )
	{
		strncpy( pEnt->kvPairs[pEnt->numpairs].szKeyName, key, sizeof( pEnt->kvPairs[0].szKeyName ) );
		strncpy( pEnt->kvPairs[pEnt->numpairs].szValue, value, sizeof( pEnt->kvPairs[0].szValue ) );
		pEnt->numpairs++;
	}
}
