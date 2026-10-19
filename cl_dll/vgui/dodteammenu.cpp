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
// dodteammenu.cpp
//
// The team menu of DoD, see dodteammenu.h. The layout, the localization tokens and the commands are the ones of
// Resource/UI/TeamMenu.res of the original, the polygons of the background are the ones of DoDViewport::Start.
//

#include <string.h>
#include <stdio.h>

#include "vgui/vgui_int.h"
#include <VGUI_Font.h>
#include <VGUI_TextImage.h>
#include <VGUI_TextPanel.h>

#include "hud.h"
#include "cl_util.h"
#include "dod_localize.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/idodviewport.h"
#include "vgui/dodteammenu.h"

namespace
{

// the names of the buttons in the layout, the allies and the british are on the same place
const char *s_szButtonNames[] =
{
	"amerbutton",
	"britbutton",
	"axisbutton",
	"specbutton",
	"autobutton"
};

// the team of the player list under the button
const int s_iButtonTeams[] =
{
	1,
	1,
	2,
	3,
	4
};

// the colors of the player list: team1, team2 and team3 of the scheme
const int s_iTeamColors[4][3] =
{
	{ 255, 255, 255 },
	{ 76, 102, 76 },
	{ 255, 64, 64 },
	{ 255, 179, 0 }
};

}

CDoDTeamMenu::CDoDTeamMenu( int iRemoveMe, int x, int y, int wide, int tall ) : CDoDMenuPanel( iRemoveMe, x, y, wide, tall )
{
	m_bUpdatedMapName = false;
	m_iActiveTeam = 0;
	m_iLayoutHeight = 0;
	m_szNumOnTeam[0] = '\0';

	// the controls of the layout: the labels, the buttons and the divider lines
	m_pPage = new CDoDResPage( 0, 0, ScreenWidth, ScreenHeight );
	m_pPage->setParent( this );

	if( !m_pPage->LoadControls( "Resource/UI/TeamMenu.res" ) )
		gEngfuncs.Con_Printf( "Missing the layout of the team menu: Resource/UI/TeamMenu.res\n" );

	m_pMapName = m_pPage->FindControl( "mapname" );
	m_pCancelButton = m_pPage->FindControl( "CancelButton" );
	m_pMapInfo = m_pPage->FindControl( "MapInfo" );
	m_pNumOnTeamControl = m_pPage->FindControl( "numberOnTeam" );

	// the buttons of the teams: the command of the layout, the mouse over a button shows the players of its team
	for( int i = 0; i < NUM_BUTTONS; i++ )
	{
		m_pButtons[i] = m_pPage->FindControl( s_szButtonNames[i] );

		CDoDButton *pButton = GetButton( i );

		if( !pButton )
			continue;

		pButton->setVisible( false );
		pButton->addActionSignal( new CMenuHandler_StringCommandWatch( m_pButtons[i]->szCommand, true ) );
		pButton->addInputSignal( new CHandler_MenuButtonOver( this, s_iButtonTeams[i] ) );
	}

	// the cancel button is shown to a player that is in a team already
	if( m_pCancelButton && m_pCancelButton->pPanel )
	{
		CDoDButton *pCancel = static_cast<CDoDButton *>( m_pCancelButton->pPanel );

		pCancel->setVisible( false );
		pCancel->addActionSignal( new CMenuHandler_TextWindow( HIDE_TEXTWINDOW ) );
		pCancel->addInputSignal( new CHandler_MenuButtonOver( this, 5 ) );
	}

	// the description of the map is a rich text and the players of the team are a wrapped text with the color of the team,
	// the labels MapInfo and numberOnTeam of the layout only keep their places
	if( m_pMapInfo && m_pMapInfo->pPanel )
		m_pMapInfo->pPanel->setVisible( false );

	if( m_pNumOnTeamControl && m_pNumOnTeamControl->pPanel )
		m_pNumOnTeamControl->pPanel->setVisible( false );

	m_pBriefing = new CDoDRichText( 0, 0, 10, 10 );
	m_pBriefing->setParent( this );

	m_pNumOnTeam = new TextPanel( "", 0, 0, 10, 10 );
	m_pNumOnTeam->setParent( this );
	m_pNumOnTeam->setFgColor( 255, 255, 255, 0 );
	m_pNumOnTeam->setBgColor( 0, 0, 0, 255 );
	m_pNumOnTeam->setVisible( false );

	Layout();
	Initialize();
}

CDoDButton *CDoDTeamMenu::GetButton( int iButton )
{
	if( !m_pButtons[iButton] || !m_pButtons[iButton]->pPanel )
		return NULL;

	return static_cast<CDoDButton *>( m_pButtons[iButton]->pPanel );
}

void CDoDTeamMenu::Layout( void )
{
	m_iLayoutHeight = ScreenHeight;

	setBounds( 0, 0, ScreenWidth, ScreenHeight );

	// the controls of the layout: the fonts, the sizes and the texts of the labels depend on the height of the screen
	m_pPage->setBounds( 0, 0, ScreenWidth, ScreenHeight );
	m_pPage->Layout();

	// the labels that only keep a place stay hidden
	if( m_pMapInfo && m_pMapInfo->pPanel )
		m_pMapInfo->pPanel->setVisible( false );

	if( m_pNumOnTeamControl && m_pNumOnTeamControl->pPanel )
		m_pNumOnTeamControl->pPanel->setVisible( false );

	// the window with the description of the map has a border of 1 pixel, the text is inset by 4; the scroll bar is 21 wide in the
	// original: the arrow buttons are squares of this size
	if( m_pMapInfo )
	{
		m_pBriefing->SetRichBounds( DODMENU_SIZE( m_pMapInfo->x ) + DODMENU_SIZE( 4 ), DODMENU_SIZE( m_pMapInfo->y ) + 1, DODMENU_SIZE( m_pMapInfo->wide ) - DODMENU_SIZE( 4 ) - 1, DODMENU_SIZE( m_pMapInfo->tall ) - 2, DODMENU_SIZE( 21 ) );
		m_pBriefing->SetRichFont( DoD_GetMenuFont( DODFONT_BODY ) );
	}

	if( m_pNumOnTeamControl )
	{
		m_pNumOnTeam->setFont( DoD_GetMenuFont( m_pNumOnTeamControl->iFont ) );
		m_pNumOnTeam->setBounds( DODMENU_SIZE( m_pNumOnTeamControl->x ), DODMENU_SIZE( m_pNumOnTeamControl->y ), DODMENU_SIZE( m_pNumOnTeamControl->wide ), DODMENU_SIZE( m_pNumOnTeamControl->tall ) );
	}

	// the description and the list of the players are made again with the new fonts, the name of the map is set again
	m_bUpdatedMapName = false;
	m_szNumOnTeam[0] = '\0';
}

// Called each time a new level is started
void CDoDTeamMenu::Initialize( void )
{
	m_bUpdatedMapName = false;
	m_iActiveTeam = 0;
	m_szNumOnTeam[0] = '\0';

	m_pNumOnTeam->setVisible( false );
	m_pBriefing->setScrollValue( 0, 0 );
}

void CDoDTeamMenu::SetVisibleButton( int iButton, bool bVisible )
{
	CDoDButton *pButton = GetButton( iButton );

	if( pButton )
		pButton->setVisible( bVisible );
}

// Called every time the team menu is displayed
void CDoDTeamMenu::Update( void )
{
	if( m_iLayoutHeight != ScreenHeight )
		Layout();

	// the british have their own button on the place of the allies one
	SetVisibleButton( BUTTON_ALLIES, !gHUD.m_bBritish );
	SetVisibleButton( BUTTON_BRITISH, gHUD.m_bBritish );
	SetVisibleButton( BUTTON_AXIS, true );
	SetVisibleButton( BUTTON_SPECTATE, true );
	SetVisibleButton( BUTTON_AUTOASSIGN, true );

	// a player that is in a team can close the menu
	if( m_pCancelButton && m_pCancelButton->pPanel )
		m_pCancelButton->pPanel->setVisible( g_iTeamNumber != 0 );

	// the name and the description of the map
	if( !m_bUpdatedMapName )
	{
		const char *pszLevel = gEngfuncs.pfnGetLevelName();

		if( pszLevel && pszLevel[0] )
		{
			char szPath[256];
			char szName[128];
			const char *pszSlash;
			char *pszDot;

			// the name of the map is the file name of the level without the directory and the extension
			pszSlash = strrchr( pszLevel, '/' );

			if( !pszSlash )
				pszSlash = strrchr( pszLevel, '\\' );

			strncpy( szName, pszSlash ? pszSlash + 1 : pszLevel, sizeof( szName ) - 1 );
			szName[sizeof( szName ) - 1] = '\0';

			pszDot = strchr( szName, '.' );

			if( pszDot )
				*pszDot = '\0';

			m_pPage->SetControlText( m_pMapName, szName );

			// the description is the text file of the map
			strncpy( szPath, pszLevel, sizeof( szPath ) - 5 );
			szPath[sizeof( szPath ) - 5] = '\0';

			pszDot = strrchr( szPath, '.' );

			if( pszDot )
				*pszDot = '\0';

			strcat( szPath, ".txt" );

			char *pszFile = (char *)gEngfuncs.COM_LoadFile( szPath, 5, NULL );

			if( pszFile )
			{
				m_pBriefing->SetRichText( pszFile );

				gEngfuncs.COM_FreeFile( pszFile );
			}
			else
			{
				m_pBriefing->SetRichText( "" );
			}

			m_bUpdatedMapName = true;
		}
	}
}

// The list of the players of the team that the mouse is over
void CDoDTeamMenu::UpdateNumOnTeamLabel( void )
{
	char szPlayerList[512];
	char szNumPlayers[16];
	char szPlayers[128];
	char szTeamName[64];
	char szScore[16];
	char szLabel[1024];
	int iNumPlayers = 0;
	bool bFirst = true;

	if( m_iActiveTeam < 1 || m_iActiveTeam > 3 )
	{
		m_pNumOnTeam->setVisible( false );
		return;
	}

	szPlayerList[0] = '\0';

	for( int i = 1; i < MAX_PLAYERS; i++ )
	{
		gEngfuncs.pfnGetPlayerInfo( i, &g_PlayerInfoList[i] );

		if( !g_PlayerInfoList[i].name || g_PlayerExtraInfo[i].teamnumber != m_iActiveTeam )
			continue;

		iNumPlayers++;

		if( bFirst )
		{
			snprintf( szPlayerList, sizeof( szPlayerList ), "%s", g_PlayerInfoList[i].name );
			bFirst = false;
		}
		else
		{
			strncat( szPlayerList, ", ", sizeof( szPlayerList ) - strlen( szPlayerList ) - 1 );
			strncat( szPlayerList, g_PlayerInfoList[i].name, sizeof( szPlayerList ) - strlen( szPlayerList ) - 1 );
		}
	}

	if( iNumPlayers == 0 )
	{
		DoD_GetLocalized( "Dod_no_players_on_this_team", szLabel, sizeof( szLabel ) );

		m_pNumOnTeam->setFgColor( s_iTeamColors[0][0], s_iTeamColors[0][1], s_iTeamColors[0][2], 0 );
	}
	else
	{
		if( iNumPlayers == 1 )
		{
			DoD_GetLocalized( "Dod_Player", szPlayers, sizeof( szPlayers ) );
		}
		else
		{
			char szFormat[128];

			snprintf( szNumPlayers, sizeof( szNumPlayers ), "%i", iNumPlayers );
			DoD_GetLocalized( "Dod_Players", szFormat, sizeof( szFormat ) );
			DoD_ConstructLocalized( szPlayers, sizeof( szPlayers ), szFormat, szNumPlayers, "", "", "" );
		}

		if( m_iActiveTeam == 1 )
			DoD_GetLocalized( "Teamname_Allies", szTeamName, sizeof( szTeamName ) );
		else if( m_iActiveTeam == 2 )
			DoD_GetLocalized( "Teamname_Axis", szTeamName, sizeof( szTeamName ) );
		else
			DoD_GetLocalized( "Teamname_Spectators", szTeamName, sizeof( szTeamName ) );

		m_pNumOnTeam->setFgColor( s_iTeamColors[m_iActiveTeam][0], s_iTeamColors[m_iActiveTeam][1], s_iTeamColors[m_iActiveTeam][2], 0 );

		if( m_iActiveTeam == 3 )
		{
			char szFormat[256];

			DoD_GetLocalized( "Dod_players_on_team_spec", szFormat, sizeof( szFormat ) );
			DoD_ConstructLocalized( szLabel, sizeof( szLabel ), szFormat, szTeamName, szPlayers, szPlayerList, "" );
		}
		else
		{
			char szFormat[256];
			int iScore = -1;

			if( gDoDViewPortInterface )
				iScore = gDoDViewPortInterface->GetTeamScore( m_iActiveTeam );

			snprintf( szScore, sizeof( szScore ), "%i", iScore );

			DoD_GetLocalized( "Dod_players_on_team", szFormat, sizeof( szFormat ) );
			DoD_ConstructLocalized( szLabel, sizeof( szLabel ), szFormat, szTeamName, szPlayers, szScore, szPlayerList );
		}
	}

	m_pNumOnTeam->setVisible( true );

	// the text is set when it is changed, the list is updated every frame
	if( strcmp( szLabel, m_szNumOnTeam ) )
	{
		strncpy( m_szNumOnTeam, szLabel, sizeof( m_szNumOnTeam ) - 1 );
		m_szNumOnTeam[sizeof( m_szNumOnTeam ) - 1] = '\0';

		m_pNumOnTeam->setText( szLabel );
	}
}

// Number keys: the buttons of the menu have the numbers 1..4 and 0 (cancel), 5 is the auto-assign of the base viewport
bool CDoDTeamMenu::SlotInput( int iSlot )
{
	vgui::Panel *pButton = NULL;

	switch( iSlot )
	{
	case 0:
		if( m_pCancelButton )
			pButton = m_pCancelButton->pPanel;
		break;
	case 1:
		pButton = GetButton( gHUD.m_bBritish ? BUTTON_BRITISH : BUTTON_ALLIES );
		break;
	case 2:
		pButton = GetButton( BUTTON_AXIS );
		break;
	case 3:
		pButton = GetButton( BUTTON_SPECTATE );
		break;
	case 4:
	case 5:
		pButton = GetButton( BUTTON_AUTOASSIGN );
		break;
	default:
		return false;
	}

	// Is the button pushable?
	if( !pButton || !pButton->isVisible() )
		return false;

	static_cast<CDoDButton *>( pButton )->fireActionSignal();

	return true;
}

// Update the menu before opening it
void CDoDTeamMenu::Open( void )
{
	Update();
	CMenuPanel::Open();
}

// The mouse is over a button, the players of its team are shown
void CDoDTeamMenu::SetActiveInfo( int iInput )
{
	m_iActiveTeam = iInput;

	UpdateNumOnTeamLabel();
}

void CDoDTeamMenu::paintBackground( void )
{
	// make sure we get the map briefing up
	if( !m_bUpdatedMapName )
		Update();

	// the list of the players is updated every frame
	UpdateNumOnTeamLabel();

	// the black polygons of the background
	CDoDMenuPanel::paintBackground();

	// ClassMenuDark: the border of the description (the divider lines are the image panels of the layout)
	if( m_pMapInfo )
	{
		drawSetColor( DODCOLOR_DARK, 0 );
		drawOutlinedRect( DODMENU_SIZE( m_pMapInfo->x ), DODMENU_SIZE( m_pMapInfo->y ), DODMENU_SIZE( m_pMapInfo->x + m_pMapInfo->wide ), DODMENU_SIZE( m_pMapInfo->y + m_pMapInfo->tall ) );
	}
}
