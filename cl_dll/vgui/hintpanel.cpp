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
// hintpanel.cpp
//
// The hint panel of DoD, see hintpanel.h.
//


#include <VGUI_Font.h>

#include "hud.h"
#include "cl_util.h"
#include "dod_localize.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodmenu.h"
#include "vgui/hintpanel.h"

CHintPanel::CHintPanel( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
{
	m_fShowTime = -1.0f;
	m_iLayoutHeight = ScreenHeight;

	setVisible( false );

	m_pPage = new CDoDResPage( 0, 0, wide, tall );
	m_pPage->setParent( this );

	if( !m_pPage->LoadControls( "Resource/UI/HintPanel.res" ) )
		gEngfuncs.Con_Printf( "Missing the layout of the hint panel: Resource/UI/HintPanel.res\n" );

	m_pFrame = m_pPage->FindControl( "HintPanel" );
	m_pLabel = m_pPage->FindControl( "hintlabel" );

	// the text is drawn by the panel: the label of the layout has no place for the lines of VGUI2
	if( m_pLabel && m_pLabel->pPanel )
		m_pLabel->pPanel->setVisible( false );

	Layout();
}

// The box and the controls are in 640x480, they depend on the height of the screen
void CHintPanel::Layout( void )
{
	m_iLayoutHeight = ScreenHeight;

	if( m_pFrame )
		setBounds( DODMENU_SIZE( m_pFrame->x ), DODMENU_SIZE( m_pFrame->y ), DODMENU_SIZE( m_pFrame->wide ), DODMENU_SIZE( m_pFrame->tall ) );

	int wide, tall;

	getSize( wide, tall );

	m_pPage->setBounds( 0, 0, wide, tall );
	m_pPage->Layout();

	// the page is laid out again: the hidden label stays hidden
	if( m_pLabel && m_pLabel->pPanel )
		m_pLabel->pPanel->setVisible( false );

	m_Lines.RemoveAll();

	if( m_pLabel && !m_Text.IsEmpty() )
		DoD_WrapText( DoD_GetMenuFont( DODFONT_SCORE ), m_Text.Get(), DODMENU_SIZE( m_pLabel->wide ), m_Lines );
}

void CHintPanel::SetVisibleImagePanel( const char *pszName, bool bState )
{
	ResControl_t *pControl = m_pPage->FindControl( pszName );

	if( pControl && pControl->pPanel )
		pControl->pPanel->setVisible( bState );
}

// The text is localized with the titles of the game, a token that is left is one of the localization of VGUI2 (the label of
// the original does it when the text is set); the panel is shown for 1 second per 20 characters
void CHintPanel::ShowHintMessage( int team, const char *pszText )
{
	char szText[1024];

	const char *pszLocalized = CHudTextMessage::BufferedLocaliseTextString( pszText );

	if( pszLocalized[0] == '#' )
	{
		const char *pszToken = DoD_FindLocalized( pszLocalized );

		if( pszToken )
			pszLocalized = pszToken;
	}

	DoD_ConvertText( pszLocalized, szText, sizeof( szText ) );

	if( m_iLayoutHeight != ScreenHeight )
		Layout();

	SetVisibleImagePanel( "britimage", false );
	SetVisibleImagePanel( "gerimage", false );
	SetVisibleImagePanel( "amerimage", false );

	if( team == 0 )
		SetVisibleImagePanel( "britimage", true );
	else if( team == 2 )
		SetVisibleImagePanel( "gerimage", true );
	else
		SetVisibleImagePanel( "amerimage", true );

	m_Text = szText;

	m_Lines.RemoveAll();

	if( m_pLabel )
		DoD_WrapText( DoD_GetMenuFont( DODFONT_SCORE ), m_Text.Get(), DODMENU_SIZE( m_pLabel->wide ), m_Lines );

	// the characters of the text, not the bytes of the UTF-8
	int iLength = 0;

	for( const char *p = pszLocalized; *p; p++ )
	{
		if( ( *p & 0xC0 ) != 0x80 )
			iLength++;
	}

	m_fShowTime = (float)iLength / 20.0f + gHUD.m_flTime;

	setVisible( true );
	repaint();
}

void CHintPanel::OnTick( void )
{
	if( m_fShowTime > 0.0f && isVisible() && gHUD.m_flTime >= m_fShowTime )
	{
		m_fShowTime = -1.0f;
		setVisible( false );
	}
}

// HintBackgroundColor of the scheme (0,0,0,128) and the border HintBorder in HintColor (255,255,255,160); the alpha of VGUI1 is
// the transparency, 0 is opaque
void CHintPanel::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	drawSetColor( 0, 0, 0, 255 - 128 );
	drawFilledRect( 0, 0, wide, tall );

	drawSetColor( 255, 255, 255, 255 - 160 );
	drawOutlinedRect( 0, 0, wide, tall );
}

// The block of the lines is in the middle of the label of the layout vertically, the lines start at its left edge (the original
// screenshot: the text of the one line starts at the label although its alignment is center); the pitch of the lines is 12 units
// as in the rest of the text of the font Default (30 pixels at 2.5 pixels of a unit)
void CHintPanel::paint( void )
{
	if( !m_pLabel || m_Lines.IsEmpty() )
		return;

	vgui::Font *pFont = DoD_GetMenuFont( DODFONT_SCORE );
	int iPitch = DODMENU_SIZE( 12 );
	int iLabelX = DODMENU_SIZE( m_pLabel->x );
	int iLabelY = DODMENU_SIZE( m_pLabel->y );
	int iLabelWide = DODMENU_SIZE( m_pLabel->wide );
	int iLabelTall = DODMENU_SIZE( m_pLabel->tall );
	int iTop = iLabelY + ( iLabelTall - m_Lines.Count() * iPitch ) / 2;
	int textWide, textTall;

	drawSetTextFont( pFont );

	// the text color of the label is white (SetFgColor of ApplySchemeSettings)
	drawSetTextColor( 255, 255, 255, 0 );

	pFont->getTextSize( "A", textWide, textTall );

	for( int i = 0; i < m_Lines.Count(); i++ )
	{
		if( m_Lines[i].IsEmpty() )
			continue;

		pFont->getTextSize( m_Lines[i].Get(), textWide, textTall );

		drawSetTextPos( iLabelX, iTop + (int)i * iPitch + ( iPitch - textTall ) / 4 );
		drawPrintText( m_Lines[i].Get(), (int)m_Lines[i].Length() );
	}
}
