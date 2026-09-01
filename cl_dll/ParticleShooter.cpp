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
//  ParticleShooter.cpp - implementation of the CParticleShooter class
//

#include "hud.h"
#include "dod_shared.h"

//DECLARE_MESSAGE( m_PShooter, PShoot )

extern cvar_t *cl_particlefx;

int CParticleShooter::Init( void )
{
	return 1;
}

int CParticleShooter::VidInit( void )
{
	return 1;
}

void CParticleShooter::AddParticleSystem( particle_shooter_t *pShooter )
{

}

int CParticleShooter::MsgFunc_PShoot( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

void CParticleShooter::Think( void )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}