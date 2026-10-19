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
// Message.cpp
//
// implementation of CHudMessage class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "vgui/dod_localize.h"
#include "vgui/idodviewport.h"

DECLARE_MESSAGE( m_Message, HudText )
DECLARE_MESSAGE( m_Message, GameTitle )

// 1 Global client_textmessage_t for custom messages that aren't in the titles.txt
client_textmessage_t	g_pCustomMessage;
const char *g_pCustomName = "Custom";
char g_pCustomText[1024];

int CHudMessage::Init( void )
{
	HOOK_MESSAGE( HudText );
	HOOK_MESSAGE( GameTitle );

	gHUD.AddHudElem( this );

	m_Fonts[0] = 0;
	m_Fonts[1] = 0;
	m_Fonts[2] = 0;

	Reset();

	return 1;
}

int CHudMessage::VidInit( void )
{
	m_HUD_title_half = gHUD.GetSpriteIndex( "title_half" );
	m_HUD_title_life = gHUD.GetSpriteIndex( "title_life" );

	// the original gets the fonts CreditsFont, Legacy_CreditsFont and EngineFont from the scheme of the
	// VGUI2 here, the text is drawn with the font of the engine

	return 1;
}

unsigned long CHudMessage::GetFont( void )
{
	return m_Fonts[0];
}

void CHudMessage::Reset( void )
{
 	memset( m_pMessages, 0, sizeof(m_pMessages[0]) * maxHUDMessages );
	memset( m_startTime, 0, sizeof(m_startTime[0]) * maxHUDMessages );

	m_gameTitleTime = 0;
	m_pGameTitle = NULL;
}

float CHudMessage::FadeBlend( float fadein, float fadeout, float hold, float localTime )
{
	float fadeTime = fadein + hold;
	float fadeBlend;

	if( localTime < 0.0f )
		return 0;

	if( localTime < fadein )
	{
		fadeBlend = 1.0f - ( ( fadein - localTime ) / fadein );
	}
	else if( localTime > fadeTime )
	{
		if( fadeout > 0.0f )
			fadeBlend = 1.0f - ( ( localTime - fadeTime ) / fadeout );
		else
			fadeBlend = 0.0f;
	}
	else
		fadeBlend = 1.0f;

	return fadeBlend;
}


int CHudMessage::XPosition( float x, int width, int totalWidth )
{
	int xPos;

	if( x == -1.0f )
	{
		xPos = ( ScreenWidth - width ) / 2;
	}
	else
	{
		if( x < 0.0f )
			xPos = ( 1.0f + x ) * ScreenWidth - totalWidth;	// Alight right
		else
			xPos = x * ScreenWidth;
	}

	if( xPos + width > ScreenWidth )
		xPos = ScreenWidth - width;
	else if( xPos < 0 )
		xPos = 0;

	return xPos;
}

int CHudMessage::YPosition( float y, int height )
{
	int yPos;

	if( y == -1 )	// Centered?
		yPos = ( ScreenHeight - height ) * 0.5f;
	else
	{
		// Alight bottom?
		if ( y < 0 )
			yPos = ( 1.0f + y ) * ScreenHeight - height;	// Alight bottom
		else // align top
			yPos = y * ScreenHeight;
	}

	if ( yPos + height > ScreenHeight )
		yPos = ScreenHeight - height;
	else if ( yPos < 0 )
		yPos = 0;

	return yPos;
}

void CHudMessage::MessageScanNextChar( unsigned int font )
{
	int srcRed, srcGreen, srcBlue, destRed = 0, destGreen = 0, destBlue = 0;
	int blend;

	srcRed = m_parms.pMessage->r1;
	srcGreen = m_parms.pMessage->g1;
	srcBlue = m_parms.pMessage->b1;
	blend = 0;	// Pure source

	switch( m_parms.pMessage->effect )
	{
	// Fade-in / Fade-out
	case 0:
	case 1:
		destRed = destGreen = destBlue = 0;
		blend = m_parms.fadeBlend;
		break;
	case 2:
		m_parms.charTime += m_parms.pMessage->fadein;
		if( m_parms.charTime > m_parms.time )
		{
			srcRed = srcGreen = srcBlue = 0;
			blend = 0;	// pure source
		}
		else
		{
			float deltaTime = m_parms.time - m_parms.charTime;

			destRed = destGreen = destBlue = 0;
			if( m_parms.time > m_parms.fadeTime )
			{
				blend = m_parms.fadeBlend;
			}
			else if( deltaTime > m_parms.pMessage->fxtime )
			{
				blend = 0;	// pure dest
			}
			else
			{
				destRed = m_parms.pMessage->r2;
				destGreen = m_parms.pMessage->g2;
				destBlue = m_parms.pMessage->b2;
				blend = 255 - ( deltaTime * ( 1.0f / m_parms.pMessage->fxtime) * 255.0f + 0.5f );
			}
		}
		break;
	}
	if( blend > 255 )
		blend = 255;
	else if( blend < 0 )
		blend = 0;

	m_parms.r = ( ( srcRed * ( 255 - blend ) ) + ( destRed * blend ) ) >> 8;
	m_parms.g = ( ( srcGreen * (255 - blend ) ) + ( destGreen * blend ) ) >> 8;
	m_parms.b = ( ( srcBlue * ( 255 - blend ) ) + ( destBlue * blend ) ) >> 8;

	if( m_parms.pMessage->effect == 1 && m_parms.charTime != 0 )
	{
		if( m_parms.x >= 0 && m_parms.y >= 0 )
			TextMessageDrawChar( m_parms.x, m_parms.y, m_parms.text, m_parms.pMessage->r2, m_parms.pMessage->g2, m_parms.pMessage->b2 );
	}
}

void CHudMessage::MessageScanStart( void )
{
	switch( m_parms.pMessage->effect )
	{
	// Fade-in / out with flicker
	case 1:
	case 0:
		m_parms.fadeTime = m_parms.pMessage->fadein + m_parms.pMessage->holdtime;
		

		if( m_parms.time < m_parms.pMessage->fadein )
		{
			m_parms.fadeBlend = ( ( m_parms.pMessage->fadein - m_parms.time ) * ( 1.0f / m_parms.pMessage->fadein ) * 255 );
		}
		else if( m_parms.time > m_parms.fadeTime )
		{
			if( m_parms.pMessage->fadeout > 0 )
				m_parms.fadeBlend = ( ( ( m_parms.time - m_parms.fadeTime ) / m_parms.pMessage->fadeout) * 255);
			else
				m_parms.fadeBlend = 255; // Pure dest (off)
		}
		else
			m_parms.fadeBlend = 0;	// Pure source (on)
		m_parms.charTime = 0;

		if( m_parms.pMessage->effect == 1 && ( rand() % 100 ) < 10 )
			m_parms.charTime = 1;
		break;
	case 2:
		m_parms.fadeTime = (m_parms.pMessage->fadein * m_parms.length) + m_parms.pMessage->holdtime;
		
		if ( m_parms.time > m_parms.fadeTime && m_parms.pMessage->fadeout > 0 )
			m_parms.fadeBlend = ( ( ( m_parms.time - m_parms.fadeTime ) / m_parms.pMessage->fadeout ) * 255 );
		else
			m_parms.fadeBlend = 0;
		break;
	}
}


void CHudMessage::MessageDrawScan( client_textmessage_t *pMessage, float time, unsigned int font )
{
	int i, j, length, width;
	const char *pText;
	const char *pLineStart;
	char buf[80];

	m_parms.lines = 1;
	m_parms.pMessage = pMessage;
	m_parms.time = time;
	m_parms.totalWidth = 0;

	if( *pMessage->pMessage == '#' )
	{
		// the text is a token of the localization
		snprintf( buf, sizeof( buf ), "%s", pMessage->pMessage );

		int len = strlen( buf ) - 1;

		if( buf[len] == '\r' )
			buf[len] = 0;

		pText = DoD_FindLocalized( buf );

		if( !pText )
		{
			gEngfuncs.Con_DPrintf( "ERROR: Missing %s from the dod_english.txt file!\n", pMessage->pMessage );
			return;
		}
	}
	else
	{
		pText = pMessage->pMessage;
	}

	// Count lines
	const char *pCount = pText;

	length = 0;
	width = 0;

	while( *pCount )
	{
		if( *pCount == '\n' )
		{
			m_parms.lines++;

			if( width > m_parms.totalWidth )
				m_parms.totalWidth = width;

			width = 0;
		}
		else
			width += gHUD.m_scrinfo.charWidths[(unsigned char)*pCount];

		pCount++;
		length++;
	}

	m_parms.length = length;
	m_parms.totalHeight = ( m_parms.lines * gHUD.m_scrinfo.iCharHeight );

	m_parms.y = YPosition( pMessage->y, m_parms.totalHeight );

	m_parms.charTime = 0;

	MessageScanStart();

	for( i = 0; i < m_parms.lines; i++ )
	{
		m_parms.lineLength = 0;
		m_parms.width = 0;
		pLineStart = pText;

		while( *pText && *pText != '\n' )
		{
			unsigned char c = *pText;
			m_parms.width += gHUD.m_scrinfo.charWidths[c];
			m_parms.lineLength++;
			pText++;
		}

		pText++;		// Skip LF

		m_parms.x = XPosition( pMessage->x, m_parms.width, m_parms.totalWidth );

		for( j = 0; j < m_parms.lineLength; j++ )
		{
			m_parms.text = (unsigned char)pLineStart[j];

			int next = m_parms.x + gHUD.m_scrinfo.charWidths[m_parms.text];

			MessageScanNextChar( font );

			if( m_parms.x >= 0 && m_parms.y >= 0 && next <= ScreenWidth )
				TextMessageDrawChar( m_parms.x, m_parms.y, m_parms.text, m_parms.r, m_parms.g, m_parms.b );

			m_parms.x = next;
		}

		m_parms.y += gHUD.m_scrinfo.iCharHeight;
	}
}

int CHudMessage::Draw( float fTime )
{
	int i, drawn;
	client_textmessage_t *pMessage;
	float endTime = 0.0f;

	drawn = 0;

	if( m_gameTitleTime > 0 )
	{
		float localTime = gHUD.m_flTime - m_gameTitleTime;
		float brightness;

		// Maybe timer isn't set yet
		if( m_gameTitleTime > gHUD.m_flTime )
			m_gameTitleTime = gHUD.m_flTime;

		if( localTime > ( m_pGameTitle->fadein + m_pGameTitle->holdtime + m_pGameTitle->fadeout ) )
			m_gameTitleTime = 0;
		else
		{
			brightness = FadeBlend( m_pGameTitle->fadein, m_pGameTitle->fadeout, m_pGameTitle->holdtime, localTime );

			int halfWidth = gHUD.GetSpriteRect( m_HUD_title_half ).right - gHUD.GetSpriteRect( m_HUD_title_half ).left;
			int fullWidth = halfWidth + gHUD.GetSpriteRect( m_HUD_title_life ).right - gHUD.GetSpriteRect( m_HUD_title_life ).left;
			int fullHeight = gHUD.GetSpriteRect( m_HUD_title_half ).bottom - gHUD.GetSpriteRect( m_HUD_title_half ).top;

			int x = XPosition( m_pGameTitle->x, fullWidth, fullWidth );
			int y = YPosition( m_pGameTitle->y, fullHeight );

			SPR_Set( gHUD.GetSprite( m_HUD_title_half ), brightness * m_pGameTitle->r1, brightness * m_pGameTitle->g1, brightness * m_pGameTitle->b1 );
			SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect( m_HUD_title_half ) );

			SPR_Set( gHUD.GetSprite( m_HUD_title_life ), brightness * m_pGameTitle->r1, brightness * m_pGameTitle->g1, brightness * m_pGameTitle->b1 );
			SPR_DrawAdditive( 0, x + halfWidth, y, &gHUD.GetSpriteRect( m_HUD_title_life ) );

			drawn = 1;
		}
	}
	// Fixup level transitions
	for( i = 0; i < maxHUDMessages; i++ )
	{
		// Assume m_parms.time contains last time
		if( m_pMessages[i].pMessage )
		{
			if( m_startTime[i] > gHUD.m_flTime || m_startTime[i] == 1.0f )
				m_startTime[i] = gHUD.m_flTime + m_parms.time - m_startTime[i] + 0.2f;	// Server takes 0.2 seconds to spawn, adjust for this
		}
	}

	for( i = 0; i < maxHUDMessages; i++ )
	{
		if( m_pMessages[i].pMessage )
		{
			pMessage = m_pMessages[i].pMessage;

			// This is when the message is over
			switch( pMessage->effect )
			{
			case 0:
			case 1:
				endTime = m_startTime[i] + pMessage->fadein + pMessage->fadeout + pMessage->holdtime;
				break;

			// Fade in is per character in scanning messages
			case 2:
				endTime = m_startTime[i] + ( pMessage->fadein * strlen( pMessage->pMessage ) ) + pMessage->fadeout + pMessage->holdtime;
				break;
			}

			if( fTime <= endTime )
			{
				float messageTime = fTime - m_startTime[i];

				// Draw the message
				// effect 0 is fade in/fade out
				// effect 1 is flickery credits
				// effect 2 is write out (training room)
				MessageDrawScan( pMessage, messageTime, m_pMessages[i].font );

				drawn++;
			}
			else
			{
				// The message is over
				m_pMessages[i].pMessage = NULL;
				m_pMessages[i].font = 0;
			}
		}
	}

	// Remember the time -- to fix up level transitions
	m_parms.time = gHUD.m_flTime;
	// Don't call until we get another message
	if( !drawn )
		m_iFlags &= ~HUD_ACTIVE;

	return 1;
}

void CHudMessage::MessageAdd( const char *pName, float time, int hintMessage, unsigned int font )
{
	int i;
	client_textmessage_t *tempMessage;

	// the first free slot
	for( i = 0; i < maxHUDMessages; i++ )
	{
		if( !m_pMessages[i].pMessage )
			break;
	}

	if( i == maxHUDMessages )
		return;

	if( pName[0] == '#' )
	{
		// Trim off a leading # if it's there
		tempMessage = gEngfuncs.pfnTextMessageGet( pName + 1 );

		if( tempMessage && hintMessage )
		{
			// the hints are green, they are written out and they stay longer when they are long
			tempMessage->effect = 2;
			tempMessage->r1 = 40;
			tempMessage->g1 = 255;
			tempMessage->b1 = 40;
			tempMessage->a1 = 200;
			tempMessage->r2 = 0;
			tempMessage->g2 = 255;
			tempMessage->b2 = 0;
			tempMessage->a2 = 200;
			tempMessage->x = -1.0f;
			tempMessage->y = 0.7f;
			tempMessage->fadein = 0.01f;
			tempMessage->fadeout = 0.7f;
			tempMessage->fxtime = 0.07f;
			tempMessage->holdtime = 5.0f;

			tempMessage->holdtime = (float)( strlen( tempMessage->pMessage ) / 25 );

			if( tempMessage->holdtime < 1.0f )
				tempMessage->holdtime = 1.0f;
		}
	}
	else
		tempMessage = gEngfuncs.pfnTextMessageGet( pName );

	m_pMessages[i].pMessage = tempMessage;
	m_pMessages[i].font = font;
	m_startTime[i] = time;
}

int CHudMessage::MsgFunc_HudText( const char *pszName,  int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	char *pString = READ_STRING();
	int hintMessage = READ_BYTE();

	if( hintMessage > 0 && gDoDViewPortInterface )
		gDoDViewPortInterface->ShowHintMessage( pString );
	else
		MessageAdd( pString, gHUD.m_flTime, hintMessage, GetFont() );

	// Remember the time -- to fix up level transitions
	m_parms.time = gHUD.m_flTime;

	// Turn on drawing
	if( !( m_iFlags & HUD_ACTIVE ) )
		m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudMessage::MsgFunc_GameTitle( const char *pszName,  int iSize, void *pbuf )
{
	m_pGameTitle = gEngfuncs.pfnTextMessageGet( "GAMETITLE" );

	if( m_pGameTitle != NULL )
	{
		m_gameTitleTime = gHUD.m_flTime;

		// Turn on drawing
		if( !( m_iFlags & HUD_ACTIVE ) )
			m_iFlags |= HUD_ACTIVE;
	}

	return 1;
}

void CHudMessage::MessageAdd( client_textmessage_t * newMessage )
{
	m_parms.time = gHUD.m_flTime;

	// Turn on drawing
	if( !( m_iFlags & HUD_ACTIVE ) )
		m_iFlags |= HUD_ACTIVE;

	for( int i = 0; i < maxHUDMessages; i++ )
	{
		if( !m_pMessages[i].pMessage )
		{
			m_pMessages[i].pMessage = newMessage;
			m_pMessages[i].font = GetFont();
			m_startTime[i] = gHUD.m_flTime;
			return;
		}
	}
}

void CHudMessage::HintMessageAdd( const char *pText )
{
	if( gDoDViewPortInterface )
		gDoDViewPortInterface->ShowHintMessage( pText );

	m_parms.time = gHUD.m_flTime;

	if( !( m_iFlags & HUD_ACTIVE ) )
		m_iFlags |= HUD_ACTIVE;
}
