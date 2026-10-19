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
// mortarpanel.cpp
//
// The mortar panel of DoD, see mortarpanel.h.
//

#include <string.h>
#include <stdio.h>

#include <VGUI_Font.h>

#include "hud.h"
#include "cl_util.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodmenu.h"
#include "vgui/mortarpanel.h"

// the attack button of the player
extern bool IsAttackPressed( void );

CMortarPanel::CMortarPanel( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
{
	m_bAttack = false;
	m_fAimValue = 0.0;
	m_fAimVelocity = 0.0;
	m_iNumPasses = 0;
	m_iLayoutHeight = ScreenHeight;
	m_szLabel[0] = '\0';

	setVisible( false );

	m_pPage = new CDoDResPage( 0, 0, wide, tall );
	m_pPage->setParent( this );

	if( !m_pPage->LoadControls( "Resource/UI/MortarPanel.res" ) )
		gEngfuncs.Con_Printf( "Missing the layout of the mortar panel: Resource/UI/MortarPanel.res\n" );

	m_pFrame = m_pPage->FindControl( "MortarPanel" );
	m_pLabel = m_pPage->FindControl( "mortarlabel" );

	// the text is drawn by the panel, the label of the layout has the one of the file until the pointer starts
	if( m_pLabel )
	{
		char szText[256];

		DoD_GetLocalized( m_pLabel->szText, szText, sizeof( szText ) );
		DoD_ConvertText( szText, m_szLabel, sizeof( m_szLabel ) );

		if( m_pLabel->pPanel )
			m_pLabel->pPanel->setVisible( false );
	}

	Layout();
}

// The box and the label are in 640x480, they depend on the height of the screen
void CMortarPanel::Layout( void )
{
	m_iLayoutHeight = ScreenHeight;

	if( m_pFrame )
		setBounds( DODMENU_SIZE( m_pFrame->x ), DODMENU_SIZE( m_pFrame->y ), DODMENU_SIZE( m_pFrame->wide ), DODMENU_SIZE( m_pFrame->tall ) );

	int wide, tall;

	getSize( wide, tall );

	m_pPage->setBounds( 0, 0, wide, tall );
	m_pPage->Layout();

	if( m_pLabel && m_pLabel->pPanel )
		m_pLabel->pPanel->setVisible( false );
}

void CMortarPanel::SetLabelText( const char *pszText )
{
	strlcpy( m_szLabel, pszText, sizeof( m_szLabel ) );
}

void CMortarPanel::ShowMortarPanel( void )
{
	if( m_iLayoutHeight != ScreenHeight )
		Layout();

	setVisible( true );
}

void CMortarPanel::HideMortarPanel( void )
{
	m_bAttack = false;

	setVisible( false );
}

// The attack button starts the pointer: it starts at a random place and runs to the farther end of the bar
void CMortarPanel::AttackDown( void )
{
	char szText[64];

	if( m_bAttack )
		return;

	snprintf( szText, sizeof( szText ), "Button Down! %f", m_fAimValue );
	SetLabelText( szText );

	m_fAimValue = (double)gEngfuncs.pfnRandomLong( 0, 1000 );
	m_fAimVelocity = 2500.0;
	m_iNumPasses = 0;

	if( m_fAimValue > 500.0 )
		m_fAimVelocity = -2500.0;

	m_bAttack = true;
}

// The attack button is released: the value of the pointer is the power of the shot, one byte of it goes to the server
void CMortarPanel::AttackUp( void )
{
	char szText[64];
	char szCommand[32];

	if( !m_bAttack )
		return;

	snprintf( szText, sizeof( szText ), "Button UP!   %.0f", m_fAimValue );
	SetLabelText( szText );

	snprintf( szCommand, sizeof( szCommand ), "maim %i", (unsigned char)(int)( m_fAimValue / 1000.0 * 256.0 ) );
	gEngfuncs.pfnServerCmd( szCommand );

	m_bAttack = false;
}

void CMortarPanel::PostMortarValue( float flValue )
{
	if( flValue > 0.0f )
		AttackDown();

	if( flValue < 0.0f )
	{
		m_bAttack = false;
		setVisible( false );
	}
}

// The pointer runs with its velocity, at the ends of the bar it turns back (the first five passes slow it down by a tenth)
void CMortarPanel::OnTick( void )
{
	double frametime = gHUD.m_flTimeDelta;
	bool bAttackPressed = IsAttackPressed();
	char szText[64];

	if( !m_bAttack )
		return;

	if( !bAttackPressed )
	{
		AttackUp();
		return;
	}

	m_fAimValue += frametime * m_fAimVelocity;

	if( m_fAimVelocity > 0.0 && m_fAimValue > 1000.0 )
	{
		m_fAimValue = 2000.0 - m_fAimValue;
		m_fAimVelocity = -m_fAimVelocity;
		m_iNumPasses++;

		if( m_iNumPasses <= 5 )
			m_fAimVelocity *= 0.9;
	}
	else if( m_fAimVelocity < 0.0 && m_fAimValue < 0.0 )
	{
		m_fAimValue = -m_fAimValue;
		m_fAimVelocity = -m_fAimVelocity;

		if( m_iNumPasses <= 5 )
			m_fAimVelocity *= 0.9;
	}

	snprintf( szText, sizeof( szText ), "Button Down! %.0f", m_fAimValue );
	SetLabelText( szText );
}

// HintBackgroundColor of the scheme (0,0,0,128) and the border HintBorder in HintColor (255,255,255,160): the box of the hint panel;
// the alpha of VGUI1 is the transparency, 0 is opaque
void CMortarPanel::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	drawSetColor( 0, 0, 0, 255 - 128 );
	drawFilledRect( 0, 0, wide, tall );

	drawSetColor( 255, 255, 255, 255 - 160 );
	drawOutlinedRect( 0, 0, wide, tall );
}

// The bar is drawn in pixels of the panel, not in the units of the layout (Paint of the original): it is 400 wide at 10,50 with
// the zone of the shot in red (the full red in the middle, the yellow mark in the center of it) and the white pointer over it
void CMortarPanel::paint( void )
{
	int iBarPos = (int)( m_fAimValue / 1000.0 * 400.0 );

	drawSetColor( 0, 0, 255, 255 - 128 );
	drawFilledRect( 10, 50, 410, 70 );

	drawSetColor( 255, 0, 0, 255 - 128 );
	drawFilledRect( 150, 50, 190, 70 );

	drawSetColor( 255, 0, 0, 0 );
	drawFilledRect( 190, 50, 230, 70 );

	drawSetColor( 255, 0, 0, 255 - 128 );
	drawFilledRect( 230, 50, 270, 70 );

	drawSetColor( 255, 255, 0, 0 );
	drawFilledRect( 208, 50, 212, 70 );

	drawSetColor( 255, 255, 255, 0 );
	drawFilledRect( iBarPos + 9, 51, iBarPos + 11, 69 );

	// the text of the label: one line at the left edge of it, in the middle of its height, white
	if( !m_pLabel || !m_szLabel[0] )
		return;

	vgui::Font *pFont = DoD_GetMenuFont( DODFONT_SCORE );
	int textWide, textTall;
	int iPitch = DODMENU_SIZE( 12 );

	pFont->getTextSize( m_szLabel, textWide, textTall );

	drawSetTextFont( pFont );
	drawSetTextColor( 255, 255, 255, 0 );
	drawSetTextPos( DODMENU_SIZE( m_pLabel->x ), DODMENU_SIZE( m_pLabel->y ) + ( DODMENU_SIZE( m_pLabel->tall ) - iPitch ) / 2 + ( iPitch - textTall ) / 4 );
	drawPrintText( m_szLabel, (int)strlen( m_szLabel ) );
}
