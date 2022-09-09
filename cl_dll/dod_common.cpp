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
// dod_common.cpp
//
// implementation of CHudDoDCommon class
//

#include "hud.h"
#include "cl_util.h"
#include "ref_params.h"

bool b_cutscene_return;
int i_lastspectoggle;
float fl_DeadTargetSwitch;

extern int i_dodmusic;
extern int g_iDeadFlag;
extern int g_iVuser1x;
extern int g_iUser3;
extern int g_iinjump;
extern int g_ionground;

int g_ihidexhair;
float i_ProneCounter;

DECLARE_MESSAGE( m_DoDCommon, GameRules )
DECLARE_MESSAGE( m_DoDCommon, ParaLand )
DECLARE_MESSAGE( m_DoDCommon, ResetSens )
DECLARE_MESSAGE( m_DoDCommon, CameraView )

void ClientProne( void )
{

}

int CHudDoDCommon::Init( void )
{

}

void CHudDoDCommon::InitHUDData( void )
{

}

int CHudDoDCommon::MsgFunc_GameRules( const char *pszName, int iSize, void *pbuf )
{

}

int CHudDoDCommon::MsgFunc_ResetSens( const char *pszName, int iSize, void *pbuf )
{

}

int CHudDoDCommon::Draw( float flTime )
{

}

int CHudDoDCommon::VidInit( void )
{

}

float DoDGunGoOnOffScreen( float v_modeloffscreen, ref_params_s *pparams )
{

}

int CHudDoDCommon::MsgFunc_CameraView( const char *pszName, int iSize, void *pbuf )
{

}

bool ShowHudElement( int i_hudElement )
{

}