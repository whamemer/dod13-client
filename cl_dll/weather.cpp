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
//  weather.cpp - implementation of the CWeatherManager class
//

#include "hud.h"
#include "tri.h"
#include "cl_util.h"
#include "pmtrace.h"
#include "event_api.h"

extern CBaseParticle *g_pBaseParticle;

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
	HSPRITE hRain = gEngfuncs.pfnSPR_Load( "sprites/rain.spr" );
	SetRainSprite( (model_s *)gEngfuncs.GetSpritePointer( hRain ) );

	HSPRITE hSnow = gEngfuncs.pfnSPR_Load( "sprites/snowflake.spr" );
	SetSnowSprite( (model_s *)gEngfuncs.GetSpritePointer( hSnow ) );

	HSPRITE hSplash = gEngfuncs.pfnSPR_Load( "sprites/wsplash3.spr" );
	SetSplashSprite( (model_s *)gEngfuncs.GetSpritePointer( hSplash ) );

	HSPRITE hRipple = gEngfuncs.pfnSPR_Load( "sprites/ripple.spr" );
	SetRippleSprite( (model_s *)gEngfuncs.GetSpritePointer( hRipple ) );

	g_flWeatherTime = 0.0f;
	g_iWeatherType = 0;

	return 1;
}

void CDoDRainDrop::Think( float flTime )
{
	m_flBrightness = 130.0;
	CDoDParticle::Think( flTime );
}

void CWeatherManager::CreateRainParticle( float *origin )
{
	if( !m_pRainSprite )
		return;

	float flTime = gEngfuncs.GetClientTime();

	CDoDRainDrop *pParticle = new CDoDRainDrop();

	if( !pParticle )
		return;

	vec3_t vecOrigin = Vector( origin[0], origin[1], origin[2] );
	vec3_t vecNormal = Vector( 0.0f, 0.0f, 0.0f );

	pParticle->InitializeSprite( &vecOrigin, &vecNormal, m_pRainSprite, 2.0f, 1.0f );
	pParticle->m_flStretchY = 40.0f;

	pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -24.0f, 48.0f );
	pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -24.0f, 48.0f );
	pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( -400.0f, -200.0f );
	pParticle->m_flGravity = 0.0f;

	float flSpeed = pParticle->m_vVelocity.Length();
	vec3_t vel = pParticle->m_vVelocity;
	vel.Normalize();

	vec3_t end;
	VectorMA( origin, 2000.0f, &vel.x, &end.x );

	pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecOrigin, end, 8, -1, &tr );

	vec3_t len = tr.endpos - vecOrigin;
	float flLen = len.Length();

	pParticle->SetCollisionFlags( TRI_COLLIDESLIDE | TRI_COLLIDEDAMP | TRI_COLLIDEKILL_ANIM | TRI_WATERTRACE );
	pParticle->SetLightFlag( LIGHT_NONE );
	pParticle->SetCullFlag( CULL_FRUSTUM_SPHERE );
	pParticle->SetRenderFlag( RENDER_FACEPLAYER );

	pParticle->m_iPFlags = PFLAG_DOD_COLLIDE_SPLASH | PFLAG_DOD_WATER_RIPPLE | PFLAG_DOD_WIND_AFFECTED;
	pParticle->m_iRendermode = kRenderTransAlpha;
	pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );

	pParticle->m_flDieTime = ( flLen / flSpeed ) + flTime;
	pParticle->AddGlobalWind();
}

void CDoDSnowFlake::Think( float time )
{
	if( time > m_flFadeOutTime )
	{
		Fade( time );
		m_flOldTime = time;
	}
	else
	{
		float frametime = time - m_flOldTime;

		if( m_bSpiral )
		{
			float flWave = sin( 5.0f * time + m_vRingOrigin.x );

			m_vOrigin.x += ( m_vVelocity.x * frametime ) + ( 2.0f * flWave );
		}
		else
		{
			m_vOrigin.x += m_vVelocity.x * frametime;
		}

		m_vOrigin.y += m_vVelocity.y * frametime;
		m_vOrigin.z += m_vVelocity.z * frametime;

		CDoDParticle::Think( time );

		m_flOldTime = time;
	}
}

void CDoDSnowFlake::Touch( vec3_t pos, vec3_t normal, int index )
{
	if( !m_bTouched )
	{
		float flOriginalBrightness = m_flBrightness;

		m_bTouched = true;
		m_flOriginalBrightness = flOriginalBrightness;

		SetRenderFlag( RENDER_FACEPLAYER );

		m_bSpiral = false;
		m_vVelocity = Vector( 0.0f, 0.0f, 0.0f );
		m_flGravity = 0.0f;
		m_flMass = 1.0f;

		m_iRendermode = kRenderTransAdd;

		m_flFadeSpeed = 0.0f;
		m_flScaleSpeed = 0.0f;
		m_flDampingTime = 0.0f;
		m_iFrame = 0;

		m_vColor = Vector( 128.0f, 128.0f, 128.0f );

		float flClientTime = gEngfuncs.GetClientTime();
		m_flDieTime = flClientTime + 0.5f;
		m_flTimeCreated = flClientTime;

		m_iPFlags = 0;
	}
}

CDoDSnowFlake *CDoDSnowFlake::Create( vec3_t org, vec3_t normal, model_s *sprite, float size, float brightness, const char *classname )
{
	if( !sprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDParticle::Create called with a null sprite\n" );
		return NULL;
	}

	CDoDSnowFlake *pEffect = NULL;

	if( cl_particlefx->value > 0.0f )
	{
		pEffect = new CDoDSnowFlake();

		if( pEffect )
		{
			vec3_t vecOrigin = org;
			vec3_t vecNormal = normal;

			pEffect->InitializeSprite( &vecOrigin, &vecNormal, sprite, size, brightness );
			pEffect->m_iPFlags = 0;
			pEffect->m_iNumFrames = sprite->numframes;

			strncpy( pEffect->m_szClassname, classname, sizeof( pEffect->m_szClassname ) - 1 );
			pEffect->m_szClassname[sizeof( pEffect->m_szClassname ) - 1] = '\0';
		}
	}

	return pEffect;
}

void CWeatherManager::CreateSnowParticle( float *origin )
{
	CDoDSnowFlake *pParticle;

	if( !m_pSnowSprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDParticle::Create called with a null sprite\n" );
		return;
	}

	if( cl_particlefx->value > 0.0f )
	{
		vec3_t vecOrigin = Vector( *origin, *origin, *origin );
		vec3_t vecNormal = Vector( 0.0f, 0.0f, 0.0f );

		pParticle = pParticle->Create( vecOrigin, vecNormal, m_pSnowSprite, 2.0f, 1.0f, "dod_snow" );

		if( !pParticle )
			return;

		pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -15.0f, 15.0f );
		pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -15.0f, 15.0f );
		pParticle->m_vVelocity.z = -110.0f;

		pParticle->m_flGravity = 0.0f;

		float r = gEngfuncs.pfnRandomFloat( 0.0f, 1.0f );
		if( r >= 0.1f )
		{
			if( r < 0.2f )
				pParticle->m_vVelocity.z = -65.0f;
			else if( r < 0.3f )
				pParticle->m_vVelocity.z = -75.0f;
		}
		else
		{
			pParticle->m_vVelocity.x *= 0.5f;
			pParticle->m_vVelocity.y *= 0.5f;
		}

		float flClientTime = gEngfuncs.GetClientTime();

		pParticle->m_iRendermode = kRenderTransAdd;

		pParticle->m_flScaleSpeed = 0.0f;
		pParticle->m_flDampingTime = 0.0f;
		pParticle->m_iFrame = 0;
		pParticle->m_flMass = 1.0f;
		pParticle->m_flBounceFactor = 0.0f;
		pParticle->m_flBrightness = 128.0f;
		pParticle->m_vColor = Vector( 128.0f, 128.0f, 128.0f );

		pParticle->m_flOldTime = flClientTime;
		pParticle->m_flFadeOutTime = flClientTime + 5.0f;
		pParticle->m_iPFlags = PFLAG_DOD_WEATHER_RAIN;

		pParticle->m_flDieTime = flClientTime + 15.0f;
		pParticle->m_bSpiral = ( gEngfuncs.pfnRandomLong( 0, 2 ) <= 0 );

		pParticle->SetCollisionFlags( TRI_COLLIDEKILL | TRI_WATERTRACE );
		pParticle->SetLightFlag( LIGHT_NONE );
		pParticle->SetCullFlag( CULL_FRUSTUM_SPHERE | CULL_PVS );
		pParticle->SetRenderFlag( RENDER_FACEPLAYER_ROTATEZ );

		pParticle->AddGlobalWind();
	}
}