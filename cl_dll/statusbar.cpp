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
// statusbar.cpp
//
// generic text status bar, set by game dll
// runs across bottom of screen
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_StatusBar, StatusValue )

#define STATUSBAR_ID_LINE		1

float *GetClientColor( int clientIndex );
extern float g_ColorYellow[3];
extern cvar_t *cl_identiconmode;
extern float g_lastFOV;
extern int g_iAlive;

int CHudStatusBar::Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( StatusValue );

	Reset();

	CVAR_CREATE( "hud_centerid", "0", FCVAR_ARCHIVE );

	return 1;
}

int CHudStatusBar::VidInit( void )
{
	// Load sprites here
	return 1;
}

void CHudStatusBar::Reset( void )
{
	int i = 0;

	m_iFlags &= ~HUD_ACTIVE;  // start out inactive
	for( i = 0; i < MAX_STATUSBAR_LINES; i++ )
		m_szStatusText[i][0] = 0;
	memset( m_iStatusValues, 0, sizeof m_iStatusValues );

	m_iStatusValues[0] = 1;  // 0 is the special index, which always returns true

	// reset our colors for the status bar lines (yellow is default)
	for ( i = 0; i < MAX_STATUSBAR_LINES; i++ )
		m_pflNameColors[i] = g_ColorYellow;
}

extern vec3_t v_angles;
extern int g_iDeadFlag;

bool CHudStatusBar::InDeathCamMode( void )
{
	return false;
}

void CHudStatusBar::Think( void )
{

}



int CHudStatusBar::Draw( float fTime )
{
	if( m_bReparseString )
	{
		for( int i = 0; i < MAX_STATUSBAR_LINES; i++ )
		{
			m_pflNameColors[i] = g_ColorYellow;
			//ParseStatusString( i );
		}
		m_bReparseString = FALSE;
	}

	int Y_START = ScreenHeight - YRES( 32 + 4 );

	// Draw the status bar lines
	for( int i = 0; i < MAX_STATUSBAR_LINES; i++ )
	{
		int TextHeight, TextWidth;
		GetConsoleStringSize( m_szStatusBar[i], &TextWidth, &TextHeight );

		int x = 4;
		int y = Y_START - ( 4 + TextHeight * i ); // draw along bottom of screen

		// let user set status ID bar centering
		if( ( i == STATUSBAR_ID_LINE ) && CVAR_GET_FLOAT( "hud_centerid" ) )
		{
			x = Q_max( 0, Q_max( 2, ( ScreenWidth - TextWidth ) ) / 2 );
			y = ( ScreenHeight / 2 ) + ( TextHeight * CVAR_GET_FLOAT( "hud_centerid" ) );
		}

		if( m_pflNameColors[i] )
			DrawSetTextColor( m_pflNameColors[i][0], m_pflNameColors[i][1], m_pflNameColors[i][2] );

		DrawConsoleString( x, y, m_szStatusBar[i] );
	}

	return 1;
}

// Message handler for StatusText message
// accepts two values:
//		byte: index into the status value array
//		short: value to store
int CHudStatusBar::MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int index = READ_BYTE();
	if( index < 1 || index >= MAX_STATUSBAR_VALUES )
		return 1; // index out of range

	m_iStatusValues[index] = READ_SHORT();

	m_bReparseString = TRUE;

	return 1;
}

char *CHudStatusBar::GetTargetName( void )
{
	return "";
}

void CHudStatusBar::CreateEntities( void )
{

}

void CHudStatusBar::DrawEntitiesOverTeam( void )
{

}

void CHudStatusBar::DrawEntitiesOverTarget( void )
{

}