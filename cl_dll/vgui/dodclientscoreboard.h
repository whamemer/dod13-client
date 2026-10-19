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
// dodclientscoreboard.h
//
// The scoreboard of DoD (CDoDClientScoreBoardDialog of the original). The layout is Resource/UI/ScoreBoard.res: the
// dialog is 444x360 in 640x480 and is centered, the top bar with the name of the server and the list under it.
// The original has a SectionedListPanel with the header section and the sections of the teams, the panel here draws
// the same lines itself: the header with the names of the columns, then the row of a team (name, number of players,
// score, kills, deaths, ping) and the players of the team sorted by the score, the spectators at the end.
//

#ifndef DODCLIENTSCOREBOARD_H
#define DODCLIENTSCOREBOARD_H

#include <VGUI_Panel.h>

#include "vgui/idodviewport.h"

class ScorePanel : public vgui::Panel
{
public:
	ScorePanel( int x, int y, int wide, int tall );

	// Called each time a new level is started
	void Initialize( void );

	// Called every half of a second while the scoreboard is visible, and when it is opened
	void Update( void );
	void Open( void );

	virtual void paintBackground( void );
	virtual void paint( void );

	// the score of the teams from the TeamScore message (CDoDClientScoreBoardDialog of the original)
	ExtraScoreBoardInfo GetExtraScoreBoardInfo( int teamIndex );
	void SetExtraScoreBoardInfo( int teamIndex, int score );
	int GetTeamScore( int teamIndex );

	// The wheel of the mouse scrolls the list when it does not fit
	void ScrollByWheel( int iDelta );

private:
	enum
	{
		LINE_HEADER = 0,	// the names of the columns
		LINE_TEAM,	// the row of a team
		LINE_PLAYER,	// a player of a team
		LINE_SPECTATOR,	// a spectator, only the name is shown
		MAX_LINES = 80
	};

	struct Line_t
	{
		int iType;
		int iTeam;	// 1 allies, 2 axis, 3 spectators
		bool bThisPlayer;

		char szName[64];
		char szClass[64];
		char szStatus[24];

		int iScore;
		int iFrags;
		int iDeaths;
		int iPing;
	};

	Line_t *AddLine( int iType, int iTeam );
	void AddTeam( int iTeam );
	void SortPlayers( int *piPlayers, int iCount );
	void Layout( void );

	int GetLineHeight( int iType );
	void DrawText( vgui::Font *pFont, int x, int y, int iRowHeight, const char *pszText, bool bRight, int r, int g, int b );

	Line_t m_Lines[MAX_LINES];
	int m_iNumLines;

	// the name of the server, the title of the dialog
	char m_szServerName[64];

	// the scrolled part of the list, in pixels; the height of all the lines and of the part of the list that is shown
	int m_iScroll;
	int m_iListTall;
	int m_iViewTall;

	ExtraScoreBoardInfo m_TeamExtraInfo[5];
	ExtraScoreBoardInfo m_BlankExtraInfo;
};

#endif // DODCLIENTSCOREBOARD_H
