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
//  hud_msg.cpp
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "particleman.h"
#include "dod_camera.h"

extern BEAM *pBeam;
extern BEAM *pBeam2;

extern float g_lastFOV;

/// USER-DEFINED SERVER MESSAGE HANDLERS

int CHud::MsgFunc_ResetHUD( const char *pszName, int iSize, void *pbuf )
{
	// clear all hud data
	HUDLIST *pList = m_pHudList;

	while( pList )
	{
		if( pList->p )
			pList->p->Reset();
		pList = pList->pNext;
	}

	if( g_iAlive )
	{
		g_lastFOV = 0.0f;
		SetFOV( 0 );

		// reset sensitivity
		m_flMouseSensitivity = 0;

		for( int i = 0; i < MAX_PLAYERS; i++ )
			m_PlayerFOV[i] = 90;

		if( g_pParticleMan )
			g_pParticleMan->ResetParticles();
	}

	return 1;
}

int CHud::MsgFunc_YouDied( const char *pszName, int iSize, void *pbuf )
{
	HUDLIST *pList = m_pHudList;

	while( pList )
	{
		if( pList->p )
			pList->p->PlayerDied();
		pList = pList->pNext;
	}

	m_flMouseSensitivity = 0;

	// the original returns here when the VGUI2 viewport shows the widescreen bars (IDoDViewPort::isWidescreenVisible()) and g_iVuser1z is 0
	SetFOV( 0 );
	g_lastFOV = 0.0f;

	return 1;
}

void CHud::MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	// the original first calls IViewPort::OnLevelChange() (VGUI2 viewport)

	// prepare all hud data
	HUDLIST *pList = m_pHudList;

	while( pList )
	{
		if( pList->p )
			pList->p->InitHUDData();
		pList = pList->pNext;
	}

	//Probably not a good place to put this.
	pBeam = pBeam2 = NULL;

	// the points of the new map come with InitObj (the original forgets them in VidInit)
	m_ObjectiveIcons.ClearAllCapPoints();

	DoD_LoadClientEnts( gEngfuncs.pfnGetLevelName() );
}

int CHud::MsgFunc_Damage( const char *pszName, int iSize, void *pbuf )
{
	int		armor, blood;
	vec3_t	from;
	int		i;
	float	count;

	BEGIN_READ( pbuf, iSize );
	armor = READ_BYTE();
	blood = READ_BYTE();

	for( i = 0; i < 3; i++)
		from[i] = READ_COORD();

	count = ( blood * 0.5 ) + ( armor * 0.5 );

	if( count < 10 )
		count = 10;

	// TODO: kick viewangles,  show damage visually
	return 1;
}
