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
// statusbar.cpp
//
// implementation of CHudStatusBar class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

extern cvar_t *cl_identiconmode;

extern float g_lastFOV;
extern int g_iAlive;

DECLARE_MESSAGE( m_StatusBar, StatusValue )

int CHudStatusBar::Init( void )
{

}

int CHudStatusBar::VidInit( void )
{

}

void CHudStatusBar::Reset( void )
{

}

extern vec3_t v_angles;
extern int g_iDeadFlag;

bool CHudStatusBar::InDeathCamMode( void )
{

}

void CHudStatusBar::Think( void )
{

}

int CHudStatusBar::Draw( float flTime )
{

}

int CHudStatusBar::MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf )
{

}

char *CHudStatusBar::GetTargetName( void )
{

}

void CHudStatusBar::CreateEntities( void )
{

}

void CHudStatusBar::DrawEntitiesOverTeam( void )
{

}

void CHudStatusBar::DrawEntitiesOverTarget( void )
{

}