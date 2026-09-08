//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#ifndef TRI_H
#define TRI_H

#include "com_model.h"
#include "util_vector.h"

#ifdef USE_PMAN
#include "Particleman.h"

extern IParticleMan *g_pParticleman;

class CBaseDoDParticle : public CBaseParticle
{
public:
	virtual void Think( float time ) { g_pParticleMan->CoreThink( this, time ); }
	virtual void Draw( void ) { g_pParticleMan->CoreDraw( this ); }
	virtual void Animate( float time ) { g_pParticleMan->CoreAnimate( this, time ); }
	virtual void AnimateAndDie( float time ) { g_pParticleMan->CoreAnimateAndDie( this, time ); }
	virtual void Expand( float time ) { g_pParticleMan->CoreExpand( this, time ); }
	virtual void Contract( float time ) { g_pParticleMan->CoreContract( this, time ); }
	virtual void Fade( float time ) { g_pParticleMan->CoreFade( this, time ); }
	virtual void Spin( float time ) { g_pParticleMan->CoreSpin( this, time ); }
	virtual void CalculateVelocity( float time ) { g_pParticleMan->CoreCalculateVelocity( this, time ); }
	virtual void CheckCollision( float time ) { g_pParticleMan->CoreCheckCollision( this, time ); }
	virtual void Touch( vec3_t *pos, vec3_t *normal, int index ) { g_pParticleMan->CoreTouch( this, *pos, *normal, index ); }
	virtual void Die( void ) { g_pParticleMan->CoreDie( this ); }
	virtual void Force( void ) { g_pParticleMan->CoreForce( this ); }

	virtual void InitializeSprite( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness ) 
	{ 
		g_pParticleMan->CoreInitializeSprite( this, *pos, *normal, sprite, size, brightness ); 
	}
};

class CDoDParticle : public CBaseDoDParticle
{
public:
	void SetGlobalWind( float *vecWind );
	void AddGlobalWind( void );
	virtual void Think( float time );
	virtual void Force( void );
	virtual void Die( void );
	virtual void Touch( vec3_t *pos, vec3_t *normal, int index );
	CDoDParticle *Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness, 
		const char *classname, bool bDistCull );

	bool m_bInsideSmoke;
	bool m_bSpawnInside;
	bool m_bDampMod;

	vec3_t m_vRingOrigin;
	bool m_bAffectedByForce;

	int m_iPFlags;

	static vec3_t m_vGlobalWind;
};

class TriangleWallPuff : public CBaseParticle
{
public:
	virtual void Think( float time );

	float m_flActivateTime;
};

class CDoDRocketTrail : public CBaseDoDParticle
{
public:
	virtual void Think( float time );
	CDoDRocketTrail *Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness,
		const char *classname, bool bDistCull );

	bool m_bRocketTrail;
};

class CDoDDirtExploDust : public CBaseDoDParticle
{
public:
	virtual void Think( float time );
	CDoDDirtExploDust *Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness,
		const char *classname, bool bDistCull );

	bool m_bFire;
	float m_flActivateTime;
};

class CDoDSnowFlake : public CDoDParticle
{
public:
	virtual void Think( float time );
	virtual void Touch( vec3_t *pos, vec3_t *normal, int index );
	CDoDSnowFlake *Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness,
		const char *classname, bool bDistCull )
	{
		return NULL;
	}

	bool m_bSpiral, m_bTouched;
	float m_flOldTime, m_flFadeOutTime;
};

class CDoDRainDrop : public CDoDParticle
{
public:
	virtual void Think( float time )
	{
		m_flBrightness = 130.0f;
		CDoDParticle::Think( time );
	}
};
#endif // USE_PMAN

#endif // TRI_H
