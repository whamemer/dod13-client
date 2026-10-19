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
// weather.cpp
//
// the rain and the snow of the maps
//

#include <math.h>
#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "pm_defs.h"

#include "tri.h"

extern float g_flWeatherTime;
extern int g_iWeatherType;

extern cvar_t *cl_particlefx;

int CWeatherManager::Init( void )
{
	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem( this );

	g_iWeatherType = 0;
	g_flWeatherTime = 0.0f;

	return 1;
}

int CWeatherManager::VidInit( void )
{
	SetRainSprite( (model_t *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/rain.spr" ) ) );
	SetSnowSprite( (model_t *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/snowflake.spr" ) ) );
	SetSplashSprite( (model_t *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/wsplash3.spr" ) ) );
	SetRippleSprite( (model_t *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/ripple.spr" ) ) );

	g_flWeatherTime = 0.0f;
	g_iWeatherType = 0;

	return 1;
}

// the rain drops are shown with the same brightness
void CDoDRainDrop::Think( float flTime )
{
	m_flBrightness = 130.0f;

	CDoDParticle::Think( flTime );
}

// the drop falls till the point it hits, that is when it dies
void CWeatherManager::CreateRainParticle( float *origin )
{
	float flTime;
	CDoDRainDrop *pParticle;
	vec3_t vel;
	vec3_t end;
	pmtrace_t *tr;
	vec3_t len;

	flTime = gEngfuncs.GetClientTime();

	pParticle = new CDoDRainDrop;

	pParticle->InitializeSprite( Vector( origin ), Vector( 0.0f, 0.0f, 0.0f ), m_pRainSprite, 2.0f, 1.0f );

	pParticle->m_flStretchY = 40.0f;
	pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -50.0f, 50.0f );
	pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -50.0f, 50.0f );
	pParticle->m_flGravity = 0.0f;
	pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( -500.0f, -1500.0f );

	vel = pParticle->m_vVelocity.Normalize();

	VectorMA( origin, 2000.0f, vel, end );

	tr = gEngfuncs.PM_TraceLine( origin, end, PM_TRACELINE_PHYSENTSONLY, 2, -1 );

	len = Vector( tr->endpos ) - Vector( origin );

	pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_ANIMATEDIE | TRI_WATERTRACE );

	pParticle->m_iPFlags = PFLAG_DOD_COLLIDE_SPLASH | PFLAG_DOD_VELOCITY_ROTATE | PFLAG_DOD_WATER_RIPPLE | PFLAG_DOD_DIRT_DEBRIS;
	pParticle->m_iRendermode = kRenderTransAlpha;

	pParticle->SetLightFlag( LIGHT_NONE );
	pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_PLANE );

	pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );
	pParticle->m_flDieTime = len.Length() / pParticle->m_vVelocity.Length() + flTime;

	pParticle->AddGlobalWind();
}

void CDoDSnowFlake::Think( float time )
{
	float frametime;

	if( time > m_flFadeOutTime )
	{
		CBaseParticle::Fade( time );

		m_flOldTime = time;
	}
	else
	{
		frametime = time - m_flOldTime;

		// the spiral snowflakes swing on the x axis, every one with an own phase
		if( m_bSpiral )
			m_vOrigin.x = m_vVelocity.x * frametime + sin( 5.0f * time + (float)(int)(size_t)this ) * 2.0f + m_vOrigin.x;
		else
			m_vOrigin.x = m_vVelocity.x * frametime + m_vOrigin.x;

		m_vOrigin.y = m_vVelocity.y * frametime + m_vOrigin.y;
		m_vOrigin.z = m_vVelocity.z * frametime + m_vOrigin.z;

		CheckCollision( time );

		m_flOldTime = time;
	}
}

// the snowflake that lands stays for a while on the ground
void CDoDSnowFlake::Touch( Vector pos, Vector normal, int index )
{
	if( !m_bTouched )
	{
		SetRenderFlag( RENDER_FACEPLAYER );

		m_bTouched = true;
		m_flOriginalBrightness = m_flBrightness;
		m_bSpiral = false;
		m_vVelocity = Vector( 0.0f, 0.0f, 0.0f );
		m_iRendermode = kRenderTransAdd;
		m_flFadeSpeed = 0.0f;
		m_flScaleSpeed = 0.0f;
		m_flDampingTime = 0.0f;
		m_iFrame = 0;
		m_flMass = 1.0f;
		m_flGravity = 0.0f;
		m_vColor = Vector( 128.0f, 128.0f, 128.0f );
		m_flDieTime = gEngfuncs.GetClientTime() + 0.5f;
		m_iPFlags = 0;
		m_flTimeCreated = gEngfuncs.GetClientTime();
	}
}

CDoDSnowFlake *CDoDSnowFlake::Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname )
{
	CDoDSnowFlake *pEffect;

	if( !sprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDParticle::Create called with a null sprite\n" );
		return NULL;
	}

	if( cl_particlefx->value <= 0.0f )
		return NULL;

	pEffect = new CDoDSnowFlake;

	pEffect->InitializeSprite( org, normal, sprite, size, brightness );

	pEffect->m_iPFlags = 0;
	pEffect->m_iNumFrames = sprite->numframes;

	strcpy( pEffect->m_szClassname, classname );

	return pEffect;
}

void CWeatherManager::CreateSnowParticle( float *origin )
{
	CDoDSnowFlake *pParticle;
	float r;

	pParticle = CDoDSnowFlake::Create( Vector( origin ), Vector( 0.0f, 0.0f, 0.0f ), m_pSnowSprite, 2.0f, 1.0f, "dod_snow" );

	if( !pParticle )
		return;

	pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -30.0f, 30.0f );
	pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -30.0f, 30.0f );
	pParticle->m_vVelocity.z = -110.0f;

	// some of the flakes are slower
	r = gEngfuncs.pfnRandomFloat( 0.0f, 1.0f );

	if( r < 0.1f )
	{
		pParticle->m_vVelocity.x = pParticle->m_vVelocity.x * 0.5f;
		pParticle->m_vVelocity.y = pParticle->m_vVelocity.y * 0.5f;
	}
	else if( r >= 0.1f && r < 0.2f )
	{
		pParticle->m_vVelocity.z = -65.0f;
	}
	else if( r >= 0.2f && r < 0.3f )
	{
		pParticle->m_vVelocity.z = -75.0f;
	}

	pParticle->m_iRendermode = kRenderTransAdd;

	pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
	pParticle->SetLightFlag( LIGHT_NONE );
	pParticle->SetRenderFlag( RENDER_FACEPLAYER );

	pParticle->m_flScaleSpeed = 0.0f;
	pParticle->m_flDampingTime = 0.0f;
	pParticle->m_iFrame = 0;
	pParticle->m_flMass = 1.0f;
	pParticle->m_flGravity = 0.0f;
	pParticle->m_flBounceFactor = 0.0f;
	pParticle->m_flBrightness = 128.0f;
	pParticle->m_flFadeOutTime = gEngfuncs.GetClientTime() + 5.0f;
	pParticle->m_vColor = Vector( 128.0f, 128.0f, 128.0f );
	pParticle->m_iPFlags = PFLAG_DOD_WEATHER_RAIN;
	pParticle->m_bTouched = false;
	pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 15.0f;
	pParticle->m_bSpiral = ( gEngfuncs.pfnRandomLong( 0, 2 ) <= 0 );

	pParticle->AddGlobalWind();

	pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_COLLIDEKILL );
}
