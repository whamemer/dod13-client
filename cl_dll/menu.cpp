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
// menu.cpp
//
// implementation of CHudMenu class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <stdio.h>

#define MAX_MENU_STRING	512
char g_szMenuString[MAX_MENU_STRING];
char g_szPrelocalisedMenuString[MAX_MENU_STRING];

DECLARE_MESSAGE( m_Menu, ShowMenu )

int CHudMenu::Init( void )
{

}

void CHudMenu::InitHUDData( void )
{

}

void CHudMenu::Reset( void )
{

}

int CHudMenu::VidInit( void )
{

}

static int menu_ralign;

inline const char *ParseEscapeToken( const char  *token )
{

}

int CHudMenu::Draw( float flTime )
{

}

void CHudMenu::SelectMenuItem( int menu_item )
{

}

int CHudMenu::MsgFunc_ShowMenu( const char *pszName, int iSize, void *pbuf )
{

}