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
//  hud_vgui2print.cpp - implementation of the CHudVGUI2Print class
//

/*
================================================================================
C HUD VGUI2 PRINT (REFACTORING FOR ANSI CHAR ARCHITECTURE WITHOUT VGUI2)

CLASS DESCRIPTION:
	Originally in Day of Defeat 1.3, 'CHudVGUI2Print' served as a bridge between
	the GoldSrc engine and the VGUI2 subsystem (vgui2::surface, vgui2::localize).
	It handled high-level Unicode text rendering (wchar_t), parsed localization
	tokens (the '#' prefix), and calculated pixel-perfect ABC font widths across
	different screen resolutions.

	Due to the absence of VGUI2 in the current project, this class has been
	specifically refactored to use a flat char model (ANSI ASCII). It now acts
	as a high-level wrapper around the standard low-level console font printing
	functions of the Half-Life SDK (gEngfuncs.pfnDrawConsoleString/Len).

================================================================================
SYSTEM METHODS DOCUMENTATION:

* Init()
	- BEFORE: Registered the element in the HUD hierarchy and cleared VGUI HFont handles.
	- NOW:    Maintains HUD_ACTIVE registration and zeros out local font index values.

* VidInit()
	- BEFORE: Accessed 'ClientScheme.res' via vgui2::scheme() to fetch Unicode
			  fonts: 'HudFontSmall', 'HudFont', and 'HudFontBig'.
	- NOW:    Completely standalone. Initializes the m_Fonts array with fixed
			  character heights of the engine's default HUD font (iCharHeight).

* Draw( float flTime )
	- BEFORE: Retained overlay messages on screen for 4.0 seconds, calling
			  the Unicode version of the renderer for the m_wCharBuf array.
	- NOW:    Calls the rewritten char-based DrawVGUI2String for the byte buffer
			  m_szCharBuf.

================================================================================
RENDERING & GEOMETRY METHODS DOCUMENTATION (ACTUAL CHANGES):

* DrawVGUI2String( char *charMsg, int x, int y, float r, float g, float b )
	- BEFORE: Accepted char*, converted it to wchar_t via vgui2::localize,
			  and sent it to the VGUI surface for rendering.
	- NOW:    Serves as the final rendering endpoint. Dynamically intercepts
			  the 'con_color' CVAR, temporarily overwriting it with RGB byte
			  components (0-255), prints text via pfnDrawConsoleString, and
			  returns the trailing X coordinate (critical for appending
			  the chat message immediately after the colored player name in saytext.cpp).

* DrawVGUI2StringReverse( char *charMsg, int x, int y, float r, float g, float b )
	- BEFORE: Transit converter (char -> wchar_t) for right-aligned VGUI text.
	- NOW:    Calculates total ANSI string width via GetStringSize, shifts
			  the X coordinate to the left by the text size (effectively treating
			  X as the right boundary), and prints right-aligned text using con_color.

* GetStringSize( const char *string, int *width, int *height )
	- BEFORE: Accepted const wchar_t*, looped through vgui2::surface via
			  GetCharABCWidths() for every broad character.
	- NOW:    Accepts const char*. Preserves original DoD cvar-based logic ('cl_hudfont')
			  for selecting small, medium, or large fonts. Measures string
			  dimensions using low-level pfnDrawConsoleStringLen.

* GetHudFontHeight()
	- BEFORE: Queried vgui2::surface()->GetFontTall() for the active HFont.
	- NOW:    Returns the pre-cached font height from VidInit according to the
			  currently active 'cl_hudfont' cvar value.

* VGUI2HudPrint() / VGUI2HudPrintArgs()
	- BEFORE: Invoked vgui2::localize()->ConstructString to parse '#' tokens
			  and substitute parameters into %s1-%s4 placeholders in the Unicode buffer.
	- NOW:    Stripped of all VGUI dependencies. Safely formats ANSI text directly
			  into m_szCharBuf using 'snprintf'.

================================================================================
DEPRECATED WCHAR_T FUNCTIONS REFERENCE:

	To clean up binary space and save memory, the following overloaded methods
	have been completely EXCLUDED from the header and source files:

	1. int DrawVGUI2String( wchar_t *msg, ... );
	2. int DrawVGUI2StringReverse( wchar_t *msg, ... );
	3. vgui2::HFont GetFont( void );
	4. Internal wide character array: wchar_t m_wCharBuf;

	Support for these has been discontinued, as the core chat text buffer
	'g_szLineBuffer' and the entire 'CHudSayText' subsystem have been fully
	ported to standard byte-based char arrays for optimization purposes.
================================================================================
*/

#include "hud.h"

int CHudVGUI2Print::Init( void )
{
	gHUD.AddHudElem( this );

	m_Fonts[0] = 0;
	m_Fonts[1] = 0;
	m_Fonts[2] = 0;

	m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudVGUI2Print::VidInit( void )
{
	m_flVGUI2StringTime = 0.0f;

	m_Fonts[0] = gHUD.m_scrinfo.iCharHeight;
	m_Fonts[1] = gHUD.m_scrinfo.iCharHeight;
	m_Fonts[2] = gHUD.m_scrinfo.iCharHeight;

	return 1;
}

int CHudVGUI2Print::DrawVGUI2String( char *charMsg, int x, int y, float r, float g, float b )
{
	if( !charMsg || !*charMsg )
		return x;

	int iStringWidth = 0;
	int iDummyHeight = 0;
	GetStringSize( charMsg, &iStringWidth, &iDummyHeight );

	cvar_t *pConColor = gEngfuncs.pfnGetCvarPointer( "con_color" );
	char szOldColor[32] = { 0 };

	if( pConColor && pConColor->string )
		strncpy( szOldColor, pConColor->string, sizeof( szOldColor ) - 1 );

	char szNewColor[32];

	snprintf( szNewColor, sizeof( szNewColor ), "%i %i %i",
		( int ) ( r * 255.0f ),
		( int ) ( g * 255.0f ),
		( int ) ( b * 255.0f ) );

	gEngfuncs.Cvar_Set( "con_color", szNewColor );
	gEngfuncs.pfnDrawConsoleString( x, y, charMsg );

	if( pConColor && szOldColor[0] != '\0' )
		gEngfuncs.Cvar_Set( "con_color", szOldColor );

	return x + iStringWidth;
}

int CHudVGUI2Print::DrawVGUI2StringReverse( char *charMsg, int x, int y, float r, float g, float b )
{
	if( !charMsg || !*charMsg )
		return x;

	int iStringWidth = 0;
	int iDummyHeight = 0;
	GetStringSize( charMsg, &iStringWidth, &iDummyHeight );

	int iAlignedX = x - iStringWidth;

	cvar_t *pConColor = gEngfuncs.pfnGetCvarPointer( "con_color" );
	char szOldColor[32] = { 0 };

	if( pConColor && pConColor->string )
		strncpy( szOldColor, pConColor->string, sizeof( szOldColor ) - 1 );

	char szNewColor[32];

	snprintf( szNewColor, sizeof( szNewColor ), "%i %i %i",
		( int ) ( r * 255.0f ),
		( int ) ( g * 255.0f ),
		( int ) ( b * 255.0f ) );
	gEngfuncs.Cvar_Set( "con_color", szNewColor );
	gEngfuncs.pfnDrawConsoleString( iAlignedX, y, charMsg );

	if( pConColor && szOldColor[0] != '\0' )
		gEngfuncs.Cvar_Set( "con_color", szOldColor );

	return iAlignedX;
}

void CHudVGUI2Print::VGUI2HudPrintArgs( char *charMsg, char *sstr1, char *sstr2, char *sstr3, char *sstr4,
										int x, int y, float r, float g, float b )
{
	if( !charMsg || !*charMsg )
		return;

	m_iX = x;
	m_iY = y;
	m_fR = r;
	m_fG = g;
	m_fB = b;

	m_flVGUI2StringTime = gEngfuncs.GetClientTime() + 4.0f;

	if( sstr1 && *sstr1 )
		snprintf( m_szCharBuf, sizeof( m_szCharBuf ), charMsg, sstr1, sstr2, sstr3, sstr4 );
	else
		strncpy( m_szCharBuf, charMsg, sizeof( m_szCharBuf ) - 1 );

	m_szCharBuf[sizeof( m_szCharBuf ) - 1] = '\0';
}

void CHudVGUI2Print::VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b )
{
	if( !charMsg || !*charMsg )
		return;

	strncpy( m_szCharBuf, charMsg, sizeof( m_szCharBuf ) - 1 );
	m_szCharBuf[sizeof( m_szCharBuf ) - 1] = '\0';

	m_iX = x;
	m_iY = y;
	m_fR = r;
	m_fG = g;
	m_fB = b;

	m_flVGUI2StringTime = gEngfuncs.GetClientTime() + 4.0f;
}

int CHudVGUI2Print::Draw( float flTime )
{
	if( m_flVGUI2StringTime > gEngfuncs.GetClientTime() )
		DrawVGUI2String( m_szCharBuf, m_iX, m_iY, m_fR, m_fG, m_fB );

	return 1;
}
int CHudVGUI2Print::GetHudFontHeight( void )
{
	int value = ( int ) gHUD.cl_hudfont->value;
	unsigned long h = 0;

	if( value )
	{
		if( value == 2 )
			h = m_Fonts[2];
		else
			h = m_Fonts[1];
	}
	else
	{
		h = m_Fonts[0];
	}

	if( h )
		return ( int ) h;

	return gHUD.m_scrinfo.iCharHeight;
}


void CHudVGUI2Print::GetStringSize( const char *string, int *width, int *height )
{
	if( width )  *width = 0;
	if( height ) *height = 0;

	if( !string || !*string )
		return;

	int value = ( int ) gHUD.cl_hudfont->value;
	unsigned long hudfont = 0;

	if( value )
	{
		if( value == 2 )
			hudfont = m_Fonts[2];
		else
			hudfont = m_Fonts[1];
	}
	else
	{
		hudfont = m_Fonts[0];
	}

	int iTempWidth = 0;
	int iTempHeight = 0;

	gEngfuncs.pfnDrawConsoleStringLen( string, &iTempWidth, &iTempHeight );

	if( width )
		*width = iTempWidth;

	if( height )
		*height = ( hudfont != 0 ) ? hudfont : iTempHeight;
}