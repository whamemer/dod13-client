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
// text_message.cpp
//
// implementation of CHudTextMessage class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

DECLARE_MESSAGE( m_TextMessage, TextMsg )
DECLARE_MESSAGE( m_TextMessage, CapMsg )

extern cvar_t *cl_hud_msgs;

int CHudTextMessage::Init( void )
{

}

void LocaliseTextString( const char *msg, char *dst_buffer, int buffer_size )
{

}

void BufferedLocaliseTextString( const char *msg )
{

}

const char *CHudTextMessage::LookupString( const char *msg_name, int *msg_dest = NULL )
{

}

void StripEndNewlineFromString( char *str )
{

}

char *ConvertCRtoNL( char *str )
{

}

int CHudTextMessage::MsgFunc_TextMsg( const char *pszName, int iSize, void *pbuf )
{

}

char *StripCRFromEnd( char *str )
{

}

int CHudTextMessage::MsgFunc_CapMsg( const char *pszName, int iSize, void *pbuf )
{

}

