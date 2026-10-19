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

#include "wrect.h"
#include "cl_dll.h"
#include "particleman.h"
#include "pman_main.h"
#include "pman_misc.h"
#include "pman_force.h"
#include "pman_frustum.h"
#include "cvardef.h"

// gEngfuncs and g_pParticleMan are defined by the client (cdll_int.cpp)

// Show statistics.
cvar_t* cl_pmanstats = NULL;
ForceList g_pForceList;

Vector g_vViewAngles;
float g_flGravity;
int g_iRenderMode = 1;


void ParticleMan_Shutdown( void )
{
	CMiniMem::Instance()->Reset();
	CMiniMem::Instance()->Shutdown();

	g_pForceList.ClearForceMembers();
}


bool IsGamePaused( void )
{
	return gEngfuncs.GetClientTime() == g_flOldTime;
}

void IParticleMan_Active::SetUp( cl_enginefunc_t* pEnginefuncs )
{
	// the engine interface is already stored in the client's gEngfuncs
	cl_pmanstats = gEngfuncs.pfnRegisterVariable("cl_pmanstats", "0", 0);
	g_lMaxParticleClassSize = 0;
	MaxParticleClassSize(g_lMaxParticleClassSize);
}

void IParticleMan_Active::Update( void )
{
	g_pParticleMan = this;
	g_pForceList.UpdateForce(gEngfuncs.GetClientTime());
	g_cFrustum.CalculateFrustum();
	UpdateParticles();

	if (cl_pmanstats && cl_pmanstats->value == 1.0)
	{
		gEngfuncs.Con_NPrintf(15, "Number of Particles: %d", CMiniMem::Instance()->GetTotalParticles());
		gEngfuncs.Con_NPrintf(16, "Particles Drawn: %d", CMiniMem::Instance()->GetDrawnParticles());
		gEngfuncs.Con_NPrintf(17, "CMiniMem Free: %d%%", CMiniMem::Instance()->PercentUsed());
	}
}

void IParticleMan_Active::SetRender( int iRender )
{
	g_iRenderMode = iRender;
}

void IParticleMan_Active::SetVariables( float flGravity, Vector vViewAngles )
{
	g_flGravity = flGravity;

	if (!IsGamePaused())
		g_vViewAngles = vViewAngles;
}

void IParticleMan_Active::ResetParticles( void )
{
	CMiniMem::Instance()->Reset();
	g_pForceList.ClearForceMembers();
}

CBaseParticle* IParticleMan_Active::CreateParticle( Vector org, Vector normal, model_s* sprite, float size, float brightness, const char* classname )
{
	CBaseParticle* pEffect = new CBaseParticle;

	pEffect->InitializeSprite(org, normal, sprite, size, brightness);
	strcpy(pEffect->m_szClassname, classname);
	return pEffect;
}

void IParticleMan_Active::ApplyForce( Vector vOrigin, Vector vDirection, float flRadius, float flStrength, float flDuration )
{
	ForceMember* pNewForce = g_pForceList.AddForceMember();
	if (!pNewForce)
		return;

	pNewForce->m_vOrigin = vOrigin;
	pNewForce->m_vDirection = vDirection;
	pNewForce->m_flRadius = flRadius;
	pNewForce->m_flStrength = flStrength;
	pNewForce->m_flDieTime = gEngfuncs.GetClientTime() + flDuration;
}

void IParticleMan_Active::AddCustomParticleClassSize( unsigned long lSize )
{
	MaxParticleClassSize(lSize);
}

char* IParticleMan_Active::RequestNewMemBlock( int iSize )
{
	if (g_iRenderMode == 0)
	{
		return NULL;
	}

	return CMiniMem::Instance()->newBlock();
}

void IParticleMan_Active::CoreInitializeSprite( CCoreTriangleEffect* pParticle, Vector org, Vector normal, model_s* sprite, float size, float brightness )
{
	pParticle->CCoreTriangleEffect::InitializeSprite(org, normal, sprite, size, brightness);
}

void IParticleMan_Active::CoreThink( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::Think(time);
}

void IParticleMan_Active::CoreDraw( CCoreTriangleEffect* pParticle )
{
	pParticle->CCoreTriangleEffect::Draw();
}

void IParticleMan_Active::CoreAnimate( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::Animate(time);
}

void IParticleMan_Active::CoreAnimateAndDie( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::AnimateAndDie(time);
}

void IParticleMan_Active::CoreExpand( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::Expand(time);
}

void IParticleMan_Active::CoreContract( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::Contract(time);
}

void IParticleMan_Active::CoreFade( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::Fade(time);
}

void IParticleMan_Active::CoreSpin( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::Spin(time);
}

void IParticleMan_Active::CoreCalculateVelocity( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::CalculateVelocity(time);
}

void IParticleMan_Active::CoreCheckCollision( CCoreTriangleEffect* pParticle, float time )
{
	pParticle->CCoreTriangleEffect::CheckCollision(time);
}

void IParticleMan_Active::CoreTouch( CCoreTriangleEffect* pParticle, Vector pos, Vector normal, int index )
{
	pParticle->CCoreTriangleEffect::Touch(pos, normal, index);
}

void IParticleMan_Active::CoreDie( CCoreTriangleEffect* pParticle )
{
	pParticle->CCoreTriangleEffect::Die();
}

void IParticleMan_Active::CoreForce( CCoreTriangleEffect* pParticle )
{
	pParticle->CCoreTriangleEffect::Force();
}

bool IParticleMan_Active::CoreCheckVisibility( CCoreTriangleEffect* pParticle )
{
	return pParticle->CCoreTriangleEffect::CheckVisibility();
}

IParticleMan* CreateActiveMan( void )
{
	return new IParticleMan_Active;
}

