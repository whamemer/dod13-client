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
//  dod_crosshair.cpp - implementation of the CHudDoDCrossHair class
//

#include "hud.h"
#include "dod_shared.h"

//extern engine_studio_api_t IEngineStudio;

extern cvar_t *cl_dynamic_xhair;
extern cvar_s *cl_xhair_style;

//DECLARE_MESSAGE( m_DoDCrosshair, ClanTimer );

float flBoltHideXHair;

int CHudDoDCrossHair::Init( void )
{
	return 1;
}

int CHudDoDCrossHair::VidInit( void )
{
	return 1;
}

int CHudDoDCrossHair::MsgFunc_ClanTimer( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

extern int g_iWeaponFlags;
extern int g_ihidexhair;
extern int g_iDeadFlag;

int CHudDoDCrossHair::Draw( float flTime )
{
	return 1;
}

void CHudDoDCrossHair::DrawDynamicCrossHair( void )
{

}

void CHudDoDCrossHair::DrawCustomCrossHair( int style )
{

}

int CHudDoDCrossHair::GetCrossHairWidth( void )
{
	return 1;
}

extern p_wpninfo_s P_WpnInfo[];

float CHudDoDCrossHair::GetCurrentWeaponAccuracy( void )
{
	return 1.0f;
}

bool CHudDoDCrossHair::ShouldDrawCrossHair( void )
{
	return true;
}

void CHudDoDCrossHair::DrawSpectatorCrossHair( void )
{

}

void CHudDoDCrossHair::DrawClanTimer( float flTime )
{

}