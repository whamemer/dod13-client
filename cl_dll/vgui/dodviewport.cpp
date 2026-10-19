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
// dodviewport.cpp
//
// The DoD viewport, see dodviewport.h. The panels of the original (team and class menus, scoreboard, spectator GUI,
// MOTD, hint, widescreen and mortar panels) are VGUI2, they are restored with freevgui one by one. Until a panel
// exists the viewport handles the data of the messages only (the HUD uses it) and does not draw anything.
//

#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "vgui/dodviewport.h"
#include "vgui/dodteammenu.h"
#include "vgui/dodclassmenu.h"
#include "vgui/dodmotd.h"
#include "vgui/dodclientscoreboard.h"
#include "vgui/dodspectatorgui.h"
#include "vgui/hintpanel.h"
#include "vgui/mortarpanel.h"

// the team name by the team number (1 allies, 2 axis, everything else spectators)
static void SetPlayerTeam( int playerIndex, int team )
{
	g_PlayerExtraInfo[playerIndex].teamnumber = team;

	if( g_PlayerExtraInfo[playerIndex].teamnumber < 0 )
		g_PlayerExtraInfo[playerIndex].teamnumber = 0;

	if( team == 1 )
		strncpy( g_PlayerExtraInfo[playerIndex].teamname, gHUD.GetTeamName( 1 ), sizeof( g_PlayerExtraInfo[0].teamname ) );
	else if( team == 2 )
		strncpy( g_PlayerExtraInfo[playerIndex].teamname, gHUD.GetTeamName( 2 ), sizeof( g_PlayerExtraInfo[0].teamname ) );
	else
		strncpy( g_PlayerExtraInfo[playerIndex].teamname, gHUD.GetTeamName( 3 ), sizeof( g_PlayerExtraInfo[0].teamname ) );
}

DoDViewport::DoDViewport( int x, int y, int wide, int tall ) : TeamFortressViewport( x, y, wide, tall )
{
	m_iForceCamera = 0;
	m_iForceChaseCam = 0;
	m_iFadeToBlack = 0;
	m_iClassMask = 0;
	m_pMOTD = NULL;
	m_pHintPanel = NULL;
	m_pMortarPanel = NULL;

	m_bWidescreenVisible = false;
	m_iWidescreenSubtitle = 0;

	gDoDViewPortInterface = this;
	gDoDViewPortMsgs = this;
}

DoDViewport::~DoDViewport()
{
	gDoDViewPortInterface = NULL;
	gDoDViewPortMsgs = NULL;
}

void DoDViewport::ShowVGUIMenu( int iMenu )
{
	if( gEngfuncs.IsSpectateOnly() )
		return;

	// the team menu and the class menus of the original are the panels that are not restored yet,
	// the menus of the base viewport are used for the rest
	if( iMenu == MENU_INTRO )
		gHUD.m_iIntermission = 0;

	// the class menu has the layouts of the five menus of DoD; it is shown as the class menu of the base viewport
	if( iMenu >= MENU_CLASS_ALLIES && iMenu <= MENU_CLASS_AXIS_PARA )
	{
		m_pClassMenu->SetMenu( iMenu );
		m_pClassMenu->SetClassMask( m_iClassMask );
		m_pClassMenu->Update();

		iMenu = MENU_CLASS;
	}

	TeamFortressViewport::ShowVGUIMenu( iMenu );
}

CMenuPanel *DoDViewport::ShowMOTDMenu( void )
{
	if( !m_pMOTD )
	{
		m_pMOTD = new CDoDMOTD( false, 0, 0, ScreenWidth, ScreenHeight );
		m_pMOTD->setParent( this );
		m_pMOTD->setVisible( false );
	}

	m_pMOTD->Reset();
	m_pMOTD->Activate( m_szServerName[0] ? m_szServerName : "Half-Life", gHUD.m_MOTD.m_szMOTD );

	return m_pMOTD;
}

int DoDViewport::GetForceCamera( void )
{
	if( m_iFadeToBlack )
		return 2;

	if( m_iForceChaseCam )
		return m_iForceChaseCam;

	return m_iForceCamera;
}

void DoDViewport::setWidescreenVisible( bool state )
{
	m_bWidescreenVisible = state;
}

bool DoDViewport::isWidescreenVisible( void )
{
	return m_bWidescreenVisible;
}

void DoDViewport::setWidescreenSubtitle( int subtitle )
{
	m_iWidescreenSubtitle = subtitle;
}

void DoDViewport::updateTeamMenu( void )
{
	if( m_pTeamMenu )
		m_pTeamMenu->Update();
}

// The message is shown with the head of the team of the player: the British are 0, the Americans are 1 and the Germans are 2
void DoDViewport::ShowHintMessage( const char *msg )
{
	int team = g_iTeamNumber;

	if( g_iTeamNumber == 1 )
		team = !gHUD.m_bBritish;

	if( !m_pHintPanel )
	{
		m_pHintPanel = new CHintPanel( 0, 0, 10, 10 );
		m_pHintPanel->setParent( this );
	}

	m_pHintPanel->ShowHintMessage( team, msg );
}

void DoDViewport::paintBackground( void )
{
	TeamFortressViewport::paintBackground();

	if( m_pHintPanel )
		m_pHintPanel->OnTick();

	if( m_pMortarPanel && m_pMortarPanel->isVisible() )
		m_pMortarPanel->OnTick();
}

void DoDViewport::UpdateSpectatorPanel( void )
{
	TeamFortressViewport::UpdateSpectatorPanel();

	if( m_pSpectatorGUI )
		m_pSpectatorGUI->UpdateTimer();
}

// spec_menu 1 and spec_menu 0
void DoDViewport::ShowSpectatorGUI( void )
{
	TeamFortressViewport::ShowSpectatorGUI();
}

void DoDViewport::DeactivateSpectatorGUI( void )
{
	TeamFortressViewport::DeactivateSpectatorGUI();
}

void DoDViewport::PaintBackgroundPanel( void )
{
	// VGUI2 draws the polygons of the background with ISurface, the panels of freevgui draw their own background
}

int DoDViewport::GetTeamScore( int team )
{
	if( m_pScoreBoard )
		return m_pScoreBoard->GetTeamScore( team );

	return -1;
}

// The mortar panel is made when it is needed for the first time
CMortarPanel *DoDViewport::GetMortarPanel( void )
{
	if( !m_pMortarPanel )
	{
		m_pMortarPanel = new CMortarPanel( 0, 0, 10, 10 );
		m_pMortarPanel->setParent( this );
	}

	return m_pMortarPanel;
}

void DoDViewport::ShowMortarPanel( void )
{
	GetMortarPanel()->ShowMortarPanel();
}

void DoDViewport::HideMortarPanel( void )
{
	if( m_pMortarPanel )
		m_pMortarPanel->HideMortarPanel();
}

void DoDViewport::PostMortarValue( float value )
{
	GetMortarPanel()->PostMortarValue( value );
}

ExtraScoreBoardInfo DoDViewport::GetExtraScoreBoardInfo( int teamIndex )
{
	ExtraScoreBoardInfo info;

	if( m_pScoreBoard )
	{
		info = m_pScoreBoard->GetExtraScoreBoardInfo( teamIndex );
	}
	else
	{
		info.score = 0;
	}

	return info;
}

VGuiLibraryTeamInfo_t DoDViewport::GetPlayerTeamInfo( int teamIndex )
{
	VGuiLibraryTeamInfo_t info;

	memset( &info, 0, sizeof( info ) );

	if( teamIndex >= 0 && teamIndex <= MAX_TEAMS )
	{
		info.name = g_TeamInfo[teamIndex].name;
		info.frags = g_TeamInfo[teamIndex].frags;
		info.deaths = g_TeamInfo[teamIndex].deaths;
		info.ping = g_TeamInfo[teamIndex].ping;
		info.players = g_TeamInfo[teamIndex].players;
		info.packetloss = g_TeamInfo[teamIndex].packetloss;
		info.teamnumber = g_TeamInfo[teamIndex].teamnumber;
	}

	return info;
}

int DoDViewport::MsgFunc_ForceCam( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iForceCamera = READ_BYTE();
	m_iForceChaseCam = READ_BYTE();
	m_iFadeToBlack = READ_BYTE();

	return 1;
}

int DoDViewport::MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf )
{
	int playerIndex;
	int objscore;
	int frags;
	int deaths;
	int playerclass;
	int team;
	int status;

	BEGIN_READ( pbuf, iSize );

	playerIndex = READ_BYTE();
	objscore = READ_BYTE();
	frags = READ_BYTE();
	deaths = READ_BYTE();
	playerclass = READ_BYTE();
	team = READ_BYTE();
	status = READ_BYTE();

	if( playerIndex >= 1 && playerIndex <= MAX_PLAYERS )
	{
		g_PlayerExtraInfo[playerIndex].objscore = objscore;
		g_PlayerExtraInfo[playerIndex].frags = frags;
		g_PlayerExtraInfo[playerIndex].deaths = deaths;
		g_PlayerExtraInfo[playerIndex].playerclass = playerclass;

		SetPlayerTeam( playerIndex, team );

		g_PlayerExtraInfo[playerIndex].status = status;
		g_PlayerExtraInfo[playerIndex].dead = ( status == 1 );
	}

	return 1;
}

int DoDViewport::MsgFunc_ScoreInfoLong( const char *pszName, int iSize, void *pbuf )
{
	int playerIndex;
	int objscore;
	int frags;
	int deaths;
	int playerclass;
	int team;
	int status;

	BEGIN_READ( pbuf, iSize );

	playerIndex = READ_BYTE();
	objscore = READ_SHORT();
	frags = READ_SHORT();
	deaths = READ_SHORT();
	playerclass = READ_BYTE();
	team = READ_BYTE();
	status = READ_BYTE();

	if( playerIndex >= 1 && playerIndex <= MAX_PLAYERS )
	{
		g_PlayerExtraInfo[playerIndex].objscore = objscore;
		g_PlayerExtraInfo[playerIndex].frags = frags;
		g_PlayerExtraInfo[playerIndex].deaths = deaths;
		g_PlayerExtraInfo[playerIndex].playerclass = playerclass;

		SetPlayerTeam( playerIndex, team );

		g_PlayerExtraInfo[playerIndex].status = status;
		g_PlayerExtraInfo[playerIndex].dead = ( status == 1 );
	}

	return 1;
}

int DoDViewport::MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf )
{
	int teamnumber;

	BEGIN_READ( pbuf, iSize );

	teamnumber = READ_BYTE();

	// the score goes to the slot of the team in the list of the teams
	for( int i = 1; i < MAX_PLAYERS + 1; i++ )
	{
		if( GetPlayerTeamInfo( i ).teamnumber == teamnumber )
		{
			if( m_pScoreBoard )
				m_pScoreBoard->SetExtraScoreBoardInfo( i, READ_SHORT() );

			UpdateSpectatorPanel();

			return 1;
		}
	}

	READ_SHORT();

	return 1;
}

int DoDViewport::MsgFunc_MapMarker( const char *pszName, int iSize, void *pbuf )
{
	int entindex;

	BEGIN_READ( pbuf, iSize );

	entindex = READ_BYTE();
	READ_BYTE();
	READ_SHORT();
	READ_SHORT();

	gEngfuncs.GetLocalPlayer();
	gEngfuncs.GetEntityByIndex( entindex );

	return 1;
}

int DoDViewport::MsgFunc_WaveTime( const char *pszName, int iSize, void *pbuf )
{
	int time;

	BEGIN_READ( pbuf, iSize );

	time = READ_CHAR();

	if( time <= 0 )
		gHUD.m_ObjectiveIcons.SetWaveTime( 0.0f );
	else
		gHUD.m_ObjectiveIcons.SetWaveTime( (float)time + gHUD.m_flTime );

	return 1;
}

int DoDViewport::MsgFunc_WaveStatus( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	gHUD.m_ObjectiveIcons.SetWaveStatus( READ_BYTE() );

	UpdateSpectatorPanel();

	return 1;
}

int DoDViewport::MsgFunc_WideScreen( const char *pszName, int iSize, void *pbuf )
{
	int state;

	BEGIN_READ( pbuf, iSize );

	state = READ_BYTE();

	setWidescreenVisible( state != 0 );

	return 1;
}

int DoDViewport::MsgFunc_ScoreShort( const char *pszName, int iSize, void *pbuf )
{
	int playerIndex;
	int objscore;
	int frags;
	int deaths;
	int status;

	BEGIN_READ( pbuf, iSize );

	playerIndex = READ_BYTE();
	objscore = READ_SHORT();
	frags = READ_SHORT();
	deaths = READ_SHORT();
	status = READ_BYTE();

	if( playerIndex >= 1 && playerIndex <= MAX_PLAYERS )
	{
		g_PlayerExtraInfo[playerIndex].status = status;
		g_PlayerExtraInfo[playerIndex].frags = frags;
		g_PlayerExtraInfo[playerIndex].objscore = objscore;
		g_PlayerExtraInfo[playerIndex].deaths = deaths;
		g_PlayerExtraInfo[playerIndex].dead = ( status == 1 );
	}

	return 1;
}

int DoDViewport::MsgFunc_Frags( const char *pszName, int iSize, void *pbuf )
{
	int playerIndex;
	int frags;

	BEGIN_READ( pbuf, iSize );

	playerIndex = READ_BYTE();
	frags = READ_SHORT();

	if( playerIndex >= 1 && playerIndex <= MAX_PLAYERS )
		g_PlayerExtraInfo[playerIndex].frags = frags;

	return 1;
}

int DoDViewport::MsgFunc_ObjScore( const char *pszName, int iSize, void *pbuf )
{
	int playerIndex;
	int objscore;

	BEGIN_READ( pbuf, iSize );

	playerIndex = READ_BYTE();
	objscore = READ_SHORT();

	if( playerIndex >= 1 && playerIndex <= MAX_PLAYERS )
		g_PlayerExtraInfo[playerIndex].objscore = objscore;

	return 1;
}

int DoDViewport::MsgFunc_PStatus( const char *pszName, int iSize, void *pbuf )
{
	int playerIndex;
	int status;

	BEGIN_READ( pbuf, iSize );

	playerIndex = READ_BYTE();
	status = READ_BYTE();

	if( playerIndex >= 1 && playerIndex <= MAX_PLAYERS )
	{
		g_PlayerExtraInfo[playerIndex].status = status;
		g_PlayerExtraInfo[playerIndex].dead = ( status == 1 );
	}

	return 1;
}

int DoDViewport::MsgFunc_PTeam( const char *pszName, int iSize, void *pbuf )
{
	int playerIndex;
	int team;

	BEGIN_READ( pbuf, iSize );

	playerIndex = READ_BYTE();
	team = READ_BYTE();

	if( playerIndex >= 1 && playerIndex <= MAX_PLAYERS )
		SetPlayerTeam( playerIndex, team );

	return 1;
}

int DoDViewport::MsgFunc_PClass( const char *pszName, int iSize, void *pbuf )
{
	int playerIndex;
	int playerclass;

	BEGIN_READ( pbuf, iSize );

	playerIndex = READ_BYTE();
	playerclass = READ_BYTE();

	if( playerIndex >= 1 && playerIndex <= MAX_PLAYERS )
		g_PlayerExtraInfo[playerIndex].playerclass = playerclass;

	return 1;
}

int DoDViewport::MsgFunc_CurMarker( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	SetMapMarker( READ_BYTE() );

	return 1;
}

int DoDViewport::MsgFunc_VGUIMenu( const char *pszName, int iSize, void *pbuf )
{
	int iMenu;
	int iValue;

	BEGIN_READ( pbuf, iSize );

	iMenu = READ_BYTE();

	// the briefing of the map is the string with the name of the map, the base viewport reads it
	if( iMenu == MENU_MAPBRIEFING )
		return TeamFortressViewport::MsgFunc_VGUIMenu( pszName, iSize, pbuf );

	iValue = READ_LONG();

	if( iMenu >= MENU_CLASS_ALLIES && iMenu <= MENU_CLASS_AXIS_PARA )
	{
		SetClassMask( iValue );
	}
	else if( iMenu == MENU_MORTAR )
	{
		if( iValue == 0 )
			HideMortarPanel();
		else
			ShowMortarPanel();

		return 1;
	}

	ShowVGUIMenu( iMenu );

	return 1;
}

void DoDViewport::SetMapMarker( int marker )
{
	if( marker >= 0 && marker <= 14 )
		gHUD.m_Icons.DrawMarkerIcon( gHUD.m_Spectator.GetMarkerSPR( marker ) );
}

HSPRITE DoDViewport::GetMapMarkerSpr( void )
{
	return 0;
}
