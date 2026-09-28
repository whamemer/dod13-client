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
#include "cl_util.h"
#include "parsemsg.h"
#include "com_model.h"
#include "tri.h"
#include "fx_flags.h"
#include "dod_shared.h"

DECLARE_MESSAGE( m_PShooter, PShoot )

extern cvar_t *cl_particlefx;

int CParticleShooter::Init( void )
{
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );
	HOOK_MESSAGE( PShoot );
	memset( m_sShooters, 0, sizeof( m_sShooters ) );
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
		particle_shooter_t *pCurrentShooter = &m_sShooters[m_iNumShooters];

		*pCurrentShooter = *pShooter;

		HSPRITE hSpriteHandle = gEngfuncs.pfnSPR_Load( pCurrentShooter->szSprite );

		pCurrentShooter->pSprite = (model_s *)gEngfuncs.GetSpritePointer( hSpriteHandle );
		pCurrentShooter->fNextShootTime = -1.0f;
		pCurrentShooter->iState = 0;

		m_iNumShooters++;
	}
}

int CParticleShooter::MsgFunc_PShoot( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int group_id = READ_BYTE();
	int state = READ_BYTE();

	if( state != 0 )
	{
		vec3_t vel;
		vel.x = READ_COORD();
		vel.y = READ_COORD();
		vel.z = READ_COORD();

		for( int i = 0; i < MAX_SHOOTERS; i++ )
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
		for( int j = 0; j < MAX_SHOOTERS; j++ )
		{
			if( m_sShooters[j].id == group_id )
			{
				m_sShooters[j].iState = 0;
				m_sShooters[j].iParticlesRemaining = 0;
			}
		}
	}

	return 1;
}

void CParticleShooter::Think( void )
{
	if( cl_particlefx->value < 2.0f )
		return;

	float fTime = gEngfuncs.GetClientTime();

	for( int i = 0; i < m_iNumShooters; i++ )
	{
		particle_shooter_t *pShooter = &m_sShooters[i];

		if( !pShooter->iState )
			continue;

		if( pShooter->iParticlesRemaining > 0 || pShooter->iNumParticles == -1 )
		{
			if( fTime <= pShooter->fNextShootTime || pShooter->fNextShootTime == 0.0f )
				continue;

			if( pShooter->iParticlesRemaining > 0 )
				pShooter->iParticlesRemaining--;

			pShooter->fNextShootTime = fTime + pShooter->fFireDelay;

			model_s *sprite = pShooter->pSprite;

			if( !sprite )
			{
				gEngfuncs.Con_DPrintf( "Bad sprite in particle_shooter: %s\n", pShooter->szSprite );
				continue;
			}

			vec3_t vecOrigin = pShooter->vOrigin;
			vec3_t vecNormal = Vector( 0.0f, 0.0f, 1.0f );

			CDoDParticle *pParticle;

			pParticle = pParticle->Create( vecOrigin, vecNormal, sprite, pShooter->fSize, pShooter->fBrightness, "dod_particle", true );

			if( !pParticle )
				return;

			pParticle->m_vVelocity.x = pShooter->vVelocity.x + ( gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * pShooter->fVariance );
			pParticle->m_vVelocity.y = pShooter->vVelocity.y + ( gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * pShooter->fVariance );
			pParticle->m_vVelocity.z = pShooter->vVelocity.z + ( gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * pShooter->fVariance );

			pParticle->m_iFrame = 0;
			pParticle->m_flDieTime = fTime + pShooter->fParticleLife;
			pParticle->m_iRendermode = pShooter->iRenderMode;
			pParticle->m_iPFlags = 0;

			int iColFlag = 0;

			if( ( pShooter->iFlags & SF_PARTICLESHOOTER_SPIRAL ) != 0 )
				iColFlag |= TRI_WIND;

			if( ( pShooter->iFlags & SF_PARTICLESHOOTER_COLLIDE_WITH_WORLD ) != 0 )
				iColFlag |= TRI_WATERTRACE;

			if( ( pShooter->iFlags & SF_PARTICLESHOOTER_AFFECTED_BY_FORCE ) != 0 )
				pParticle->m_bAffectedByForce = false;

			if( ( pShooter->iFlags & SF_PARTICLESHOOTER_ANIMATED ) != 0 )
			{
				pParticle->m_iFramerate = pShooter->iFramerate;
				pParticle->m_iNumFrames = sprite->numframes;
			}
			else
			{
				pParticle->m_iFramerate = 0;
			}

			if( ( pShooter->iFlags & SF_PARTICLESHOOTER_KILLED_ON_COLLIDE ) != 0 )
				iColFlag |= TRI_COLLIDEDAMP;

			if( ( pShooter->iFlags & SF_PARTICLESHOOTER_RIPPLE_WHEN_HITTING_WATER ) != 0 )
			{
				pParticle->m_iPFlags |= PFLAG_DOD_WATER_RIPPLE;
				iColFlag |= ( TRI_COLLIDEKILL_ANIM | TRI_COLLIDESLIDE );
			}

			if( pShooter->iFlags & SF_PARTICLESHOOTER_AFFECTED_BY_WIND )
				pParticle->AddGlobalWind();

			pParticle->SetCollisionFlags( iColFlag );
			pParticle->SetLightFlag( LIGHT_INTENSITY );
			pParticle->SetCullFlag( CULL_FRUSTUM_SPHERE | CULL_PVS );
			pParticle->SetRenderFlag( RENDER_FACEPLAYER );

			pParticle->m_flMass = gEngfuncs.pfnRandomFloat( 1.0f, 1.5f );

			pParticle->m_flFadeSpeed = pShooter->fFadeSpeed;
			pParticle->m_flScaleSpeed = pShooter->fScaleSpeed;
			pParticle->m_flGravity = pShooter->fGravity;

			pParticle->m_vAVelocity = Vector( 0, 0, 0 );

			pParticle->m_flDampingTime = pShooter->fDampingTime;
			pParticle->m_flDampingVelocity = pShooter->fDampingVel;

			pParticle->m_vColor.x = ( float ) pShooter->iColour[0];
			pParticle->m_vColor.y = ( float ) pShooter->iColour[1];
			pParticle->m_vColor.z = ( float ) pShooter->iColour[2];
		}
	}
}