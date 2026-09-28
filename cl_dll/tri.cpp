//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "r_studioint.h"
#include "triangleapi.h"
#include "particleman.h"
#include "tri.h"
#include "fx_flags.h"
#include "pmtrace.h"
#include "event_api.h"

CBaseParticle *g_pBaseParticle = NULL;
extern IParticleMan *g_pParticleMan;
extern engine_studio_api_t IEngineStudio;

float g_flWeatherTime;
int g_iWeatherType;

cvar_t *cl_weatherdis;

extern cvar_t *cl_particlefx;
extern cvar_t *cl_fog;
extern float g_flGravity;
extern vec3_t v_origin, v_angles, v_lastAngles;
extern int g_iOnlyClientDraw;

extern void RenderDoDFog( void );

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
}

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{
	// Nothing.
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
		if( g_iOnlyClientDraw == 1 && gHUD.GetMinimapState() > 0 )
		{
			gHUD.m_ObjectiveIcons.CalcIconLocations();
			gHUD.m_DoDMap.DrawOverview();
		}
	}
	else
	{
		if( cl_fog->value != 0.0f )
		{
			RenderDoDFog();
		}

		if( cl_particlefx->value >= 1.0f && IEngineStudio.IsHardware() )
		{
			if( g_iUser1 != 3 && g_iUser1 != 0 )
			{
				vec3_t vecAngles = v_angles;
				g_pParticleMan->SetVariables( g_flGravity, vecAngles );
			}
			else
			{
				g_pParticleMan->SetVariables( g_flGravity, gHUD.m_vecAngles );
			}

			g_pParticleMan->Update();

			float time = gEngfuncs.GetClientTime();

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

CDoDParticle *CDoDParticle::Create( vec3_t org, vec3_t normal, model_s *sprite, float size, float brightness, 
									const char *classname, bool bDistCull )
{
	if( !sprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDParticle::Create called with a null sprite\n" );
		return NULL;
	}

	CDoDParticle *pEffect = NULL;


	if( cl_particlefx->value > 0.0f && IEngineStudio.IsHardware() )
	{
		if( cl_weatherdis->value < 100.0f )
			cl_weatherdis->value = 100.0f;

		if( bDistCull )
		{
			cl_entity_t *player = gEngfuncs.GetLocalPlayer();

			if( player )
			{
				vec3_t vOurOrigin = player->curstate.origin;
				vec3_t vecDelta = org - vOurOrigin;
				float flDistance = vecDelta.Length();

				if( flDistance > cl_weatherdis->value )
					return NULL;
			}
		}

		pEffect = new CDoDParticle();

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

void CDoDParticle::Touch( vec3_t pos, vec3_t normal, int index )
{
	if( m_iPFlags & TRI_COLLIDEDAMP )
	{
		model_t *pSplash = gHUD.m_Weather.GetSplashSprite();

		if( pSplash )
		{
			float fTime = gEngfuncs.GetClientTime();

			CBaseParticle *pParticleSplash = new CBaseParticle();
			if( !pParticleSplash )
				return;

			float size = ( float ) gEngfuncs.pfnRandomLong( 25, 30 );

			vec3_t vecNormal = Vector( 90.0f, 0.0f, 0.0f );
			vec3_t p_org = m_vOrigin + normal;

			pParticleSplash->InitializeSprite( p_org, vecNormal, pSplash, size, 150.0f );

			pParticleSplash->m_iRendermode = kRenderTransAdd;
			pParticleSplash->m_iNumFrames = pSplash->numframes;
			pParticleSplash->m_iFramerate = gEngfuncs.pfnRandomLong( 15, 60 );
			pParticleSplash->m_flDieTime = fTime + 0.3f;
			pParticleSplash->m_flScaleSpeed = 0.1f;
			pParticleSplash->m_vColor = Vector( 255.0f, 255.0f, 255.0f );

			pParticleSplash->SetLightFlag( LIGHT_COLOR );
			pParticleSplash->SetCullFlag( CULL_FRUSTUM_POINT | CULL_FRUSTUM_PLANE );
			pParticleSplash->SetCollisionFlags( TRI_COLLIDEKILL_ANIM | TRI_COLLIDEDAMP | TRI_COLLIDESLIDE );
			pParticleSplash->SetRenderFlag( RENDER_FACEPLAYER );
		}
	}
}

extern int EV_HLDM_WaterEntryPoint( pmtrace_t *pTrace, float *vecSrc, float *vecResult );

void CDoDParticle::Die( void )
{
	if( m_iPFlags & PFLAG_DOD_WATER_RIPPLE )
	{
		model_t *pRipple = gHUD.m_Weather.GetRippleSprite();

		if( pRipple )
		{
			vec3_t start = m_vOrigin;
			vec3_t end = m_vOrigin;
			pmtrace_t tr;
			vec3_t pt;

			start.z += 32.0f;
			end.z -= 16.0f;

			gEngfuncs.pEventAPI->EV_SetTraceHull( 0 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( start, end, 8, -1, &tr );

			if( EV_HLDM_WaterEntryPoint( &tr, start, pt ) )
			{
				vec3_t vecNormal = Vector( 0, 0, 15 );
				float scale = 15.0f;
				float brightness = 254.0f;

				CDoDParticle *pParticle = CDoDParticle::Create( pt, vecNormal, pRipple, scale, brightness, "water_ripple", false );

				if( pParticle )
				{
					pParticle->m_iRendermode = kRenderTransAdd;
					pParticle->m_flFadeSpeed = 4.0f;
					pParticle->m_flScaleSpeed = 2.0f;

					pParticle->SetLightFlag( LIGHT_COLOR );
					pParticle->SetCullFlag( CULL_FRUSTUM_SPHERE );

					pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );
					pParticle->m_iPFlags = PFLAG_DOD_SPARK_INSTANT;
					pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
				}
			}
		}
	}
	CBaseDoDParticle::Die();
}

extern void V_GetDeathCam( cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin );

vec3_t GetViewAngles( void )
{
	vec3_t angles;

	if( g_iUser1 )
	{
		if( g_iUser3 > 0 && g_iUser2 > 0 )
		{
			cl_entity_t *pTarget1 = gEngfuncs.GetEntityByIndex( g_iUser2 );
			cl_entity_t *pTarget2 = gEngfuncs.GetEntityByIndex( g_iUser3 );

			vec3_t dummyOrigin;

			V_GetDeathCam( pTarget1, pTarget2, angles, dummyOrigin );
			return angles;
		}
		else
		{
			if( g_iUser1 != 1 && g_iUser1 != 4 )
			{
				if( g_iUser1 == 2 || g_iUser1 == 3 )
					gEngfuncs.GetViewAngles( angles );
				else
					angles = v_lastAngles;
			}
			else
			{
				angles = v_angles;
			}

			return angles;
		}
	}

	gEngfuncs.GetViewAngles( angles );

	if( gHUD.IsInMortarDeploy() )
		angles.x = 0.0f;

	return angles;
}

void CDoDParticle::Think( float flTime )
{
	float timeleft = m_flDieTime - flTime;

	if( m_flBrightness >= 255.0f && timeleft < 3.0f )
	{
		m_flScaleSpeed = 0.0f;
		m_vAVelocity = Vector( 0.0f, 0.0f, 0.0f );
		m_flTimeCreated = flTime;
	}

	if( timeleft <= 5.0f )
	{
		if( m_flBrightness > 0.0f )
			m_flBrightness -= ( flTime - m_flTimeCreated ) * 0.1f;

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
				m_flBrightness += 5.0f * ( flTime - m_flTimeCreated ) + 8.0f;
		}
		else if( m_flBrightness < 255.0f && m_flFadeSpeed < -0.5f )
			m_flBrightness += 5.0f * ( flTime - m_flTimeCreated ) + 10.0f;

		if( m_flBrightness > 255.0f )
			m_flBrightness = 255.0f;
	}

	if( m_iPFlags & PFLAG_DOD_SHRINK_DIE  && timeleft <= 3.0f )
	{
		m_flSize -= 0.02f;

		if( m_flSize < 0.0f )
		{
			m_flSize = 0.0f;
			m_flDieTime = gEngfuncs.GetClientTime();
		}
	}

	g_pBaseParticle->Think( flTime );

	vec3_t vViewAngles = GetViewAngles();
	vec3_t forward, right, up;
	gEngfuncs.pfnAngleVectors( vViewAngles, forward, right, up );

	if( m_iPFlags & PFLAG_DOD_VELOCITY_ROTATE && m_vVelocity.z != 0.0f )
		float dotLength = DotProduct( m_vVelocity, right );

	if( !( m_iPFlags & PFLAG_DOD_SPARK_INSTANT ) )
	{
		if( !( m_iPFlags & PFLAG_DOD_DIRT_DEBRIS ) )
			m_vAngles.x = vViewAngles.x;

		m_vAngles.y = vViewAngles.y;
	}

	if( m_flDampingTime != 0.0f && flTime >= m_flDampingTime )
	{
		if( !m_bDampMod )
		{
			m_bDampMod = true;
			m_flDampingTime = gEngfuncs.GetClientTime() + 0.2f;

			if( m_bSpawnInside )
				CreateExplosionSmokeInside( m_vRingOrigin );
		}
		else
		{
			m_flDampingTime = 0.0f;
			m_vVelocity.x *= 0.035f;
			m_vVelocity.y *= 0.035f;
			m_vVelocity.z *= 0.035f;
		}
	}

	if( m_iPFlags & PFLAG_DOD_EXPLOSION_PULSE )
	{
		float ssize_dem = ( 25.0f - timeleft ) / 0.2f;
		float ssize = ( ssize_dem * 300.0f ) / 100.0f * 100.0f;

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

void CDoDParticle::SetGlobalWind( float *vecWind )
{
	m_vGlobalWind.x = vecWind[0];
	m_vGlobalWind.y = vecWind[1];
	m_vGlobalWind.z = vecWind[2];
}

void CDoDParticle::AddGlobalWind( void )
{
	m_vVelocity.x = m_vVelocity.x + m_vGlobalWind.x;
	m_vVelocity.y = m_vVelocity.y + m_vGlobalWind.y;
}

void CreateExplosionSmoke( vec3_t origin, vec3_t vVelocity, bool bInsideSmoke, bool bSpawnInside, bool bBlowable )
{
	if( !g_pParticleMan )
		return;

	CDoDParticle *pParticle;

	HSPRITE hSpriteHandle = gEngfuncs.pfnSPR_Load( "sprites/gas_puff_01.spr" );
	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( hSpriteHandle );

	if( !pSprite )
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", "sprites/gas_puff_01.spr" );
		return;
	}

	float scale = gEngfuncs.pfnRandomFloat( 175.0f, 225.0f );
	float brightness = 90.0f;

	vec3_t p_normal = Vector( 0.0f, 0.0f, 1.0f );

	pParticle = pParticle->Create( origin, p_normal, pSprite, scale, brightness, "explosion_smoke", false );

	if( !pParticle )
		return;

	pParticle->m_iPFlags |= ( PFLAG_DOD_SMOKE_CORE | PFLAG_DOD_EXPLOSION_PULSE );
	pParticle->SetCollisionFlags( TRI_WATERTRACE );
	pParticle->m_flGravity = 0.0f;
	pParticle->m_iRendermode = kRenderTransAlpha;
	pParticle->m_vRingOrigin = origin;
	pParticle->m_vVelocity = vVelocity;
	pParticle->m_flSize = 0.0f;
	pParticle->m_flScaleSpeed = 0.1f;
	pParticle->m_iFrame = gEngfuncs.pfnRandomLong( 0, 1 );

	float flClientTime = gEngfuncs.GetClientTime();

	if( bInsideSmoke )
	{
		pParticle->m_vAVelocity.x = 0.0f;
		pParticle->m_vAVelocity.y = 0.0f;
		pParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -0.8f, 0.8f );
		pParticle->m_flDampingTime = 0.0f;

		pParticle->m_bInsideSmoke = true;
		pParticle->m_bSpawnInside = bSpawnInside;
		pParticle->m_flScaleSpeed = 0.05f;
		pParticle->m_flFadeSpeed = 0.4f;

		float flLifeTime = gEngfuncs.pfnRandomFloat( 6.0f, 9.0f );
		pParticle->m_flDieTime = flClientTime + flLifeTime;
	}
	else
	{
		pParticle->m_vAVelocity.x = 0.0f;
		pParticle->m_vAVelocity.y = 0.0f;
		pParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );

		pParticle->m_flDampingTime = flClientTime + 0.001f;
		pParticle->m_bInsideSmoke = false;
		pParticle->m_bSpawnInside = bSpawnInside;
		pParticle->m_flDieTime = flClientTime + 25.0f;
	}

	pParticle->m_flMass = gEngfuncs.pfnRandomFloat( 1.0f, 1.5f );

	pParticle->m_vColor.x = gEngfuncs.pfnRandomFloat( 87.0f, 102.0f );
	pParticle->m_vColor.y = gEngfuncs.pfnRandomFloat( 83.0f, 98.0f );
	pParticle->m_vColor.z = gEngfuncs.pfnRandomFloat( 78.0f, 93.0f );

	pParticle->SetLightFlag( LIGHT_NONE );

	pParticle->m_bAffectedByForce = !bBlowable;
	pParticle->m_vVelocity.x += pParticle->m_vGlobalWind.x;
	pParticle->m_vVelocity.y += pParticle->m_vGlobalWind.y;
}

void CreateExplosionSmokeInside( vec3_t origin )
{
	if( !IEngineStudio.IsHardware() )
		return;

	vec3_t vOrigin = origin;
	vec3_t vAngles;
	vec3_t vforward, vright, vup;

	static const float arrYaw[5] =		 { 0.0f,   0.0f,   45.0f,  270.0f,  0.0f };
	static const float arrUpOffset[5] =  { 45.0f,  60.0f,  55.0f,  45.0f,   0.0f };
	static const float arrSizeParam[5] = { 90.0f,  90.0f,  55.0f,  45.0f,  45.0f };

	for( int i = 0; i < 5; i++ )
	{
		vec3_t smokeOrigin;
		vec3_t smokeVelocity = Vector( 0, 0, 0 );

		if( i < 4 )
		{
			vAngles.x = 0.0f;
			vAngles.y = arrYaw[i];
			vAngles.z = 0.0f;

			gEngfuncs.pfnAngleVectors( vAngles, vforward, vright, vup );
			vforward.Normalize();

			float flDist = ( i == 0 ) ? gEngfuncs.pfnRandomFloat( 0.0f, 90.0f ) : 45.0f;
			smokeOrigin = ( vforward * flDist ) + vOrigin + ( vup * arrUpOffset[i] );
		}
		else
		{
			smokeOrigin = vOrigin;
		}

		CreateExplosionSmoke( smokeOrigin, smokeVelocity, 1, 110.0f, arrSizeParam[i] );
	}
}

void TriangleWallPuff::Think( float flTime )
{
	if( m_iFrame <= 7 )
		Animate( flTime );

	Fade( flTime );

	if( m_flBrightness < 1.0f )
		m_flBrightness = 1.0f;

	Expand( flTime );
	Spin( flTime );
	CheckCollision( flTime );
	CalculateVelocity( flTime );

	if( m_flActivateTime != 0.0f && flTime >= m_flActivateTime )
	{
		m_flActivateTime = 0.0f;
		m_flGravity = -0.02f;

		m_vAVelocity.x = 0.0f;
		m_vAVelocity.y = 0.0f;
		m_vAVelocity.z = 0.4f;

		m_vVelocity.x *= 0.1f;
		m_vVelocity.y *= 0.1f;
		m_vVelocity.z *= 0.1f;
	}
}

void CreateDebrisWallPuff( vec3_t origin, vec3_t vVelocity, vec3_t vColor, int iPuff )
{
	HSPRITE hSpriteHandle;
	model_s *pSprite = NULL;

	if( !iPuff )
	{
		hSpriteHandle = gEngfuncs.pfnSPR_Load( "sprites/large_smoke_01_ind.spr" );
		pSprite = (model_s *)gEngfuncs.GetSpritePointer( hSpriteHandle );
		if( !pSprite )
		{
			gEngfuncs.Con_DPrintf( "Couldn't load Sprite: sprites/large_smoke_01_ind.spr\n" );
			return;
		}
	}
	else
	{
		hSpriteHandle = gEngfuncs.pfnSPR_Load( "sprites/puff.spr" );
		pSprite = (model_s *)gEngfuncs.GetSpritePointer( hSpriteHandle );
		if( !pSprite )
		{
			gEngfuncs.Con_DPrintf( "Couldn't load Sprite: sprites/puff.spr\n" );
			return;
		}
	}

	if( !IEngineStudio.IsHardware() )
		return;

	float scale = 0.0f;
	float brightness = 100.0f;

	float flPhysicalRandom = gEngfuncs.pfnRandomFloat( 25.0f, 35.0f );

	if( iPuff > 0 )
		scale = gEngfuncs.pfnRandomFloat( ( float ) ( 5 * iPuff ), ( float ) ( 10 * iPuff ) );

	vec3_t vecToPlayer = v_origin - origin;
	float flDist = vecToPlayer.Length();

	if( flDist <= 1000.0f )
	{
		if( flDist > 500.0f )
		{
			brightness = 140.0f;
			scale = scale * ( ( flDist - 500.0f ) / 500.0f + 1.0f );
		}
	}
	else
	{
		scale = scale * 2.5f;
		brightness = 200.0f;
	}

	TriangleWallPuff *pSmoke = new TriangleWallPuff();

	if( !pSmoke )
		return;

	vec3_t p_normal = Vector( 0, 0, 1 );

	pSmoke->InitializeSprite( origin, p_normal, pSprite, scale, brightness );

	pSmoke->SetCollisionFlags( TRI_WATERTRACE );
	pSmoke->SetRenderFlag( RENDER_FACEPLAYER_ROTATEZ | LIGHT_INTENSITY );
	pSmoke->SetLightFlag( PARTICLE_LIGHT_DEFAULT );

	pSmoke->m_flGravity = -0.1f;
	pSmoke->m_iRendermode = kRenderTransAlpha;
	pSmoke->m_flSize = scale;

	pSmoke->m_vAVelocity.x = 0.0f;
	pSmoke->m_vAVelocity.y = 0.0f;
	pSmoke->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -4.0f, 2.0f );

	pSmoke->m_flActivateTime = gEngfuncs.GetClientTime() + 0.1f;
	pSmoke->m_flFadeSpeed = ( flPhysicalRandom / 3.0f ) + 0.5f;
	pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 4.0f ) * ( flPhysicalRandom / 30.0f );
	pSmoke->m_vColor = vColor;

	if( iPuff <= 0 )
	{
		pSmoke->m_flScaleSpeed = 0.9f;
		pSmoke->m_vVelocity = vVelocity * 95.0f;
		pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 3.0f;
	}
	else
	{
		float flPuffScale = ( float ) ( 30 * iPuff );
		pSmoke->m_iNumFrames = 9;
		pSmoke->m_iFramerate = 15;
		pSmoke->m_flScaleSpeed = 0.5f;
		pSmoke->m_vVelocity = vVelocity * flPuffScale;
		pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 0.7f;
	}
}

void CDoDRocketTrail::Think( float flTime )
{
	float flCurrentBrightness = m_flBrightness;

	if( m_flBrightness > 0.0f )
	{
		float flTimeDelta = flTime - m_flTimeCreated;

		if( m_bRocketTrail )
		{
			m_flBrightness -= gEngfuncs.pfnRandomFloat( 0.2f, 0.4f ) * flTimeDelta;
			flCurrentBrightness = m_flBrightness;

			m_vColor.x += 0.55f;
			m_vColor.y += 0.55f;
			m_vColor.z += 0.55f;

			if( m_vColor.x < 0.0f ) m_vColor.x = 0.0f;
			if( m_vColor.y < 0.0f ) m_vColor.y = 0.0f;
			if( m_vColor.z < 0.0f ) m_vColor.z = 0.0f;
		}
		else
		{
			m_flBrightness -= gEngfuncs.pfnRandomFloat( 1.5f, 2.0f ) * flTimeDelta;
			flCurrentBrightness = m_flBrightness;
		}
	}

	if( flCurrentBrightness < 0.0f )
	{
		m_flBrightness = 0.0f;
		m_flDieTime = gEngfuncs.GetClientTime();
	}

	CBaseDoDParticle::Think( flTime );

	vec3_t vTemp = GetViewAngles();
	m_vAngles.x = vTemp.x;
	m_vAngles.y = vTemp.y;
}

CDoDRocketTrail *CDoDRocketTrail::Create( vec3_t org, vec3_t normal, model_s *sprite, float size, float brightness, 
										const char *classname )
{
	if( !sprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDRocketTrail::Create called with a null sprite\n" );
		return NULL;
	}

	CDoDRocketTrail *pEffect = NULL;
	CDoDParticle *pParticle;

	if( IEngineStudio.IsHardware() )
	{
		pEffect = new CDoDRocketTrail();

		if( pEffect )
		{
			vec3_t vecOrigin = org;
			vec3_t vecNormal = normal;

			pEffect->InitializeSprite( &vecOrigin, &vecNormal, sprite, size, brightness );
			pEffect->m_iNumFrames = sprite->numframes;
			pEffect->m_vVelocity.x += pParticle->m_vGlobalWind.x;
			pEffect->m_vVelocity.y += pParticle->m_vGlobalWind.y;
		}
	}

	return pEffect;
}

void CDoDDirtExploDust::Think( float flTime )
{
	float flCurrentBrightness = m_flBrightness;

	if( m_flBrightness > 0.0f )
	{
		float flFadeMin = 0.7f;
		float flFadeMax = 0.8f;

		if( m_bFire )
		{
			flFadeMin = 1.0f;
			flFadeMax = 3.0f;
		}

		m_flBrightness -= gEngfuncs.pfnRandomFloat( flFadeMin, flFadeMax ) * ( flTime - m_flTimeCreated );
		flCurrentBrightness = m_flBrightness;
	}

	if( flCurrentBrightness < 0.0f )
	{
		m_flBrightness = 0.0f;
		m_flDieTime = gEngfuncs.GetClientTime();
	}

	m_vVelocity = m_vVelocity * m_flActivateTime;

	CBaseDoDParticle::Think( flTime );

	vec3_t vTemp = GetViewAngles();
	m_vAngles.x = vTemp.x;
	m_vAngles.y = vTemp.y;
}

CDoDDirtExploDust *CDoDDirtExploDust::Create( vec3_t org, vec3_t normal, model_s *sprite, float size, float brightness, const char *classname )
{
	if( !sprite )
	{
		gEngfuncs.Con_DPrintf( "CDoDDirtExploDust::Create called with a null sprite\n" );
		return NULL;
	}

	CDoDDirtExploDust *pEffect = NULL;
	CDoDParticle *pParticle;

	if( IEngineStudio.IsHardware() )
	{
		pEffect = new CDoDDirtExploDust();

		if( pEffect )
		{
			vec3_t vecOrigin = org;
			vec3_t vecNormal = normal;

			pEffect->InitializeSprite( &vecOrigin, &vecNormal, sprite, size, brightness );
			pEffect->m_iNumFrames = sprite->numframes;
			pEffect->m_vVelocity.x += pParticle->m_vGlobalWind.x;
			pEffect->m_vVelocity.y += pParticle->m_vGlobalWind.y;
		}
	}

	return pEffect;
}

void UpdateSnow( void )
{
	if( !gHUD.m_Weather.GetSnowSprite() )
		return;

	static float negdist = -400.0f;
	const float dist = 400.0f;

	for( int j = 150; j > 0; j-- )
	{
		vec3_t origin, vEndPos;
		pmtrace_t pmtrace;

		while( true )
		{
			origin = gHUD.m_vecOrigin;

			origin.x = gEngfuncs.pfnRandomFloat( negdist, dist ) + origin.x;
			origin.y = gEngfuncs.pfnRandomFloat( negdist, dist ) + origin.y;

			float flRandomHeight = gEngfuncs.pfnRandomFloat( 100.0f, 300.0f );
			origin.z = flRandomHeight + origin.z;

			vEndPos = origin;
			vEndPos.z = 8000.0f;

			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( origin, vEndPos, 8, -1, &pmtrace );

			char pTextureName = (char)gEngfuncs.pEventAPI->EV_TraceTexture( pmtrace.ent, pmtrace.endpos, vEndPos );

			if( pTextureName )
			{
				if( pTextureName == 's' && pTextureName == 'k' && pTextureName == 'y' )
					break;
			}

			if( --j <= 0 )
				return;
		}

		gHUD.m_Weather.CreateSnowParticle( &origin.x );
	}
}

void UpdateRain( void )
{
	if( !gHUD.m_Weather.GetRainSprite() )
		return;

	static float negdist = -650.0f;
	const float dist = 650.0f;

	vec3_t vecWeatherOrigin = gHUD.m_vecOrigin + Vector( 0.0f, 0.0f, 36.0f );

	for( int j = 350; j > 0; j-- )
	{
		vec3_t origin, vEndPos;
		pmtrace_t pmtrace;

		while( true )
		{
			origin.x = gEngfuncs.pfnRandomFloat( negdist, dist ) + vecWeatherOrigin.x;
			origin.y = gEngfuncs.pfnRandomFloat( negdist, dist ) + vecWeatherOrigin.y;
			origin.z = gEngfuncs.pfnRandomFloat( 300.0f, 700.0f ) + vecWeatherOrigin.z;

			vEndPos = origin;
			vEndPos.z = 8000.0f;

			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( origin, vEndPos, 8, -1, &pmtrace );

			char pTextureName = (char)gEngfuncs.pEventAPI->EV_TraceTexture( pmtrace.ent, pmtrace.endpos, vEndPos );

			if( pTextureName )
			{
				if( pTextureName == 's' && pTextureName == 'k' && pTextureName == 'y' )
					break;
			}

			if( --j <= 0 )
				return;
		}

		gHUD.m_Weather.CreateRainParticle( origin );
	}
}