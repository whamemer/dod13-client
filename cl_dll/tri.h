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
// tri.h
//
// the particles of Day of Defeat, they are built on the particleman
//
#pragma once
#if !defined(TRI_H)
#define TRI_H

#include "particleman.h"

extern IParticleMan *g_pParticleMan;

// the particle classes of the game forward all the work to the particleman
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
	virtual void Touch( Vector pos, Vector normal, int index ) { g_pParticleMan->CoreTouch( this, pos, normal, index ); }
	virtual void Die( void ) { g_pParticleMan->CoreDie( this ); }
	virtual void Force( void ) { g_pParticleMan->CoreForce( this ); }

	virtual void InitializeSprite( Vector org, Vector normal, model_s *sprite, float size, float brightness )
	{
		g_pParticleMan->CoreInitializeSprite( this, org, normal, sprite, size, brightness );
	}

	void *operator new( size_t size ) //this asks for a new block of memory from the MiniMem class
	{
		return g_pParticleMan->RequestNewMemBlock( size );
	}
};

// for m_iPFlags of CDoDParticle
#define PFLAG_DOD_SMOKE_CORE		( 1 << 0 )
#define PFLAG_DOD_SPARK_INSTANT		( 1 << 6 )	// does not turn to the view
#define PFLAG_DOD_DIRT_DEBRIS		( 1 << 7 )	// only the yaw follows the view
#define PFLAG_DOD_WIND_AFFECTED		( 1 << 8 )
#define PFLAG_DOD_EXPLOSION_PULSE	( 1 << 9 )	// grows for the first seconds, like the smoke of an explosion
#define PFLAG_DOD_WATER_RIPPLE		( 1 << 10 )	// makes a ripple when it dies in the water
#define PFLAG_DOD_WEATHER_RAIN		( 1 << 11 )
#define PFLAG_DOD_SHRINK_DIE		( 1 << 12 )	// shrinks in the last seconds
#define PFLAG_DOD_VELOCITY_ROTATE	( 1 << 13 )	// the roll follows the velocity
#define PFLAG_DOD_COLLIDE_SPLASH	( 1 << 14 )	// makes a splash when it touches something

class CDoDParticle : public CBaseDoDParticle
{
public:
	bool m_bInsideSmoke;
	bool m_bSpawnInside;
	bool m_bDampMod;
	vec3_t m_vRingOrigin;
	bool m_bAffectedByForce; // hides the one of CCoreTriangleEffect, this is the one that Force() checks
	int m_iPFlags;

	static vec3_t m_vGlobalWind;

	static void SetGlobalWind( float *vecWind );
	void AddGlobalWind( void );

	virtual void Think( float flTime );
	virtual void Force( void );
	virtual void Die( void );
	virtual void Touch( Vector pos, Vector normal, int index );

	static CDoDParticle *Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname, bool bDistCull );
};

class TriangleWallPuff : public CBaseParticle
{
public:
	float m_flActivateTime;

	virtual void Think( float flTime );
};

class CDoDRocketTrail : public CBaseDoDParticle
{
public:
	bool m_bRocketTrail;

	static CDoDRocketTrail *Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname );

	virtual void Think( float flTime );
};

class CDoDDirtExploDust : public CBaseDoDParticle
{
public:
	bool m_bFire;
	float m_flActivateTime;

	static CDoDDirtExploDust *Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname );

	virtual void Think( float flTime );
};

class CDoDSnowFlake : public CDoDParticle
{
public:
	bool m_bSpiral;
	bool m_bTouched;
	float m_flOldTime;
	float m_flFadeOutTime;

	static CDoDSnowFlake *Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname );

	virtual void Touch( Vector pos, Vector normal, int index );
	virtual void Think( float flTime );
};

class CDoDRainDrop : public CDoDParticle
{
public:
	virtual void Think( float flTime );
};

void CreateExplosionSmoke( Vector origin, Vector vVelocity, bool bInsideSmoke, bool bSpawnInside, bool bBlowable );
void CreateExplosionSmokeInside( Vector origin );
void CreateDebrisWallPuff( Vector origin, Vector vVelocity, Vector vColor, int iPuff );

void UpdateSnow( void );
void UpdateRain( void );

void RenderDoDFog( void );
void SuspendDoDFog( void );

Vector GetViewAngles( void );

#endif // TRI_H
