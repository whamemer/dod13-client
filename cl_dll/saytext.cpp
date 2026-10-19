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
// saytext.cpp
//
// implementation of CHudSayText class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "vgui/dod_localize.h"

#include "vgui/vgui_TeamFortressViewport.h"

extern float *GetClientColor( int clientIndex );

#define MAX_LINES	5
#define MAX_CHARS_PER_LINE	256  /* it can be less than this, depending on char size */

// allow 20 pixels on either side of the text
#define MAX_LINE_WIDTH  ( ScreenWidth - 40 )
#define LINE_START  10
static float SCROLL_SPEED = 5;

static char g_szLineBuffer[MAX_LINES + 1][MAX_CHARS_PER_LINE];
static float *g_pflNameColors[MAX_LINES + 1];
static int g_iNameLengths[MAX_LINES + 1];
static float flScrollTime = 0;  // the time at which the lines next scroll up

static int Y_START = 0;
static int line_height = 0;

// the color of the messages of the server (spectators only)
float ColorBlue[3] = { 1.0f, 0.7f, 0.0f };

DECLARE_MESSAGE( m_SayText, SayText )

// the command "hud_saytext" switches the cvar "hud_saytext_internal"
void HudSayTextToggle( void )
{
	if( gHUD.m_SayText.m_HUD_saytext->value == 0.0f )
		gHUD.m_SayText.m_HUD_saytext->value = 1.0f;
	else
		gHUD.m_SayText.m_HUD_saytext->value = 0.0f;
}

int CHudSayText::Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( SayText );

	InitHUDData();

	gEngfuncs.pfnAddCommand( "hud_saytext", HudSayTextToggle );

	m_HUD_saytext =		gEngfuncs.pfnRegisterVariable( "hud_saytext_internal", "1", 0 );
	m_HUD_saytext_time =	gEngfuncs.pfnRegisterVariable( "hud_saytext_time", "5", 0 );

	m_iFlags |= HUD_INTERMISSION; // is always drawn during an intermission

	return 1;
}

void CHudSayText::InitHUDData( void )
{
	memset( g_szLineBuffer, 0, sizeof g_szLineBuffer );
	memset( g_pflNameColors, 0, sizeof g_pflNameColors );
	memset( g_iNameLengths, 0, sizeof g_iNameLengths );
}

int CHudSayText::VidInit( void )
{
	return 1;
}

int ScrollTextUp( void )
{
	char buf[MAX_CHARS_PER_LINE + 2];

	// move the first line into the console buffer
	strlcpy( buf, g_szLineBuffer[0], MAX_CHARS_PER_LINE );
	strlcat( buf, "\n", sizeof( buf ) );
	ConsolePrint( buf );

	g_szLineBuffer[MAX_LINES][0] = 0;
	memmove( g_szLineBuffer[0], g_szLineBuffer[1], sizeof(g_szLineBuffer) - sizeof(g_szLineBuffer[0]) ); // overwrite the first line
	memmove( &g_pflNameColors[0], &g_pflNameColors[1], sizeof(g_pflNameColors) - sizeof(g_pflNameColors[0]) );
	memmove( &g_iNameLengths[0], &g_iNameLengths[1], sizeof(g_iNameLengths) - sizeof(g_iNameLengths[0]) );
	g_szLineBuffer[MAX_LINES-1][0] = 0;

	if( g_szLineBuffer[0][0] == ' ' ) // also scroll up following lines
	{
		g_szLineBuffer[0][0] = 2;
		return 1 + ScrollTextUp();
	}

	return 1;
}

int CHudSayText::Draw( float flTime )
{
	int y = Y_START;
	int r = 255, g = 255, b = 255;

	if( ( gViewPort && gViewPort->AllowedToPrintText() == FALSE ) || !m_HUD_saytext->value )
		return 1;

	// make sure the scrolltime is within reasonable bounds,  to guard against the clock being reset
	flScrollTime = Q_min( flScrollTime, flTime + m_HUD_saytext_time->value );

	if( flScrollTime <= flTime )
	{
		if( *g_szLineBuffer[0] )
		{
			flScrollTime = flTime + m_HUD_saytext_time->value;
			// push the console up
			ScrollTextUp();
		}
		else
		{
			// buffer is empty,  just disable drawing of this section
			m_iFlags &= ~HUD_ACTIVE;
		}
	}

	DrawConsoleString( LINE_START, y, "" );

	// the text is drawn in the color of the console
	char *colour = CVAR_GET_STRING( "con_color" );
	sscanf( colour, "%i %i %i", &r, &g, &b );

	float fR = r / 255.0f;
	float fG = g / 255.0f;
	float fB = b / 255.0f;

	for( int i = 0; i < MAX_LINES; i++ )
	{
		if( *g_szLineBuffer[i] )
		{
			if( *g_szLineBuffer[i] == 2 && g_pflNameColors[i] )
			{
				// it's a saytext string
				char buf[MAX_PLAYER_NAME_LENGTH + 32];

				// draw the first x characters in the player color
				strlcpy( buf, g_szLineBuffer[i], Q_min( g_iNameLengths[i] + 1, (int)sizeof( buf ) ) );
				int x = gHUD.m_VGUI2Print.DrawLocalizedString( buf, LINE_START, y, g_pflNameColors[i][0], g_pflNameColors[i][1], g_pflNameColors[i][2] );

				// color is reset after each string draw
				gHUD.m_VGUI2Print.DrawLocalizedString( g_szLineBuffer[i] + g_iNameLengths[i], x, y, fR, fG, fB );
			}
			else
			{
				// normal draw
				gHUD.m_VGUI2Print.DrawLocalizedString( g_szLineBuffer[i], LINE_START, y, fR, fG, fB );
			}
		}

		y += line_height;
	}

	return 1;
}

int CHudSayText::MsgFunc_SayText( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int client_index = READ_BYTE();		// the client who spoke the message
	SayTextPrint( READ_STRING(), iSize - 1, client_index, NULL, NULL, NULL, NULL );

	return 1;
}

int CHudSayText::GetTextPrintY( void )
{
	int iHeight = ScreenHeight;
	int y;

	if( !g_iUser1 && !gEngfuncs.IsSpectateOnly() )
	{
		y = iHeight - (int)( (float)gHUD.m_iFontHeight * 2.75f );
	}
	else
	{
		// the bottom bar of the spectator GUI (VGUI2) is not here
		y = iHeight - 4;
	}

	return y - 5 * line_height - (int)( (float)line_height * 0.5f );
}

// the words that begin with '#' are tokens of the localization, %s1..%s4 are replaced with sstr1..sstr4
void CHudSayText::SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex, char *sstr1, char *sstr2, char *sstr3, char *sstr4 )
{
	int i;
	bool colorName;
	const char *localized;
	char buf[512];
	int len;
	char text[MAX_CHARS_PER_LINE];
	const char *args[4];
	int j;
	const char *pName;
	const char *nameInString;

	if( gViewPort && gViewPort->AllowedToPrintText() == FALSE )
	{
		// Print it straight to the console
		ConsolePrint( pszBuf );
		return;
	}

	// find an empty string slot
	for( i = 0; i < MAX_LINES; i++ )
	{
		if( !( *g_szLineBuffer[i] ) )
			break;
	}

	if( i == MAX_LINES )
	{
		// force scroll buffer up
		ScrollTextUp();
		i = MAX_LINES - 1;
	}

	g_iNameLengths[i] = 0;
	g_pflNameColors[i] = NULL;

	if( gEngfuncs.IsSpectateOnly() && !clientIndex )
		g_pflNameColors[i] = ColorBlue;

	colorName = ( *pszBuf == 2 && clientIndex > 0 );

	// the line ends are not shown
	len = strlen( pszBuf );

	if( len > 0 && ( pszBuf[len - 1] == '\n' || pszBuf[len - 1] == '\r' ) )
	{
		strlcpy( buf, pszBuf, sizeof( buf ) );
		len = strlen( buf );

		while( len > 0 && ( buf[len - 1] == '\n' || buf[len - 1] == '\r' ) )
			buf[--len] = 0;

		pszBuf = buf;
	}

	localized = DoD_FindLocalized( pszBuf );

	if( localized )
	{
		strlcpy( text, localized, sizeof( text ) );
	}
	else
	{
		const char *src = pszBuf;
		int outPos = 0;

		while( *src && outPos < (int)sizeof( text ) - 1 )
		{
			if( *src == '#' )
			{
				char word[128];
				int wordLen = 0;
				int n;

				while( src[wordLen] && !isspace( (unsigned char)src[wordLen] ) )
					wordLen++;

				strlcpy( word, src, Q_min( wordLen + 1, (int)sizeof( word ) ) );

				localized = DoD_FindLocalized( word );

				if( !localized )
					localized = word;

				n = Q_min( (int)strlen( localized ), (int)sizeof( text ) - 1 - outPos );
				memcpy( text + outPos, localized, n );
				outPos += n;

				src += wordLen;
			}
			else
			{
				text[outPos++] = *src++;
			}
		}

		text[outPos] = 0;
	}

	args[0] = sstr1;
	args[1] = sstr2;
	args[2] = sstr3;
	args[3] = sstr4;

	// the arguments are tokens too
	for( j = 0; j < 4; j++ )
	{
		localized = args[j] ? DoD_FindLocalized( args[j] ) : NULL;

		if( localized )
			args[j] = localized;
		else if( !args[j] )
			args[j] = "";
	}

	DoD_ConstructLocalized( g_szLineBuffer[i], MAX_CHARS_PER_LINE, text, args[0], args[1], args[2], args[3] );

	// if it's a say message, search for the players name in the string
	if( colorName )
	{
		GetPlayerInfo( clientIndex, &g_PlayerInfoList[clientIndex] );
		pName = g_PlayerInfoList[clientIndex].name;

		if( pName )
		{
			nameInString = strstr( g_szLineBuffer[i], pName );

			if( nameInString )
			{
				g_iNameLengths[i] = strlen( pName ) + ( nameInString - g_szLineBuffer[i] );
				g_pflNameColors[i] = GetClientColor( clientIndex );
			}
		}
	}

	// make sure the text fits in one line
	EnsureTextFitsInOneLineAndWrapIfHaveTo( i );

	// Set scroll time
	if( i == 0 )
	{
		flScrollTime = gHUD.m_flTime + m_HUD_saytext_time->value;
	}

	m_iFlags |= HUD_ACTIVE;

	Y_START = GetTextPrintY();
}

void CHudSayText::EnsureTextFitsInOneLineAndWrapIfHaveTo( int line )
{
	int line_width = 0;
	gHUD.m_VGUI2Print.GetStringSize( g_szLineBuffer[line], &line_width, &line_height );

	if( ( line_width + LINE_START ) > MAX_LINE_WIDTH )
	{
		// string is too long to fit on line
		// scan the string until we find what word is too long,  and wrap the end of the sentence after the word
		int length = LINE_START;
		int tmp_len = 0;
		char *last_break = NULL;
		for( char *x = g_szLineBuffer[line]; *x != 0; x++ )
		{
			// check for a color change, if so skip past it
			if( x[0] == '/' && x[1] == '(' )
			{
				x += 2;
				// skip forward until past mode specifier
				while ( *x != 0 && *x != ')' )
					x++;

				if( *x != 0 )
					x++;

				if( *x == 0 )
					break;
			}

			char buf[2];
			buf[1] = 0;

			if( *x == ' ' && x != g_szLineBuffer[line] )  // store each line break,  except for the very first character
				last_break = x;

			buf[0] = *x;  // get the length of the current character
			gHUD.m_VGUI2Print.GetStringSize( buf, &tmp_len, &line_height );
			length += tmp_len;

			if( length > MAX_LINE_WIDTH )
			{
				// needs to be broken up
				if( !last_break )
					last_break = x - 1;

				// x = last_break;

				// find an empty string slot
				int j;
				do 
				{
					for( j = 0; j < MAX_LINES; j++ )
					{
						if( !( *g_szLineBuffer[j] ) )
							break;
					}
					if( j == MAX_LINES )
					{
						// need to make more room to display text, scroll stuff up then fix the pointers
						int linesmoved = ScrollTextUp();
						line -= linesmoved;
						last_break = last_break - ( sizeof(g_szLineBuffer[0]) * linesmoved );
					}
				}
				while( j == MAX_LINES );

				// copy remaining string into next buffer,  making sure it starts with a space character
				if( (char)*last_break == (char)' ' )
				{
					int linelen = strlen( g_szLineBuffer[j] );
					int remaininglen = strlen( last_break );

					if( ( linelen - remaininglen ) <= MAX_CHARS_PER_LINE )
						strcat( g_szLineBuffer[j], last_break );
				}
				else
				{
					if ( ( strlen( g_szLineBuffer[j] ) - strlen( last_break ) - 2 ) < MAX_CHARS_PER_LINE )
					{
						strcat( g_szLineBuffer[j], " " );
						strcat( g_szLineBuffer[j], last_break );
					}
				}

				*last_break = 0; // cut off the last string

				EnsureTextFitsInOneLineAndWrapIfHaveTo( j );
				break;
			}
		}
	}
}
