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
// dod_icons.cpp
//
// implementation of CHudDodIcons class
//

#include "hud.h"
#include "cl_util.h"

float animTimer;

extern int i_dodmusic;

DECLARE_MESSAGE( m_Icons, Health )
DECLARE_MESSAGE( m_Icons, Object )
DECLARE_MESSAGE( m_Icons, ClientAreas )
DECLARE_MESSAGE( m_Icons, ClCorpse )

void Credits( void )
{

}

int CHudDodIcons::Init( void )
{

}

void CHudDodIcons::Reset( void )
{

}

void CHudDodIcons::PlayerDied( void )
{

}

int CHudDodIcons::VidInit( void )
{

}

int CHudDodIcons::MsgFunc_Health( const char *pszName, int iSize, void *pbuf )
{

}

int CHudDodIcons::MsgFunc_Object( const char *pszName, int iSize, void *pbuf )
{

}

int CHudDodIcons::MsgFunc_ClientAreas( const char *pszName, int iSize, void *pbuf )
{

}

int CHudDodIcons::Draw( float flTime )
{

}

void CHudDodIcons::DrawMarkerIcon( HSPRITE pSpr )
{

}

void ShowHideWeapons( cl_entity_t *player )
{

}

void CHudDodIcons::MapMarkerPosted( void )
{

}

void CHudDodIcons::ActivateHintBacking( int team, float flTime )
{

}

void CHudDodIcons::GetHintMessageLocation( const int *x, const int *y )
{

}

void CHudDodIcons::StartDrawingCredits( void )
{

}

void CHudDodIcons::DrawCredits( float flTime )
{

}

int CHudDodIcons::MsgFunc_ClCorpse( const char *pszName, int iSize, void *pbuf )
{

}