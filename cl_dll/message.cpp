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

DECLARE_MESSAGE( m_Message, HudText )
DECLARE_MESSAGE( m_Message, GameTitle )

client_textmessage_t	g_pCustomMessage;
const char *g_pCustomName = "Custom";
char g_pCustomText[1024];

int CHudMessage::Init( void )
{

}

int CHudMessage::VidInit( void )
{

}

/*
WHAMER: TODO: need vgui2 :(
vgui2::HFont CHudMessage::GetFont( void )
{

}
*/

void CHudMessage::Reset( void )
{

}

float CHudMessage::FadeBlend( float fadein, float fadeout, float hold, float localTime )
{

}

int CHudMessage::XPosition( float x, int width, int lineWidth )
{

}

int CHudMessage::YPosition( float y, int height )
{

}

void CHudMessage::MessageScanNextChar( void )
{

}

void CHudMessage::MessageScanStart( void )
{

}

int CHudMessage::Draw( float flTime )
{

}

void CHudMessage::MessageAdd( const char *pName, float time )
{

}

int CHudMessage::MsgFunc_HudText( const char *pszName, int iSize, void *pbuf )
{

}

int CHudMessage::MsgFunc_GameTitle( const char *pszName, int iSize, void *pbuf )
{

}

void CHudMessage::MessageAdd( client_textmessage_t *newMessage )
{

}

void CHudMessage::HintMessageAdd( const char *pText )
{

}