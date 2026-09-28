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

float ColorBlue[3];

DECLARE_MESSAGE( m_SayText, SayText )

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
	ConsolePrint( g_szLineBuffer[0] ); // move the first line into the console buffer
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

	float flTargetScroll = flTime + m_HUD_saytext_time->value;

	if( flTargetScroll <= flScrollTime )
		flScrollTime = flTargetScroll;

	if( flTime >= flScrollTime )
	{
		if( g_szLineBuffer[0][0] != '\0' )
		{
			flScrollTime = flTargetScroll;
			ScrollTextUp();
		}
		else
		{
			m_iFlags &= ~HUD_ACTIVE;
		}
	}

	gEngfuncs.pfnDrawConsoleString( 10, y, "" );

	int r = 255, g = 255, b = 255;
	const char *colour = gEngfuncs.pfnGetCvarString( "con_color" );

	if( colour )
		sscanf( colour, "%i %i %i", &r, &g, &b );

	float fR = ( float ) r / 255.0f;
	float fG = ( float ) g / 255.0f;
	float fB = ( float ) b / 255.0f;

	for( int i = 0; i < 5; ++i )
	{
		if( g_szLineBuffer[i][0] != '\0' )
		{
			if( g_szLineBuffer[i][0] == 2 && g_pflNameColors[i] != NULL )
			{
				char buf[256];

				int iNameLen = g_iNameLengths[i];

				if( iNameLen > 64 ) 
					iNameLen = 64;

				strncpy( buf, g_szLineBuffer[i], iNameLen );

				int iTruncateLen = g_iNameLengths[i];

				if( iTruncateLen > 63 ) 
					iTruncateLen = 63;

				buf[iTruncateLen] = '\0';

				int x = gHUD.m_VGUI2Print.DrawVGUI2String( buf, 10, y, g_pflNameColors[i][0], g_pflNameColors[i][1], g_pflNameColors[i][2] );

				char *pszMessageText = &g_szLineBuffer[i][g_iNameLengths[i]];
				int iMsgLen = strlen( pszMessageText );
				if( iMsgLen > ( sizeof( buf ) - 1 ) ) iMsgLen = sizeof( buf ) - 1;

				strncpy( buf, pszMessageText, iMsgLen );
				buf[iMsgLen] = '\0';

				gHUD.m_VGUI2Print.DrawVGUI2String( buf, x, y, fR, fG, fB );
			}
			else
			{
				gHUD.m_VGUI2Print.DrawVGUI2String( g_szLineBuffer[i], 10, y, fR, fG, fB );
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
	SayTextPrint( READ_STRING(), iSize - 1,  client_index, '\0', '\0', '\0', '\0' );
	return 1;
}

int CHudSayText::GetTextPrintY( void )
{
	int iRetVal = 0;

	if( !g_iUser1 )
	{
		if( !gEngfuncs.IsSpectateOnly() )
		{
			iRetVal = ScreenHeight - ( gHUD.m_iFontHeight * 2.75f );
			return iRetVal - 5 * line_height - ( line_height * 0.5f );
		}
	}

	int iSpectatorPanelHeight = gHUD.m_iFontHeight * 4;
	iRetVal = ScreenHeight - iSpectatorPanelHeight - 4;

	return iRetVal - 5 * line_height - ( line_height * 0.5f );
}

void CHudSayText::SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex, char *sstr1, char *sstr2, char *sstr3, char *sstr4 )
{
	if( !pszBuf || !*pszBuf )
		return;

	int i = 0;
	for( i = 0; i < 5; i++ )
	{
		if( !( *g_szLineBuffer[i] ) )
			break;
	}

	if( i == 5 )
	{
		ScrollTextUp();
		i = 4;
	}

	g_iNameLengths[i] = 0;
	g_pflNameColors[i] = NULL;

	if( gEngfuncs.IsSpectateOnly() && !clientIndex )
		g_pflNameColors[i] = ColorBlue;

	bool colorName = ( *pszBuf == 2 && clientIndex > 0 );

	char localized[540];
	strncpy( localized, pszBuf, sizeof( localized ) - 1 );
	localized[sizeof( localized ) - 1] = '\0';

	int len = strlen( localized );
	while( len > 0 && ( localized[len - 1] == '\n' || localized[len - 1] == '\r' ) )
	{
		localized[len - 1] = '\0';
		len--;
	}

	if( sstr1 && *sstr1 )
		snprintf( g_szLineBuffer[i], 255, localized, sstr1, sstr2, sstr3, sstr4 );
	else
		strncpy( g_szLineBuffer[i], localized, 255 );

	g_szLineBuffer[i][255] = '\0';

	if( colorName )
	{
		hud_player_info_t playerInfo;
		gEngfuncs.pfnGetPlayerInfo( clientIndex, &playerInfo );

		const char *pName = playerInfo.name;
		if( pName && *pName )
		{
			const char *nameInString = strstr( g_szLineBuffer[i], pName );

			if( nameInString )
			{
				g_iNameLengths[i] = strlen( pName ) + ( nameInString - g_szLineBuffer[i] );
				g_pflNameColors[i] = GetClientColor( clientIndex );
			}
		}
	}

	EnsureTextFitsInOneLineAndWrapIfHaveTo( i );

	if( i == 0 )
		flScrollTime = gHUD.m_flTime + m_HUD_saytext_time->value;

	m_iFlags |= HUD_ACTIVE;

	PlaySound( "misc/talk.wav", 1 );

	Y_START = GetTextPrintY();
}

void CHudSayText::EnsureTextFitsInOneLineAndWrapIfHaveTo( int line )
{
	int line_width = 0;
	GetConsoleStringSize( g_szLineBuffer[line], &line_width, &line_height );

	if( ( line_width + LINE_START ) > MAX_LINE_WIDTH )
	{
		int length = LINE_START;
		int tmp_len = 0;
		char *last_break = NULL;

		for( char *x = g_szLineBuffer[line]; *x != 0; x++ )
		{
			if( x[0] == '/' && x[1] == '(' )
			{
				x += 2;
				while( *x != 0 && *x != ')' )
					x++;

				if( *x != 0 )
					x++;

				if( *x == 0 )
					break;
			}

			char buf[2];
			buf[1] = 0;

			if( *x == ' ' && x != g_szLineBuffer[line] )
				last_break = x;

			buf[0] = *x;
			GetConsoleStringSize( buf, &tmp_len, &line_height );
			length += tmp_len;

			if( length > MAX_LINE_WIDTH )
			{
				if( !last_break )
					last_break = x - 1;

				int iSavedBreakIndex = last_break - g_szLineBuffer[line];

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
						int linesmoved = ScrollTextUp();
						line -= linesmoved;

						last_break = &g_szLineBuffer[line][iSavedBreakIndex];
					}
				} while( j == MAX_LINES );

				int linelen = strlen( g_szLineBuffer[j] );
				int remaininglen = strlen( last_break );

				if( *last_break == ' ' )
				{
					if( ( linelen - remaininglen ) <= MAX_CHARS_PER_LINE )
					{
						strncat( g_szLineBuffer[j], last_break, 256 - linelen - 1 );
					}
				}
				else
				{
					if( ( linelen - remaininglen - 2 ) < MAX_CHARS_PER_LINE )
					{
						strncat( g_szLineBuffer[j], " ", 256 - linelen - 1 );
						linelen = strlen( g_szLineBuffer[j] );
						strncat( g_szLineBuffer[j], last_break, 256 - linelen - 1 );
					}
				}

				*last_break = 0;

				EnsureTextFitsInOneLineAndWrapIfHaveTo( j );
				break;
			}
		}
	}
}