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
//  weather.cpp - implementation of the CWeatherManager class
//

#include "hud.h"
#include "dod_shared.h"

extern float g_flWeatherTime;
extern int g_iWeatherType;
extern cvar_t *cl_particlefx;

int CWeatherManager::Init( void )
{
	return 1;
}

int CWeatherManager::VidInit( void )
{
	return 1;
}

#ifdef USE_PMAN
void CDoDRainDrop::Think( float flTime )
{

}
#endif // USE_PMAN

void CWeatherManager::CreateRainParticle( float *origin )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

#ifdef USE_PMAN
void CDoDSnowFlake::Think( float time )
{

}

void CDoDSnowFlake::Touch( vec3_t *pos, vec3_t *normal, int index )
{

}

void CDoDSnowFlake::Create( vec3_t org, vec3_t normal, model_s *sprite, float size, float brightness,
	const char *classname )
{

}
#endif // USE_PMAN

void CWeatherManager::CreateSnowParticle( float *origin )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}