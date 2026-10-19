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
// ParticleShooter.cpp
//
// the particle_shooter entities of the map are parsed by the client (see DoD_ProcessEnts),
// the server only starts and stops them with the message PShoot
//

#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "tri.h"
#include "fx_flags.h"

DECLARE_MESSAGE( m_PShooter, PShoot )

extern cvar_t *cl_particlefx;

int CParticleShooter::Init( void )
{
	m_iFlags |= HUD_ACTIVE;

	HOOK_MESSAGE( PShoot );

	memset( m_sShooters, 0, sizeof( m_sShooters ) );

	gHUD.AddHudElem( this );

	m_iNumShooters = 0;

	return 1;
}

int CParticleShooter::VidInit( void )
{
	m_iNumShooters = 0;

	memset( m_sShooters, 0, sizeof( m_sShooters ) );

	return 1;
}

void CParticleShooter::AddParticleSystem( particle_shooter_t *pShooter )
{
	if( m_iNumShooters < MAX_SHOOTERS )
	{
		m_sShooters[m_iNumShooters] = *pShooter;

		m_sShooters[m_iNumShooters].pSprite = (model_t *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( m_sShooters[m_iNumShooters].szSprite ) );

		// it does not shoot until the server starts it
		m_sShooters[m_iNumShooters].fNextShootTime = -1.0f;
		m_sShooters[m_iNumShooters].iParticlesRemaining = 0;

		m_iNumShooters++;
	}
}

int CParticleShooter::MsgFunc_PShoot( const char *pszName, int iSize, void *pbuf )
{
	int group_id;
	int state;
	vec3_t vel;
	int i;

	BEGIN_READ( pbuf, iSize );

	group_id = READ_BYTE();
	state = READ_BYTE();

	if( state )
	{
		vel.x = READ_COORD();
		vel.y = READ_COORD();
		vel.z = READ_COORD();

		for( i = 0; i < MAX_SHOOTERS; i++ )
		{
			if( m_sShooters[i].id == group_id )
			{
				m_sShooters[i].iState = state;
				m_sShooters[i].vVelocity = vel;
				m_sShooters[i].fNextShootTime = gHUD.m_flTime;
				m_sShooters[i].iParticlesRemaining = m_sShooters[i].iNumParticles;
			}
		}
	}
	else
	{
		for( i = 0; i < MAX_SHOOTERS; i++ )
		{
			if( m_sShooters[i].id == group_id )
			{
				m_sShooters[i].iState = 0;
				m_sShooters[i].iParticlesRemaining = 0;
			}
		}
	}

	return 1;
}

void CParticleShooter::Think( void )
{
	particle_shooter_t *pShooter;
	float fTime;
	int i;
	CDoDParticle *pParticle;
	int iColFlag;

	fTime = gEngfuncs.GetClientTime();

	if( cl_particlefx->value < 2.0f )
		return;

	for( i = 0; i < m_iNumShooters; i++ )
	{
		pShooter = &m_sShooters[i];

		if( !pShooter->iState )
			continue;

		// -1 particles are shot for ever
		if( pShooter->iParticlesRemaining <= 0 && pShooter->iNumParticles != -1 )
			continue;

		if( fTime <= pShooter->fNextShootTime || pShooter->fNextShootTime == 0.0f )
			continue;

		if( pShooter->iParticlesRemaining > 0 )
			pShooter->iParticlesRemaining--;

		pShooter->fNextShootTime = fTime + pShooter->fFireDelay;

		if( !pShooter->pSprite )
		{
			gEngfuncs.Con_DPrintf( "Bad sprite in particle_shooter: %s\n", pShooter->szSprite );
			continue;
		}

		pParticle = CDoDParticle::Create( pShooter->vOrigin, Vector( 0, 0, 1 ), pShooter->pSprite, pShooter->fSize, pShooter->fBrightness, "dod_particle", true );

		if( !pParticle )
			return;

		pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * pShooter->fVariance + pShooter->vVelocity.x;
		pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * pShooter->fVariance + pShooter->vVelocity.y;
		pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * pShooter->fVariance + pShooter->vVelocity.z;

		pParticle->m_iFrame = 0;
		pParticle->m_flDieTime = fTime + pShooter->fParticleLife;
		pParticle->m_iRendermode = pShooter->iRenderMode;
		pParticle->m_iPFlags = 0;

		// the spawnflags of the entity
		iColFlag = ( pShooter->iFlags & SF_PARTICLESHOOTER_SPIRAL ) ? TRI_SPIRAL : 0;

		if( pShooter->iFlags & SF_PARTICLESHOOTER_COLLIDE_WITH_WORLD )
			iColFlag |= TRI_COLLIDEWORLD;

		if( pShooter->iFlags & SF_PARTICLESHOOTER_AFFECTED_BY_FORCE )
			pParticle->m_bAffectedByForce = false;

		if( pShooter->iFlags & SF_PARTICLESHOOTER_ANIMATED )
		{
			pParticle->m_iFramerate = pShooter->iFramerate;
			pParticle->m_iNumFrames = pShooter->pSprite->numframes;
		}
		else
		{
			pParticle->m_iFramerate = 0;
		}

		if( pShooter->iFlags & SF_PARTICLESHOOTER_KILLED_ON_COLLIDE )
			iColFlag |= TRI_COLLIDEKILL;

		if( pShooter->iFlags & SF_PARTICLESHOOTER_RIPPLE_WHEN_HITTING_WATER )
		{
			pParticle->m_iPFlags |= PFLAG_DOD_WATER_RIPPLE;
			iColFlag |= TRI_WATERTRACE;
		}

		if( pShooter->iFlags & SF_PARTICLESHOOTER_AFFECTED_BY_WIND )
			pParticle->AddGlobalWind();

		pParticle->SetLightFlag( LIGHT_NONE );
		pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_PLANE );

		pParticle->m_flMass = gEngfuncs.pfnRandomFloat( 1.0f, 1.5f );
		pParticle->m_flFadeSpeed = pShooter->fFadeSpeed;
		pParticle->m_flScaleSpeed = pShooter->fScaleSpeed;
		pParticle->m_flGravity = pShooter->fGravity;
		pParticle->SetCollisionFlags( iColFlag );
		pParticle->m_vAVelocity = Vector( 0.0f, 0.0f, 0.0f );
		pParticle->m_flDampingTime = pShooter->fDampingTime;
		pParticle->m_flDampingVelocity = pShooter->fDampingVel;
		pParticle->m_vColor = Vector( (float)pShooter->iColour[0], (float)pShooter->iColour[1], (float)pShooter->iColour[2] );
	}
}
