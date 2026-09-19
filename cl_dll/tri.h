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

#include "particleman.h"
#include "CBaseParticle.h"

#define TRI_COLLIDEKILL_ANIM	(1 << 13)
#define TRI_COLLIDEDAMP			(1 << 14)
#define TRI_WIND				(1 << 15)
#define TRI_COLLIDESLIDE		(1 << 16)
#define TRI_COLLIDEBREAK		(1 << 17)
#define FTENT_HITSOUNDPHYSICS	(1 << 18)
#define FTENT_PERSISTPHYSICS	(1 << 19)
#define FTENT_INFINITE			(1 << 20)
#define FTENT_CUSTOMGRAVITY		(1 << 21)
#define FTENT_ARC_BALLISTICS	(FTENT_INFINITE | FTENT_CUSTOMGRAVITY)

extern IParticleMan *g_pParticleman;

void CreateExplosionSmokeInside( vec3_t origin );
void CreateDebrisWallPuff( vec3_t origin, vec3_t vVelocity, vec3_t vColor, int iPuff );

class CBaseDoDParticle : public CBaseParticle
{
public:
	virtual void Think( float time ) { g_pBaseParticle->Think( time ); }
	virtual void Draw( void ) { g_pBaseParticle->Draw(); }
	virtual void Animate( float time ) { g_pBaseParticle->Animate( time ); }
	virtual void AnimateAndDie( float time ) { g_pBaseParticle->AnimateAndDie( time ); }
	virtual void Expand( float time ) { g_pBaseParticle->Expand( time ); }
	virtual void Contract( float time ) { g_pBaseParticle->Contract( time ); }
	virtual void Fade( float time ) { g_pBaseParticle->Fade( time ); }
	virtual void Spin( float time ) { g_pBaseParticle->Spin( time ); }
	virtual void CalculateVelocity( float time ) { g_pBaseParticle->CalculateVelocity( time ); }
	virtual void CheckCollision( float time ) { g_pBaseParticle->CheckCollision( time ); }
	virtual void Touch( vec3_t *pos, vec3_t *normal, int index ) { g_pBaseParticle->Touch( *pos, *normal, index, false ); }
	virtual void Die( void ) { g_pBaseParticle->Die(); }
	virtual void Force( void ) { g_pBaseParticle->Force(); }

	virtual void InitializeSprite( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness ) 
	{ 
		g_pBaseParticle->InitializeSprite( *pos, *normal, sprite, size, brightness );
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
		const char *classname);

	bool m_bRocketTrail;
};

class CDoDDirtExploDust : public CBaseDoDParticle
{
public:
	virtual void Think( float time );
	CDoDDirtExploDust *Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness,
		const char *classname );

	bool m_bFire;
	float m_flActivateTime;
};

class CDoDSnowFlake : public CDoDParticle
{
public:
	virtual void Think( float time );
	virtual void Touch( vec3_t *pos, vec3_t *normal, int index );
	CDoDSnowFlake *Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness,
		const char *classname );

	bool m_bSpiral, m_bTouched;
	float m_flOldTime, m_flFadeOutTime;
};

class CDoDRainDrop : public CDoDParticle
{
public:
	virtual void Think( float time );
};

#endif // TRI_H
