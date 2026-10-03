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

extern IParticleMan *g_pParticleMan;

#define MAX_CLIENTS 32

extern BEAM *pBeam;
extern BEAM *pBeam2;

extern float g_lastFOV;
extern int g_iAlive;

/// USER-DEFINED SERVER MESSAGE HANDLERS

int CHud::MsgFunc_ResetHUD( const char *pszName, int iSize, void *pbuf )
{
	ASSERT( iSize == 0 );

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
		m_iFOV = 0;
		m_flMouseSensitivity = 0.0f;

		for( int i = 0; i < MAX_PLAYERS; i++ )
		{
			m_PlayerFOV[i] = 90;
		}
	}

	if( g_pParticleMan )
		g_pParticleMan->ResetParticles();

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

	m_flMouseSensitivity = 0.0f;

	m_iFOV = 0;
	g_lastFOV = 0.0f;
	return 1;
}

extern void DoD_LoadClientEnts( const char *map );

void CHud::MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
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
	DoD_LoadClientEnts( gEngfuncs.pfnGetLevelName() );
}

int CHud::MsgFunc_Damage( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	READ_BYTE();
	READ_BYTE();
	READ_COORD();
	READ_COORD();
	READ_COORD();
	return 1;
}
