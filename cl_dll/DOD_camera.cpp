//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "util_vector.h"
#include "DOD_camera.h"

extern float g_lastFOV;

void DoD_ProcessEnts( char *buffer )
{

}

void Parse_dod_camera( dod_bspparse_ent *pEnt )
{

}

void Parse_path_corner( dod_bspparse_ent *pEnt )
{

}

void Parse_particle_shooter( dod_bspparse_ent *pEnt )
{

}

extern int g_iWeatherType;

void Parse_info_doddetect( dod_bspparse_ent *pEnt )
{

}

void Parse_env_model( dod_bspparse_ent *pEnt )
{

}

char *DoD_LoadEntityLump( char *filename )
{
	return '\0';
}

void DoD_LoadClientEnts( const char *map )
{

}

dod_camera_t *DoD_FindTarget( const char *name )
{
	return 0;
}

void DoD_ReadCameraSettings( const char *Targetname )
{

}

void DoD_ScreenFades( int flags, float holdtime, int r, int g, int b, int a, int speed )
{

}

char *ParseSingleEnt( char *buffer, dod_bspparse_ent *pEnt, int *error )
{
	return '\0';
}

void DoD_SetKVPair( dod_bspparse_ent *pEnt, const char *key, const char *value )
{

}