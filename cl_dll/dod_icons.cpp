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
//  dod_icons.cpp - implementation of the CHudDodIcons class
//

#include "hud.h"
#include "dod_shared.h"

float animTimer;
extern int i_dodmusic;

/*DECLARE_MESSAGE( m_DoDIcons, Health )
DECLARE_MESSAGE( m_DoDIcons, Object )
DECLARE_MESSAGE( m_DoDIcons, ClientAreas )
DECLARE_MESSAGE( m_DoDIcons, ClCorpse )*/

void Credits( void )
{

}

int CHudDodIcons::Init( void )
{
	return 1;
}

void CHudDodIcons::Reset( void )
{

}

void CHudDodIcons::PlayerDied( void )
{

}

int CHudDodIcons::VidInit( void )
{
	return 1;
}

int CHudDodIcons::MsgFunc_Health( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CHudDodIcons::MsgFunc_Object( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

int CHudDodIcons::MsgFunc_ClientAreas( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

extern int g_iDeadFlag;

int CHudDodIcons::Draw( float flTime )
{
	return 1;
}

void CHudDodIcons::DrawMarkerIcon( HSPRITE pSpr )
{

}

void ShowHideWeapons( cl_entity_t *player )
{

}

void CHudDodIcons::MapMarkerPosted( void )
{
	m_fLastMapMarkerTime = gEngfuncs.GetClientTime();
}

void CHudDodIcons::ActivateHintBacking( int team, float flTime )
{

}

void CHudDodIcons::GetHintMessageLocation( int &x, int &y )
{

}

char *szCreditNames[14];

void CHudDodIcons::StartDrawingCredits( void )
{

}

void CHudDodIcons::DrawCredits( float flTime )
{

}

int CHudDodIcons::MsgFunc_ClCorpse( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}