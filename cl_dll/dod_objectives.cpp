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
//  dod_objectives.cpp - implementation of the CObjectiveIcons class
//

#include "hud.h"
#include "dod_shared.h"

/*DECLARE_MESSAGE( m_ObjectiveIcons, InitObj )
DECLARE_MESSAGE( m_ObjectiveIcons, SetObj )
DECLARE_MESSAGE( m_ObjectiveIcons, StartProg )
DECLARE_MESSAGE( m_ObjectiveIcons, StartProgF )
DECLARE_MESSAGE( m_ObjectiveIcons, ProgUpdate )
DECLARE_MESSAGE( m_ObjectiveIcons, CancelProg )
DECLARE_MESSAGE( m_ObjectiveIcons, TimerStatus )
DECLARE_MESSAGE( m_ObjectiveIcons, PlayersIn )*/

enum CP_Icons 
{
	CAP_ICON_NEUTRAL_FLAG = 0,
	CAP_ICON_ALLIES_FLAG,
	CAP_ICON_AXIS_FLAG,
	CAP_ICON_NEUTRAL_88,
	CAP_ICON_ALLIES_88,
	CAP_ICON_AXIS_88,
	CAP_ICON_NEUTRAL_BRIDGE,
	CAP_ICON_ALLIES_BRIDGE,
	CAP_ICON_AXIS_BRIDGE,
	CAP_ICON_NEUTRAL_RADIO,
	CAP_ICON_ALLIES_RADIO,
	CAP_ICON_AXIS_RADIO,
	CAP_ICON_NEUTRAL_DOCS,
	CAP_ICON_ALLIES_DOCS,
	CAP_ICON_AXIS_DOCS,
	CAP_ICON_NEUTRAL_TRUCK,
	CAP_ICON_ALLIES_TRUCK ,
	CAP_ICON_AXIS_TRUCK,
	CAP_ICON_NEUTRAL_CUSTOM1,
	CAP_ICON_ALLIES_CUSTOM1,
	CAP_ICON_AXIS_CUSTOM1,
	CAP_ICON_NEUTRAL_CUSTOM2,
	CAP_ICON_ALLIES_CUSTOM2,
	CAP_ICON_AXIS_CUSTOM2,
	CAP_ICON_NEUTRAL_CUSTOM3,
	CAP_ICON_ALLIES_CUSTOM3,
	CAP_ICON_AXIS_CUSTOM3,
	CAP_ICON_BRIT_FLAG,
	CAP_ICON_NEUTRAL_CUSTOM4,
	CAP_ICON_ALLIES_CUSTOM4,
	CAP_ICON_AXIS_CUSTOM4,
	CAP_ICON_NEUTRAL_CUSTOM5,
	CAP_ICON_ALLIES_CUSTOM5,
	CAP_ICON_AXIS_CUSTOM5,
	CAP_ICON_NEUTRAL_CUSTOM6,
	CAP_ICON_ALLIES_CUSTOM6,
	CAP_ICON_AXIS_CUSTOM6,
	NUM_CAP_ICONS
};

static const char szCPIcons[NUM_CAP_ICONS][64];

void CreatePickingRay( float fov, int mousex, int mousey, int screenwidth, int screenheight, 
	const vec3_t &vecRenderOrigin, const vec3_t &vecRenderAngles, vec3_t &vecPickingRay )
{

}

void ScreenToWorld( int x, int y, vec3_t &pick )
{

}

int CObjectiveIcons::Init( void )
{
	return 1;
}

extern bool b_StopDemo;

int CObjectiveIcons::VidInit( void )
{
	return 1;
}

void CObjectiveIcons::UpdateObjectiveIcons( void )
{

}

int CObjectiveIcons::MsgFunc_InitObj( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CObjectiveIcons::MsgFunc_SetObj( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CObjectiveIcons::MsgFunc_StartProg( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CObjectiveIcons::MsgFunc_StartProgF( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CObjectiveIcons::MsgFunc_ProgUpdate( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CObjectiveIcons::MsgFunc_CancelProg( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CObjectiveIcons::MsgFunc_TimerStatus( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CObjectiveIcons::MsgFunc_PlayersIn( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

void CObjectiveIcons::ClearAllCapPoints( void )
{

}

void CObjectiveIcons::ChangeCapPoint( int point, int newowner, int timedCap )
{

}

void CObjectiveIcons::SetVisible( int point, int visible )
{
	m_eControlPoints[point].visible = visible;
}

void CObjectiveIcons::StartCapProgress( int point, int newOwner, float time )
{

}

void CObjectiveIcons::CancelCapProgress( int point, int owner )
{

}

void CObjectiveIcons::SetNumPlayersInArea( int point, int team, int numplayers, int required )
{

}

void CObjectiveIcons::Think( void )
{

}

extern int g_iDeadFlag;
int i_iObjBottomY;

int CObjectiveIcons::Draw( float flTime )
{
	return 1;
}

void CObjectiveIcons::DrawDigit( int digit, int x, int y )
{

}

void CObjectiveIcons::CalcIconLocations( void )
{

}

void HUD_ChatInputPosition( int *x, int *y )
{

}