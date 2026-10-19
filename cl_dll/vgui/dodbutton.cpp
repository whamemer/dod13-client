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
// dodbutton.cpp
//

#include <string.h>

#include "hud.h"
#include "cl_util.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodmenu.h"
#include "vgui/dodbutton.h"

CDoDButton::CDoDButton( const char *text, int x, int y, int wide, int tall, vgui::Font *font ) : Button( "", x, y, wide, tall )
{
	m_pFont = font;
	m_szCommand[0] = '\0';
	m_bGreyedOut = false;
	m_bPlain = false;
	m_bFocusRect = false;

	// the button is highlighted under the mouse
	addInputSignal( new CHandler_ButtonHighlight( this ) );

	SetButtonText( text );
}

void CDoDButton::SetButtonText( const char *text )
{
	strncpy( m_szText, text, sizeof( m_szText ) - 1 );
	m_szText[sizeof( m_szText ) - 1] = '\0';

	repaint();
}

void CDoDButton::SetButtonFont( vgui::Font *font )
{
	m_pFont = font;

	repaint();
}

void CDoDButton::SetCommand( const char *command )
{
	strncpy( m_szCommand, command, sizeof( m_szCommand ) - 1 );
	m_szCommand[sizeof( m_szCommand ) - 1] = '\0';
}

void CDoDButton::SetPlain( bool bPlain )
{
	m_bPlain = bPlain;

	repaint();
}

void CDoDButton::SetFocusRect( bool bFocusRect )
{
	m_bFocusRect = bFocusRect;

	repaint();
}

void CDoDButton::SetGreyedOut( bool bGreyedOut )
{
	m_bGreyedOut = bGreyedOut;

	repaint();
}

void CDoDButton::paint( void )
{
	int wide, tall;
	int textWide, textTall;

	if( !m_pFont )
		return;

	getPaintSize( wide, tall );
	m_pFont->getTextSize( m_szText, textWide, textTall );

	// the alpha of VGUI1 is the transparency, 0 is opaque: a full class is dim (alpha 80) and red under the mouse
	drawSetTextFont( m_pFont );

	if( m_bGreyedOut )
	{
		if( isArmed() )
			drawSetTextColor( 255, 0, 0, 55 );
		else
			drawSetTextColor( DODCOLOR_BRIGHT, 175 );
	}
	else
	{
		drawSetTextColor( DODCOLOR_BRIGHT, 0 );
	}

	// the text of the original is a bit right of the middle: 2.4 units at the screenshot of 1920x1200
	if( m_bPlain )
		drawSetTextPos( ( wide - textWide ) / 2 + YRES( 2.4f ), ( tall - textTall ) / 2 - YRES( 0.6f ) );
	else
		drawSetTextPos( YRES( 3 ), ( tall - textTall ) / 2 );

	drawPrintText( m_szText, strlen( m_szText ) );

	// the dotted rectangle: the pixels of one of two are black, 3 pixels inside of the edge (at 2.5 pixels of a unit)
	if( m_bFocusRect )
	{
		int iInset = YRES( 1.2f );

		drawSetColor( 0, 0, 0, 0 );

		for( int x = iInset; x < wide - iInset; x += 2 )
		{
			drawFilledRect( x, iInset, x + 1, iInset + 1 );
			drawFilledRect( x, tall - iInset - 1, x + 1, tall - iInset );
		}

		for( int y = iInset; y < tall - iInset; y += 2 )
		{
			drawFilledRect( iInset, y, iInset + 1, y + 1 );
			drawFilledRect( wide - iInset - 1, y, wide - iInset, y + 1 );
		}
	}
}

void CDoDButton::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	if( m_bGreyedOut )
		drawSetColor( DODCOLOR_BUTTON, 175 );
	else if( isArmed() )
		drawSetColor( DODCOLOR_LIGHT, 0 );
	else if( m_bPlain )
		drawSetColor( DODCOLOR_DARK, 0 );
	else
		drawSetColor( DODCOLOR_BUTTON, 0 );

	drawFilledRect( 0, 0, wide, tall );
}
