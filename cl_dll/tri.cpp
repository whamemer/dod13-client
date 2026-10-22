//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include <string.h>

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "event_api.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "r_studioint.h"

#include "view.h"
#include "tri.h"
#include "ev_dod.h"

extern engine_studio_api_t IEngineStudio;

// the sizes of the classes of the original (the memory blocks of the particleman are made for them), 32-bit only
#define CHECK_ORIGINAL_SIZE( type, size ) typedef char size_of_##type[( sizeof( void * ) != 4 || sizeof( type ) == ( size ) ) ? 1 : -1]

CHECK_ORIGINAL_SIZE( CBaseParticle, 276 );
CHECK_ORIGINAL_SIZE( CBaseDoDParticle, 276 );
CHECK_ORIGINAL_SIZE( CDoDParticle, 300 );
CHECK_ORIGINAL_SIZE( TriangleWallPuff, 280 );
CHECK_ORIGINAL_SIZE( CDoDRocketTrail, 280 );
CHECK_ORIGINAL_SIZE( CDoDDirtExploDust, 284 );
CHECK_ORIGINAL_SIZE( CDoDSnowFlake, 312 );
CHECK_ORIGINAL_SIZE( CDoDRainDrop, 300 );

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
}

float g_flWeatherTime;
int g_iWeatherType;

extern cvar_t *cl_particlefx;
extern cvar_t *cl_fog;

extern float g_flGravity;

extern vec3_t v_origin;
extern vec3_t v_angles;
extern vec3_t v_lastAngles;

void V_GetDeathCam( cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin );

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{
	/* Nothing. */
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{
	if( g_iOnlyClientDraw )
	{
		// the second view of the frame: the map
		if( g_iOnlyClientDraw == 1 && gHUD.GetMinimapState() > 0 )
		{
#if FOG_MINIMAP_FIX
			SuspendDoDFog();
#endif
			gHUD.m_ObjectiveIcons.CalcIconLocations();
			gHUD.m_DoDMap.DrawOverview();
#if FOG_MINIMAP_FIX
			if( cl_fog->value != 0.0f )
				RenderDoDFog();
#endif
		}
	}
	else
	{
		if( cl_fog->value != 0.0f )
			RenderDoDFog();

		if( cl_particlefx->value >= 1.0f && IEngineStudio.IsHardware() )
		{
			float time;

			// the spectators see the particles from the view of the spectator
			if( g_iUser1 != 3 && g_iUser1 )
				g_pParticleMan->SetVariables( g_flGravity, v_angles );
			else
				g_pParticleMan->SetVariables( g_flGravity, gHUD.m_vecAngles );

			g_pParticleMan->Update();

			time = gEngfuncs.GetClientTime();

			if( time > g_flWeatherTime && cl_particlefx->value >= 2.0f )
			{
				if( g_iWeatherType == 1 )
				{
					UpdateRain();
					g_flWeatherTime = time + 0.3f;
				}
				else if( g_iWeatherType == 2 )
				{
					UpdateSnow();
					g_flWeatherTime = time + 0.7f;
				}
			}
		}

		if( gHUD.m_iFOV <= 89 )
			gHUD.m_Scope.DrawTriApiScope();

		gHUD.m_MortarHud.DrawPredictedMortarImpactSite();
	}
}

cvar_t *cl_weatherdis;

CDoDParticle *CDoDParticle::Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname, bool bDistCull )
{
	CDoDParticle *pEffect;
	cl_entity_t *player;
	vec3_t vOurOrigin;
	float flDistance;

	if( !sprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDParticle::Create called with a null sprite\n" );
		return NULL;
	}

	if( cl_particlefx->value <= 0.0f )
		return NULL;

	if( !IEngineStudio.IsHardware() )
		return NULL;

	if( cl_weatherdis->value < 100.0f )
		cl_weatherdis->value = 100.0f;

	// the particles that are too far are not created, the height does not count
	if( bDistCull )
	{
		player = gEngfuncs.GetLocalPlayer();

		if( !player )
			return NULL;

		vOurOrigin = player->curstate.origin;
		vOurOrigin.z = org.z;

		flDistance = ( org - vOurOrigin ).Length();

		if( flDistance > cl_weatherdis->value )
			return NULL;
	}

	pEffect = new CDoDParticle;

	pEffect->InitializeSprite( org, normal, sprite, size, brightness );

	pEffect->m_iPFlags = 0;
	pEffect->m_iNumFrames = sprite->numframes;

	strcpy( pEffect->m_szClassname, classname );

	return pEffect;
}

// the splash of the rain
void CDoDParticle::Touch( Vector pos, Vector normal, int index )
{
	float fTime;
	model_t *pSplash;
	CBaseParticle *pParticleSplash;

	if( m_iPFlags & PFLAG_DOD_COLLIDE_SPLASH )
	{
		fTime = gEngfuncs.GetClientTime();
		pSplash = gHUD.m_Weather.GetSplashSprite();

		if( pSplash )
		{
			pParticleSplash = new CBaseParticle;

			pParticleSplash->InitializeSprite( m_vOrigin + normal, Vector( 90, 0, 0 ), pSplash, (float)gEngfuncs.pfnRandomLong( 25, 30 ), 150.0f );

			pParticleSplash->m_iRendermode = kRenderTransAdd;
			pParticleSplash->m_flMass = 1.0f;
			pParticleSplash->m_flGravity = 0.1f;

			pParticleSplash->SetLightFlag( LIGHT_INTENSITY );
			pParticleSplash->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );

			pParticleSplash->m_vColor = Vector( 256, 256, 256 );

			pParticleSplash->m_iNumFrames = pSplash->numframes;
			pParticleSplash->m_iFramerate = gEngfuncs.pfnRandomLong( 15, 60 );
			pParticleSplash->m_flDieTime = fTime + 0.3f;

			pParticleSplash->SetCollisionFlags( TRI_ANIMATEDIE );
			pParticleSplash->SetRenderFlag( RENDER_FACEPLAYER );
		}
	}
}

// the ripple of the rain on the water
void CDoDParticle::Die( void )
{
	vec3_t start;
	vec3_t end;
	vec3_t pt;
	model_t *pRipple;
	pmtrace_t tr;

	if( m_iPFlags & PFLAG_DOD_WATER_RIPPLE )
	{
		pRipple = gHUD.m_Weather.GetRippleSprite();

		if( pRipple )
		{
			start = m_vOrigin;
			start.z += 32.0f;

			end = m_vOrigin;
			end.z -= 16.0f;

			gEngfuncs.pEventAPI->EV_PlayerTrace( start, end, PM_NORMAL, -1, &tr );

			if( EV_DOD_WaterEntryPoint( &tr, start, pt ) )
			{
				CDoDParticle *pParticle = CDoDParticle::Create( pt, Vector( 90, 0, 0 ), pRipple, 15.0f, 254.0f, "dod_particle", false );

				if( pParticle )
				{
					pParticle->m_iRendermode = kRenderTransAdd;
					pParticle->m_flFadeSpeed = 4.0f;
					pParticle->m_flScaleSpeed = 2.0f;

					pParticle->SetLightFlag( LIGHT_COLOR );
					pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );

					pParticle->m_vColor = Vector( 255, 255, 255 );

					pParticle->m_iPFlags = PFLAG_DOD_SPARK_INSTANT;
					pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
				}
			}
		}
	}

	CBaseDoDParticle::Die();
}

// the angles of the view of the one who is watched, as the particles have to face it
Vector GetViewAngles( void )
{
	vec3_t vViewAngles;

	if( g_iUser1 )
	{
		if( g_iUser3 > 0 && g_iUser2 > 0 )
		{
			vec3_t org;

			V_GetDeathCam( gEngfuncs.GetEntityByIndex( g_iUser2 ), gEngfuncs.GetEntityByIndex( g_iUser3 ), vViewAngles, org );
		}
		else if( g_iUser1 == 1 || g_iUser1 == 4 )
		{
			vViewAngles = v_angles;
		}
		else if( g_iUser1 == 2 || g_iUser1 == 3 )
		{
			gEngfuncs.GetViewAngles( vViewAngles );
		}
		else
		{
			vViewAngles = v_lastAngles;
		}
	}
	else
	{
		gEngfuncs.GetViewAngles( vViewAngles );

		if( gHUD.IsInMortarDeploy() )
			vViewAngles.x = 0.0f;
	}

	return vViewAngles;
}

void CDoDParticle::Think( float flTime )
{
	vec3_t right, forward, up;
	vec3_t vViewAngles;

	if( m_flBrightness >= 255.0f && m_flDieTime - flTime < 3.0f )
	{
		m_flScaleSpeed = 0.0f;
		m_vAVelocity = Vector( 0, 0, 0 );
		m_flTimeCreated = flTime;
	}

	if( m_flDieTime - flTime <= 5.0f )
	{
		if( m_flBrightness > 0.0f )
			m_flBrightness = m_flBrightness - ( flTime - m_flTimeCreated ) * 0.1f;

		if( m_flBrightness < 0.0f )
		{
			m_flBrightness = 0.0f;
			m_flDieTime = gEngfuncs.GetClientTime();
		}
	}
	else
	{
		if( m_bInsideSmoke )
		{
			if( m_flBrightness < 255.0f && m_flFadeSpeed < -0.5f )
				m_flBrightness = m_flBrightness + 5.0f * ( flTime - m_flTimeCreated ) + 8.0f;
		}
		else if( m_flBrightness < 255.0f && m_flFadeSpeed < -0.5f )
		{
			m_flBrightness = m_flBrightness + 5.0f * ( flTime - m_flTimeCreated ) + 10.0f;
		}

		if( m_flBrightness > 255.0f )
			m_flBrightness = 255.0f;
	}

	if( ( m_iPFlags & PFLAG_DOD_SHRINK_DIE ) && m_flDieTime - flTime <= 3.0f )
	{
		m_flSize -= 0.02f;

		if( m_flSize < 0.0f )
		{
			m_flSize = 0.0f;
			m_flDieTime = gEngfuncs.GetClientTime();
		}
	}

	CBaseDoDParticle::Think( flTime );

	vViewAngles = GetViewAngles();

	AngleVectors( vViewAngles, forward, right, up );

	if( m_iPFlags & PFLAG_DOD_VELOCITY_ROTATE )
	{
		float dotLength = DotProduct( m_vVelocity, right );

		m_vAngles.z = atan( dotLength / m_vVelocity.z ) * ( 180.0 / M_PI );
	}

	if( !( m_iPFlags & PFLAG_DOD_SPARK_INSTANT ) )
	{
		if( !( m_iPFlags & PFLAG_DOD_DIRT_DEBRIS ) )
			m_vAngles.x = vViewAngles.x;

		m_vAngles.y = vViewAngles.y;
	}

	if( m_flDampingTime != 0.0f && flTime >= m_flDampingTime )
	{
		if( m_bDampMod )
		{
			m_flDampingTime = 0.0f;
			m_vVelocity = m_vVelocity * 0.035f;
		}
		else
		{
			m_flDampingTime = gEngfuncs.GetClientTime() + 0.2f;
			m_bDampMod = true;

			if( m_bSpawnInside )
				CreateExplosionSmokeInside( m_vRingOrigin );
		}
	}

	if( m_iPFlags & PFLAG_DOD_EXPLOSION_PULSE )
	{
		float timeleft = m_flDieTime - flTime;
		float time = 25.0f - timeleft;
		float ssize_dem = 0.2f;
		float ssize = time / ssize_dem * 300.0f / 100.0f * 100.0f;

		if( ssize > 300.0f )
			ssize = 300.0f;

		m_flSize = ssize;
	}
}

void CDoDParticle::Force( void )
{
	if( !m_bAffectedByForce )
	{
		CreateExplosionSmoke( m_vOrigin, Vector( 0, 0, 0 ), false, false, false );
		m_bAffectedByForce = true;
	}
}

vec3_t CDoDParticle::m_vGlobalWind;

void CDoDParticle::SetGlobalWind( float *vecWind )
{
	m_vGlobalWind = Vector( vecWind[0], vecWind[1], vecWind[2] );
}

void CDoDParticle::AddGlobalWind( void )
{
	m_vVelocity.x += m_vGlobalWind.x;
	m_vVelocity.y += m_vGlobalWind.y;
}

void CreateExplosionSmoke( Vector origin, Vector vVelocity, bool bInsideSmoke, bool bSpawnInside, bool bBlowable )
{
	const char *sprname;
	model_s *pSprite;
	float scale;
	CDoDParticle *pSmoke;
	vec3_t vColor;
	float flRandomColorR;
	float flRandomColorG;
	float flRandomColorB;

	if( !g_pParticleMan )
		return;

	sprname = "sprites/gas_puff_01.spr";
	pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( sprname ) );

	if( !pSprite )
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", sprname );
		return;
	}

	scale = gEngfuncs.pfnRandomFloat( 175.0f, 225.0f );

	pSmoke = CDoDParticle::Create( origin, Vector( 0, 0, 1 ), pSprite, scale, 190.0f, "dod_particle", false );

	if( !pSmoke )
		return;

	pSmoke->m_iPFlags |= PFLAG_DOD_EXPLOSION_PULSE | PFLAG_DOD_SMOKE_CORE;
	pSmoke->SetCollisionFlags( TRI_COLLIDEWORLD );
	pSmoke->m_flGravity = 0.0f;
	pSmoke->m_iRendermode = kRenderTransAlpha;
	pSmoke->m_vRingOrigin = origin;
	pSmoke->m_vVelocity = vVelocity;
	pSmoke->m_flSize = 0.0f;
	pSmoke->m_iFrame = gEngfuncs.pfnRandomLong( 0, 1 );
	pSmoke->m_flScaleSpeed = 0.1f;

	if( bInsideSmoke )
	{
		pSmoke->m_vAVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( -0.8f, 0.8f ) );
		pSmoke->m_flDampingTime = 0.0f;
		pSmoke->m_bInsideSmoke = true;
		pSmoke->m_bSpawnInside = bSpawnInside;
		pSmoke->m_flScaleSpeed = 0.05f;
		pSmoke->m_flFadeSpeed = 0.4f;
		pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 6.0f, 9.0f );
	}
	else
	{
		pSmoke->m_vAVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) );
		pSmoke->m_flDampingTime = gEngfuncs.GetClientTime() + 0.001f;
		pSmoke->m_bInsideSmoke = false;
		pSmoke->m_bSpawnInside = bSpawnInside;
		pSmoke->m_bDampMod = false;
		pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 25.0f;
	}

	pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 1.0f, 1.5f );

	flRandomColorR = gEngfuncs.pfnRandomFloat( 89.0f, 104.0f );
	flRandomColorG = gEngfuncs.pfnRandomFloat( 85.0f, 100.0f );
	flRandomColorB = gEngfuncs.pfnRandomFloat( 80.0f, 95.0f );

	vColor = Vector( flRandomColorR, flRandomColorG, flRandomColorB );

	pSmoke->SetLightFlag( LIGHT_NONE );
	pSmoke->SetCullFlag( CULL_PVS | CULL_FRUSTUM_PLANE );
	pSmoke->m_vColor = vColor;

	pSmoke->AddGlobalWind();
	pSmoke->m_bAffectedByForce = !bBlowable;
}

// the smoke of an explosion that is closed in a room: a ring of puffs around the point
void CreateExplosionSmokeInside( Vector origin )
{
	vec3_t vOrigin;
	vec3_t vAngles;
	vec3_t vforward, vright, vup;
	int i;

	vOrigin = origin;

	vAngles = Vector( 0, 0, 0 );
	AngleVectors( vAngles, vforward, vright, vup );
	VectorNormalize( vforward );

	CreateExplosionSmoke( vOrigin + vforward * gEngfuncs.pfnRandomFloat( 90.0f, 110.0f ) + vup * 45.0f, Vector( 0, 0, 0 ), true, false, true );

	vAngles = Vector( 0, 0, 0 );
	AngleVectors( vAngles, vforward, vright, vup );
	VectorNormalize( vforward );

	CreateExplosionSmoke( vOrigin + vforward * 90.0f + vup * 60.0f, Vector( 0, 0, 0 ), true, false, true );

	vAngles = Vector( 0, 45, 0 );
	AngleVectors( vAngles, vforward, vright, vup );
	VectorNormalize( vforward );

	CreateExplosionSmoke( vOrigin + vforward * 45.0f + vup * 55.0f, Vector( 0, 0, 0 ), true, false, true );

	vAngles = Vector( 0, 270, 0 );
	AngleVectors( vAngles, vforward, vright, vup );
	VectorNormalize( vforward );

	CreateExplosionSmoke( vOrigin + vforward * 45.0f + vup * 55.0f, Vector( 0, 0, 0 ), true, false, true );

	CreateExplosionSmoke( vOrigin, Vector( 0, 0, 0 ), true, false, true );
}

void TriangleWallPuff::Think( float flTime )
{
	if( m_iFrame <= 7 )
		CBaseParticle::Animate( flTime );

	CBaseParticle::Fade( flTime );

	if( m_flBrightness < 1.0f )
		m_flBrightness = 1.0f;

	CBaseParticle::Expand( flTime );
	CBaseParticle::Spin( flTime );
	CBaseParticle::CheckCollision( flTime );
	CBaseParticle::CalculateVelocity( flTime );

	if( m_flActivateTime != 0.0f && flTime >= m_flActivateTime )
	{
		m_flActivateTime = 0.0f;
		m_flGravity = -0.02f;
		m_vAVelocity = Vector( 0.0f, 0.0f, 0.4f );
		m_vVelocity = m_vVelocity * 0.1f;
	}
}

// the puff of the debris of a wall that is hit by an explosion
void CreateDebrisWallPuff( Vector origin, Vector vVelocity, Vector vColor, int iPuff )
{
	model_s *pSprite;
	float scale;
	float brightness;
	float flDist;
	TriangleWallPuff *pSmoke;

	if( iPuff == 0 )
		pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/large_smoke_01_ind.spr" ) );
	else
		pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/puff.spr" ) );

	if( !pSprite )
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: sprites/large_smoke_01_ind.spr\n" );
		return;
	}

	if( !IEngineStudio.IsHardware() )
		return;

	scale = gEngfuncs.pfnRandomFloat( 25.0f, 36.0f );

	if( iPuff > 0 )
	{
		scale = gEngfuncs.pfnRandomFloat( 5.0f * iPuff, 10.0f * iPuff );
		brightness = 200.0f;
	}
	else if( iPuff == 0 )
	{
		brightness = 256.0f;
	}
	else
	{
		brightness = 200.0f;
	}

	// the puffs that are far are bigger and brighter to be seen
	flDist = ( v_origin - origin ).Length();

	if( flDist > 1000.0f )
	{
		scale *= 2.5f;
		brightness = 256.0f;
	}
	else if( flDist > 500.0f )
	{
		float s = ( flDist - 500.0f ) / 500.0f + 1.0f;

		scale *= s;
		brightness = 225.0f;
	}

	pSmoke = new TriangleWallPuff;

	pSmoke->InitializeSprite( origin, Vector( 0, 0, 1 ), pSprite, scale, brightness );

	pSmoke->SetCollisionFlags( TRI_COLLIDEWORLD );
	pSmoke->SetRenderFlag( RENDER_FACEPLAYER_ROTATEZ );
	pSmoke->SetLightFlag( LIGHT_INTENSITY );

	pSmoke->m_flGravity = -0.1f;
	pSmoke->m_iRendermode = kRenderTransAlpha;
	pSmoke->m_flSize = scale;
	pSmoke->m_vAVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( -3.0f, 1.5f ) );
	pSmoke->m_flActivateTime = gEngfuncs.GetClientTime() + 0.1f;
	pSmoke->m_flFadeSpeed = 9.5f;
	pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );

	if( iPuff <= 0 )
	{
		pSmoke->m_flScaleSpeed = 0.9f;
		pSmoke->m_vVelocity = vVelocity * 95.0f;
		pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 3.0f;
	}
	else
	{
		pSmoke->m_iNumFrames = 9;
		pSmoke->m_iFramerate = 15;
		pSmoke->m_flScaleSpeed = 0.5f;
		pSmoke->m_vVelocity = vVelocity * (float)( 30 * iPuff );
		pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 0.7f;
	}

	pSmoke->m_vColor = vColor;
}

void CDoDRocketTrail::Think( float flTime )
{
	vec3_t vTemp;

	if( m_flBrightness > 0.0f )
	{
		if( m_bRocketTrail )
		{
			m_flBrightness = m_flBrightness - gEngfuncs.pfnRandomFloat( 0.2f, 0.4f ) * ( flTime - m_flTimeCreated );

			m_vColor.x += 0.55f;
			m_vColor.y += 0.55f;
			m_vColor.z += 0.55f;

			if( m_vColor.x < 0.0f )
				m_vColor.x = 0.0f;

			if( m_vColor.y < 0.0f )
				m_vColor.y = 0.0f;

			if( m_vColor.z < 0.0f )
				m_vColor.z = 0.0f;
		}
		else
		{
			m_flBrightness = m_flBrightness - gEngfuncs.pfnRandomFloat( 1.5f, 2.0f ) * ( flTime - m_flTimeCreated );
		}
	}

	if( m_flBrightness < 0.0f )
	{
		m_flBrightness = 0.0f;
		m_flDieTime = gEngfuncs.GetClientTime();
	}

	CBaseDoDParticle::Think( flTime );

	vTemp = GetViewAngles();

	m_vAngles.x = vTemp.x;
	m_vAngles.y = vTemp.y;
}

CDoDRocketTrail *CDoDRocketTrail::Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname )
{
	CDoDRocketTrail *pEffect;

	if( !sprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDRocketTrail::Create called with a null sprite\n" );
		return NULL;
	}

	if( !IEngineStudio.IsHardware() )
		return NULL;

	pEffect = new CDoDRocketTrail;

	pEffect->InitializeSprite( org, normal, sprite, size, brightness );

	pEffect->m_iNumFrames = sprite->numframes;

	pEffect->m_vVelocity.x += CDoDParticle::m_vGlobalWind.x;
	pEffect->m_vVelocity.y += CDoDParticle::m_vGlobalWind.y;

	return pEffect;
}

void CDoDDirtExploDust::Think( float flTime )
{
	vec3_t vTemp;

	if( m_flBrightness > 0.0f )
	{
		if( m_bFire )
			m_flBrightness = m_flBrightness - gEngfuncs.pfnRandomFloat( 1.0f, 2.5f ) * ( flTime - m_flTimeCreated );
		else
			m_flBrightness = m_flBrightness - gEngfuncs.pfnRandomFloat( 0.8f, 0.9f ) * ( flTime - m_flTimeCreated );
	}

	if( m_flBrightness < 0.0f )
	{
		m_flBrightness = 0.0f;
		m_flDieTime = gEngfuncs.GetClientTime();
	}

	// m_flActivateTime is the factor that slows down the dust, every frame
	m_vVelocity = m_vVelocity * m_flActivateTime;

	CBaseDoDParticle::Think( flTime );

	vTemp = GetViewAngles();

	m_vAngles.x = vTemp.x;
	m_vAngles.y = vTemp.y;
}

CDoDDirtExploDust *CDoDDirtExploDust::Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname )
{
	CDoDDirtExploDust *pEffect;

	if( !sprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDDirtExploDust::Create called with a null sprite\n" );
		return NULL;
	}

	if( !IEngineStudio.IsHardware() )
		return NULL;

	pEffect = new CDoDDirtExploDust;

	pEffect->InitializeSprite( org, normal, sprite, size, brightness );

	pEffect->m_iNumFrames = sprite->numframes;

	pEffect->m_vVelocity.x += CDoDParticle::m_vGlobalWind.x;
	pEffect->m_vVelocity.y += CDoDParticle::m_vGlobalWind.y;

	return pEffect;
}

// The texture above is looked for along the whole line of the trace, not from its end point: the end point of the trace of the player
// is DIST_EPSILON (1/32) before the surface that it hit, and the search of Xash3D (PM_RecursiveSurfCheck) compares the distance to
// the plane with the same 1/32, so whether it finds the surface depends on how the floats were rounded. On dod_kalt it found the sky
// never, on dod_zalec for one trace of four.
// the snowflakes are created over the player, where the sky is above
void UpdateSnow( void )
{
	pmtrace_t pmtrace;
	vec3_t origin;
	model_t *pSnowSprite;
	const float dist = 400.0f;
	static float negdist = -dist;
	int j;

	pSnowSprite = gHUD.m_Weather.GetSnowSprite();

	if( pSnowSprite )
	{
		for( j = 0; j < 150; j++ )
		{
			vec3_t vEndPos;
			const char *pTextureName;

			origin = gHUD.m_vecOrigin;

			origin.x += gEngfuncs.pfnRandomFloat( negdist, dist );
			origin.y += gEngfuncs.pfnRandomFloat( negdist, dist );
			origin.z += gEngfuncs.pfnRandomFloat( 100.0f, 300.0f );

			vEndPos = origin;
			vEndPos.z = 8000.0f;

			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( origin, vEndPos, PM_WORLD_ONLY, -1, &pmtrace );

			pTextureName = gEngfuncs.pEventAPI->EV_TraceTexture( pmtrace.ent, origin, vEndPos );

			// only where the roof is the sky
			if( pTextureName && !strncmp( pTextureName, "sky", 3 ) )
				gHUD.m_Weather.CreateSnowParticle( origin );
		}
	}
}

void UpdateRain( void )
{
	model_t *pRainSprite;
	pmtrace_t pmtrace;
	vec3_t origin;
	const float dist = 650.0f;
	static float negdist = -dist;
	vec3_t vecWeatherOrigin;
	int j;

	pRainSprite = gHUD.m_Weather.GetRainSprite();

	if( pRainSprite )
	{
		vecWeatherOrigin = gHUD.m_vecOrigin + Vector( 0.0f, 0.0f, 36.0f );

		for( j = 0; j < 350; j++ )
		{
			vec3_t vEndPos;
			const char *pTextureName;

			origin = vecWeatherOrigin;

			origin.x += gEngfuncs.pfnRandomFloat( negdist, dist );
			origin.y += gEngfuncs.pfnRandomFloat( negdist, dist );
			origin.z += gEngfuncs.pfnRandomFloat( 100.0f, 300.0f );

			vEndPos = origin;
			vEndPos.z = 8000.0f;

			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( origin, vEndPos, PM_WORLD_ONLY, -1, &pmtrace );

			pTextureName = gEngfuncs.pEventAPI->EV_TraceTexture( pmtrace.ent, origin, vEndPos );

			if( pTextureName && !strncmp( pTextureName, "sky", 3 ) )
				gHUD.m_Weather.CreateRainParticle( origin );
		}
	}
}
