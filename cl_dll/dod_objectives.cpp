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
// dod_objectives.cpp
//
// implementation of CObjectiveIcons class
//

#include "hud.h"
#include "cl_util.h"

enum CP_Icons
{
    CAP_ICON_NEUTRAL_FLAG    = 0,
    CAP_ICON_ALLIES_FLAG     = 1,
    CAP_ICON_AXIS_FLAG       = 2,
    CAP_ICON_NEUTRAL_88      = 3,
    CAP_ICON_ALLIES_88       = 4,
    CAP_ICON_AXIS_88         = 5,
    CAP_ICON_NEUTRAL_BRIDGE  = 6,
    CAP_ICON_ALLIES_BRIDGE   = 7,
    CAP_ICON_AXIS_BRIDGE     = 8,
    CAP_ICON_NEUTRAL_RADIO   = 9,
    CAP_ICON_ALLIES_RADIO    = 10,
    CAP_ICON_AXIS_RADIO      = 11,
    CAP_ICON_NEUTRAL_DOCS    = 12,
    CAP_ICON_ALLIES_DOCS     = 13,
    CAP_ICON_AXIS_DOCS       = 14,
    CAP_ICON_NEUTRAL_TRUCK   = 15,
    CAP_ICON_ALLIES_TRUCK    = 16,
    CAP_ICON_AXIS_TRUCK      = 17,
    CAP_ICON_NEUTRAL_CUSTOM1 = 18,
    CAP_ICON_ALLIES_CUSTOM1  = 19,
    CAP_ICON_AXIS_CUSTOM1    = 20,
    CAP_ICON_NEUTRAL_CUSTOM2 = 21,
    CAP_ICON_ALLIES_CUSTOM2  = 22,
    CAP_ICON_AXIS_CUSTOM2    = 23,
    CAP_ICON_NEUTRAL_CUSTOM3 = 24,
    CAP_ICON_ALLIES_CUSTOM3  = 25,
    CAP_ICON_AXIS_CUSTOM3    = 26,
    CAP_ICON_BRIT_FLAG       = 27,
    CAP_ICON_NEUTRAL_CUSTOM4 = 28,
    CAP_ICON_ALLIES_CUSTOM4  = 29,
    CAP_ICON_AXIS_CUSTOM4    = 30,
    CAP_ICON_NEUTRAL_CUSTOM5 = 31,
    CAP_ICON_ALLIES_CUSTOM5  = 32,
    CAP_ICON_AXIS_CUSTOM5    = 33,
    CAP_ICON_NEUTRAL_CUSTOM6 = 34,
    CAP_ICON_ALLIES_CUSTOM6  = 35,
    CAP_ICON_AXIS_CUSTOM6    = 36,
    NUM_CAP_ICONS            = 37,
};

static const char szCPIcons[NUM_CAP_ICONS][64];

DECLARE_MESSAGE( m_ObjectiveIcons, InitObj )
DECLARE_MESSAGE( m_ObjectiveIcons, SetObj )
DECLARE_MESSAGE( m_ObjectiveIcons, StartProg )
DECLARE_MESSAGE( m_ObjectiveIcons, StartProgF )
DECLARE_MESSAGE( m_ObjectiveIcons, ProgUpdate )
DECLARE_MESSAGE( m_ObjectiveIcons, CancelProg )
DECLARE_MESSAGE( m_ObjectiveIcons, TimerStatus )
DECLARE_MESSAGE( m_ObjectiveIcons, PlayersIn )

void CreatePickingRay( float fov, int mousex, int mousey, int screenwidth, int screenheight, const Vector vecRenderOrigin, const Vector vecRenderAngles, const Vector vecPickingRay )
{

}

void ScreenToWorld( int x, int y, const struct Vector pick )
{

}

int CObjectiveIcons::Init( void )
{

}

extern bool b_StopDemo;

int CObjectiveIcons::VidInit( void )
{

}

void CObjectiveIcons::UpdateObjectiveIcons( void )
{

}

int CObjectiveIcons::MsgFunc_InitObj( const char *pszName, int iSize, void *pbuf )
{

}

int CObjectiveIcons::MsgFunc_SetObj( const char *pszName, int iSize, void *pbuf )
{

}

int CObjectiveIcons::MsgFunc_StartProg( const char *pszName, int iSize, void *pbuf )
{

}

int CObjectiveIcons::MsgFunc_StartProgF( const char *pszName, int iSize, void *pbuf )
{

}

int CObjectiveIcons::MsgFunc_ProgUpdate( const char *pszName, int iSize, void *pbuf )
{

}

int CObjectiveIcons::MsgFunc_CancelProg( const char *pszName, int iSize, void *pbuf )
{

}

int CObjectiveIcons::MsgFunc_TimerStatus( const char *pszName, int iSize, void *pbuf )
{

}

int CObjectiveIcons::MsgFunc_PlayersIn( const char *pszName, int iSize, void *pbuf )
{

}

void CObjectiveIcons::ClearAllCapPoints( void )
{

}

void CObjectiveIcons::ChangeCapPoint( int point, int newowner, int timedCap )
{

}

void CObjectiveIcons::SetVisible( int point, int visible )
{

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