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

#ifndef PMAN_MAIN_H
#define PMAN_MAIN_H

//---------------------------------------------------------------------------
// The particle manager interface.
class IParticleMan_Active : public IParticleMan
{
public:
	virtual void SetUp( cl_enginefunc_t* pEnginefuncs );
	virtual void Update( void );
	virtual void SetVariables( float flGravity, Vector vViewAngles );
	virtual void ResetParticles( void );
	virtual void ApplyForce( Vector vOrigin, Vector vDirection, float flRadius, float flStrength, float flDuration );
	virtual void AddCustomParticleClassSize( unsigned long lSize );

	//Use this if you want to create a new particle without any overloaded functions, Think, Touch, etc.
	//Just call this function, set the particle's behavior and let it rip.
	virtual CBaseParticle* CreateParticle( Vector org, Vector normal, struct model_s* sprite, float size, float brightness, const char* classname  );
	
	//Use this to take a block from the mempool for custom particles ( used in new ).
	virtual char* RequestNewMemBlock( int iSize ); 

	//These ones are used along a custom Create for new particles you want to override their behavior.
	//You can call these whenever you want, but they are mainly used by CBaseParticle.
	virtual void CoreInitializeSprite( CCoreTriangleEffect* pParticle, Vector org, Vector normal, struct model_s* sprite, float size, float brightness ); //Only use this for TrianglePlanes
	virtual void CoreThink( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreDraw( CCoreTriangleEffect* pParticle );
	virtual void CoreAnimate( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreAnimateAndDie( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreExpand( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreContract( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreFade( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreSpin( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreCalculateVelocity( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreCheckCollision( CCoreTriangleEffect* pParticle, float time );
	virtual void CoreTouch( CCoreTriangleEffect* pParticle, Vector pos, Vector normal, int index );
	virtual void CoreDie( CCoreTriangleEffect* pParticle );
	virtual void CoreForce( CCoreTriangleEffect* pParticle );
	virtual bool CoreCheckVisibility( CCoreTriangleEffect* pParticle );
	virtual void SetRender( int iRender );
};

// Entry points used by the client instead of the interface factory (the code is built into client.dll)
IParticleMan* CreateActiveMan( void );
void ParticleMan_Shutdown( void );

#endif //PMAN_MAIN_H
