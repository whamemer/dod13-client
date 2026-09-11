//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "util_vector.h"
#include "DOD_camera.h"
#include "DMC_BSPFile.h"
#include "dod_shared.h"
#include "tri.h"
#include "screenfade.h"

extern float g_lastFOV;

void DoD_ProcessEnts( char *buffer )
{
	dod_bspparse_ent entKVPairs;
	char token;
	int error = 0;

	s_num_cameras = 0;
	gHUD.m_CEnvModel.RemoveAllModels();

	while( true )
	{
		buffer = gEngfuncs.COM_ParseFile( buffer, &token );

		if( !buffer )
			return;

		if( token != '{' )
		{
			gEngfuncs.Con_Printf( "DoD_ProcessEnts: found %s when expecting {\n", token );
			return;
		}

		memset( &entKVPairs, 0, sizeof( entKVPairs ) );

		buffer = ParseSingleEnt( buffer, &entKVPairs, &error );

		if( error || !buffer )
			return;

		if( !strcasecmp( entKVPairs.classname, "dod_camera" ) )
			Parse_dod_camera( &entKVPairs );

		else if( !strcasecmp( entKVPairs.classname, "path_corner" ) )
			Parse_path_corner( &entKVPairs );

		else if( !strcasecmp( entKVPairs.classname, "env_model" ) )
			Parse_env_model( &entKVPairs );

		else if( !strcasecmp( entKVPairs.classname, "info_doddetect" ) )
			Parse_info_doddetect( &entKVPairs );

		else if( !strcasecmp( entKVPairs.classname, "particle_shooter" ) )
			Parse_particle_shooter( &entKVPairs );
	}
}

void Parse_dod_camera( dod_bspparse_ent *pEnt )
{
	dod_camera_t *pCamera;
	char *key;
	char *value;
	int i;

	if( s_num_cameras <= 255 )
	{
		int idx = s_num_cameras++;

		pCamera = &s_cameras[idx];
		pCamera->type = DOD_CAMERA;

		strcpy( pCamera->classname, pEnt->classname );

		if( pEnt->numpairs > 0 )
		{
			for( i = 0; i < pEnt->numpairs; ++i )
			{
				key = pEnt->kvPairs[i].szKeyName;
				value = pEnt->kvPairs[i].szValue;

				if( !strcasecmp( key, "target" ) )
					strcpy( pCamera->target, value );

				else if( !strcasecmp( key, "targetname" ) )
					strcpy( pCamera->targetname, value );

				else if( !strcasecmp( key, "cam_overlay" ) )
					pCamera->overlay = strtol( value, NULL, 10 );

				else if( !strcasecmp( key, "fov" ) )
					pCamera->fov = strtol( value, NULL, 10 );

				else if( !strcasecmp( key, "subtitle" ) )
					pCamera->subtitle = strtol( value, NULL, 10 );

				else if( !strcasecmp( key, "cam_fade" ) )
					pCamera->fade = strtol( value, NULL, 10 );

			}
		}
	}
}

void Parse_path_corner( dod_bspparse_ent *pEnt )
{
	dod_camera_t *pCamera;
	char *key;
	char *value;
	int i;

	if( s_num_cameras <= 255 )
	{
		int idx = s_num_cameras++;

		pCamera = &s_cameras[idx];
		pCamera->type = DOD_TARGET;

		strcpy( pCamera->classname, pEnt->classname );

		if( pEnt->numpairs > 0 )
		{
			for( i = 0; i < pEnt->numpairs; ++i )
			{
				key = pEnt->kvPairs[i].szKeyName;
				value = pEnt->kvPairs[i].szValue;

				if( !strcasecmp( key, "target" ) )
					strcpy( pCamera->target, value );

				else if( !strcasecmp( key, "targetname" ) )
					strcpy( pCamera->targetname, value );

				else if( !strcasecmp( key, "cam_overlay" ) )
					pCamera->overlay = strtol( value, NULL, 10 );

				else if( !strcasecmp( key, "fov" ) )
					pCamera->fov = strtol( value, NULL, 10 );

				else if( !strcasecmp( key, "subtitle" ) )
					pCamera->subtitle = strtol( value, NULL, 10 );

				else if( !strcasecmp( key, "cam_fade" ) )
					pCamera->fade = strtol( value, NULL, 10 );
			}
		}
	}
}

void Parse_particle_shooter( dod_bspparse_ent *pEnt )
{
	particle_shooter_t shooter;
	char *key;
	char *value;
	int i;
	float z;

	memset( &shooter, 0, sizeof( particle_shooter_t ) );

	if( pEnt->numpairs > 0 )
	{
		for( i = 0; i < pEnt->numpairs; ++i )
		{
			key = pEnt->kvPairs[i].szKeyName;
			value = pEnt->kvPairs[i].szValue;

			if( !strcasecmp( key, "group_id" ) )
				shooter.id = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "num_particles" ) )
				shooter.iNumParticles = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "fire_delay" ) )
				shooter.fFireDelay = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "particle_life" ) )
				shooter.fParticleLife = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "spawnflags" ) )
				shooter.iFlags = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "origin" ) )
			{
				sscanf( value, "%f %f %f", &shooter.vOrigin.x, &shooter.vOrigin.y, &z );
				shooter.vOrigin.z = z;
			}
			else if( !strcasecmp( key, "model" ) )
				strncpy( shooter.szSprite, value, 128 );

			else if( !strcasecmp( key, "colour_red" ) )
				shooter.iColour[0] = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "colour_green" ) )
				shooter.iColour[1] = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "colour_blue" ) )
				shooter.iColour[2] = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "size" ) )
				shooter.fSize = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "gravity" ) )
				shooter.fGravity = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "fade_speed" ) )
				shooter.fFadeSpeed = ( float ) strtod( value, NULL );
			
			else if( !strcasecmp( key, "spin_speed" ) )
				shooter.iSpinDegPerSec = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "damping_time" ) || !strcasecmp( key, "damping_vel" ) )
				shooter.fDampingTime = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "vel_variance" ) )
				shooter.fVariance = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "brightness" ) )
				shooter.fBrightness = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "scale_speed" ) )
				shooter.fScaleSpeed = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "framerate" ) )
				shooter.iFramerate = ( int ) strtod( value, NULL );

			else if( !strcasecmp( key, "rendermode" ) )
				shooter.iRenderMode = strtol( value, NULL, 10 );
		}
	}

	if( shooter.fFireDelay < 0.1f )
		shooter.fFireDelay = 0.1f;

	gHUD.m_PShooter.AddParticleSystem( &shooter );
}

extern int g_iWeatherType;

void Parse_info_doddetect( dod_bspparse_ent *pEnt )
{
	vec3_t vWind;
	char *key;
	char *value;
	int i;

	vWind[0] = 0.0f;
	vWind[1] = 0.0f;

	if( pEnt->numpairs > 0 )
	{
		for( i = 0; i < pEnt->numpairs; ++i )
		{
			key = pEnt->kvPairs[i].szKeyName;
			value = pEnt->kvPairs[i].szValue;

			if( !strcasecmp( key, "detect_wind_velocity_x" ) )
				vWind[0] = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "detect_wind_velocity_y" ) )
				vWind[1] = ( float ) strtod( value, NULL );

			else if( !strcasecmp( key, "detect_weather_type" ) )
				g_iWeatherType = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "detect_allies_country" ) )
				gHUD.m_bBritish = ( strtol( value, NULL, 10 ) == 1 );
		}
	}
#ifdef USE_PMAN
	CDoDParticle::SetGlobalWind( &vWind[0] );
#endif
}

void Parse_env_model( dod_bspparse_ent *pEnt )
{
	env_model_t model;
	char *key;
	char *value;
	int i;

	memset( &model, 0, sizeof( env_model_t ) );

	if( pEnt->numpairs > 0 )
	{
		for( i = 0; i < pEnt->numpairs; ++i )
		{
			key = pEnt->kvPairs[i].szKeyName;
			value = pEnt->kvPairs[i].szValue;

			if( !strcasecmp( key, "targetname" ) )
				return;

			if( !strcasecmp( key, "spawnflags" ) )
			{
				int iSpawnflags = strtol( value, NULL, 10 );
				model.spawnflags |= iSpawnflags;

				if( ( model.spawnflags & SF_ENVMODEL_OFF ) != 0 )
					return;
			}
			else if( !strcasecmp( key, "model" ) )
				strncpy( model.szModel, value, 64 );

			else if( !strcasecmp( key, "body" ) )
				model.iBody = strtol( value, NULL, 10 );

			else if( !strcasecmp( key, "origin" ) )
				sscanf( value, "%f %f %f\n", &model.vecOrigin.x, &model.vecOrigin.y, &model.vecOrigin.z );

			else if( !strcasecmp( key, "angles" ) )
				sscanf( value, "%f %f %f\n", &model.vecAngles.x, &model.vecAngles.y, &model.vecAngles.z );

			else if( !strcasecmp( key, "m_iAction_On" ) )
			{
				if( strtol( value, NULL, 10 ) > 0 )
					model.spawnflags |= SF_ENVMODEL_ACTION_ON;
			}
			else if( !strcasecmp( key, "m_iszSequence_On" ) )
				strncpy( model.szSequence, value, 64 );
		}
	}

	gHUD.m_CEnvModel.AddEnvModel( &model );
}

char *DoD_LoadEntityLump( char *filename )
{
	dheader_t header;
	int size;
	lump_t *curLump;
	char *buffer;
	byte *pBSPData;

	pBSPData = gEngfuncs.COM_LoadFile( filename, 5, &size );

	if( !pBSPData )
		return NULL;

	memcpy( &header, pBSPData, sizeof( dheader_t ) );

	if( header.version < 29 || header.version > 30 )
	{
		gEngfuncs.Con_Printf( "DoD_LoadEntityLump:  Map [%s] has incorrect BSP version (%i should be %i).\n",
			filename, header.version, 30 );
		return NULL;
	}

	curLump = &header.lumps[0];

	buffer = ( char * ) malloc( curLump->filelen + 1 );

	if( !buffer )
	{
		gEngfuncs.Con_Printf( "DoD_LoadEntityLump:  Couldn't allocate %i bytes\n", curLump->filelen + 1 );
		return NULL;
	}

	memcpy( buffer, pBSPData + curLump->fileofs, curLump->filelen );
	buffer[curLump->filelen] = '\0';

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
		gEngfuncs.Con_DPrintf(
			"ERROR - Couldn't load bsp to parse client ents - %s - Check working folder is set properly\n",
			filename );
	}
}

dod_camera_t *DoD_FindTarget( const char *name )
{
	dod_camera_t *target;
	int i = 0;

	while( true )
	{
		target = &s_cameras[i];

		if( !strcasecmp( target->targetname, name ) )
			return target;

		if( ++i == 256 )
			return NULL;
	}

	return target;
}

void DoD_ReadCameraSettings( const char *Targetname )
{
	dod_camera_t *target;
	int def_fov;
	int subtitle;

	target = DoD_FindTarget( Targetname );

	if( !target )
		return;

	switch( target->overlay )
	{
	case 1:
		gHUD.m_Scope.SetScope( WEAPON_SPRING );
		gHUD.m_Scope.m_iFlags |= HUD_ACTIVE;
		break;
	case 2:
		gHUD.m_Scope.SetScope( WEAPON_BINOC );
		gHUD.m_Scope.m_iFlags |= HUD_ACTIVE;
		break;
	case 3:
	default:
		gHUD.m_Scope.SetScope( WEAPON_NONE );
		break;
	}

	def_fov = target->fov;
	g_lastFOV = def_fov;

	if( !def_fov )
		def_fov = 90;

	gHUD.m_iFOV = def_fov;
	subtitle = target->subtitle;
}

void DoD_ScreenFades( int flags, float holdtime, int r, int g, int b, int a, int speed )
{
	screenfade_t sf;
	float endtime;

	gEngfuncs.pfnGetScreenFade( &sf );

	sf.fader = r;
	sf.fadeg = g;
	sf.fadeb = b;
	sf.fadeSpeed = speed;
	sf.fadealpha = a;
	sf.fadeFlags = flags;

	endtime = gEngfuncs.GetClientTime() + holdtime;

	sf.fadeReset = endtime;
	sf.fadeTotalEnd = endtime;
	sf.fadeEnd = endtime;

	gEngfuncs.pfnSetScreenFade( &sf );
}

char *ParseSingleEnt( char *buffer, dod_bspparse_ent *pEnt, int *error )
{
	char token[1024];
	char key[256];
	int n;
	char *pTail;

	memset( pEnt, 0, sizeof( dod_bspparse_ent ) );

	while( true )
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

		n = strlen( key );

		if( n > 0 && key[n - 1] == ' ' )
		{
			pTail = &key[n - 1];
			do
			{
				*pTail = '\0';

				if( pTail == key )
					break;

				--pTail;
			} while( *pTail == ' ' );
		}

		buffer = gEngfuncs.COM_ParseFile( buffer, token );

		if( !buffer || token[0] == '}' )
		{
			*error = 1;
			return buffer;
		}

		if( !strcmp( key, "classname" ) )
			strcpy( pEnt->classname, token );
		else
			DoD_SetKVPair( pEnt, key, token );
	}
}

void DoD_SetKVPair( dod_bspparse_ent *pEnt, const char *key, const char *value )
{
	if( !strcmp( key, "classname" ) )
		strcpy( pEnt->classname, value );
	else
	{
		int idx = pEnt->numpairs;
		if( idx <= 127 )
		{
			strncpy( pEnt->kvPairs[idx].szKeyName, key, 32 );
			strncpy( pEnt->kvPairs[idx].szValue, value, 128 );
			pEnt->numpairs++;
		}
	}
}