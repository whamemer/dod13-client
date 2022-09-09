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

#define MAX_LINES 5
#define MAX_CHARS_PER_LINE 256

#define MAX_LINE_WIDTH  ( ScreenWidth - 40 )
#define LINE_START 10

static float SCROLL_SPEED = 5;
static float *g_pflNameColors[MAX_LINES + 1];
static int g_iNameLengths[MAX_LINES + 1];

static float flScrollTime = 0;

static int Y_START = 0;
static int line_height = 0;

float ColorBlue[3];

static wchar_t g_wcLineBuffer[MAX_LINES + 1][MAX_CHARS_PER_LINE];

DECLARE_MESSAGE( m_SayText, SayText )

void HudSayTextToggle( void )
{

}

int CHudSayText::Init( void )
{

}

void CHudSayText::InitHUDData( void )
{

}

int CHudSayText::VidInit( void )
{

}

int ScrollTextUp( void )
{

}

int CHudSayText::Draw( float flTime )
{

}

int CHudSayText::MsgFunc_SayText( const char *pszName, int iSize, void *pbuf )
{

}

int CHudSayText::GetTextPrintY( void )
{

}

void CHudSayText::SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex, char *sstr1, char *sstr2, char *sstr3, char *sstr4 )
{

}

void CHudSayText::EnsureTextFitsInOneLineAndWrapIfHaveTo( int line )
{

}
