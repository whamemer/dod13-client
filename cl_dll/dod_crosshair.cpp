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
// dod_crosshair.cpp
//
// implementation of CHudDoDCrossHair class
//

#include "hud.h"
#include "r_studioint.h"
#include "cl_util.h"

extern cvar_t *cl_dynamic_xhair;
extern cvar_t *cl_xhair_style;

extern engine_studio_api_t IEngineStudio;

DECLARE_MESSAGE( m_DoDCrossHair, ClanTimer )

int CHudDoDCrossHair::Init( void )
{

}

int CHudDoDCrossHair::VidInit( void )
{

}

void CHudDoDCrossHair::Reset( void )
{

}

int CHudDoDCrossHair::MsgFunc_ClanTimer( const char *pszName, int iSize, void *pbuf )
{

}

int CHudDoDCrossHair::Draw( float flTime )
{

}

void CHudDoDCrossHair::DrawDynamicCrossHair( void )
{

}

void CHudDoDCrossHair::DrawCustomCrossHair( int style )
{

}

int CHudDoDCrossHair::GetCrossHairWidth( void )
{

}

float CHudDoDCrossHair::GetCurrentWeaponAccuracy( void )
{

}

bool CHudDoDCrossHair::ShouldDrawCrossHair( void )
{

}

void CHudDoDCrossHair::DrawSpectatorCrossHair( void )
{

}

void CHudDoDCrossHair::DrawClanTimer( float flTime )
{

}