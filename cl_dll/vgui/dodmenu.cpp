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
// dodmenu.cpp
//

#include <string.h>

#include <VGUI_Font.h>

#include "hud.h"
#include "cl_util.h"
#include "dod_localize.h"

#include "vgui/dodmenu.h"

namespace
{

struct FontInfo_t
{
	const char *pszName;
	const char *pszFace;
	int iTall;
	int iWeight;
	float flScale;	// the size of the text relative to the tall scaled with the height of the screen
};

// The fonts of Resource/ClientScheme.res: ClassMenuDefault, ClassMenuTitleFont, ClassMenuBoldFont and ClassMenuBodyFont,
// Default (the rows of the scoreboard) and DefaultVerySmall (its header)
const FontInfo_t s_FontInfo[NUM_DODFONTS] =
{
	{ "ClassMenuDefault", "Times New Roman", 15, 400, DODMENU_FONT_SCALE },
	{ "ClassMenuTitleFont", "Times New Roman", 20, 500, DODMENU_FONT_SCALE },
	{ "ClassMenuBoldFont", "Times New Roman", 11, 800, DODMENU_FONT_SCALE },
	{ "ClassMenuBodyFont", "Times New Roman", 13, 500, DODMENU_FONT_SCALE },
	// the text of Default is 5% narrower than the one of the other fonts in the original (the line of the help is 433 pixels at
	// 1920x1200, 421 pixels at 1090 pixels of height with 0.88)
	{ "Default", "Times New Roman", 13, 400, 0.84f },
	{ "DefaultVerySmall", "Courier New", 11, 0, DODMENU_FONT_SCALE }
};

vgui::Font *s_pFonts[NUM_DODFONTS];
int s_iFontHeight = 0;

}

vgui::Font *DoD_GetMenuFont( int iFont )
{
	if( s_iFontHeight != ScreenHeight )
	{
		s_iFontHeight = ScreenHeight;

		for( int i = 0; i < NUM_DODFONTS; i++ )
		{
			// the text of the original is about 12% smaller than the tall of the scheme scaled with the height of the
			// screen (measured on the screenshots at 1920x1200: the title is 44 pixels with the tall 20)
			int iTall = (int)( DODMENU_SIZE( s_FontInfo[i].iTall ) * s_FontInfo[i].flScale + 0.5f );

			s_pFonts[i] = new vgui::Font( s_FontInfo[i].pszFace, NULL, 0, iTall, 0, 0, s_FontInfo[i].iWeight, false, false, false, false );
		}
	}

	return s_pFonts[iFont];
}

int DoD_FindMenuFont( const char *pszSchemeName )
{
	for( int i = 0; i < NUM_DODFONTS; i++ )
	{
		if( !stricmp( s_FontInfo[i].pszName, pszSchemeName ) )
			return i;
	}

	return DODFONT_DEFAULT;
}

// The code point of the UTF-8 sequence of 1..3 bytes at psz, -1 when it is not a valid sequence (a single byte
// of a one-byte charset)
static int DecodeUTF8( const unsigned char *psz, int &iLength )
{
	iLength = 1;

	if( psz[0] < 0x80 )
		return psz[0];

	if( psz[0] >= 0xC2 && psz[0] <= 0xDF && ( psz[1] & 0xC0 ) == 0x80 )
	{
		iLength = 2;
		return ( ( psz[0] & 0x1F ) << 6 ) | ( psz[1] & 0x3F );
	}

	if( psz[0] >= 0xE0 && psz[0] <= 0xEF && ( psz[1] & 0xC0 ) == 0x80 && ( psz[2] & 0xC0 ) == 0x80 )
	{
		iLength = 3;
		return ( ( psz[0] & 0x0F ) << 12 ) | ( ( psz[1] & 0x3F ) << 6 ) | ( psz[2] & 0x3F );
	}

	return -1;
}

char DoD_CodePointToLatin1( int iCodePoint )
{
	if( iCodePoint < 0 )
		return '?';

	if( iCodePoint < 0x80 || ( iCodePoint >= 0xA0 && iCodePoint <= 0xFF ) )
		return (char)iCodePoint;

	switch( iCodePoint )
	{
	case 0x2018:
	case 0x2019:
		return '\'';
	case 0x201C:
	case 0x201D:
		return '"';
	case 0x2013:
	case 0x2014:
		return '-';
	case 0x2022:
		return (char)0xB7;
	case 0x2026:
		return '.';
	}

	return '?';
}

void DoD_ConvertText( const char *pszText, char *pszDest, int iDestSize )
{
	int iLength = 0;

	// VGUI1 draws a text byte by byte in the one-byte charset of the engine (Latin-1 here), the engine decodes UTF-8
	// only when its cvar vgui_utf8 is set
	cvar_t *pUTF8 = gEngfuncs.pfnGetCvarPointer( "vgui_utf8" );
	bool bConvert = !pUTF8 || pUTF8->value == 0.0f;

	while( *pszText && iLength < iDestSize - 1 )
	{
		if( bConvert )
		{
			int iSequence;
			int iCodePoint = DecodeUTF8( (const unsigned char *)pszText, iSequence );

			// a byte that is not UTF-8 is a byte of the one-byte charset already
			if( iCodePoint < 0 )
				pszDest[iLength++] = *pszText;
			else
				pszDest[iLength++] = DoD_CodePointToLatin1( iCodePoint );

			pszText += iSequence;
		}
		else
		{
			pszDest[iLength++] = *pszText++;
		}
	}

	pszDest[iLength] = '\0';
}

void DoD_GetLocalized( const char *pszToken, char *pszDest, int iDestSize )
{
	// the tokens are written with and without '#', a text that is not a token stays as it is
	const char *pszText = DoD_FindLocalized( pszToken );
	char *pszOut = pszDest;
	int iLength = 0;

	if( pszText )
	{
		// the localized texts are UTF-8
		DoD_ConvertText( pszText, pszDest, iDestSize );
		pszText = pszDest;
	}
	else
	{
		pszText = ( pszToken[0] == '#' ) ? pszToken + 1 : pszToken;
	}

	// the mnemonic marks are not shown
	while( *pszText && iLength < iDestSize - 1 )
	{
		if( *pszText != '&' )
			pszOut[iLength++] = *pszText;

		pszText++;
	}

	pszOut[iLength] = '\0';
}

int DoD_GetMnemonic( const char *pszToken )
{
	const char *pszText = DoD_FindLocalized( pszToken );

	if( !pszText )
		pszText = pszToken;

	const char *pszMark = strchr( pszText, '&' );

	if( pszMark && pszMark[1] >= '0' && pszMark[1] <= '9' )
		return pszMark[1] - '0';

	return -1;
}

void DoD_PlaceLabel( vgui::Label *pLabel, vgui::Font *pFont, int x, int y, int wide, int tall )
{
	int iX = DODMENU_SIZE( x );
	int iY = DODMENU_SIZE( y );
	int iWide = DODMENU_SIZE( wide );
	int iTall = DODMENU_SIZE( tall );
	int iFontTall = pFont->getTall() + 4;

	if( iTall < iFontTall )
	{
		iY -= ( iFontTall - iTall ) / 2;
		iTall = iFontTall;
	}

	pLabel->setFont( pFont );
	pLabel->setBounds( iX, iY, iWide, iTall );
}

void DoD_FitLabel( vgui::Label *pLabel, vgui::Label::Alignment alignment )
{
	int iTextWide, iTextTall;
	int x, y, wide, tall;

	pLabel->getTextSize( iTextWide, iTextTall );
	pLabel->getBounds( x, y, wide, tall );

	iTextWide += 4;

	if( iTextWide <= wide )
		return;

	if( alignment == vgui::Label::a_east )
		x -= iTextWide - wide;
	else if( alignment == vgui::Label::a_center )
		x -= ( iTextWide - wide ) / 2;

	pLabel->setBounds( x, y, iTextWide, tall );
}

CDoDMenuPanel::CDoDMenuPanel( int iRemoveMe, int x, int y, int wide, int tall ) : CMenuPanel( 255, iRemoveMe, x, y, wide, tall )
{
	/* Nothing. */
}

// The polygons are the ones of DoDViewport::Start (640x480): the top one is 48,42 406,42 428,64 48,64 and the main
// one is 48,64 588,64 588,416 566,438 48,438, both are black
void CDoDMenuPanel::paintBackground( void )
{
	int iLeft = DODMENU_SIZE( 48 );
	int iTopY = DODMENU_SIZE( 42 );
	int iMainY = DODMENU_SIZE( 64 );
	int iCutY = DODMENU_SIZE( 416 );
	int iBottomY = DODMENU_SIZE( 438 );

	drawSetColor( 0, 0, 0, 0 );

	// the top polygon, its right edge goes from 406 to 428
	for( int y = iTopY; y < iMainY; y++ )
	{
		int iRight = DODMENU_SIZE( 406 ) + ( ( y - iTopY ) * ( DODMENU_SIZE( 428 ) - DODMENU_SIZE( 406 ) ) ) / ( iMainY - iTopY );

		drawFilledRect( iLeft, y, iRight, y + 1 );
	}

	// the main polygon, the bottom right corner is cut from 588 to 566
	drawFilledRect( iLeft, iMainY, DODMENU_SIZE( 588 ), iCutY );

	for( int y = iCutY; y < iBottomY; y++ )
	{
		int iRight = DODMENU_SIZE( 588 ) - ( ( y - iCutY ) * ( DODMENU_SIZE( 588 ) - DODMENU_SIZE( 566 ) ) ) / ( iBottomY - iCutY );

		drawFilledRect( iLeft, y, iRight, y + 1 );
	}
}

void DoD_GetTeamColor( int iTeam, int &r, int &g, int &b )
{
	switch( iTeam )
	{
	case 1:	// team1
		r = 76;
		g = 102;
		b = 76;
		break;
	case 2:	// team2
		r = 255;
		g = 64;
		b = 64;
		break;
	default:	// team0, team3 and team4
		r = 255;
		g = 179;
		b = 0;
		break;
	}
}
