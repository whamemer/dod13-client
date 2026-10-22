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
// dodtextwindow.cpp
//
// The text window of DoD, see dodtextwindow.h.
//


#include "hud.h"
#include "cl_util.h"
#include "dod_localize.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodtextwindow.h"

namespace
{

// The border of the box: a panel over the others that draws the outline only and takes no input, the text under it
// is clipped to the inside of the box in the original
class CTextOutline : public vgui::Panel
{
public:
	CTextOutline( void ) : Panel( 0, 0, 10, 10 ) {}

	virtual void paintBackground( void ) {}

	virtual void paint( void )
	{
		int wide, tall;

		getPaintSize( wide, tall );

		drawSetColor( DODCOLOR_DARK, 0 );
		drawOutlinedRect( 0, 0, wide, tall );
	}

	// the clicks go to the scroll bar and the text under the border
	virtual vgui::Panel *isWithinTraverse( int x, int y )
	{
		return NULL;
	}
};

}

CDoDTextWindow::CDoDTextWindow( int iRemoveMe, int x, int y, int wide, int tall ) : CMenuPanel( 0, iRemoveMe, x, y, wide, tall )
{
	m_bSpectatorHelp = false;
	m_iLayoutHeight = ScreenHeight;

	m_pPage = new CDoDResPage( 0, 0, ScreenWidth, ScreenHeight );
	m_pPage->setParent( this );

	if( !m_pPage->LoadControls( "Resource/UI/TextWindow.res" ) )
		gEngfuncs.Con_Printf( "Missing the layout of the text window: Resource/UI/TextWindow.res\n" );

	m_pMessage = m_pPage->FindControl( "Message" );
	m_pOK = m_pPage->FindControl( "ok" );

	// the outline is created after the page: it is painted over it
	m_pOutline = new CTextOutline;
	m_pOutline->setParent( this );

	// the font of the text is Default of the scheme
	if( m_pMessage )
		m_pMessage->iFont = DODFONT_SCORE;

	if( m_pOK && m_pOK->pPanel )
	{
		const char *pszOK = DoD_FindLocalized( "#PropertyDialog_OK" );

		// the token is in the localization of the platform of VGUI2, it is not in the files of the game
		static_cast<CDoDButton *>( m_pOK->pPanel )->SetButtonText( pszOK ? pszOK : "OK" );

		// it is the default button of the layout, it has the focus
		static_cast<CDoDButton *>( m_pOK->pPanel )->SetFocusRect( true );

		// the button closes the window (OnCommand "okay" of the original)
		static_cast<CDoDButton *>( m_pOK->pPanel )->addActionSignal( new CMenuHandler_TextWindow( HIDE_TEXTWINDOW ) );
	}

	Layout();
}

// The fonts and the sizes depend on the height of the screen
void CDoDTextWindow::Layout( void )
{
	m_iLayoutHeight = ScreenHeight;

	setBounds( 0, 0, ScreenWidth, ScreenHeight );

	// the text starts 3.2 units from the border of the box (8 pixels at the screenshot of 1920x1200)
	if( m_pMessage && m_pMessage->pPanel )
	{
		CDoDRichText *pText = static_cast<CDoDRichText *>( m_pMessage->pPanel );

		pText->SetRichInset( DODMENU_SIZE( 3.2f ) );

		// the lines of the original are 12 units apart (30 pixels at the screenshot of 1920x1200)
		pText->SetRichLineTall( DODMENU_SIZE( 12 ) );
	}

	m_pPage->setBounds( 0, 0, ScreenWidth, ScreenHeight );
	m_pPage->Layout();

	ApplyMessage();

	if( m_pMessage )
		m_pOutline->setBounds( DODMENU_SIZE( m_pMessage->x ), DODMENU_SIZE( m_pMessage->y ), DODMENU_SIZE( m_pMessage->wide ), DODMENU_SIZE( m_pMessage->tall ) );
}

// The text of a control of a layout is short, the message is set to the box itself
void CDoDTextWindow::ApplyMessage( void )
{
	if( m_pMessage && m_pMessage->pPanel )
	{
		char szText[2048];

		DoD_ConvertText( m_Message.Get(), szText, sizeof( szText ) );

		static_cast<CDoDRichText *>( m_pMessage->pPanel )->SetRichText( szText );
	}
}

void CDoDTextWindow::Open( void )
{
	if( m_iLayoutHeight != ScreenHeight )
		Layout();

	CMenuPanel::Open();
}

// The frame has no background, the box of the text is black (WindowBG) with the border of the scheme and the scroll bar is
// darker (ControlDarkBG over it)
void CDoDTextWindow::paintBackground( void )
{
	if( !m_pMessage )
		return;

	int x = DODMENU_SIZE( m_pMessage->x );
	int y = DODMENU_SIZE( m_pMessage->y );
	int wide = DODMENU_SIZE( m_pMessage->wide );
	int tall = DODMENU_SIZE( m_pMessage->tall );
	int iBar = DODMENU_SIZE( 21 );

	drawSetColor( 0, 0, 0, 255 - 200 );
	drawFilledRect( x, y, x + wide, y + tall );

	drawSetColor( 0, 0, 0, 255 - 128 );
	drawFilledRect( x + wide - iBar, y, x + wide, y + tall );

}

// CSpecHelpWindow::OnClose of the original: the spectator GUI comes back to the state with the label of the player, as if
// the bottom bar was closed with DUCK
void CDoDTextWindow::Close( void )
{
	CMenuPanel::Close();

	if( m_bSpectatorHelp && ( gEngfuncs.IsSpectateOnly() || g_iUser1 ) )
		gViewPort->ShowSpectatorGUI();
}

void CDoDTextWindow::Activate( const char *pszMessage )
{
	// CR LF are two line breaks in the original, the text of the help is double spaced
	m_Message = pszMessage;
	m_Message.Replace( '\r', '\n' );

	if( m_iLayoutHeight != ScreenHeight )
		Layout();
	else
		ApplyMessage();
}
