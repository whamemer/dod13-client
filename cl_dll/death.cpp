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
// death.cpp
//
// implementation of CHudDeathNotice class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_DeathNotice, DeathMsg )

struct DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH * 2];
	char szVictim[MAX_PLAYER_NAME_LENGTH * 2];
	int iId;
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
};

#define MAX_DEATHNOTICES 4
static int DEATHNOTICE_DISPLAY_TIME = 6;

#define DEATHNOTICE_TOP 32

DeathNoticeItem rgDeathNoticeList[MAX_DEATHNOTICES + 1];

float g_ColorBlue[3] = { 0.6, 0.8, 1.0 };
float g_ColorRed[3]	= { 1.0, 0.25, 0.25 };
float g_ColorGreen[3] = { 0.6, 1.0, 0.6 };
float g_ColorYellow[3] = { 1.0, 0.7, 0.0 };
float g_ColorGrey[3] = { 0.8, 0.8, 0.8 };
float g_ColorDoDGreen[3] = {  }; // WHAMER: TODO

float *GetClientColor( int clientIndex )
{

}

float *GetTeamColor( int teamIndex )
{

}

int CHudDeathNotice::Init( void )
{

}

void CHudDeathNotice::InitHUDData( void )
{

}

int CHudDeathNotice::VidInit( void )
{

}

int CHudDeathNotice::Draw( float flTime )
{

}

int CHudDeathNotice::MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf )
{

}