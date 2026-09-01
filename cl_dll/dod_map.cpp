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
//  dod_map.cpp - implementation of the CHudDoDMap class
//

#include "hud.h"
#include "dod_shared.h"

extern vec3_t v_sim_org, v_cl_angles;
extern cvar_t *cl_dmsmallmap;
extern int g_iTeamNumber;
bool b_turnedoffmap;
int flZoomLevels[3];

int CHudDoDMap::Draw( float flTime )
{
	return 1;
}

void CHudDoDMap::SetMapState( int mapstate )
{
	gHUD.SetMinimapState( mapstate );
}

int CHudDoDMap::VidInit( void )
{
	return 1;
}

int CHudDoDMap::Init( void )
{
	return 1;
}

float PM_SplineFraction( float value, float scale )
{
	return 1.0f;
}

void CHudDoDMap::DrawOverview( void )
{

}

void CHudDoDMap::DrawOverviewLayer( void )
{

}

bool CHudDoDMap::AddMapEntityToMap( HSPRITE sprite, double lifeTime, vec3_t *origin )
{
	return true;
}

void CHudDoDMap::DrawOverviewIcon( vec3_t *origin, HSPRITE hIcon, int iconScale, vec3_t *angles )
{

}

void CHudDoDMap::DrawOverviewEntities( void )
{

}

void CHudDoDMap::CheckOverviewEntities( void )
{

}

void CHudDoDMap::HandleMapButton( void )
{

}

void CHudDoDMap::HandleMapZoomButton( void )
{

}

void CHudDoDMap::GetSmallMapOffset( float &x, float &y, float &z )
{

}