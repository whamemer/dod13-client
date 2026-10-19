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
// dodspectatorgui.h
//
// The spectator GUI of DoD (CDoDSpectatorGUI of the original, CSpectatorGUI of game_controls under it). The layouts are
// Resource/UI/Spectator.res (the top bar with the scores and the timer, the label of the player) and
// Resource/UI/BottomSpectator.res (the lists of the bottom bar, CDoDSpectatorMenu here).
//
// The GUI has three states: it is hidden, it shows the label of the player in the bottom bar (Activate), or it shows
// the bottom bar with the lists (ShowBottomBar, the key DUCK).
//

#ifndef DODSPECTATORGUI_H
#define DODSPECTATORGUI_H

#include <VGUI_Panel.h>
#include <VGUI_Label.h>
#include <VGUI_Button.h>
#include <VGUI_ActionSignal.h>

// the height of the bars of the GUI in the layout of 640x480, the HUD scoreboard keeps its rows between them
#define PANEL_HEIGHT 64

// BottomSpectator.res in the layout of 640x480: the boxes of the lists, the lists open above them
#define SPECTATOR_LIST_Y			430	// the top of the boxes: the bar is at 416 and they are 14 under it
#define SPECTATOR_SETTINGS_X		15
#define SPECTATOR_SETTINGS_WIDE		140
#define SPECTATOR_PLAYER_X			212
#define SPECTATOR_PLAYER_WIDE		216
#define SPECTATOR_VIEW_X			485
#define SPECTATOR_VIEW_WIDE			140

// the commands of the buttons of the bottom bar
#define SPECTATOR_PANEL_CMD_NONE			0
#define SPECTATOR_PANEL_CMD_OPTIONS			1
#define SPECTATOR_PANEL_CMD_PREVPLAYER		2
#define SPECTATOR_PANEL_CMD_NEXTPLAYER		3
#define SPECTATOR_PANEL_CMD_CAMERA			4
#define SPECTATOR_PANEL_CMD_PLAYERS			5

// the text in the middle of the screen with the localized token (DuckMessage of the original)
void DuckMessage( const char *pszToken );

class CTransparentPanel;
class CImageLabel;
class CDoDSpectatorGUI;
class CDoDSpectatorMenu;

// A list of the bottom bar (the drop down box of the original: a text with the arrow, the lists are the command menus
// of the viewport) or a button with the arrow (CLeftButton and CRightButton, the glyphs of Marlett)
class CDoDSpectatorButton : public vgui::Button
{
public:
	enum
	{
		BUTTON_COMBO = 0,
		BUTTON_LEFT,
		BUTTON_RIGHT
	};

	CDoDSpectatorButton( int iType );

	void SetButtonText( const char *pszText );
	void SetButtonColor( int r, int g, int b );
	void SetCenterText( bool bCenter ) { m_bCenter = bCenter; }

	virtual void paint( void );
	virtual void paintBackground( void );

private:
	void DrawArrow( int x, int y, int iSize, int iDirection );

	int m_iType;
	char m_szText[128];
	int m_r, m_g, m_b;
	bool m_bCenter;
};

// The frame around the inset window
class CDoDSpectatorInsetBox : public vgui::Panel
{
public:
	CDoDSpectatorInsetBox( void ) : Panel( 0, 0, 1, 1 ) {}

	virtual void paintBackground( void );
};

// The list that drops up from a box of the bottom bar (the menu of the ComboBox of the original). The items are the lines of
// a command menu file (spectatormenu.txt, spectcammenu.txt) or the players that can be followed, the mark of a checkable
// item shows the state of its command.
class CDoDSpectatorMenu : public vgui::Panel
{
public:
	enum
	{
		ITEM_COMMAND = 0,	// the command is sent to the console
		ITEM_TOGGLE,		// the same, the item has a mark: the state of the cvar "<command>_internal"
		ITEM_PLAYER			// the command is the name of the player that is followed
	};

	CDoDSpectatorMenu( void );

	void Clear( void );
	void AddItem( const char *pszText, const char *pszCommand, int iType, bool bChecked );
	void LoadFile( const char *pszFile );
	int GetNumItems( void ) { return m_iNumItems; }

	// the list drops up from the box: its bottom is a pixel over the top of the box
	void Open( int x, int iBoxTop, int wide );
	void Close( void );
	bool IsOpen( void ) { return isVisible(); }

	virtual void paintBackground( void );
	virtual void paint( void );

	// the input of the list
	void CursorMoved( int x, int y );
	void CursorExited( void );
	void MousePressed( vgui::MouseCode code );
	void MouseWheeled( int delta );

private:
	enum
	{
		LIST_MAX_ITEMS = 64,
		LIST_MAX_VISIBLE = 10	// the lines that the list shows, then it is scrolled
	};

	struct Item_t
	{
		char szText[64];
		char szCommand[128];
		int iType;
		bool bChecked;
	};

	int GetItemTall( void );
	int GetNumVisible( void );
	int GetItemAt( int x, int y );
	void UpdateChecks( void );
	void DrawCheck( int x, int y );

	Item_t m_Items[LIST_MAX_ITEMS];
	int m_iNumItems;
	int m_iHover;
	int m_iScroll;
};

class CDoDSpectatorGUI : public vgui::Panel
{
public:
	CDoDSpectatorGUI( int x, int y, int wide, int tall );
	virtual ~CDoDSpectatorGUI();

	// ISpectatorInterface of the original
	void Activate( void );
	void ShowBottomBar( void );
	void HideGUI( void );
	void DeactivateGUI( void );
	bool IsGUIVisible( void ) { return isVisible(); }
	bool IsActiveGUIVisible( void );
	void EnableInsetView( int iValue );
	void SetImage( const char *pszImage );
	void Update( const char *pszPlayer, int iPlayerNum, int iUser1, bool bSpectateOnly, int iSpecNum, const char *pszMapName );
	void UpdateSpectatorPlayerList( void );
	void UpdateTimer( void );

	int GetTopBarHeight( void );
	int GetBottomBarHeight( void );

	// the key DUCK while the bottom bar is shown closes it (CSpectatorGUI::OnKeyCodePressed of the original)
	void OnDuckKey( void );

	// the buttons of the bottom bar
	void ActionSignal( int cmd );

	// the lists of the bottom bar are closed by a click out of them
	void CloseLists( void );

	// the list of the players has the players that can be followed
	bool CanSpectateTeam( int iTeam );

protected:
	void GetPlayerLabelText( int iPlayerNum, char *pszText, int iTextLen );
	void GetPlayerLabelColor( int iPlayerNum, int &r, int &g, int &b );

private:
	void Layout( void );
	void CheckLayout( void );
	void OpenList( CDoDSpectatorMenu *pMenu, CDoDSpectatorButton *pBox );
	void UpdatePlayerMenu( void );
	void AddClickAway( vgui::Panel *pPanel );
	void SetPlayerText( const char *pszText );
	void SetViewText( const char *pszText );
	void SetLabelText( vgui::Label *pLabel, const char *pszText );
	vgui::Label *CreateLabel( vgui::Label::Alignment alignment );
	void PlaceLabel( vgui::Label *pLabel, int x, int y, int wide, int tall, bool bCenter );

	// the bars and the controls of Spectator.res
	CTransparentPanel *m_pTopBar;
	CTransparentPanel *m_pBottomBarBlank;
	vgui::Panel *m_pBottomBar;
	CDoDSpectatorInsetBox *m_pInsetBox;
	CImageLabel *m_pBannerImage;

	vgui::Label *m_pPlayerLabel;
	vgui::Label *m_pExtraInfo;
	vgui::Label *m_pTimerLabel;
	vgui::Label *m_pReinforcementsLabel;
	vgui::Label *m_pAlliesScoreCaption;
	vgui::Label *m_pAlliesScoreLabel;
	vgui::Label *m_pAxisScoreCaption;
	vgui::Label *m_pAxisScoreLabel;

	// the controls of BottomSpectator.res
	CDoDSpectatorButton *m_pConfigSettings;
	CDoDSpectatorButton *m_pPlayerList;
	CDoDSpectatorButton *m_pViewOptions;
	CDoDSpectatorButton *m_pLeftButton;
	CDoDSpectatorButton *m_pRightButton;

	// the lists that the boxes open
	CDoDSpectatorMenu *m_pSettingsMenu;
	CDoDSpectatorMenu *m_pPlayerMenu;
	CDoDSpectatorMenu *m_pViewMenu;

	bool m_bHelpShown;
	bool m_bInsetVisible;

	// the size of the screen that the layout is made for, the position of the top bar (it moves with the inset window)
	int m_iLayoutWide;
	int m_iLayoutTall;
	int m_iTopBarX;
};

class CSpectatorHandler_Command : public vgui::ActionSignal
{
private:
	CDoDSpectatorGUI *m_pFather;
	int m_cmd;

public:
	CSpectatorHandler_Command( CDoDSpectatorGUI *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( vgui::Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

#endif // DODSPECTATORGUI_H
