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
// dodclientscoreboard.cpp
//
// The scoreboard of DoD, see dodclientscoreboard.h. The columns, the sorting, the colors and the texts are the ones
// of CDoDClientScoreBoardDialog (AddHeader, AddSection, UpdateTeamInfo, GetPlayerScoreInfo, StaticPlayerSortFunc and
// GetTeamColor of the original).
//

#include <string.h>
#include <stdio.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "dod_localize.h"

#include "vgui_defaultinputsignal.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodmenu.h"
#include "vgui/dodclientscoreboard.h"

namespace
{

// The size of the dialog and of its parts in 640x480 (ScoreBoard.res)
const int DIALOG_WIDE = 444;
const int DIALOG_TALL = 360;
const int TOPBAR_TALL = 22;
const int LIST_Y = 24;

// The heights of the lines with the 1 pixel line at the bottom of the header and of the row of a team: measured on the
// screenshot of the original at 1200p (36, 39 and 32 pixels, the scale is 2.5)
const float HEADER_TALL = 14.4f;
const float TEAM_TALL = 15.6f;
const float PLAYER_TALL = 12.8f;

// The columns of the list: the left edge and the width in 640x480 (AddHeader), the numbers are on the right
const int COLUMN_NAME = 0;
const int COLUMN_CLASS = 135;
const int COLUMN_STATUS = 232;
const int COLUMN_SCORE_RIGHT = 267 + 38;
const int COLUMN_FRAGS_RIGHT = 305 + 38;
const int COLUMN_DEATHS_RIGHT = 343 + 38;
const int COLUMN_PING_RIGHT = 381 + 46;

// The text of a column starts a little to the right of its edge, the numbers end a little after it (measured)
const float TEXT_INSET = 2;	// the class and the status
const float TEAM_NAME_INSET = 3;	// the name of a team
const float NAME_INSET = 5.5f;	// the name of a player
const float NUMBER_INSET = 2;

// The selected row (the player himself) is lighter, SelectionBG2 (128,128,128,64); it is a little narrower than the list
const float HIGHLIGHT_INSET = 2.8f;

// The baseline of the text of Courier New is lower in the original than the one of a centered text (the header and the
// row of a team)
const float SMALL_FONT_SHIFT = 1.3f;

// GetTeamColor: team1 (the allies), team2 (the axis) and the others (the spectators)
void GetTeamColor( int iTeam, int &r, int &g, int &b )
{
	if( iTeam == 1 )
	{
		r = 76;
		g = 102;
		b = 76;
	}
	else if( iTeam == 2 )
	{
		r = 255;
		g = 64;
		b = 64;
	}
	else
	{
		r = 255;
		g = 179;
		b = 0;
	}
}

// The wheel of the mouse scrolls the list
class CScoreWheelSignal : public vgui::CDefaultInputSignal
{
public:
	CScoreWheelSignal( ScorePanel *pPanel ) : m_pPanel( pPanel ) {}

	virtual void mouseWheeled( int delta, vgui::Panel *panel )
	{
		m_pPanel->ScrollByWheel( delta );
	}

private:
	ScorePanel *m_pPanel;
};

}

// Used by the voice status (CVoiceStatus::UpdateBanButton)
bool HACK_GetPlayerUniqueID( int iPlayer, char playerID[16] )
{
	return !!gEngfuncs.GetPlayerUniqueID( iPlayer, playerID );
}

ScorePanel::ScorePanel( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
{
	m_iNumLines = 0;
	m_iScroll = 0;
	m_iListTall = 0;
	m_iViewTall = 0;
	m_szServerName[0] = '\0';

	for( int i = 0; i < 5; i++ )
		m_TeamExtraInfo[i].score = 0;

	m_BlankExtraInfo.score = 0;

	addInputSignal( new CScoreWheelSignal( this ) );

	setPaintBackgroundEnabled( true );
}

ExtraScoreBoardInfo ScorePanel::GetExtraScoreBoardInfo( int teamIndex )
{
	if( (unsigned int)teamIndex <= 4 )
		return m_TeamExtraInfo[teamIndex];

	return m_BlankExtraInfo;
}

void ScorePanel::SetExtraScoreBoardInfo( int teamIndex, int score )
{
	if( (unsigned int)teamIndex <= 4 )
		m_TeamExtraInfo[teamIndex].score = score;
}

int ScorePanel::GetTeamScore( int teamIndex )
{
	if( (unsigned int)( teamIndex - 1 ) <= 3 )
		return m_TeamExtraInfo[teamIndex].score;

	return -1;
}

void ScorePanel::Initialize( void )
{
	m_iNumLines = 0;
	m_iScroll = 0;

	for( int i = 0; i < 5; i++ )
		m_TeamExtraInfo[i].score = 0;
}

// NULL when the list is full
ScorePanel::Line_t *ScorePanel::AddLine( int iType, int iTeam )
{
	if( m_iNumLines >= MAX_LINES )
		return NULL;

	Line_t *pLine = &m_Lines[m_iNumLines++];

	memset( pLine, 0, sizeof( Line_t ) );

	pLine->iType = iType;
	pLine->iTeam = iTeam;

	return pLine;
}

// The players are sorted by the score, then by the kills, then fewer deaths first, then by the number (StaticPlayerSortFunc)
void ScorePanel::SortPlayers( int *piPlayers, int iCount )
{
	for( int i = 1; i < iCount; i++ )
	{
		int iPlayer = piPlayers[i];
		int j = i;

		while( j > 0 )
		{
			const extra_player_info_t &previous = g_PlayerExtraInfo[piPlayers[j - 1]];
			const extra_player_info_t &current = g_PlayerExtraInfo[iPlayer];
			bool bSwap;

			if( previous.objscore != current.objscore )
				bSwap = previous.objscore < current.objscore;
			else if( previous.frags != current.frags )
				bSwap = previous.frags < current.frags;
			else if( previous.deaths != current.deaths )
				bSwap = previous.deaths > current.deaths;
			else
				bSwap = piPlayers[j - 1] > iPlayer;

			if( !bSwap )
				break;

			piPlayers[j] = piPlayers[j - 1];
			j--;
		}

		piPlayers[j] = iPlayer;
	}
}

// The row of a team and its players: the players that have no team are in the section of the spectators. A section
// without players is not shown (SectionedListPanel hides an empty section that is not always visible).
void ScorePanel::AddTeam( int iTeam )
{
	int iPlayers[MAX_PLAYERS];
	int iCount = 0;
	int iTeamPlayers = 0;
	int iFrags = 0;
	int iDeaths = 0;
	int iPing = 0;

	for( int i = 1; i < MAX_PLAYERS; i++ )
	{
		gEngfuncs.pfnGetPlayerInfo( i, &g_PlayerInfoList[i] );

		if( !g_PlayerInfoList[i].name )
			continue;

		int iPlayerTeam = g_PlayerExtraInfo[i].teamnumber;
		int iSection = ( iPlayerTeam == 1 || iPlayerTeam == 2 ) ? iPlayerTeam : 3;

		if( iSection != iTeam )
			continue;

		iPlayers[iCount++] = i;

		// the team numbers 1..3 are counted in the row of the team
		if( iPlayerTeam == iTeam )
		{
			iTeamPlayers++;
			iFrags += g_PlayerExtraInfo[i].frags;
			iDeaths += g_PlayerExtraInfo[i].deaths;
			iPing += g_PlayerInfoList[i].ping;
		}
	}

	if( iCount == 0 )
		return;

	// the row of the team: "Allies   -   10 players"
	Line_t *pTeamLine = AddLine( LINE_TEAM, iTeam );

	if( pTeamLine )
	{
		char szTeamName[64];
		char szFormat[128];
		char szPlayers[8];
		const char *pszTeamToken;

		if( iTeam == 1 )
			pszTeamToken = gHUD.m_bBritish ? "#Teamname_Brit" : "#Teamname_Allies";
		else if( iTeam == 2 )
			pszTeamToken = "#Teamname_Axis";
		else
			pszTeamToken = "#Teamname_Spectators";

		DoD_GetLocalized( pszTeamToken, szTeamName, sizeof( szTeamName ) );
		DoD_GetLocalized( iTeamPlayers == 1 ? "#scoreboard_Player" : "#scoreboard_Players", szFormat, sizeof( szFormat ) );
		snprintf( szPlayers, sizeof( szPlayers ), "%d", iTeamPlayers );
		DoD_ConstructLocalized( pTeamLine->szName, sizeof( pTeamLine->szName ), szFormat, szTeamName, szPlayers, "", "" );

		pTeamLine->iScore = m_TeamExtraInfo[iTeam].score;
		pTeamLine->iFrags = iFrags;
		pTeamLine->iDeaths = iDeaths;
		pTeamLine->iPing = iTeamPlayers > 0 ? iPing / iTeamPlayers : 0;
	}

	SortPlayers( iPlayers, iCount );

	for( int n = 0; n < iCount; n++ )
	{
		int iPlayer = iPlayers[n];
		const hud_player_info_t &info = g_PlayerInfoList[iPlayer];
		const extra_player_info_t &extra = g_PlayerExtraInfo[iPlayer];

		Line_t *pLine = AddLine( iTeam == 3 ? LINE_SPECTATOR : LINE_PLAYER, iTeam );

		if( !pLine )
			return;

		Line_t &line = *pLine;

		DoD_ConvertText( info.name, line.szName, sizeof( line.szName ) );

		line.bThisPlayer = info.thisplayer != 0;
		line.iScore = extra.objscore;
		line.iFrags = extra.frags;
		line.iDeaths = extra.deaths;
		line.iPing = info.ping;

		// the status and the class are shown for the team of the player; a spectator and a player that has no team
		// see them for everybody (GetPlayerScoreInfo of the original)
		bool bVisible = true;

		if( g_iTeamNumber && !gEngfuncs.IsSpectateOnly() )
			bVisible = ( extra.teamnumber == g_iTeamNumber );

		if( bVisible && extra.dead )
			DoD_GetLocalized( "#scoreboard_status_dead", line.szStatus, sizeof( line.szStatus ) );
		else if( bVisible && extra.status == 5 )
			DoD_GetLocalized( "#scoreboard_status_hasobject", line.szStatus, sizeof( line.szStatus ) );

		if( bVisible && extra.playerclass > 0 && extra.playerclass < 28 )
			DoD_GetLocalized( gHUD.GetPlayerClassName( extra.playerclass ), line.szClass, sizeof( line.szClass ) );
	}
}

void ScorePanel::Update( void )
{
	m_iNumLines = 0;

	AddLine( LINE_HEADER, 0 );
	AddTeam( 1 );
	AddTeam( 2 );
	AddTeam( 3 );

	// the name of the server is the title
	if( gViewPort && gViewPort->m_szServerName[0] )
		DoD_ConvertText( gViewPort->m_szServerName, m_szServerName, sizeof( m_szServerName ) );
	else
		m_szServerName[0] = '\0';

	Layout();
}

void ScorePanel::Open( void )
{
	m_iScroll = 0;

	Update();
	setVisible( true );
}

int ScorePanel::GetLineHeight( int iType )
{
	if( iType == LINE_HEADER )
		return DODMENU_SIZE( HEADER_TALL );

	if( iType == LINE_TEAM )
		return DODMENU_SIZE( TEAM_TALL );

	return DODMENU_SIZE( PLAYER_TALL );
}

// The dialog is centered and is never lower than the 360 of the layout, it grows with the list (CClientScoreBoardDialog::Update);
// what does not fit on the screen is scrolled
void ScorePanel::Layout( void )
{
	m_iListTall = 0;

	for( int i = 0; i < m_iNumLines; i++ )
		m_iListTall += GetLineHeight( m_Lines[i].iType );

	int iWide = DODMENU_SIZE( DIALOG_WIDE );
	int iTall = DODMENU_SIZE( LIST_Y ) + m_iListTall;

	if( iTall < DODMENU_SIZE( DIALOG_TALL ) )
		iTall = DODMENU_SIZE( DIALOG_TALL );

	if( iTall > ScreenHeight )
		iTall = ScreenHeight;

	m_iViewTall = iTall - DODMENU_SIZE( LIST_Y );

	if( m_iScroll > m_iListTall - m_iViewTall )
		m_iScroll = m_iListTall - m_iViewTall;

	if( m_iScroll < 0 )
		m_iScroll = 0;

	setBounds( ( ScreenWidth - iWide ) / 2, ( ScreenHeight - iTall ) / 2, iWide, iTall );
}

void ScorePanel::ScrollByWheel( int iDelta )
{
	m_iScroll -= iDelta * DODMENU_SIZE( PLAYER_TALL ) * 2;

	if( m_iScroll > m_iListTall - m_iViewTall )
		m_iScroll = m_iListTall - m_iViewTall;

	if( m_iScroll < 0 )
		m_iScroll = 0;

	repaint();
}

// The top bar and the list are black and half transparent (ListBG of the scheme and the bar of the layout)
void ScorePanel::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	// the alpha of VGUI1 is the transparency: 127 is the alpha 128
	drawSetColor( 0, 0, 0, 127 );
	drawFilledRect( 0, 0, wide, DODMENU_SIZE( TOPBAR_TALL ) );
	drawFilledRect( 0, DODMENU_SIZE( LIST_Y ), wide, tall );

	// the gap between them is not painted, the game is seen through it; the list has a border of 1 pixel
	drawSetColor( DODCOLOR_DARK, 0 );
	drawFilledRect( 0, DODMENU_SIZE( LIST_Y ), wide, DODMENU_SIZE( LIST_Y ) + 1 );
	drawFilledRect( 0, DODMENU_SIZE( LIST_Y ), 1, tall );
	drawFilledRect( wide - 1, DODMENU_SIZE( LIST_Y ), wide, tall );
	drawFilledRect( 0, tall - 1, wide, tall );
}

// Text in a row: the top of the text is centered in the height of the row
void ScorePanel::DrawText( vgui::Font *pFont, int x, int y, int iRowHeight, const char *pszText, bool bRight, int r, int g, int b )
{
	int iWide, iTall;

	if( !pszText[0] )
		return;

	pFont->getTextSize( pszText, iWide, iTall );

	if( bRight )
		x -= iWide;

	drawSetTextFont( pFont );
	drawSetTextColor( r, g, b, 0 );
	drawPrintText( x, y + ( iRowHeight - pFont->getTall() ) / 2, pszText, (int)strlen( pszText ) );
}

void ScorePanel::paint( void )
{
	int wide, tall;
	char sz[16];

	getPaintSize( wide, tall );

	vgui::Font *pRowFont = DoD_GetMenuFont( DODFONT_SCORE );
	vgui::Font *pHeaderFont = DoD_GetMenuFont( DODFONT_SCORESMALL );

	// the title: the name of the server
	DrawText( pRowFont, DODMENU_SIZE( 4 ), 0, DODMENU_SIZE( TOPBAR_TALL ), m_szServerName, false, DODCOLOR_BRIGHT );

	// the lines of the list, a line that is not shown completely is not drawn
	int iListTop = DODMENU_SIZE( LIST_Y );
	int y = iListTop - m_iScroll;

	for( int i = 0; i < m_iNumLines; i++ )
	{
		const Line_t &line = m_Lines[i];
		int iHeight = GetLineHeight( line.iType );
		int r, g, b;

		if( y >= iListTop && y + iHeight <= iListTop + m_iViewTall )
		{
			GetTeamColor( line.iTeam, r, g, b );

			int ySmall = y + DODMENU_SIZE( SMALL_FONT_SHIFT );
			int xName = DODMENU_SIZE( COLUMN_NAME + TEAM_NAME_INSET );
			int xClass = DODMENU_SIZE( COLUMN_CLASS + TEXT_INSET );
			int xStatus = DODMENU_SIZE( COLUMN_STATUS + TEXT_INSET );
			int xScore = DODMENU_SIZE( COLUMN_SCORE_RIGHT + NUMBER_INSET );
			int xFrags = DODMENU_SIZE( COLUMN_FRAGS_RIGHT + NUMBER_INSET );
			int xDeaths = DODMENU_SIZE( COLUMN_DEATHS_RIGHT + NUMBER_INSET );
			int xPing = DODMENU_SIZE( COLUMN_PING_RIGHT + NUMBER_INSET );

			if( line.iType == LINE_HEADER )
			{
				// the names of the columns in white, the line under them
				DoD_GetLocalized( "#PlayerClass", sz, sizeof( sz ) );
				DrawText( pHeaderFont, xClass, ySmall, iHeight, sz, false, DODCOLOR_BRIGHT );

				DoD_GetLocalized( "#PlayerStatus", sz, sizeof( sz ) );
				DrawText( pHeaderFont, xStatus, ySmall, iHeight, sz, false, DODCOLOR_BRIGHT );

				DoD_GetLocalized( "#PlayerScore", sz, sizeof( sz ) );
				DrawText( pHeaderFont, xScore, ySmall, iHeight, sz, true, DODCOLOR_BRIGHT );

				DoD_GetLocalized( "#PlayerFrags", sz, sizeof( sz ) );
				DrawText( pHeaderFont, xFrags, ySmall, iHeight, sz, true, DODCOLOR_BRIGHT );

				DoD_GetLocalized( "#PlayerDeath", sz, sizeof( sz ) );
				DrawText( pHeaderFont, xDeaths, ySmall, iHeight, sz, true, DODCOLOR_BRIGHT );

				DoD_GetLocalized( "#PlayerPing", sz, sizeof( sz ) );
				DrawText( pHeaderFont, xPing, ySmall, iHeight, sz, true, DODCOLOR_BRIGHT );

				drawSetColor( DODCOLOR_DARK, 0 );
				drawFilledRect( 0, y + iHeight - 1, wide, y + iHeight );
			}
			else if( line.iType == LINE_TEAM )
			{
				// the row of a team: in the color of the team, the numbers are of the teams of the players
				DrawText( pHeaderFont, xName, ySmall, iHeight, line.szName, false, r, g, b );

				if( line.iTeam != 3 )
				{
					snprintf( sz, sizeof( sz ), "%d", line.iScore );
					DrawText( pHeaderFont, xScore, ySmall, iHeight, sz, true, r, g, b );

					snprintf( sz, sizeof( sz ), "%d", line.iFrags );
					DrawText( pHeaderFont, xFrags, ySmall, iHeight, sz, true, r, g, b );

					snprintf( sz, sizeof( sz ), "%d", line.iDeaths );
					DrawText( pHeaderFont, xDeaths, ySmall, iHeight, sz, true, r, g, b );

					snprintf( sz, sizeof( sz ), "%d", line.iPing );
					DrawText( pHeaderFont, xPing, ySmall, iHeight, sz, true, r, g, b );
				}

				drawSetColor( r, g, b, 0 );
				drawFilledRect( 0, y + iHeight - 1, wide, y + iHeight );
			}
			else
			{
				// a player: the row of the player is highlighted (the selected row of the list)
				if( line.bThisPlayer )
				{
					int iInset = DODMENU_SIZE( HIGHLIGHT_INSET );

					drawSetColor( 128, 128, 128, 191 );
					drawFilledRect( iInset, y, wide - iInset, y + iHeight );
				}

				DrawText( pRowFont, DODMENU_SIZE( COLUMN_NAME + NAME_INSET ), y, iHeight, line.szName, false, r, g, b );

				if( line.iType == LINE_PLAYER )
				{
					DrawText( pRowFont, xClass, y, iHeight, line.szClass, false, r, g, b );
					DrawText( pRowFont, xStatus, y, iHeight, line.szStatus, false, r, g, b );

					snprintf( sz, sizeof( sz ), "%d", line.iScore );
					DrawText( pRowFont, xScore, y, iHeight, sz, true, r, g, b );

					snprintf( sz, sizeof( sz ), "%d", line.iFrags );
					DrawText( pRowFont, xFrags, y, iHeight, sz, true, r, g, b );

					snprintf( sz, sizeof( sz ), "%d", line.iDeaths );
					DrawText( pRowFont, xDeaths, y, iHeight, sz, true, r, g, b );

					snprintf( sz, sizeof( sz ), "%d", line.iPing );
					DrawText( pRowFont, xPing, y, iHeight, sz, true, r, g, b );
				}
			}
		}

		y += iHeight;
	}

	// the list does not fit: a bar shows the part of it that is visible
	if( m_iListTall > m_iViewTall && m_iListTall > 0 )
	{
		int iBarTall = m_iViewTall * m_iViewTall / m_iListTall;
		int iBarTop = iListTop + m_iScroll * m_iViewTall / m_iListTall;
		int iBarWide = DODMENU_SIZE( 4 );

		drawSetColor( 190, 190, 190, 120 );
		drawFilledRect( wide - iBarWide - 1, iBarTop, wide - 1, iBarTop + iBarTall );
	}
}
