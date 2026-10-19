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
// hud_vgui2print.cpp
//
// implementation of the CHudVGUI2Print class
//
// The original draws localized text with VGUI2 fonts (vgui2::surface() and vgui2::localize()), which Xash3D FWGS
// does not have. The tokens come from DoD_FindLocalized() and the text is drawn with the engine console font.
//

#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "vgui/dod_localize.h"

int CHudVGUI2Print::Init( void )
{
	gHUD.AddHudElem( this );

	m_iFlags |= HUD_ACTIVE;

	// the game uses the tokens of the base game too (Spec_PlayerItem of the spectator GUI is only in valve_english.txt)
	DoD_LoadLocalization( "resource/valve_english.txt" );
	DoD_LoadLocalization( "resource/dod_english.txt", true );

	return 1;
}

int CHudVGUI2Print::VidInit( void )
{
	m_flVGUI2StringTime = 0.0f;

	return 1;
}

int CHudVGUI2Print::DrawVGUI2String( char *charMsg, int x, int y, float r, float g, float b )
{
	const char *msg = DoD_FindLocalized( charMsg );

	if( !msg )
		msg = charMsg;

	return DrawLocalizedString( msg, x, y, r, g, b );
}

int CHudVGUI2Print::DrawLocalizedString( const char *msg, int x, int y, float r, float g, float b )
{
	int w, h;

	gEngfuncs.pfnDrawConsoleStringLen( msg, &w, &h );

	if( x == -1 )
		x = ( ScreenWidth - w ) / 2;

	if( y == -1 )
		y = ( ScreenHeight - h ) / 2;

	// shadow
	gEngfuncs.pfnDrawSetTextColor( 0, 0, 0 );
	gEngfuncs.pfnDrawConsoleString( x + 1, y + 1, msg );

	gEngfuncs.pfnDrawSetTextColor( r, g, b );
	gEngfuncs.pfnDrawConsoleString( x, y, msg );

	return x + w;
}

int CHudVGUI2Print::DrawVGUI2StringReverse( char *charMsg, int x, int y, float r, float g, float b )
{
	const char *msg = DoD_FindLocalized( charMsg );

	if( !msg )
		msg = charMsg;

	return DrawLocalizedStringReverse( msg, x, y, r, g, b );
}

int CHudVGUI2Print::DrawLocalizedStringReverse( const char *msg, int x, int y, float r, float g, float b )
{
	int w, h;

	gEngfuncs.pfnDrawConsoleStringLen( msg, &w, &h );

	if( y == -1 )
		y = ( ScreenHeight - h ) / 2;

	x -= w;

	gEngfuncs.pfnDrawSetTextColor( r, g, b );
	gEngfuncs.pfnDrawConsoleString( x, y, msg );

	return x;
}

void CHudVGUI2Print::VGUI2HudPrintArgs( char *charMsg, char *sstr1, char *sstr2, char *sstr3, char *sstr4, int x, int y, float r, float g, float b )
{
	const char *msg = DoD_FindLocalized( charMsg );

	m_iX = x;
	m_iY = y;
	m_fR = r;
	m_fG = g;
	m_fB = b;
	m_flVGUI2StringTime = gEngfuncs.GetClientTime() + 4.0f;

	if( msg )
		DoD_ConstructLocalized( m_szBuf, sizeof( m_szBuf ), msg, sstr1, sstr2, sstr3, sstr4 );
	else
		strlcpy( m_szBuf, charMsg, sizeof( m_szBuf ) );
}

void CHudVGUI2Print::VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b )
{
	const char *msg = DoD_FindLocalized( charMsg );

	if( msg )
	{
		strlcpy( m_szBuf, msg, sizeof( m_szBuf ) );

		m_iX = x;
		m_iY = y;
		m_fR = r;
		m_fG = g;
		m_fB = b;
		m_flVGUI2StringTime = gEngfuncs.GetClientTime() + 4.0f;
	}
}

int CHudVGUI2Print::Draw( float flTime )
{
	if( m_flVGUI2StringTime > gEngfuncs.GetClientTime() )
		DrawLocalizedString( m_szBuf, m_iX, m_iY, m_fR, m_fG, m_fB );

	return 1;
}

int CHudVGUI2Print::GetHudFontHeight( void )
{
	int w, h;

	gEngfuncs.pfnDrawConsoleStringLen( "A", &w, &h );

	return h;
}

void CHudVGUI2Print::GetStringSize( const char *string, int *width, int *height )
{
	int w, h;

	gEngfuncs.pfnDrawConsoleStringLen( string, &w, &h );

	if( width )
		*width = w;

	if( height )
		*height = h;
}
