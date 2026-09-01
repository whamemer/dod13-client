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
//  dod_common.cpp - implementation of the CHudDoDCommon class
//

#include "hud.h"
#include "dod_shared.h"

/*DECLARE_MESSAGE( m_DoDCommon, GameRules )
DECLARE_MESSAGE( m_DoDCommon, CameraView )
DECLARE_MESSAGE( m_DoDCommon, ResetSens )*/

bool b_cutscene_return;
int i_lastspectoggle;
float fl_DeadTargetSwitch;
extern int i_dodmusic, g_iDeadFlag, g_iVuser1x, g_iUser3, g_iinjump, g_ionground;
int g_ihidexhair;
float i_ProneCounter;

void ClientProne( void )
{

}

int CHudDoDCommon::Init( void )
{
	return 1;
}

void CHudDoDCommon::InitHUDData( void )
{
	i_dodmusic = 1;
}

int CHudDoDCommon::MsgFunc_GameRules( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CHudDoDCommon::MsgFunc_ResetSens( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CHudDoDCommon::Draw( float flTime )
{
	return 1;
}

int CHudDoDCommon::VidInit( void )
{
	return 1;
}

/*float DoDGunGoOnOffScreen(float v_modeloffscreen, ref_params_s *pparams)
{
	return 1.0f;
}*/

int CHudDoDCommon::MsgFunc_CameraView( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

extern cvar_t *cl_hud_objectives;
extern cvar_t *cl_hud_objtimer;
extern cvar_t *cl_hud_reinforcements;
extern cvar_t *cl_hud_health;
extern cvar_t *cl_hud_ammo;

bool ShowHudElement( int i_hudElement )
{
	return true;
}