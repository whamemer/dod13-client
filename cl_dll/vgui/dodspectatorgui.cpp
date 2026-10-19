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
// dodspectatorgui.cpp
//
// The spectator GUI of DoD, see dodspectatorgui.h. The functions are the ones of CDoDSpectatorGUI and CSpectatorGUI of the
// original (Update, UpdateTimer, UpdateSpectatorPlayerList, GetPlayerLabelText, GetPlayerLabelColor, CanSpectateTeam,
// Activate, ShowBottomBar, HideGUI, DeactivateGUI, EnableInsetView, SetImage).
//

#include <string.h>
#include <stdio.h>

#include <VGUI_Font.h>

#include "vgui_defaultinputsignal.h"
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "pm_shared.h"
#include "dod_localize.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodmenu.h"
#include "vgui/dodspectatorgui.h"
#include "vgui/dodclientscoreboard.h"
#include "vgui/idodviewport.h"

extern float *GetClientColor( int clientIndex );
extern float *GetTeamColor( int teamIndex );

namespace
{

// OnThink of the original makes the bars and the frame as tall as 64 of the layout, Spectator.res has 52
const int BAR_TALL = 64;

// Spectator.res: the scores and the timer, the positions are of the layout 640x480 scaled with the height of the screen,
// "r268" is the distance from the right edge of the screen and "c-108" is the distance from the center
const int SCORE_CAPTION_RIGHT = 268;
const int SCORE_CAPTION_WIDE = 150;
const int SCORE_VALUE_RIGHT = 112;
const int SCORE_VALUE_WIDE = 45;
const int ALLIES_Y = 12;
const int AXIS_Y = 28;
const int SCORE_TALL = 15;
const int REINFORCEMENTS_X = 240;
const int REINFORCEMENTS_Y = 20;
const int REINFORCEMENTS_WIDE = 160;
const int REINFORCEMENTS_TALL = 14;
const int TIMER_RIGHT = 72;
const int TIMER_Y = 28;
const int TIMER_WIDE = 40;
const int EXTRAINFO_RIGHT = 86;
const int EXTRAINFO_Y = 12;
const int EXTRAINFO_WIDE = 150;
const int PLAYERLABEL_CENTER = 108;
const int PLAYERLABEL_Y = 441;
const int PLAYERLABEL_WIDE = 216;
const int PLAYERLABEL_TALL = 26;

// BottomSpectator.res: the controls are placed from the top of the bottom bar
const int CONTROL_Y = 14;
const int CONTROL_TALL = 22;
const int SETTINGS_X = SPECTATOR_SETTINGS_X;
const int SETTINGS_WIDE = SPECTATOR_SETTINGS_WIDE;
const int PLAYERPREV_X = 180;
const int PLAYERNEXT_X = 435;
const int ARROW_WIDE = 25;
const int PLAYERCOMBO_X = SPECTATOR_PLAYER_X;
const int PLAYERCOMBO_WIDE = SPECTATOR_PLAYER_WIDE;
const int VIEWCOMBO_X = SPECTATOR_VIEW_X;
const int VIEWCOMBO_WIDE = SPECTATOR_VIEW_WIDE;

// The bars are black, ApplySchemeSettings of the original: the color (0,0,0,196), the alpha of VGUI1 is the transparency
const int BAR_TRANSPARENCY = 255 - 196;

}

//-----------------------------------------------------------------------------
// The text in the middle of the screen
//-----------------------------------------------------------------------------
void DuckMessage( const char *pszToken )
{
	char szText[128];
	char szMessage[160];

	DoD_GetLocalized( pszToken, szText, sizeof( szText ) );

	safe_snprintf( szMessage, sizeof( szMessage ), "%c%s", HUD_PRINTCENTER, szText );

	gHUD.m_TextMessage.MsgFunc_TextMsg( NULL, strlen( szMessage ) + 1, szMessage );
}

//-----------------------------------------------------------------------------
// The buttons of the bottom bar
//-----------------------------------------------------------------------------
CDoDSpectatorButton::CDoDSpectatorButton( int iType ) : Button( "", 0, 0, 10, 10 )
{
	m_iType = iType;
	m_bCenter = false;
	m_szText[0] = '\0';
	m_r = 255;
	m_g = 176;
	m_b = 0;

	// the button is highlighted under the mouse
	addInputSignal( new CHandler_ButtonHighlight( this ) );
}

void CDoDSpectatorButton::SetButtonText( const char *pszText )
{
	DoD_ConvertText( pszText, m_szText, sizeof( m_szText ) );

	repaint();
}

void CDoDSpectatorButton::SetButtonColor( int r, int g, int b )
{
	m_r = r;
	m_g = g;
	m_b = b;

	repaint();
}

// A triangle of the arrow: the apex is at the side where it points, iSize is the half of the base
void CDoDSpectatorButton::DrawArrow( int x, int y, int iSize, int iDirection )
{
	for( int i = 0; i <= iSize; i++ )
	{
		switch( iDirection )
		{
		case ARROW_DOWN:
			drawFilledRect( x - ( iSize - i ), y + i, x + ( iSize - i ) + 1, y + i + 1 );
			break;
		case ARROW_LEFT:
			drawFilledRect( x + i, y - i, x + i + 1, y + i + 1 );
			break;
		case ARROW_RIGHT:
			drawFilledRect( x + iSize - i, y - i, x + iSize - i + 1, y + i + 1 );
			break;
		}
	}
}

// The measures are taken from the screenshot of the original at 1920x1200 (2.5 pixels of a unit of the layout)
void CDoDSpectatorButton::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	if( m_iType == BUTTON_COMBO )
	{
		// the box of a list: WindowBG (0,0,0,200) with the border of the scheme
		drawSetColor( 0, 0, 0, 255 - 200 );
		drawFilledRect( 0, 0, wide, tall );

		// the button with the arrow is black over the box, it is a bit inside of the border
		drawFilledRect( wide - DODMENU_SIZE( 8.4f ), DODMENU_SIZE( 0.8f ), wide - DODMENU_SIZE( 1.2f ), tall - 1 );

		drawSetColor( DODCOLOR_DARK, 0 );
		drawOutlinedRect( 0, 0, wide, tall );
	}
	else
	{
		// ButtonBgColor and ButtonArmedBgColor of the scheme
		if( isArmed() )
			drawSetColor( DODCOLOR_LIGHT, 0 );
		else
			drawSetColor( DODCOLOR_DARK, 0 );

		drawFilledRect( 0, 0, wide, tall );
	}
}

void CDoDSpectatorButton::paint( void )
{
	int wide, tall;
	int textWide, textTall;

	getPaintSize( wide, tall );

	if( m_iType == BUTTON_COMBO )
	{
		vgui::Font *pFont = DoD_GetMenuFont( DODFONT_SCORE );
		int iArrowWide = DODMENU_SIZE( 8.4f );

		pFont->getTextSize( m_szText, textWide, textTall );

		// the text of the player is in the middle of the list, the others start at its left edge
		int x = DODMENU_SIZE( 1.2f );

		if( m_bCenter )
			x = ( wide - iArrowWide - textWide ) / 2;

		drawSetTextFont( pFont );
		drawSetTextColor( m_r, m_g, m_b, 0 );
		drawSetTextPos( x, ( tall - textTall ) / 2 - DODMENU_SIZE( 0.7f ) );
		drawPrintText( m_szText, (int)strlen( m_szText ) );

		// the little arrow of the button is not in the color of the text, it is light gray: 7 pixels wide and 4 high
		// at 2.5 pixels of a unit, its middle is 4 units from the right edge
		int iArrowHalf = (int)( (float)ScreenHeight / 480.0f * 1.2f + 0.5f );

		drawSetColor( 176, 174, 174, 0 );
		DrawArrow( wide - DODMENU_SIZE( 4.0f ), DODMENU_SIZE( 10.0f ), iArrowHalf, ARROW_DOWN );
	}
	else
	{
		// the arrows of Marlett ("3" and "4") are white: 11 pixels wide and 21 high at 2.5 pixels of a unit, they are
		// at the left of the middle of the button
		int iSize = DODMENU_SIZE( 4.0f );

		drawSetColor( DODCOLOR_BRIGHT, 0 );

		if( m_iType == BUTTON_LEFT )
			DrawArrow( DODMENU_SIZE( 6.0f ), DODMENU_SIZE( 10.0f ), iSize, ARROW_LEFT );
		else
			DrawArrow( DODMENU_SIZE( 7.6f ), DODMENU_SIZE( 10.0f ), iSize, ARROW_RIGHT );
	}
}

//-----------------------------------------------------------------------------
// The lists of the bottom bar
//-----------------------------------------------------------------------------
namespace
{

// the measures of the menu of the ComboBox of the original (the screenshot at 1920x1200, 2.5 pixels of a unit)
const float MENU_ITEM_TALL = 26.4f;
const float MENU_TEXT_X = 17.2f;	// from the left edge of the list: the mark has the place before it
const float MENU_CHECK_X = 6.0f;
const float MENU_CHECK_Y = 10.4f;	// from the top of the item
const float MENU_BOX_X = 4.4f;	// the box of the mark under the mouse
const float MENU_BOX_Y = 7.6f;
const float MENU_BOX_SIZE = 10.8f;
const float MENU_PIXEL = 1.0f / 2.5f;	// the pixel of the screenshot in the units of the layout

// The mark of a checkable item (the glyph of Marlett): the pixels of the original, a line of them is the row and the
// first and the last column of it; the second part of a row is the right stroke
struct CheckRow_t
{
	int iLeft0, iLeft1;		// the left stroke, -1 when the row has none
	int iRight0, iRight1;	// the right stroke
};

const CheckRow_t s_CheckRows[17] =
{
	{ -1, -1, 15, 15 },
	{ -1, -1, 14, 15 },
	{ -1, -1, 13, 15 },
	{ -1, -1, 12, 15 },
	{ -1, -1, 11, 15 },
	{ 0, 0, 10, 15 },
	{ 0, 1, 9, 15 },
	{ 0, 2, 8, 14 },
	{ 0, 3, 7, 13 },
	{ 0, 4, 6, 12 },
	{ 0, 11, -1, -1 },
	{ 0, 10, -1, -1 },
	{ 1, 9, -1, -1 },
	{ 2, 8, -1, -1 },
	{ 3, 7, -1, -1 },
	{ 4, 6, -1, -1 },
	{ 5, 5, -1, -1 }
};

// Closes the lists when a panel that is not one of them is clicked
class CSpectatorClickAwaySignal : public vgui::CDefaultInputSignal
{
public:
	CSpectatorClickAwaySignal( CDoDSpectatorGUI *pGUI ) : m_pGUI( pGUI ) {}

	virtual void mousePressed( vgui::MouseCode code, vgui::Panel *panel )
	{
		m_pGUI->CloseLists();
	}

private:
	CDoDSpectatorGUI *m_pGUI;
};

// The input of the list
class CSpectatorMenuSignal : public vgui::CDefaultInputSignal
{
public:
	CSpectatorMenuSignal( CDoDSpectatorMenu *pMenu ) : m_pMenu( pMenu ) {}

	virtual void cursorMoved( int x, int y, vgui::Panel *panel )
	{
		m_pMenu->CursorMoved( x, y );
	}

	virtual void cursorExited( vgui::Panel *panel )
	{
		m_pMenu->CursorExited();
	}

	virtual void mousePressed( vgui::MouseCode code, vgui::Panel *panel )
	{
		m_pMenu->MousePressed( code );
	}

	virtual void mouseWheeled( int delta, vgui::Panel *panel )
	{
		m_pMenu->MouseWheeled( delta );
	}

private:
	CDoDSpectatorMenu *m_pMenu;
};

}

CDoDSpectatorMenu::CDoDSpectatorMenu( void ) : Panel( 0, 0, 10, 10 )
{
	m_iNumItems = 0;
	m_iHover = -1;
	m_iScroll = 0;

	setVisible( false );
	addInputSignal( new CSpectatorMenuSignal( this ) );
}

void CDoDSpectatorMenu::Clear( void )
{
	m_iNumItems = 0;
	m_iHover = -1;
	m_iScroll = 0;
}

void CDoDSpectatorMenu::AddItem( const char *pszText, const char *pszCommand, int iType, bool bChecked )
{
	if( m_iNumItems >= LIST_MAX_ITEMS )
		return;

	Item_t &item = m_Items[m_iNumItems++];

	DoD_ConvertText( pszText, item.szText, sizeof( item.szText ) );
	strlcpy( item.szCommand, pszCommand, sizeof( item.szCommand ) );
	item.iType = iType;
	item.bChecked = bChecked;
}

// The items of a command menu file (CCreateCommandComboMenu::CreateCommandMenu of the original): the lines are the key
// that is not used, the text and the command; the keywords before a line are TOGGLE with the state that is set when the list
// is made, TEAMn for a line that only a player of the team has; the lines of submenus are skipped
void CDoDSpectatorMenu::LoadFile( const char *pszFile )
{
	char token[1024];
	char szText[64];
	char szCommand[128];

	Clear();

	char *pFileStart = (char *)gEngfuncs.COM_LoadFile( pszFile, 5, NULL );

	if( !pFileStart )
	{
		gEngfuncs.Con_DPrintf( "Unable to open %s\n", pszFile );
		return;
	}

	char *pFile = gEngfuncs.COM_ParseFile( pFileStart, token );

	while( pFile && token[0] )
	{
		int iType = ITEM_COMMAND;
		int iTeam = -1;
		bool bChecked = false;

		// a closing brace ends a submenu that was skipped
		if( token[0] == '}' )
		{
			pFile = gEngfuncs.COM_ParseFile( pFile, token );
			continue;
		}

		if( !strcmp( token, "TOGGLE" ) )
		{
			iType = ITEM_TOGGLE;

			pFile = gEngfuncs.COM_ParseFile( pFile, token );
			bChecked = ( strnicmp( token, "true", 4 ) == 0 );

			pFile = gEngfuncs.COM_ParseFile( pFile, token );
		}
		else if( !strncmp( token, "TEAM", 4 ) )
		{
			iTeam = atoi( token + 4 );

			pFile = gEngfuncs.COM_ParseFile( pFile, token );
		}

		// the key is not used, the text and the command follow it
		pFile = gEngfuncs.COM_ParseFile( pFile, token );
		DoD_GetLocalized( token, szText, sizeof( szText ) );

		pFile = gEngfuncs.COM_ParseFile( pFile, token );
		strlcpy( szCommand, token, sizeof( szCommand ) );

		if( szCommand[0] == '{' )
		{
			// a submenu: its lines are skipped up to the closing brace
			int iDepth = 1;

			while( pFile && iDepth > 0 )
			{
				pFile = gEngfuncs.COM_ParseFile( pFile, token );

				if( !token[0] )
					break;

				if( token[0] == '{' )
					iDepth++;
				else if( token[0] == '}' )
					iDepth--;
			}
		}
		else if( iTeam == -1 || iTeam == g_iTeamNumber )
		{
			AddItem( szText, szCommand, iType, bChecked );
		}

		pFile = gEngfuncs.COM_ParseFile( pFile, token );
	}

	gEngfuncs.COM_FreeFile( pFileStart );
}

int CDoDSpectatorMenu::GetItemTall( void )
{
	return DODMENU_SIZE( MENU_ITEM_TALL );
}

int CDoDSpectatorMenu::GetNumVisible( void )
{
	return ( m_iNumItems < (int)LIST_MAX_VISIBLE ) ? m_iNumItems : (int)LIST_MAX_VISIBLE;
}

// The marks show the state of the commands (CommandComboBox::SetMenuCvarChecks of the original): the scoreboard is shown
// by "togglescores", the others switch a cvar with the name of the command and "_internal" at the end
void CDoDSpectatorMenu::UpdateChecks( void )
{
	for( int i = 0; i < m_iNumItems; i++ )
	{
		Item_t &item = m_Items[i];
		char szCvar[96];

		if( item.iType != ITEM_TOGGLE )
			continue;

		if( strstr( item.szCommand, "togglescores" ) )
		{
			item.bChecked = gViewPort->IsScoreBoardVisible();
		}
		else
		{
			snprintf( szCvar, sizeof( szCvar ), "%s_internal", item.szCommand );

			cvar_t *pCvar = gEngfuncs.pfnGetCvarPointer( szCvar );

			// the client changes the value of its cvars directly, the string that the engine returns is not the current one
			item.bChecked = ( pCvar && pCvar->value > 0.0f );
		}
	}
}

// The list is above the box with its bottom a pixel over the top of the box, a long list shows ten items
void CDoDSpectatorMenu::Open( int x, int iBoxTop, int wide )
{
	int tall = GetNumVisible() * GetItemTall() + 2;

	UpdateChecks();

	m_iHover = -1;
	m_iScroll = 0;

	setBounds( x, iBoxTop - 1 - tall, wide, tall );
	setVisible( true );
	repaint();
}

void CDoDSpectatorMenu::Close( void )
{
	m_iHover = -1;

	setVisible( false );
}

int CDoDSpectatorMenu::GetItemAt( int x, int y )
{
	int wide, tall;

	getSize( wide, tall );

	if( x < 1 || x >= wide - 1 || y < 1 || y >= tall - 1 )
		return -1;

	int i = m_iScroll + ( y - 1 ) / GetItemTall();

	if( i >= m_iNumItems )
		return -1;

	return i;
}

void CDoDSpectatorMenu::CursorMoved( int x, int y )
{
	int iHover = GetItemAt( x, y );

	if( iHover != m_iHover )
	{
		m_iHover = iHover;
		repaint();
	}
}

void CDoDSpectatorMenu::CursorExited( void )
{
	m_iHover = -1;
	repaint();
}

void CDoDSpectatorMenu::MouseWheeled( int delta )
{
	int iMax = m_iNumItems - GetNumVisible();

	m_iScroll -= delta;

	if( m_iScroll > iMax )
		m_iScroll = iMax;

	if( m_iScroll < 0 )
		m_iScroll = 0;

	repaint();
}

// The item is activated: the list is closed first, the command of a player is the name that is followed
void CDoDSpectatorMenu::MousePressed( vgui::MouseCode code )
{
	if( code != vgui::MOUSE_LEFT || m_iHover < 0 || m_iHover >= m_iNumItems )
		return;

	char szCommand[128];

	Item_t &item = m_Items[m_iHover];
	int iType = item.iType;

	strlcpy( szCommand, item.szCommand, sizeof( szCommand ) );

	Close();

	if( iType == ITEM_PLAYER )
	{
		gHUD.m_Spectator.FindPlayer( szCommand );
		gViewPort->UpdateSpectatorPanel();
	}
	else
	{
		ClientCmd( szCommand );
	}
}

// WindowBG of the lists is black with the alpha 196 (CBottomBar::ApplySchemeSettings of the original), the border is the one
// of the scheme
void CDoDSpectatorMenu::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	drawSetColor( 0, 0, 0, 255 - 196 );
	drawFilledRect( 0, 0, wide, tall );

	drawSetColor( DODCOLOR_DARK, 0 );
	drawOutlinedRect( 0, 0, wide, tall );
}

// The mark: the pixels of the glyph scaled from the screenshot
void CDoDSpectatorMenu::DrawCheck( int x, int y )
{
	float flScale = (float)DODMENU_SIZE( 1.0f ) * MENU_PIXEL;

	drawSetColor( DODCOLOR_TEXT, 0 );

	for( int i = 0; i < 17; i++ )
	{
		int y0 = y + (int)( i * flScale + 0.5f );
		int y1 = y + (int)( ( i + 1 ) * flScale + 0.5f );

		if( y1 <= y0 )
			y1 = y0 + 1;

		const CheckRow_t &row = s_CheckRows[i];

		if( row.iLeft0 >= 0 )
			drawFilledRect( x + (int)( row.iLeft0 * flScale + 0.5f ), y0, x + (int)( ( row.iLeft1 + 1 ) * flScale + 0.5f ), y1 );

		if( row.iRight0 >= 0 )
			drawFilledRect( x + (int)( row.iRight0 * flScale + 0.5f ), y0, x + (int)( ( row.iRight1 + 1 ) * flScale + 0.5f ), y1 );
	}
}

void CDoDSpectatorMenu::paint( void )
{
	int wide, tall;
	int textWide, textTall;
	vgui::Font *pFont = DoD_GetMenuFont( DODFONT_SCORE );
	int iItemTall = GetItemTall();

	getPaintSize( wide, tall );

	for( int i = 0; i < GetNumVisible(); i++ )
	{
		int iItem = m_iScroll + i;
		int y = 1 + i * iItemTall;

		if( iItem >= m_iNumItems )
			break;

		// SelectionBG of the scheme is under the item that the mouse is over
		if( iItem == m_iHover )
		{
			drawSetColor( 192, 28, 0, 255 - 140 );
			drawFilledRect( 1, y, wide - 1, y + iItemTall );

			// the box of the mark is shown under the mouse for an item that can be checked: the same color over the first one
			if( m_Items[iItem].iType == ITEM_TOGGLE )
			{
				int iBoxX = DODMENU_SIZE( MENU_BOX_X );
				int iBoxY = y + DODMENU_SIZE( MENU_BOX_Y );

				drawFilledRect( iBoxX, iBoxY, iBoxX + DODMENU_SIZE( MENU_BOX_SIZE ), iBoxY + DODMENU_SIZE( MENU_BOX_SIZE ) );
			}
		}

		pFont->getTextSize( m_Items[iItem].szText, textWide, textTall );

		drawSetTextFont( pFont );
		drawSetTextColor( DODCOLOR_TEXT, 0 );
		drawSetTextPos( DODMENU_SIZE( MENU_TEXT_X ), y + ( iItemTall - textTall ) / 2 - DODMENU_SIZE( 0.7f ) );
		drawPrintText( m_Items[iItem].szText, (int)strlen( m_Items[iItem].szText ) );

		if( m_Items[iItem].bChecked )
			DrawCheck( DODMENU_SIZE( MENU_CHECK_X ), y + DODMENU_SIZE( MENU_CHECK_Y ) );
	}
}

//-----------------------------------------------------------------------------
// The frame around the inset window: the lines of the color of the text
//-----------------------------------------------------------------------------
void CDoDSpectatorInsetBox::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	drawSetColor( DODCOLOR_TEXT, 0 );
	drawOutlinedRect( 0, 0, wide, tall );
}

//-----------------------------------------------------------------------------
// The GUI
//-----------------------------------------------------------------------------
CDoDSpectatorGUI::CDoDSpectatorGUI( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
{
	m_bHelpShown = false;
	m_bInsetVisible = false;
	m_iTopBarX = 0;
	m_iLayoutWide = 0;
	m_iLayoutTall = 0;

	// the frame is transparent, the bars are drawn by the panels under the controls (they are created first)
	setPaintBackgroundEnabled( false );

	m_pTopBar = new CTransparentPanel( BAR_TRANSPARENCY, 0, 0, ScreenWidth, DODMENU_SIZE( BAR_TALL ) );
	m_pTopBar->setParent( this );

	m_pBottomBarBlank = new CTransparentPanel( BAR_TRANSPARENCY, 0, 0, ScreenWidth, DODMENU_SIZE( BAR_TALL ) );
	m_pBottomBarBlank->setParent( this );

	// the banner is an image panel without an image until the HLTV server sends one (SetImage)
	m_pBannerImage = new CImageLabel( "", 0, 0, 10, 10 );
	m_pBannerImage->setParent( m_pTopBar );
	m_pBannerImage->setVisible( false );

	m_pInsetBox = new CDoDSpectatorInsetBox;
	m_pInsetBox->setParent( this );
	m_pInsetBox->setVisible( false );

	// the labels of Spectator.res
	m_pAlliesScoreCaption = CreateLabel( vgui::Label::a_east );
	m_pAlliesScoreLabel = CreateLabel( vgui::Label::a_west );
	m_pAxisScoreCaption = CreateLabel( vgui::Label::a_east );
	m_pAxisScoreLabel = CreateLabel( vgui::Label::a_west );
	m_pReinforcementsLabel = CreateLabel( vgui::Label::a_north );
	m_pTimerLabel = CreateLabel( vgui::Label::a_west );
	m_pExtraInfo = CreateLabel( vgui::Label::a_west );
	m_pPlayerLabel = CreateLabel( vgui::Label::a_center );

	char szText[64];

	DoD_GetLocalized( "#Dod_Spec_Allies_Score", szText, sizeof( szText ) );
	SetLabelText( m_pAlliesScoreCaption, szText );

	DoD_GetLocalized( "#Dod_Spec_Axis_Score", szText, sizeof( szText ) );
	SetLabelText( m_pAxisScoreCaption, szText );

	m_pReinforcementsLabel->setVisible( false );
	m_pExtraInfo->setVisible( false );
	m_pPlayerLabel->setVisible( false );

	// the lists of the bottom bar (CBottomBar), the bar is hidden until the key DUCK
	m_pBottomBar = new vgui::Panel( 0, 0, ScreenWidth, DODMENU_SIZE( BAR_TALL ) );
	m_pBottomBar->setParent( this );
	m_pBottomBar->setPaintBackgroundEnabled( false );
	m_pBottomBar->setVisible( false );

	m_pConfigSettings = new CDoDSpectatorButton( CDoDSpectatorButton::BUTTON_COMBO );
	m_pConfigSettings->setParent( m_pBottomBar );
	m_pConfigSettings->addActionSignal( new CSpectatorHandler_Command( this, SPECTATOR_PANEL_CMD_OPTIONS ) );

	m_pPlayerList = new CDoDSpectatorButton( CDoDSpectatorButton::BUTTON_COMBO );
	m_pPlayerList->setParent( m_pBottomBar );
	m_pPlayerList->SetCenterText( true );
	m_pPlayerList->addActionSignal( new CSpectatorHandler_Command( this, SPECTATOR_PANEL_CMD_PLAYERS ) );

	m_pViewOptions = new CDoDSpectatorButton( CDoDSpectatorButton::BUTTON_COMBO );
	m_pViewOptions->setParent( m_pBottomBar );
	m_pViewOptions->addActionSignal( new CSpectatorHandler_Command( this, SPECTATOR_PANEL_CMD_CAMERA ) );

	m_pLeftButton = new CDoDSpectatorButton( CDoDSpectatorButton::BUTTON_LEFT );
	m_pLeftButton->setParent( m_pBottomBar );
	m_pLeftButton->addActionSignal( new CSpectatorHandler_Command( this, SPECTATOR_PANEL_CMD_PREVPLAYER ) );

	m_pRightButton = new CDoDSpectatorButton( CDoDSpectatorButton::BUTTON_RIGHT );
	m_pRightButton->setParent( m_pBottomBar );
	m_pRightButton->addActionSignal( new CSpectatorHandler_Command( this, SPECTATOR_PANEL_CMD_NEXTPLAYER ) );

	// the texts of the lists that do not depend on the player
	DoD_GetLocalized( "#SPECT_OPTIONS", szText, sizeof( szText ) );
	m_pConfigSettings->SetButtonText( szText );

	DoD_GetLocalized( "#CAM_OPTIONS", szText, sizeof( szText ) );
	m_pViewOptions->SetButtonText( szText );

	// the lists are over everything, they are created last
	m_pSettingsMenu = new CDoDSpectatorMenu;
	m_pSettingsMenu->setParent( this );

	m_pPlayerMenu = new CDoDSpectatorMenu;
	m_pPlayerMenu->setParent( this );

	m_pViewMenu = new CDoDSpectatorMenu;
	m_pViewMenu->setParent( this );

	// a click on the bars and on the screen under the lists closes them
	AddClickAway( this );
	AddClickAway( m_pTopBar );
	AddClickAway( m_pBottomBarBlank );
	AddClickAway( m_pBottomBar );

	Layout();
}

void CDoDSpectatorGUI::AddClickAway( vgui::Panel *pPanel )
{
	pPanel->addInputSignal( new CSpectatorClickAwaySignal( this ) );
}

void CDoDSpectatorGUI::CloseLists( void )
{
	m_pSettingsMenu->Close();
	m_pPlayerMenu->Close();
	m_pViewMenu->Close();
}

CDoDSpectatorGUI::~CDoDSpectatorGUI()
{
	/* Nothing. */
}

vgui::Label *CDoDSpectatorGUI::CreateLabel( vgui::Label::Alignment alignment )
{
	vgui::Label *pLabel = new vgui::Label( "", 0, 0, 10, 10 );

	pLabel->setParent( this );
	pLabel->setFont( DoD_GetMenuFont( DODFONT_SCORE ) );
	pLabel->setPaintBackgroundEnabled( false );
	pLabel->setFgColor( DODCOLOR_TEXT, 0 );
	pLabel->setContentAlignment( alignment );

	return pLabel;
}

// The text of a label is set after its font, the size of the text of VGUI1 is made when it is set
void CDoDSpectatorGUI::SetLabelText( vgui::Label *pLabel, const char *pszText )
{
	char szText[256];

	DoD_ConvertText( pszText, szText, sizeof( szText ) );

	pLabel->setFont( DoD_GetMenuFont( DODFONT_SCORE ) );
	pLabel->setText( "%s", szText );
}

// A label of VGUI2 centers its text vertically and does not clip it, VGUI1 clips it: the box is made as tall as the font
void CDoDSpectatorGUI::PlaceLabel( vgui::Label *pLabel, int x, int y, int wide, int tall, bool bCenter )
{
	int iFontTall = DoD_GetMenuFont( DODFONT_SCORE )->getTall() + 4;

	if( bCenter )
	{
		y += ( tall - iFontTall ) / 2;
		tall = iFontTall;
	}

	pLabel->setBounds( x, y, wide, tall );
}

// The positions of Spectator.res and BottomSpectator.res for the current size of the screen
void CDoDSpectatorGUI::Layout( void )
{
	int iBar = DODMENU_SIZE( BAR_TALL );
	int iBarY = ScreenHeight - iBar;

	m_iLayoutWide = ScreenWidth;
	m_iLayoutTall = ScreenHeight;

	setBounds( 0, 0, ScreenWidth, ScreenHeight );

	m_pTopBar->setBounds( m_iTopBarX, 0, ScreenWidth, iBar );
	m_pBottomBarBlank->setBounds( 0, iBarY, ScreenWidth, iBar );
	m_pBottomBar->setBounds( 0, iBarY, ScreenWidth, iBar );

	PlaceLabel( m_pAlliesScoreCaption, ScreenWidth - DODMENU_SIZE( SCORE_CAPTION_RIGHT ), DODMENU_SIZE( ALLIES_Y ), DODMENU_SIZE( SCORE_CAPTION_WIDE ), DODMENU_SIZE( SCORE_TALL ), true );
	PlaceLabel( m_pAlliesScoreLabel, ScreenWidth - DODMENU_SIZE( SCORE_VALUE_RIGHT ), DODMENU_SIZE( ALLIES_Y ), DODMENU_SIZE( SCORE_VALUE_WIDE ), DODMENU_SIZE( SCORE_TALL ), true );
	PlaceLabel( m_pAxisScoreCaption, ScreenWidth - DODMENU_SIZE( SCORE_CAPTION_RIGHT ), DODMENU_SIZE( AXIS_Y ), DODMENU_SIZE( SCORE_CAPTION_WIDE ), DODMENU_SIZE( SCORE_TALL ), true );
	PlaceLabel( m_pAxisScoreLabel, ScreenWidth - DODMENU_SIZE( SCORE_VALUE_RIGHT ), DODMENU_SIZE( AXIS_Y ), DODMENU_SIZE( SCORE_VALUE_WIDE ), DODMENU_SIZE( SCORE_TALL ), true );
	PlaceLabel( m_pReinforcementsLabel, DODMENU_SIZE( REINFORCEMENTS_X ), DODMENU_SIZE( REINFORCEMENTS_Y ), DODMENU_SIZE( REINFORCEMENTS_WIDE ), DODMENU_SIZE( REINFORCEMENTS_TALL ), false );
	PlaceLabel( m_pTimerLabel, ScreenWidth - DODMENU_SIZE( TIMER_RIGHT ), DODMENU_SIZE( TIMER_Y ), DODMENU_SIZE( TIMER_WIDE ), DODMENU_SIZE( SCORE_TALL ), true );
	PlaceLabel( m_pExtraInfo, ScreenWidth - DODMENU_SIZE( EXTRAINFO_RIGHT ), DODMENU_SIZE( EXTRAINFO_Y ), DODMENU_SIZE( EXTRAINFO_WIDE ), DODMENU_SIZE( SCORE_TALL ), true );
	PlaceLabel( m_pPlayerLabel, ScreenWidth / 2 - DODMENU_SIZE( PLAYERLABEL_CENTER ), DODMENU_SIZE( PLAYERLABEL_Y ), DODMENU_SIZE( PLAYERLABEL_WIDE ), DODMENU_SIZE( PLAYERLABEL_TALL ), true );

	m_pConfigSettings->setBounds( DODMENU_SIZE( SETTINGS_X ), DODMENU_SIZE( CONTROL_Y ), DODMENU_SIZE( SETTINGS_WIDE ), DODMENU_SIZE( CONTROL_TALL ) );
	m_pLeftButton->setBounds( DODMENU_SIZE( PLAYERPREV_X ), DODMENU_SIZE( CONTROL_Y ), DODMENU_SIZE( ARROW_WIDE ), DODMENU_SIZE( CONTROL_TALL ) );
	m_pPlayerList->setBounds( DODMENU_SIZE( PLAYERCOMBO_X ), DODMENU_SIZE( CONTROL_Y ), DODMENU_SIZE( PLAYERCOMBO_WIDE ), DODMENU_SIZE( CONTROL_TALL ) );
	m_pRightButton->setBounds( DODMENU_SIZE( PLAYERNEXT_X ), DODMENU_SIZE( CONTROL_Y ), DODMENU_SIZE( ARROW_WIDE ), DODMENU_SIZE( CONTROL_TALL ) );
	m_pViewOptions->setBounds( DODMENU_SIZE( VIEWCOMBO_X ), DODMENU_SIZE( CONTROL_Y ), DODMENU_SIZE( VIEWCOMBO_WIDE ), DODMENU_SIZE( CONTROL_TALL ) );
}

// OnThink of the original: the layout is made again when the size of the screen changes
void CDoDSpectatorGUI::CheckLayout( void )
{
	if( m_iLayoutWide != ScreenWidth || m_iLayoutTall != ScreenHeight )
		Layout();
}

int CDoDSpectatorGUI::GetTopBarHeight( void )
{
	return DODMENU_SIZE( BAR_TALL );
}

int CDoDSpectatorGUI::GetBottomBarHeight( void )
{
	return DODMENU_SIZE( BAR_TALL );
}

bool CDoDSpectatorGUI::IsActiveGUIVisible( void )
{
	return m_pBottomBar->isVisible();
}

void CDoDSpectatorGUI::SetPlayerText( const char *pszText )
{
	m_pPlayerList->SetButtonText( pszText );
}

void CDoDSpectatorGUI::SetViewText( const char *pszText )
{
	char szText[64];

	DoD_GetLocalized( pszText, szText, sizeof( szText ) );

	m_pViewOptions->SetButtonText( szText );
}

//-----------------------------------------------------------------------------
// The states of the GUI
//-----------------------------------------------------------------------------
// The GUI with the label of the player in the bottom bar
void CDoDSpectatorGUI::Activate( void )
{
	CheckLayout();

	m_bHelpShown = false;

	setVisible( true );

	m_pPlayerLabel->setVisible( true );
	SetLabelText( m_pPlayerLabel, "" );

	m_pBottomBar->setVisible( false );
	m_pBannerImage->setVisible( true );

	if( gHUD.m_Spectator.m_pip->value != INSET_OFF )
		EnableInsetView( 1 );

	gViewPort->UpdateCursorState();
}

// The bottom bar with the lists, the mouse is used
void CDoDSpectatorGUI::ShowBottomBar( void )
{
	CheckLayout();

	setVisible( true );
	m_pBottomBar->setVisible( true );

	UpdateSpectatorPlayerList();

	m_pPlayerLabel->setVisible( false );
	SetPlayerText( "" );

	gViewPort->UpdateCursorState();
}

void CDoDSpectatorGUI::HideGUI( void )
{
	DeactivateGUI();

	setVisible( false );

	m_pPlayerLabel->setVisible( true );
	SetLabelText( m_pPlayerLabel, "" );

	m_pInsetBox->setVisible( false );
	m_pBottomBar->setVisible( false );

	SetImage( NULL );
}

// Back to the label of the player
void CDoDSpectatorGUI::DeactivateGUI( void )
{
	m_pPlayerLabel->setVisible( true );
	SetPlayerText( "" );

	m_pInsetBox->setVisible( false );
	m_pBottomBar->setVisible( false );

	CloseLists();
	gViewPort->UpdateCursorState();
}

// The key DUCK closes the bottom bar and gives the label back
void CDoDSpectatorGUI::OnDuckKey( void )
{
	m_pBottomBar->setVisible( false );

	if( m_bInsetVisible )
		m_pInsetBox->setVisible( true );

	m_pPlayerLabel->setVisible( true );

	CloseLists();
	gViewPort->UpdateCursorState();
}

// The frame of the inset window and the top bar next to it
void CDoDSpectatorGUI::EnableInsetView( int iValue )
{
	if( iValue )
	{
		int x = DODMENU_SIZE( gHUD.m_Spectator.m_OverviewData.insetWindowX );
		int y = DODMENU_SIZE( gHUD.m_Spectator.m_OverviewData.insetWindowY );
		int wide = DODMENU_SIZE( gHUD.m_Spectator.m_OverviewData.insetWindowWidth );
		int tall = DODMENU_SIZE( gHUD.m_Spectator.m_OverviewData.insetWindowHeight );

		m_pInsetBox->setBounds( x, y, wide, tall );
		m_iTopBarX = x + wide + 2;
		m_pTopBar->setPos( m_iTopBarX, 0 );
		m_pInsetBox->setVisible( true );

		if( IsActiveGUIVisible() )
			m_bInsetVisible = true;
	}
	else
	{
		m_pInsetBox->setVisible( false );

		if( IsActiveGUIVisible() )
			m_bInsetVisible = false;

		m_iTopBarX = 0;
		m_pTopBar->setPos( 0, 0 );
	}
}

// The banner of the HLTV server (the image of the director without the extension)
void CDoDSpectatorGUI::SetImage( const char *pszImage )
{
	if( pszImage && pszImage[0] )
	{
		m_pBannerImage->LoadImage( pszImage );
		m_pBannerImage->setPos( 0, 0 );
		m_pBannerImage->setVisible( true );
	}
	else
	{
		m_pBannerImage->setVisible( false );
	}
}

//-----------------------------------------------------------------------------
// The content of the GUI
//-----------------------------------------------------------------------------
// The player cannot be spectated by a player of another team (the force camera of the server): a spectator and
// the one that is not in a team can see everybody
bool CDoDSpectatorGUI::CanSpectateTeam( int iTeam )
{
	bool bRetVal = true;
	int iTeamOnly = gDoDViewPortInterface->GetForceCamera();

	if( g_iTeamNumber && !gEngfuncs.IsSpectateOnly() && iTeamOnly )
		bRetVal = ( g_iTeamNumber == iTeam );

	return bRetVal;
}

// The name of the player with the health: "Name (100)", "Name" for a dead player
void CDoDSpectatorGUI::GetPlayerLabelText( int iPlayerNum, char *pszText, int iTextLen )
{
	char szPlayerName[64];
	char szHealth[16];

	pszText[0] = '\0';

	if( gHUD.m_StatusBar.InDeathCamMode() )
		iPlayerNum = g_iUser3;

	if( iPlayerNum > 0 )
	{
		hud_player_info_t &info = g_PlayerInfoList[iPlayerNum];
		int iHealth = g_PlayerExtraInfo[iPlayerNum].health;

		gEngfuncs.pfnGetPlayerInfo( iPlayerNum, &info );

		if( !info.name )
			return;

		DoD_ConvertText( info.name, szPlayerName, sizeof( szPlayerName ) );

		if( iHealth <= 0 )
		{
			char szFormat[64];

			DoD_GetLocalized( "#Spec_PlayerItem", szFormat, sizeof( szFormat ) );
			DoD_ConstructLocalized( pszText, iTextLen, szFormat, szPlayerName, "", "", "" );
			return;
		}

		snprintf( szHealth, sizeof( szHealth ), "%d", iHealth );
	}
	else
	{
		// the player that is under the crosshair
		char *pszName = gHUD.m_StatusBar.GetTargetName();

		if( !pszName || !*pszName )
			return;

		snprintf( szHealth, sizeof( szHealth ), "%d", gHUD.m_StatusBar.GetTargetHealth() );
		DoD_ConvertText( pszName, szPlayerName, sizeof( szPlayerName ) );
	}

	char szFormat[64];

	DoD_GetLocalized( "#Spec_PlayerItem_Team", szFormat, sizeof( szFormat ) );
	DoD_ConstructLocalized( pszText, iTextLen, szFormat, szPlayerName, szHealth, "", "" );
}

// The color of the player, the color of the team of the target of the status bar when there is no player
void CDoDSpectatorGUI::GetPlayerLabelColor( int iPlayerNum, int &r, int &g, int &b )
{
	float *pColor;

	if( gHUD.m_StatusBar.InDeathCamMode() )
		iPlayerNum = g_iUser3;

	if( iPlayerNum <= 0 )
		pColor = GetTeamColor( gHUD.m_StatusBar.GetTargetTeam() );
	else
		pColor = GetClientColor( iPlayerNum );

	r = (int)( pColor[0] * 255.0f );
	g = (int)( pColor[1] * 255.0f );
	b = (int)( pColor[2] * 255.0f );
}

// CSpectatorGUI::Update: the text of the lists and of the label of the player, the number of spectators or the map
void CDoDSpectatorGUI::Update( const char *pszPlayer, int iPlayerNum, int iUser1, bool bSpectateOnly, int iSpecNum, const char *pszMapName )
{
	char szPlayerText[80];
	char szString[256];
	int r = 0, g = 0, b = 0;

	CheckLayout();

	UpdateSpectatorPlayerList();

	SetViewText( GetSpectatorLabel( iUser1 ) );

	// the message about the key DUCK is shown once, when the GUI is up
	if( !m_bHelpShown && !IsActiveGUIVisible() && iUser1 )
	{
		m_bHelpShown = true;

		DuckMessage( "#Spec_Duck" );
	}

	GetPlayerLabelColor( iPlayerNum, r, g, b );
	m_pPlayerList->SetButtonColor( r, g, b );
	m_pPlayerLabel->setFgColor( r, g, b, 0 );

	GetPlayerLabelText( iPlayerNum, szPlayerText, sizeof( szPlayerText ) );
	SetPlayerText( szPlayerText );
	SetLabelText( m_pPlayerLabel, szPlayerText );

	if( bSpectateOnly )
	{
		char szNumber[16];
		char szFormat[64];

		snprintf( szNumber, sizeof( szNumber ), "%i", iSpecNum );
		DoD_GetLocalized( "#Spectators", szFormat, sizeof( szFormat ) );
		snprintf( szString, sizeof( szString ), "%s %s", szFormat, szNumber );
	}
	else
	{
		char szFormat[64];

		DoD_GetLocalized( "#Spec_Map", szFormat, sizeof( szFormat ) );
		DoD_ConstructLocalized( szString, sizeof( szString ), szFormat, pszMapName, "", "", "" );
	}

	SetLabelText( m_pExtraInfo, szString );

	m_pExtraInfo->setVisible( bSpectateOnly );
}

// The list of the players that can be followed (UpdateSpectatorPlayerList of CSpectatorGUI of the original, it is made while
// the bottom bar is shown): the player himself, a dead one and a player of another team under the force camera are not in
// it; the item is the name with the name of the team, the command of it is the name that is followed
void CDoDSpectatorGUI::UpdatePlayerMenu( void )
{
	gHUD.GetAllPlayersInfo();

	m_pPlayerMenu->Clear();

	for( int i = 1; i < gEngfuncs.GetMaxClients() + 1 && i < MAX_PLAYERS; i++ )
	{
		hud_player_info_t &info = g_PlayerInfoList[i];

		if( !info.name || info.thisplayer || g_PlayerExtraInfo[i].dead )
			continue;

		if( !CanSpectateTeam( g_PlayerExtraInfo[i].teamnumber ) )
			continue;

		// the team name is not always a token
		char szName[64];
		char szTeam[64];
		char szToken[64];
		char szFormat[64];
		char szText[80];

		DoD_ConvertText( info.name, szName, sizeof( szName ) );

		// the name of the team that the server sends has blanks and control characters after it
		char szTeamName[MAX_TEAM_NAME];
		int iLength;

		strlcpy( szTeamName, g_PlayerExtraInfo[i].teamname, sizeof( szTeamName ) );

		for( iLength = (int)strlen( szTeamName ); iLength > 0 && (unsigned char)szTeamName[iLength - 1] <= ' '; iLength-- )
			szTeamName[iLength - 1] = 0;

		if( szTeamName[0] )
		{
			snprintf( szToken, sizeof( szToken ), "#%s", szTeamName );
			DoD_GetLocalized( szToken, szTeam, sizeof( szTeam ) );
			DoD_GetLocalized( "#Spec_PlayerItem_Team", szFormat, sizeof( szFormat ) );
			DoD_ConstructLocalized( szText, sizeof( szText ), szFormat, szName, szTeam, "", "" );
		}
		else
		{
			DoD_GetLocalized( "#Spec_PlayerItem", szFormat, sizeof( szFormat ) );
			DoD_ConstructLocalized( szText, sizeof( szText ), szFormat, szName, "", "", "" );
		}

		m_pPlayerMenu->AddItem( szText, info.name, CDoDSpectatorMenu::ITEM_PLAYER, false );
	}
}

// The list of a box opens, the one that is open is closed and a box that is clicked again closes its list
void CDoDSpectatorGUI::OpenList( CDoDSpectatorMenu *pMenu, CDoDSpectatorButton *pBox )
{
	bool bWasOpen = pMenu->IsOpen();
	int x, y, wide, tall;

	CloseLists();

	if( bWasOpen )
		return;

	if( pMenu == m_pPlayerMenu )
		UpdatePlayerMenu();
	else if( pMenu == m_pSettingsMenu )
		pMenu->LoadFile( "spectatormenu.txt" );
	else
		pMenu->LoadFile( "spectcammenu.txt" );

	pBox->getPos( x, y );
	pBox->getSize( wide, tall );

	// the box is a child of the bottom bar
	int iBarX, iBarY;

	m_pBottomBar->getPos( iBarX, iBarY );

	pMenu->Open( iBarX + x, iBarY + y, wide );
}

// The scores of the teams
void CDoDSpectatorGUI::UpdateSpectatorPlayerList( void )
{
	char szScore[16];
	int iAlliesScore = 0;
	int iAxisScore = 0;

	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		VGuiLibraryTeamInfo_t teamInfo = gDoDViewPortInterface->GetPlayerTeamInfo( i );

		if( teamInfo.teamnumber == 1 )
			iAlliesScore = gDoDViewPortInterface->GetExtraScoreBoardInfo( i ).score;
		else if( teamInfo.teamnumber == 2 )
			iAxisScore = gDoDViewPortInterface->GetExtraScoreBoardInfo( i ).score;
	}

	snprintf( szScore, sizeof( szScore ), "%d", iAlliesScore );
	SetLabelText( m_pAlliesScoreLabel, szScore );

	snprintf( szScore, sizeof( szScore ), "%d", iAxisScore );
	SetLabelText( m_pAxisScoreLabel, szScore );
}

// The time to the next wave of the reinforcements, the label is hidden when there is no wave
void CDoDSpectatorGUI::UpdateTimer( void )
{
	if( gEngfuncs.IsSpectateOnly() )
	{
		SetLabelText( m_pTimerLabel, "" );
		SetLabelText( m_pReinforcementsLabel, " " );
	}
	else
	{
		SetLabelText( m_pTimerLabel, " " );

		float flTime = gHUD.m_ObjectiveIcons.GetWaveTime() + 1.0f;
		int iTimer = (int)( (float)(int)flTime - gHUD.m_flTime );

		if( iTimer < 0 )
		{
			m_pReinforcementsLabel->setVisible( false );
		}
		else
		{
			char szTime[16];
			char szFormat[64];
			char szText[128];

			m_pReinforcementsLabel->setVisible( true );

			snprintf( szTime, sizeof( szTime ), "%d:%02d", iTimer / 60, iTimer % 60 );
			DoD_GetLocalized( "#Dod_Reinforcements_in", szFormat, sizeof( szFormat ) );
			DoD_ConstructLocalized( szText, sizeof( szText ), szFormat, szTime, "", "", "" );

			SetLabelText( m_pReinforcementsLabel, szText );
		}
	}
}

//-----------------------------------------------------------------------------
// The buttons of the bottom bar
//-----------------------------------------------------------------------------
void CDoDSpectatorGUI::ActionSignal( int cmd )
{
	switch( cmd )
	{
	case SPECTATOR_PANEL_CMD_NONE:
		break;
	case SPECTATOR_PANEL_CMD_OPTIONS:
		OpenList( m_pSettingsMenu, m_pConfigSettings );
		break;
	case SPECTATOR_PANEL_CMD_NEXTPLAYER:
		gHUD.m_Spectator.FindNextPlayer( true );
		break;
	case SPECTATOR_PANEL_CMD_PREVPLAYER:
		gHUD.m_Spectator.FindNextPlayer( false );
		break;
	case SPECTATOR_PANEL_CMD_PLAYERS:
		OpenList( m_pPlayerMenu, m_pPlayerList );
		break;
	case SPECTATOR_PANEL_CMD_CAMERA:
		OpenList( m_pViewMenu, m_pViewOptions );
		break;
	default:
		gEngfuncs.Con_DPrintf( "Unknown CDoDSpectatorGUI ActionSignal %i.\n", cmd );
		break;
	}
}
